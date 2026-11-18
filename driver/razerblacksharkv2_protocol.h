/* SPDX-License-Identifier: GPL-2.0-or-later */
#ifndef RAZER_BLACKSHARK_PROTOCOL_H
#define RAZER_BLACKSHARK_PROTOCOL_H

/* MediaTek report 2, usage page 0xFF14. Protocol documented by
 * https://github.com/justik13/razer-blackshark-v2-hyperspeed-webhid (MIT).
 * Offsets include the report ID; replies normally omit it from the checksum.
 */
#define BLACKSHARK_REPORT_SIZE 64
#define BLACKSHARK_MAX_PAYLOAD 48
#define BLACKSHARK_STATUS_OK 0x02
#define BLACKSHARK_STATUS_UNSUPPORTED 0x05

static inline void blackshark_prepare(unsigned char *report, unsigned char seq,
                                      unsigned char domain, unsigned char command,
                                      const unsigned char *payload, unsigned int count)
{
    unsigned int i;

    memset(report, 0, BLACKSHARK_REPORT_SIZE);
    report[0] = 0x02;
    report[2] = 0x60 | (seq & 0x1F);
    report[6] = 4 + count;
    report[9] = domain;
    report[10] = command;
    report[12] = count;
    if (count)
        memcpy(report + 13, payload, count);
    for (i = 0; i < 62; i++)
        report[62] ^= report[i];
}

static inline bool blackshark_reply_valid(const unsigned char *report, unsigned int size)
{
    unsigned char checksum = 0;
    unsigned int i;

    if (size != BLACKSHARK_REPORT_SIZE || report[0] != 0x02 || report[8] != 0x80 ||
        report[12] > BLACKSHARK_MAX_PAYLOAD || report[6] != 4 + report[12])
        return false;
    for (i = 1; i < 62; i++)
        checksum ^= report[i];
    return report[62] == checksum || report[62] == (checksum ^ 0x02);
}

static inline bool blackshark_reply_matches(const unsigned char *report, unsigned int size,
        unsigned char sequence, unsigned char domain, unsigned char command, bool wired)
{
    return blackshark_reply_valid(report, size) && (report[2] & 0x1F) == (sequence & 0x1F) &&
           /* Wired requests are local, but v1.7 still identifies the replying headset as 0x80. */
           (report[9] == domain || (wired && domain == 0 && report[9] == 0x80)) && report[10] == command;
}

static inline bool blackshark_set_succeeded(const unsigned char *report)
{
    /* Byte 1 is the device status; byte 11 only marks an acknowledgement. */
    return report[1] == BLACKSHARK_STATUS_OK && report[11] == 1 && report[12] == 1 && report[13] == 0;
}

#endif
