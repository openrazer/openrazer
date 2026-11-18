/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Userspace test of the exact framing/validation helpers compiled into the driver. */
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include "../../../driver/razerblacksharkv2_protocol.h"

static void checksum_reply(unsigned char *reply)
{
    unsigned int i;
    reply[62] = 0;
    for (i = 1; i < 62; i++)
        reply[62] ^= reply[i];
}

int main(void)
{
    unsigned char request[64], reply[64], corrupt[64];
    const unsigned char gains[] = {252, 2, 5, 6, 7, 8, 9, 10, 11, 5};
    unsigned int i;

    blackshark_prepare(request, 0, 0x80, 0x21, NULL, 0);
    assert(request[0] == 2 && request[2] == 0x60 && request[6] == 4);
    assert(request[9] == 0x80 && request[10] == 0x21 && request[62] == 0xC7);
    blackshark_prepare(request, 0x25, 0x80, 0x95, gains, 10);
    assert(request[2] == 0x65 && request[6] == 14 && request[12] == 10);
    assert(!memcmp(request + 13, gains, 10));

    blackshark_prepare(reply, 1, 0x80, 0x21, (unsigned char[]){95}, 1);
    reply[1] = BLACKSHARK_STATUS_OK;
    reply[8] = 0x80;
    reply[11] = 1;
    checksum_reply(reply);
    assert(blackshark_reply_matches(reply, 64, 0x61, 0x80, 0x21, false));
    assert(!blackshark_reply_matches(reply, 64, 0x62, 0x80, 0x21, false));
    assert(!blackshark_reply_matches(reply, 64, 0x61, 0, 0x21, false));
    assert(blackshark_reply_matches(reply, 64, 0x61, 0, 0x21, true));
    assert(!blackshark_reply_matches(reply, 64, 0x61, 0x80, 0x2A, false));
    assert(!blackshark_set_succeeded(reply));
    reply[13] = 0;
    assert(blackshark_set_succeeded(reply));
    reply[1] = BLACKSHARK_STATUS_UNSUPPORTED;
    assert(!blackshark_set_succeeded(reply));
    reply[1] = BLACKSHARK_STATUS_OK;
    reply[12] = 0;
    assert(!blackshark_set_succeeded(reply));
    reply[12] = 1;
    reply[13] = 95;
    for (i = 0; i < 64; i++)
        assert(!blackshark_reply_valid(reply, i));
    for (i = 0; i < 63; i++) {
        memcpy(corrupt, reply, 64);
        corrupt[i] ^= 0x10;
        assert(!blackshark_reply_valid(corrupt, 64));
    }
    reply[62] ^= 2;
    assert(blackshark_reply_valid(reply, 64));
    reply[12] = 49;
    reply[6] = 53;
    checksum_reply(reply);
    assert(!blackshark_reply_valid(reply, 64));
    return 0;
}
