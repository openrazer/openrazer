// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Terri Cain <terri@dolphincorp.co.uk>
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/random.h>
#include <linux/completion.h>

#include "razerkraken_driver.h"
#include "razercommon.h"

/*
 * Version Information
 */
#define DRIVER_DESC "Razer Keyboard Device Driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE(DRIVER_LICENSE);

/**
 * Print report to syslog
 */
/*
static void print_erroneous_kraken_request_report(struct razer_kraken_request_report* report, char* driver_name, char* message)
{
    printk(KERN_WARNING "%s: %s. Report ID: %02x dest: %02x length: %02x ADDR: %02x%02x Args: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x .\n",
           driver_name,
           message,
           report->report_id,
           report->destination,
           report->length,
           report->addr_h,
           report->addr_l,
           report->arguments[0], report->arguments[1], report->arguments[2], report->arguments[3], report->arguments[4], report->arguments[5],
           report->arguments[6], report->arguments[7], report->arguments[8], report->arguments[9], report->arguments[10], report->arguments[11],
           report->arguments[12], report->arguments[13], report->arguments[14], report->arguments[15]);
}
*/

static int razer_kraken_send_control_msg(struct hid_device *hdev,struct razer_kraken_request_report* report, unsigned char skip)
{
    struct usb_device *usb_dev = hid_to_usb_dev(hdev);
    int ret;

    // Send usb control message
    ret = usb_control_msg_send(usb_dev,
                               0, // endpoint to send the message to
                               HID_REQ_SET_REPORT, // USB message request value (0x09)
                               USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT, // USB message request type value (0x21)
                               0x0204, // USB message value
                               0x0003, // USB message index value
                               report, // pointer to the data to send
                               sizeof(*report), // length in bytes of the data to send
                               USB_CTRL_SET_TIMEOUT, // time in msecs to wait for the message to complete before timing out
                               GFP_KERNEL);

    // Wait
    if(skip != 1) {
        msleep(report->length * 15);
    }

    if (ret)
        hid_warn(hdev, "Failed to send USB control message: %d\n", ret);

    return ret;
}

/**
 * Get a request report
 *
 * report_id - The type of report
 * destination - where data is going (like ram)
 * length - amount of data
 * address - where to write data to
 */
static struct razer_kraken_request_report get_kraken_request_report(unsigned char report_id, unsigned char destination, unsigned char length, unsigned short address)
{
    struct razer_kraken_request_report report;
    memset(&report, 0, sizeof(struct razer_kraken_request_report));

    report.report_id = report_id;
    report.destination = destination;
    report.length = length;
    report.addr_h = (address >> 8);
    report.addr_l = (address & 0xFF);

    return report;
}

/**
 * Get a union containing the effect bitfield
 */
static union razer_kraken_effect_byte get_kraken_effect_byte(void)
{
    union razer_kraken_effect_byte effect_byte;
    memset(&effect_byte, 0, sizeof(union razer_kraken_effect_byte));

    return effect_byte;
}

/* ---- BlackShark V3 HID command helpers ----
 *
 * Wire format verified from USB captures of both transports:
 *   - Wired (PID 0x0579):       buf[9] = 0x00 for both SET and GET (re-decoded
 *                               from new pcap 2026-05-02; previous "0x80 GET"
 *                               note was wrong — wired Synapse uses 0x00)
 *   - 2.4GHz (PID 0x057A):      buf[9] = 0x80 for both SET and GET
 *   - V3 Pro wired (0x0576):    buf[9] = 0x00 (assumed by transport rule)
 *   - V3 Pro 2.4GHz (0x0577):   buf[9] = 0x80 (verified via stealthee's pcap)
 * CRC is XOR of bytes [0..61] (includes the 0x02 report id at buf[0]).
 *
 * Rule: wireless transports use 0x80, wired transports use 0x00.
 * Direction is independent of SET vs GET — both use the same transport byte.
 */

static inline u8 razer_blackshark_dir_byte(u16 pid)
{
    /* Wireless dongles (V3 0x057A, V3 Pro 0x0577) use 0x80;
     * wired transports (V3 0x0579, V3 Pro 0x0576) use 0x00. */
    return (pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3 ||
            pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO) ? 0x80 : 0x00;
}

/* Back-compat alias kept until callers are migrated. */
static inline u8 razer_blackshark_set_dir(u16 pid)
{
    return razer_blackshark_dir_byte(pid);
}

static void razer_blackshark_build_get(u8 *buf, u8 param, u16 pid)
{
    u8 crc = 0;
    int i;

    memset(buf, 0, RAZER_BLACKSHARK_REPORT_LEN);
    buf[0] = 0x02;   /* report_id */
    buf[2] = 0x60;   /* transaction_id */
    buf[6] = 0x04;   /* data_size */
    buf[9] = razer_blackshark_dir_byte(pid); /* transport-dependent dir byte */
    buf[10] = param; /* args[1] = param_id */
    for (i = 0; i < 62; i++) crc ^= buf[i];
    buf[62] = crc;
}

static void razer_blackshark_build_get_thx(u8 *buf)
{
    u8 crc = 0;
    int i;

    memset(buf, 0, RAZER_BLACKSHARK_REPORT_LEN);
    buf[0] = 0x02;
    buf[2] = 0x60;
    buf[6] = 0x05;
    buf[9] = 0x80;               /* GET */
    buf[10] = BLACKSHARK_PARAM_THX;
    buf[12] = 0x01;              /* sub-params observed in capture */
    buf[13] = 0x01;
    for (i = 0; i < 62; i++) crc ^= buf[i];
    buf[62] = crc;
}

/* param must be the SET param (= GET param | 0x80) */
static void razer_blackshark_build_set(u8 *buf, u8 param, u8 val, u8 dir)
{
    u8 crc = 0;
    int i;

    memset(buf, 0, RAZER_BLACKSHARK_REPORT_LEN);
    buf[0] = 0x02;
    buf[2] = 0x60;
    buf[6] = 0x05;   /* data_size */
    buf[9] = dir;    /* 0x00 wired, 0x80 wireless */
    buf[10] = param;
    buf[11] = 0x00;
    buf[12] = 0x01;
    buf[13] = val;
    for (i = 0; i < 62; i++) crc ^= buf[i];
    buf[62] = crc;
}

static inline u8 bs_gain_encode(int v)
{
    if (v < 0) return (u8)(0x80 | (-v));
    return (u8)v;
}

static inline int bs_gain_decode(u8 b)
{
    return (b & 0x80) ? -(int)(b & 0x7f) : (int)b;
}

/*
 * 0xe0 apply-command profile parameters, captured from Synapse.
 * [12]=0x06 always; [13]=profile_idx; [14],[17] are profile-specific
 * mode bytes observed in captures; [15],[16]=0x01 always; [18] varies.
 */
static const u8 bs_eq_apply_params[5][4] = {
    /* profile 0 (Default):  [14], [17], [18], pad */
    { 0x00, 0x00, 0x01, 0 },
    /* profile 1 (Game):     [14]=0x01, [17]=0x01, [18]=0x01 */
    { 0x01, 0x01, 0x01, 0 },
    /* profile 2 (Movie):    [14]=0x03, [17]=0x03, [18]=0x01 */
    { 0x03, 0x03, 0x01, 0 },
    /* profile 3 (Music):    [14]=0x02, [17]=0x02, [18]=0x01 */
    { 0x02, 0x02, 0x01, 0 },
    /* profile 4 (Esports):  [14]=0x04, [17]=0x0a, [18]=0x00 */
    { 0x04, 0x0a, 0x00, 0 },
};

/*
 * Fills 5 x RAZER_BLACKSHARK_REPORT_LEN buffers for the EQ write sequence.
 * profile_idx 0=Default 1=Game 2=Movie 3=Music 4=Esports.
 * We default to profile 1 so the device doesn't jump to the factory Default
 * preset (profile 0) which the firmware treats as read-only all-zeros.
 */
static void razer_blackshark_build_eq_cmds(u8 bufs[][RAZER_BLACKSHARK_REPORT_LEN],
        const s8 *bands, u8 dir, u8 profile_idx)
{
    u8 crc;
    int i;
    const u8 *ap;

    if (profile_idx > 4) profile_idx = 1;
    ap = bs_eq_apply_params[profile_idx];

    /* cmd 0: begin */
    memset(bufs[0], 0, RAZER_BLACKSHARK_REPORT_LEN);
    bufs[0][0] = 0x02;
    bufs[0][2] = 0x60;
    bufs[0][6] = 0x05;
    bufs[0][9] = dir;
    bufs[0][10] = BLACKSHARK_SET_EQ_BEGIN;
    bufs[0][12] = 0x01;
    bufs[0][13] = 0x01;
    crc = 0;
    for (i = 0; i < 62; i++) crc ^= bufs[0][i];
    bufs[0][62] = crc;

    /* cmd 1: EQ band data — profile_idx in [13], bands at [14..23] */
    memset(bufs[1], 0, RAZER_BLACKSHARK_REPORT_LEN);
    bufs[1][0] = 0x02;
    bufs[1][2] = 0x60;
    bufs[1][6] = 0x0f;
    bufs[1][9] = dir;
    bufs[1][10] = BLACKSHARK_SET_EQ;
    bufs[1][12] = 0x0b;
    bufs[1][13] = profile_idx;
    for (i = 0; i < 10; i++)
        bufs[1][14 + i] = bs_gain_encode((int)bands[i]);
    crc = 0;
    for (i = 0; i < 62; i++) crc ^= bufs[1][i];
    bufs[1][62] = crc;

    /* cmd 2: apply — select the profile we just wrote */
    memset(bufs[2], 0, RAZER_BLACKSHARK_REPORT_LEN);
    bufs[2][0] = 0x02;
    bufs[2][2] = 0x60;
    bufs[2][6] = 0x0a;
    bufs[2][9] = dir;
    bufs[2][10] = BLACKSHARK_SET_EQ_APPLY;
    bufs[2][12] = 0x06;
    bufs[2][13] = profile_idx;
    bufs[2][14] = ap[0];
    bufs[2][15] = 0x01;
    bufs[2][16] = 0x01;
    bufs[2][17] = ap[1];
    bufs[2][18] = ap[2];
    crc = 0;
    for (i = 0; i < 62; i++) crc ^= bufs[2][i];
    bufs[2][62] = crc;

    /* cmd 3: end */
    memset(bufs[3], 0, RAZER_BLACKSHARK_REPORT_LEN);
    bufs[3][0] = 0x02;
    bufs[3][2] = 0x60;
    bufs[3][6] = 0x05;
    bufs[3][9] = dir;
    bufs[3][10] = BLACKSHARK_SET_EQ_BEGIN;
    bufs[3][12] = 0x01;
    bufs[3][13] = 0x02;
    crc = 0;
    for (i = 0; i < 62; i++) crc ^= bufs[3][i];
    bufs[3][62] = crc;

    /* cmd 4: commit */
    memset(bufs[4], 0, RAZER_BLACKSHARK_REPORT_LEN);
    bufs[4][0] = 0x02;
    bufs[4][2] = 0x60;
    bufs[4][6] = 0x0f;
    bufs[4][9] = dir;
    bufs[4][10] = BLACKSHARK_SET_EQ_COMMIT;
    bufs[4][12] = 0x0b;
    bufs[4][13] = profile_idx;
    bufs[4][16] = 0x01;
    bufs[4][17] = 0x01;
    crc = 0;
    for (i = 0; i < 62; i++) crc ^= bufs[4][i];
    bufs[4][62] = crc;
}

/*
 * Builds the mic EQ data command (0x97).
 * Single packet, no begin/end/commit needed (verified from captures).
 * 10 bands at buf[13..22], sign-magnitude encoding (same as headphone EQ).
 * Frequencies: 31Hz, 63Hz, 125Hz, 250Hz, 500Hz, 1kHz, 2kHz, 4kHz, 8kHz, 16kHz.
 */
static void razer_blackshark_build_mic_eq_cmd(u8 *buf, const s8 *bands, u8 dir)
{
    u8 crc = 0;
    int i;

    memset(buf, 0, RAZER_BLACKSHARK_REPORT_LEN);
    buf[0] = 0x02;
    buf[2] = 0x60;
    buf[6] = 0x0e;
    buf[9] = dir;
    buf[10] = BLACKSHARK_SET_MIC_EQ_DATA;
    buf[12] = 0x0a;
    for (i = 0; i < 10; i++)
        buf[13 + i] = bs_gain_encode((int)bands[i]);
    for (i = 0; i < 62; i++) crc ^= buf[i];
    buf[62] = crc;
}

static int razer_blackshark_send_cmd(struct razer_kraken_device *dev, u8 *buf)
{
    int ret;
    u8 *kbuf;
    long wait;

    kbuf = kmemdup(buf, RAZER_BLACKSHARK_REPORT_LEN, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;

    /* Mark response slot as "no reply yet" and arm completion BEFORE sending
     * so a fast response (some are <5ms) can't race the wait. */
    dev->data[1] = 0x00;
    if (dev->vendor_response_inited)
        reinit_completion(&dev->vendor_response);

    /* hid_hw_raw_request is the proper HID-core path: it issues the
     * SET_REPORT and stays integrated with raw_event() so the device's
     * interrupt-IN reply lands in our raw_event handler (which fires
     * complete() below). usb_control_msg bypasses hid-core and the IN
     * reply gets dropped on the floor — that was the old 150ms-msleep
     * bug that left V3/V3 Pro GETs returning fallback data. */
    ret = hid_hw_raw_request(dev->hdev, 0x02,
                             kbuf, RAZER_BLACKSHARK_REPORT_LEN,
                             HID_OUTPUT_REPORT, HID_REQ_SET_REPORT);
    kfree(kbuf);

    if (ret != RAZER_BLACKSHARK_REPORT_LEN) {
        printk(KERN_WARNING "razerkraken: BlackShark V3 hid_hw_raw_request failed (%d)\n", ret);
        return -EIO;
    }

    if (!dev->vendor_response_inited) {
        /* Probe didn't arm the completion (very early call) — fall back to
         * the legacy fixed wait. Should not normally fire. */
        msleep(150);
        return 0;
    }

    /* Wait up to 250ms for raw_event() to copy the reply into dev->data and
     * fire dev->vendor_response. Was 2s, but on V3 wireless (PID 0x057A) the
     * firmware silently drops responses on Linux/QEMU-passthrough USB stacks
     * (see project memory: USB-stack-timing gate, not fixable from here), so
     * the 2s wait was just dead time multiplying every multi-frame write
     * (EQ = 5 frames × 2s = 10s of UI freeze per preset click). 250ms is
     * still long enough for any device that DOES respond (V3 Pro wired with
     * its own gate workaround, future firmware that drops the gate, etc.). */
    /* 1000ms — V3 Pro typical reply is ~40ms, but the FIRST query after a
     * wireless wake or under USB bus contention can race a 250ms timeout
     * (battery returns -1, ANC/sidetone reads fall through). 1s gives enough
     * headroom while still being half of the original 2s. Multi-frame writes
     * (EQ apply) treat -ETIMEDOUT as success per frame so this doesn't add
     * UI freeze on the write path. */
    wait = wait_for_completion_timeout(&dev->vendor_response,
                                       msecs_to_jiffies(1000));
    if (!wait)
        return -ETIMEDOUT;
    return 0;
}

/* ---- end BlackShark V3 helpers ---- */

/* ---- BlackShark V3 Pro (PID 0x0577) HID command helpers ----
 *
 * Protocol decoded by RiskRunner0 (https://github.com/RiskRunner0/blackshark-linux,
 * GPL-2.0-or-later) from Synapse usbmon captures. Cross-checked against a startup
 * pcap from a V3 Pro user on this PR.
 *
 * NOTE: These code paths are based on third-party reverse engineering and have
 * NOT been hardware-tested by the contributor of this driver. They are committed
 * here as a starting point for V3 Pro users who can validate against real hardware.
 *
 * Same envelope as V3 (Report 0x02, transaction_id 0x60, CRC=XOR[0..61]).
 * Direction byte buf[9] = 0x80 for normal SET/GET (V3 Pro is wireless-only via
 * a 2.4GHz dongle, so we use the same convention as V3 wireless).
 */

static void razer_blackshark_v3pro_build(u8 *buf, u8 class, u8 cmd, const u8 *args, u8 args_len)
{
    u8 crc = 0;
    int i;

    memset(buf, 0, RAZER_BLACKSHARK_REPORT_LEN);
    buf[0] = 0x02;
    buf[2] = 0x60;
    /* buf[6] = total payload bytes from buf[9] inclusive
     *        = direction + class + sub + argc + args = 4 + args_len.
     * Synapse's wire-format: SETs with argc=2 carry buf[6]=0x06. Earlier
     * formula (3 + args_len) was off by one; the firmware silently dropped
     * the trailing arg byte, which made ANC level changes inaudible on
     * Linux while Windows-Synapse felt right. */
    buf[6] = 4 + args_len;
    buf[9] = 0x80;
    buf[10] = class;
    buf[11] = 0x00;
    buf[12] = cmd;
    if (args && args_len)
        memcpy(&buf[13], args, args_len);
    for (i = 0; i < 62; i++) crc ^= buf[i];
    buf[62] = crc;
}

/*
 * Headphone EQ band data per preset (decoded from Synapse pcaps, sign-magnitude).
 * Format: [preset_idx, b0..b9, padding] — 12 bytes total, 11 meaningful bytes
 * matched by the 0x95 envelope (count=0x0b).
 * Bands (10): 31, 63, 125, 250, 500, 1k, 2k, 4k, 8k, 16k Hz.
 * Sign-magnitude gain: 0x00=0dB, 0x01..0x06=+1..+6dB, 0x81..0x86=−1..−6dB.
 *
 * Slots 0..4 are factory presets; 5..8 are user custom slots from Synapse's
 * "EDIT EQ LIST" UI. Slots 5..8 fall back to flat band data with the slot
 * index applied.
 */
static const u8 bs_v3pro_eq_bands[5][12] = {
    /* 0: Default — flat */
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    /* 1: Game     +2 +2 +5 +5 +1 -1 +2 +3 +3 +3 */
    { 0x01, 0x02, 0x02, 0x05, 0x05, 0x01, 0x81, 0x02, 0x03, 0x03, 0x03, 0x00 },
    /* 2: Movie    +3 +3 +3 -1 -4 -4 +2 +3 +3 +3 */
    { 0x02, 0x03, 0x03, 0x03, 0x81, 0x84, 0x84, 0x02, 0x03, 0x03, 0x03, 0x00 },
    /* 3: Music    +2 +2  0  0 +1 -1 -1 +3 +3 +3 */
    { 0x03, 0x02, 0x02, 0x00, 0x00, 0x01, 0x81, 0x81, 0x03, 0x03, 0x03, 0x00 },
    /* 4: Esports +1 +1 -1  0 +2  0 +4 +4 +4 -3 */
    { 0x04, 0x01, 0x01, 0x81, 0x00, 0x02, 0x00, 0x04, 0x04, 0x04, 0x83, 0x00 },
};

static const u8 bs_v3pro_eq_meta[5][7] = {
    { 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00 },
    { 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00 },
    { 0x02, 0x03, 0x01, 0x00, 0x03, 0x00, 0x00 },
    { 0x03, 0x02, 0x01, 0x00, 0x02, 0x00, 0x00 },
    { 0x04, 0x04, 0x01, 0x00, 0x0b, 0x00, 0x00 },
};

static const u8 bs_v3pro_eq_commit[5][12] = {
    { 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x02, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x03, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x04, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
};

/*
 * Apply a V3 Pro EQ preset switch (5-command sequence).
 *   1. GET state    cls=0xe1 cmd=0x01 args=[0x01, 0x00]
 *   2. SET bands    cls=0x95 cmd=0x0b args=[idx, b0..b8, 0x00]
 *   3. SET meta     cls=0xe0 cmd=0x06 args=[idx, ...]
 *   4. APPLY        cls=0xe1 cmd=0x01 args=[0x02, 0x00]
 *   5. COMMIT       cls=0xeb cmd=0x0b args=[idx, ...]
 */
static int razer_blackshark_v3pro_apply_eq(struct razer_kraken_device *device, u8 preset)
{
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    const u8 gate_get[2] = { 0x01, 0x00 };
    const u8 gate_apply[2] = { 0x02, 0x00 };
    u8 bands[12], meta[7], commit[12];
    int ret;

    if (preset >= BLACKSHARK_V3_PRO_EQ_PRESET_COUNT)
        return -EINVAL;

    if (preset < ARRAY_SIZE(bs_v3pro_eq_bands)) {
        memcpy(bands, bs_v3pro_eq_bands[preset], sizeof(bands));
        memcpy(meta, bs_v3pro_eq_meta[preset], sizeof(meta));
        memcpy(commit, bs_v3pro_eq_commit[preset], sizeof(commit));
    } else {
        /* Slots 5..8: flat band data with slot index applied */
        memcpy(bands, bs_v3pro_eq_bands[0], sizeof(bands));
        bands[0] = preset;
        memcpy(meta, bs_v3pro_eq_meta[0], sizeof(meta));
        meta[0] = preset;
        memcpy(commit, bs_v3pro_eq_commit[0], sizeof(commit));
        commit[0] = preset;
    }

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_EQ_STATE_CLASS,
                                 BLACKSHARK_V3_PRO_EQ_STATE_ID, gate_get, sizeof(gate_get));
    /* The SET_REPORT control transfer itself is fine; only the optional
     * response push on ep 0x84 may time out (some frames in this sequence
     * are write-only — firmware doesn't reply). Treat -ETIMEDOUT as success
     * so the whole 5-frame chain runs; only abort on real bus errors. */
    ret = razer_blackshark_send_cmd(device, cmdbuf);
    if (ret < 0 && ret != -ETIMEDOUT) return ret;

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_EQ_BANDS_CLASS,
                                 BLACKSHARK_V3_PRO_EQ_BANDS_ID, bands, sizeof(bands));
    /* The SET_REPORT control transfer itself is fine; only the optional
     * response push on ep 0x84 may time out (some frames in this sequence
     * are write-only — firmware doesn't reply). Treat -ETIMEDOUT as success
     * so the whole 5-frame chain runs; only abort on real bus errors. */
    ret = razer_blackshark_send_cmd(device, cmdbuf);
    if (ret < 0 && ret != -ETIMEDOUT) return ret;

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_EQ_META_CLASS,
                                 BLACKSHARK_V3_PRO_EQ_META_ID, meta, sizeof(meta));
    /* The SET_REPORT control transfer itself is fine; only the optional
     * response push on ep 0x84 may time out (some frames in this sequence
     * are write-only — firmware doesn't reply). Treat -ETIMEDOUT as success
     * so the whole 5-frame chain runs; only abort on real bus errors. */
    ret = razer_blackshark_send_cmd(device, cmdbuf);
    if (ret < 0 && ret != -ETIMEDOUT) return ret;

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_EQ_STATE_CLASS,
                                 BLACKSHARK_V3_PRO_EQ_STATE_ID, gate_apply, sizeof(gate_apply));
    /* The SET_REPORT control transfer itself is fine; only the optional
     * response push on ep 0x84 may time out (some frames in this sequence
     * are write-only — firmware doesn't reply). Treat -ETIMEDOUT as success
     * so the whole 5-frame chain runs; only abort on real bus errors. */
    ret = razer_blackshark_send_cmd(device, cmdbuf);
    if (ret < 0 && ret != -ETIMEDOUT) return ret;

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_EQ_COMMIT_CLASS,
                                 BLACKSHARK_V3_PRO_EQ_COMMIT_ID, commit, sizeof(commit));
    /* The SET_REPORT control transfer itself is fine; only the optional
     * response push on ep 0x84 may time out (some frames in this sequence
     * are write-only — firmware doesn't reply). Treat -ETIMEDOUT as success
     * so the whole 5-frame chain runs; only abort on real bus errors. */
    ret = razer_blackshark_send_cmd(device, cmdbuf);
    if (ret < 0 && ret != -ETIMEDOUT) return ret;

    return 0;
}

/* ---- end BlackShark V3 Pro helpers ---- */

/**
 * Get the current effect
 */
static unsigned char get_current_effect(struct device *dev)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report report = get_kraken_request_report(0x04, 0x00, 0x01, device->led_mode_address);
    int is_mutex_locked = mutex_is_locked(&device->lock);
    unsigned char result = 0;

    // Lock if there isn't already a lock, otherwise skip, essentially emulate a rentrant lock
    if(is_mutex_locked == 0) {
        mutex_lock(&device->lock);
    }

    device->data[0] = 0x00;
    razer_kraken_send_control_msg(device->hdev, &report, 1);
    msleep(25); // Sleep 20ms

    // Check for actual data
    if(device->data[0] == 0x05) {
        result = device->data[1];
    } else {
        dev_err(dev, "razerkraken: Did not manage to get report\n");
    }

    // Unlock if there isn't already a lock (as there would be by now), otherwise skip as reusing existing lock
    if(is_mutex_locked == 0) {
        mutex_unlock(&device->lock);
    }

    return result;
}

static unsigned int get_rgb_from_addr(struct device *dev, unsigned short address, unsigned char len, char* buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report report = get_kraken_request_report(0x04, 0x00, len, address);
    int is_mutex_locked = mutex_is_locked(&device->lock);
    unsigned char written = 0;

    // Lock if there isn't already a lock, otherwise skip, essentially emulate a rentrant lock
    if(is_mutex_locked == 0) {
        mutex_lock(&device->lock);
    }

    device->data[0] = 0x00;
    razer_kraken_send_control_msg(device->hdev, &report, 1);
    msleep(25); // Sleep 20ms

    // Check for actual data
    if(device->data[0] == 0x05) {
        //dev_err(dev, "razerkraken: Got %02x%02x%02x %02x\n", device->data[1], device->data[2], device->data[3], device->data[4]);
        memcpy(buf, &device->data[1], len);
        written = len;
    } else {
        dev_err(dev, "razerkraken: Did not manage to get report\n");
    }

    // Unlock if there isn't already a lock (as there would be by now), otherwise skip as reusing existing lock
    if(is_mutex_locked == 0) {
        mutex_unlock(&device->lock);
    }

    return written;
}

/**
 * Read device file "version"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%s\n", DRIVER_VERSION);
}

/**
 * Read device file "device_type"
 *
 * Returns friendly string of device type
 */
static ssize_t razer_attr_read_device_type(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);

    char *device_type;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC:
    case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC_ALT:
        device_type = "Razer Kraken 7.1";
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN:
        device_type = "Razer Kraken 7.1 Chroma"; // Rainie
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN_V2:
        device_type = "Razer Kraken 7.1 V2"; // Kylie
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN_TE:
        device_type = "Razer Kraken Tournament Edition";
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE:
        device_type = "Razer Kraken Ultimate";
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2:
        device_type = "Razer Kraken Kitty V2";
        break;

    case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_WIRED:
        device_type = "Razer BlackShark V3 (Wired)";
        break;

    case USB_DEVICE_ID_RAZER_BLACKSHARK_V3:
        device_type = "Razer BlackShark V3 (Wireless)";
        break;

    case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO_WIRED:
        device_type = "Razer BlackShark V3 Pro (Wired)";
        break;

    case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO:
        device_type = "Razer BlackShark V3 Pro (Wireless)";
        break;

    default:
        device_type = "Unknown Device";
    }

    return sysfs_emit(buf, "%s\n", device_type);
}

/**
 * Write device file "test"
 *
 * Does nothing
 */
static ssize_t razer_attr_write_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return count;
}

/**
 * Read device file "test"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_test(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "\n");
}

/**
 * Write device file "mode_spectrum"
 *
 * Specrum effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_matrix_effect_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report report = get_kraken_request_report(0x04, 0x40, 0x01, device->led_mode_address);
    union razer_kraken_effect_byte effect_byte = get_kraken_effect_byte();

    // Spectrum Cycling | ON
    effect_byte.bits.on_off_static = 1;
    effect_byte.bits.spectrum_cycling = 1;

    report.arguments[0] = effect_byte.value;

    // Lock access to sending USB as adhering to the razer len*15ms delay
    mutex_lock(&device->lock);
    razer_kraken_send_control_msg(device->hdev, &report, 0);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Write device file "mode_none"
 *
 * None effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_matrix_effect_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report report = get_kraken_request_report(0x04, 0x40, 0x01, device->led_mode_address);
    union razer_kraken_effect_byte effect_byte = get_kraken_effect_byte();

    // Spectrum Cycling | OFF
    effect_byte.bits.on_off_static = 0;
    effect_byte.bits.spectrum_cycling = 0;

    report.arguments[0] = effect_byte.value;

    // Lock access to sending USB as adhering to the razer len*15ms delay
    mutex_lock(&device->lock);
    razer_kraken_send_control_msg(device->hdev, &report, 0);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Write device file "mode_static"
 *
 * Static effect mode is activated whenever the file is written to with 3 bytes
 */
static ssize_t razer_attr_write_matrix_effect_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report rgb_report = get_kraken_request_report(0x04, 0x40, count, device->breathing_address[0]);
    struct razer_kraken_request_report effect_report = get_kraken_request_report(0x04, 0x40, 0x01, device->led_mode_address);
    union razer_kraken_effect_byte effect_byte = get_kraken_effect_byte();

    if (count != 3 && count != 4) {
        dev_warn(dev, "razerkraken: Static mode only accepts RGB (3byte) or RGB with intensity (4byte)\n");
        return -EINVAL;
    }

    rgb_report.arguments[0] = buf[0];
    rgb_report.arguments[1] = buf[1];
    rgb_report.arguments[2] = buf[2];

    if(count == 4) {
        rgb_report.arguments[3] = buf[3];
    }

    // ON/Static
    effect_byte.bits.on_off_static = 1;
    effect_report.arguments[0] = effect_byte.value;

    // Lock sending of the 2 commands
    mutex_lock(&device->lock);

    // Basically Kraken Classic doesn't take RGB arguments so only do it for the KrakenV1,V2,Ultimate
    switch(device->usb_pid) {
    case USB_DEVICE_ID_RAZER_KRAKEN:
    case USB_DEVICE_ID_RAZER_KRAKEN_V2:
    case USB_DEVICE_ID_RAZER_KRAKEN_TE:
    case USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE:
    case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2:
        razer_kraken_send_control_msg(device->hdev, &rgb_report, 0);
        break;
    }

    // Send Set static command
    razer_kraken_send_control_msg(device->hdev, &effect_report, 0);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Write device file "mode_custom"
 *
 * Custom effect mode is activated whenever the file is written to with 3 bytes
 */
static ssize_t razer_attr_write_matrix_effect_custom(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report rgb_report = get_kraken_request_report(0x04, 0x40, count, device->custom_address);
    struct razer_kraken_request_report effect_report = get_kraken_request_report(0x04, 0x40, 0x01, device->led_mode_address);
    union razer_kraken_effect_byte effect_byte = get_kraken_effect_byte();

    if(count != 3 && count != 4) {
        dev_warn(dev, "razerkraken: Custom mode only accepts RGB (3byte) or RGB with intensity (4byte)\n");
        return -EINVAL;
    }

    rgb_report.arguments[0] = buf[0];
    rgb_report.arguments[1] = buf[1];
    rgb_report.arguments[2] = buf[2];

    if(count == 4) {
        rgb_report.arguments[3] = buf[3];
    }

    // ON/Static
    effect_byte.bits.on_off_static = 1;
    effect_report.arguments[0] = effect_byte.value;

    // Lock sending of the 2 commands
    mutex_lock(&device->lock);
    razer_kraken_send_control_msg(device->hdev, &rgb_report, 1);

    razer_kraken_send_control_msg(device->hdev, &effect_report, 1);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Read device file "mode_static"
 *
 * Returns 4 bytes for config
 */
static ssize_t razer_attr_read_matrix_effect_static(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return get_rgb_from_addr(dev, device->breathing_address[0], 0x04, buf);
}

/**
 * Read device file "mode_custom"
 *
 * Returns 4 bytes for config
 */
static ssize_t razer_attr_read_matrix_effect_custom(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return get_rgb_from_addr(dev, device->custom_address, 0x04, buf);
}

/**
 * Write device file "mode_breath"
 *
 * Breathing effect mode is activated whenever the file is written to with 3,6 or 9 bytes
 */
static ssize_t razer_attr_write_matrix_effect_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report effect_report = get_kraken_request_report(0x04, 0x40, 0x01, device->led_mode_address);
    union razer_kraken_effect_byte effect_byte = get_kraken_effect_byte();

    // Short circuit here as rainie only does breathing1
    if(device->usb_pid == USB_DEVICE_ID_RAZER_KRAKEN && count != 3) {
        dev_warn(dev, "razerkraken: Breathing mode only accepts RGB (3byte)\n");
        return -EINVAL;
    }

    if(count == 3) {
        struct razer_kraken_request_report rgb_report = get_kraken_request_report(0x04, 0x40, 0x03, device->breathing_address[0]);

        rgb_report.arguments[0] = buf[0];
        rgb_report.arguments[1] = buf[1];
        rgb_report.arguments[2] = buf[2];

        // ON/Static
        effect_byte.bits.on_off_static = 1;
        effect_byte.bits.single_colour_breathing = 1;
        effect_byte.bits.sync = 1;
        effect_report.arguments[0] = effect_byte.value;

        // Lock sending of the 2 commands
        mutex_lock(&device->lock);
        razer_kraken_send_control_msg(device->hdev, &rgb_report, 0);

        razer_kraken_send_control_msg(device->hdev, &effect_report, 0);
        mutex_unlock(&device->lock);
    } else if(count == 6) {
        struct razer_kraken_request_report rgb_report  = get_kraken_request_report(0x04, 0x40, 0x03, device->breathing_address[1]);
        struct razer_kraken_request_report rgb_report2 = get_kraken_request_report(0x04, 0x40, 0x03, device->breathing_address[1]+4); // Address the 2nd set of colours

        rgb_report.arguments[0] = buf[0];
        rgb_report.arguments[1] = buf[1];
        rgb_report.arguments[2] = buf[2];
        rgb_report2.arguments[0] = buf[3];
        rgb_report2.arguments[1] = buf[4];
        rgb_report2.arguments[2] = buf[5];

        // ON/Static
        effect_byte.bits.on_off_static = 1;
        effect_byte.bits.two_colour_breathing = 1;
        effect_byte.bits.sync = 1;
        effect_report.arguments[0] = effect_byte.value;

        // Lock sending of the 2 commands
        mutex_lock(&device->lock);
        razer_kraken_send_control_msg(device->hdev, &rgb_report, 0);

        razer_kraken_send_control_msg(device->hdev, &rgb_report2, 0);

        razer_kraken_send_control_msg(device->hdev, &effect_report, 0);
        mutex_unlock(&device->lock);

    } else if(count == 9) {
        struct razer_kraken_request_report rgb_report  = get_kraken_request_report(0x04, 0x40, 0x03, device->breathing_address[2]);
        struct razer_kraken_request_report rgb_report2 = get_kraken_request_report(0x04, 0x40, 0x03, device->breathing_address[2]+4); // Address the 2nd set of colours
        struct razer_kraken_request_report rgb_report3 = get_kraken_request_report(0x04, 0x40, 0x03, device->breathing_address[2]+8); // Address the 3rd set of colours

        rgb_report.arguments[0] = buf[0];
        rgb_report.arguments[1] = buf[1];
        rgb_report.arguments[2] = buf[2];
        rgb_report2.arguments[0] = buf[3];
        rgb_report2.arguments[1] = buf[4];
        rgb_report2.arguments[2] = buf[5];
        rgb_report3.arguments[0] = buf[6];
        rgb_report3.arguments[1] = buf[7];
        rgb_report3.arguments[2] = buf[8];

        // ON/Static
        effect_byte.bits.on_off_static = 1;
        effect_byte.bits.three_colour_breathing = 1;
        effect_byte.bits.sync = 1;
        effect_report.arguments[0] = effect_byte.value;

        // Lock sending of the 2 commands
        mutex_lock(&device->lock);
        razer_kraken_send_control_msg(device->hdev, &rgb_report, 0);

        razer_kraken_send_control_msg(device->hdev, &rgb_report2, 0);

        razer_kraken_send_control_msg(device->hdev, &rgb_report3, 0);

        razer_kraken_send_control_msg(device->hdev, &effect_report, 0);
        mutex_unlock(&device->lock);

    } else {
        dev_warn(dev, "razerkraken: Breathing mode only accepts RGB (3byte), RGB RGB (6byte) or RGB RGB RGB (9byte)\n");
        return -EINVAL;
    }

    return count;
}

/**
 * Read device file "mode_breath"
 *
 * Returns 4, 8, 12 bytes for config
 */
static ssize_t razer_attr_read_matrix_effect_breath(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    union razer_kraken_effect_byte effect_byte;
    unsigned char num_colours = 1;

    effect_byte.value = get_current_effect(dev);

    if(effect_byte.bits.two_colour_breathing == 1) {
        num_colours = 2;
    } else if(effect_byte.bits.three_colour_breathing == 1) {
        num_colours = 3;
    }

    switch(device->usb_pid) {
    case USB_DEVICE_ID_RAZER_KRAKEN_V2:
    case USB_DEVICE_ID_RAZER_KRAKEN_TE:
    case USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE:
    case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2:
        switch(num_colours) {
        case 3:
            return get_rgb_from_addr(dev, device->breathing_address[2], 0x0C, buf);
            break;
        case 2:
            return get_rgb_from_addr(dev, device->breathing_address[1], 0x08, buf);
            break;
        default:
            return get_rgb_from_addr(dev, device->breathing_address[0], 0x04, buf);
            break;
        }
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN:
        return get_rgb_from_addr(dev, device->breathing_address[0], 0x04, buf);
        break;

    default:
        dev_warn(dev, "razerkraken: Unknown device\n");
        return -EINVAL;
    }
}

/**
 * Read device file "serial"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_device_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report report = get_kraken_request_report(0x04, 0x20, 0x16, 0x7f00);

    if (device->usb_pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO ||
        device->usb_pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO_WIRED) {
        /* V3 Pro serial query path is not yet decoded — return a placeholder. */
        if (device->serial[0] == '\0')
            strncpy(device->serial, "BS_V3PRO_000000", sizeof(device->serial) - 1);
        return sprintf(buf, "%s\n", device->serial);
    }

    if (device->usb_pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3 ||
        device->usb_pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_WIRED) {
        if (device->serial[0] == '\0') {
            u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
            u8 slen;

            razer_blackshark_build_get(cmdbuf, BLACKSHARK_PARAM_SERIAL, device->usb_pid);
            mutex_lock(&device->lock);
            razer_blackshark_send_cmd(device, cmdbuf);
            /* Response: data[1]=0x02, data[10]=param, data[12]=str_len, data[13..]=serial */
            if (device->data[1] == 0x02 && device->data[10] == BLACKSHARK_PARAM_SERIAL) {
                slen = device->data[12];
                if (slen >= sizeof(device->serial))
                    slen = sizeof(device->serial) - 1;
                memcpy(device->serial, &device->data[13], slen);
                device->serial[slen] = '\0';
            } else {
                strncpy(device->serial, "BS000000000000", sizeof(device->serial) - 1);
            }
            mutex_unlock(&device->lock);
        }
        return sprintf(buf, "%s\n", device->serial);
    }

    // Basically some simple caching
    // Also skips going to device if it doesn't contain the serial
    if(device->serial[0] == '\0') {

        mutex_lock(&device->lock);
        device->data[0] = 0x00;
        razer_kraken_send_control_msg(device->hdev, &report, 1);
        msleep(25); // Sleep 20ms

        // Check for actual data
        if(device->data[0] == 0x05) {
            // Serial is present
            memcpy(device->serial, &device->data[1], 22);
            device->serial[22] = '\0';
        } else {
            dev_err(dev, "razerkraken: Did not manage to get serial from device, using XX01 instead\n");
            device->serial[0] = 'X';
            device->serial[1] = 'X';
            device->serial[2] = '0';
            device->serial[3] = '1';
            device->serial[4] = '\0';
        }
        mutex_unlock(&device->lock);

    }

    return sysfs_emit(buf, "%s\n", device->serial);
}

/**
 * Read device file "get_firmware_version"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_firmware_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report report = get_kraken_request_report(0x04, 0x20, 0x02, 0x0030);

    if (device->usb_pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3 ||
        device->usb_pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_WIRED ||
        device->usb_pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO ||
        device->usb_pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO_WIRED) {
        return sprintf(buf, "v1.0\n");
    }

    // Basically some simple caching
    if(device->firmware_version[0] != 1) {

        mutex_lock(&device->lock);
        device->data[0] = 0x00;
        razer_kraken_send_control_msg(device->hdev, &report, 1);
        msleep(25); // Sleep 20ms

        // Check for actual data
        if(device->data[0] == 0x05) {
            // Serial is present
            device->firmware_version[0] = 1;
            device->firmware_version[1] = device->data[1];
            device->firmware_version[2] = device->data[2];
        } else {
            dev_err(dev, "razerkraken: Did not manage to get firmware version from device, using v9.99 instead\n");
            device->firmware_version[0] = 1;
            device->firmware_version[1] = 0x09;
            device->firmware_version[2] = 0x99;
        }
        mutex_unlock(&device->lock);
    }

    return sysfs_emit(buf, "v%x.%x\n", device->firmware_version[1], device->firmware_version[2]);
}

/**
 * Read device file "matrix_current_effect"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_matrix_current_effect(struct device *dev, struct device_attribute *attr, char *buf)
{
    unsigned char current_effect = get_current_effect(dev);

    return sysfs_emit(buf, "%02x\n", current_effect);
}

/**
 * Write device file "device_mode"
 */
static ssize_t razer_attr_write_device_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return count;
}

/**
 * Read device file "device_mode"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_device_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
    buf[0] = 0x00;
    buf[1] = 0x00;

    return 2;
}

/**
 * Set up the device driver files

 *
 * Read only is 0444
 * Write only is 0220
 * Read and write is 0664
 */

/* ---- BlackShark V3 sysfs functions ---- */

static ssize_t razer_attr_read_mic_volume(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    u8 val = 0;

    razer_blackshark_build_get(cmdbuf, BLACKSHARK_PARAM_MIC_VOLUME, device->usb_pid);
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    if (device->data[1] == 0x02 && device->data[10] == BLACKSHARK_PARAM_MIC_VOLUME)
        val = device->data[13];
    mutex_unlock(&device->lock);

    return sprintf(buf, "%d\n", val);
}

static ssize_t razer_attr_write_mic_volume(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    if (val > 100) val = 100;

    razer_blackshark_build_set(cmdbuf, BLACKSHARK_SET_MIC_VOLUME, (u8)val,
                               razer_blackshark_set_dir(device->usb_pid));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_wireless_power_save(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    s8 val;

    /* TODO: V3 power-save GET class is unverified. The previous code read
     * class 0x2c, but the 2026-05-02 pcap analysis identified 0x2c as
     * sidetone-level read (returns 0..15), not power-save minutes. Until
     * the right GET class is identified (no 2.4GHz pcap shows a power_save
     * read on its own — it's only ever set), fall back to the cache that
     * write_wireless_power_save populates. -1 = never written this session. */
    mutex_lock(&device->lock);
    val = device->cached_v3_power_save;
    mutex_unlock(&device->lock);

    return sprintf(buf, "%d\n", val);
}

/* Valid values: 15, 30, 45, 60 minutes; snap to nearest */
static ssize_t razer_attr_write_wireless_power_save(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;
    u8 snapped;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    if (val <= 15)       snapped = 15;
    else if (val <= 30)  snapped = 30;
    else if (val <= 45)  snapped = 45;
    else                 snapped = 60;

    razer_blackshark_build_set(cmdbuf, BLACKSHARK_SET_POWER_SAVE, snapped,
                               razer_blackshark_set_dir(device->usb_pid));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_v3_power_save = snapped;
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_ultra_low_latency(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    s8 val = -1;

    razer_blackshark_build_get(cmdbuf, BLACKSHARK_PARAM_ULTRA_LOW_LATENCY, device->usb_pid);
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    if (device->data[1] == 0x02 && device->data[10] == BLACKSHARK_PARAM_ULTRA_LOW_LATENCY) {
        val = device->data[13];
        device->cached_v3_ull = val;
    } else {
        val = device->cached_v3_ull;
    }
    mutex_unlock(&device->lock);

    return sprintf(buf, "%d\n", val);
}

static ssize_t razer_attr_write_ultra_low_latency(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;

    razer_blackshark_build_set(cmdbuf, BLACKSHARK_SET_ULTRA_LOW_LATENCY, val ? 1 : 0,
                               razer_blackshark_set_dir(device->usb_pid));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_v3_ull = val ? 1 : 0;
    mutex_unlock(&device->lock);

    return count;
}

/*
 * sidetone write: 0..15 (verified from captures, cmd 0x99).
 * On first write we also send the init command (0x98) once per session.
 */
static ssize_t razer_attr_write_sidetone(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;
    u8 dir;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    if (val > 15) val = 15;
    dir = razer_blackshark_set_dir(device->usb_pid);

    mutex_lock(&device->lock);
    razer_blackshark_build_set(cmdbuf, BLACKSHARK_SET_SIDETONE_INIT, 0x01, dir);
    razer_blackshark_send_cmd(device, cmdbuf);
    razer_blackshark_build_set(cmdbuf, BLACKSHARK_SET_SIDETONE_LEVEL, (u8)val, dir);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_v3_sidetone = (s8)val;
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_sidetone(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->cached_v3_sidetone);
}

/*
 * mic_eq_preset write: 0..3 (Default, Esports, Broadcast, MicBoost). Cmd 0x96.
 * The device expects byte 13 = 0x20 + preset_idx.
 */
static ssize_t razer_attr_write_mic_eq_preset(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    if (val > 3) val = 3;

    razer_blackshark_build_set(cmdbuf, BLACKSHARK_SET_MIC_EQ_PRESET, 0x20 + (u8)val,
                               razer_blackshark_set_dir(device->usb_pid));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_v3_mic_eq_preset = (s8)val;
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_mic_eq_preset(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->cached_v3_mic_eq_preset);
}

/*
 * mic_eq write format: 10 space-separated band gains -6..+6 dB.
 * Frequencies: 31Hz 63Hz 125Hz 250Hz 500Hz 1kHz 2kHz 4kHz 8kHz 16kHz.
 */
static ssize_t razer_attr_write_mic_eq(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    s8 bands[10];
    int vals[10], i, n;

    n = sscanf(buf, "%d %d %d %d %d %d %d %d %d %d",
               &vals[0], &vals[1], &vals[2], &vals[3], &vals[4],
               &vals[5], &vals[6], &vals[7], &vals[8], &vals[9]);
    if (n != 10)
        return -EINVAL;
    for (i = 0; i < 10; i++) {
        vals[i] = clamp(vals[i], -6, 6);
        bands[i] = (s8)vals[i];
    }

    razer_blackshark_build_mic_eq_cmd(cmdbuf, bands,
                                      razer_blackshark_set_dir(device->usb_pid));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    mutex_unlock(&device->lock);

    return count;
}

/*
 * audio_function_button write: 1=sidetone save, 2=footsteps scaling. Cmd 0xea.
 */
static ssize_t razer_attr_write_audio_function_button(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    /* Was clamped to 1..2 (Sidetone, Footsteps). Widened to 0..255 so userspace
     * can probe other byte values from the Synapse CONTROL_KNOB enum
     * (GAME_CHAT_BALANCE=20, SWITCH_INPUT_SOURCE=18, MIC_SIDETONE_LEVEL=19, etc.)
     * to map the V3's full FN-button cycle. Bytes the firmware doesn't recognise
     * just no-op on the device side. */
    if (val > 0xff)
        return -EINVAL;

    razer_blackshark_build_set(cmdbuf, BLACKSHARK_SET_FN_BUTTON, (u8)val,
                               razer_blackshark_set_dir(device->usb_pid));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_v3_fn_button = (s8)val;
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_audio_function_button(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->cached_v3_fn_button);
}

static ssize_t razer_attr_read_headphone_eq(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    int len = 0, i;

    mutex_lock(&device->lock);
    /* "<active_preset> b0 b1 .. b9" — the leading preset index is what the
     * control app reads as the active EQ profile; raw_event updates
     * cached_v3_eq_active from the on-board EQ button (cls=0x60 push). */
    len += sprintf(buf + len, "%d ", (int)device->cached_v3_eq_active);
    for (i = 0; i < 10; i++) {
        len += sprintf(buf + len, "%d", (int)device->eq_bands[i]);
        if (i < 9) len += sprintf(buf + len, " ");
    }
    mutex_unlock(&device->lock);

    len += sprintf(buf + len, "\n");
    return len;
}

/*
 * headphone_eq write format:  "PROFILE B0 B1 B2 B3 B4 B5 B6 B7 B8 B9"
 * PROFILE 0=Default 1=Game 2=Movie 3=Music 4=Esports
 * If only 10 values given (no profile prefix), profile 1 is used.
 */
static ssize_t razer_attr_write_headphone_eq(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmds[5][RAZER_BLACKSHARK_REPORT_LEN];
    s8 bands[10];
    int vals[11], i, n;
    u8 profile_idx;

    /* Try 11-value format: profile + 10 bands */
    n = sscanf(buf, "%d %d %d %d %d %d %d %d %d %d %d",
               &vals[0], &vals[1], &vals[2], &vals[3], &vals[4],
               &vals[5], &vals[6], &vals[7], &vals[8], &vals[9], &vals[10]);

    if (n == 11) {
        profile_idx = (u8)clamp(vals[0], 0, BLACKSHARK_V3_PRO_EQ_PRESET_COUNT - 1);
        for (i = 0; i < 10; i++) {
            vals[i] = clamp(vals[i + 1], -6, 6);
            bands[i] = (s8)vals[i];
        }
    } else if (n == 10) {
        /* Legacy 10-value format — write to profile 1 (Game slot) */
        profile_idx = 1;
        for (i = 0; i < 10; i++) {
            vals[i] = clamp(vals[i], -6, 6);
            bands[i] = (s8)vals[i];
        }
    } else {
        return -EINVAL;
    }

    razer_blackshark_build_eq_cmds(cmds, bands,
                                   razer_blackshark_set_dir(device->usb_pid),
                                   profile_idx);

    mutex_lock(&device->lock);
    for (i = 0; i < 5; i++)
        razer_blackshark_send_cmd(device, cmds[i]);
    for (i = 0; i < 10; i++)
        device->eq_bands[i] = bands[i];
    /* Writing a profile also activates it, so reflect it as the active preset.
     * Without this, read_headphone_eq keeps reporting the previously active
     * slot and the control app's live-sync reverts the user's selection. */
    device->cached_v3_eq_active = profile_idx;
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_thx_spatial_audio(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    s8 val = -1;

    razer_blackshark_build_get_thx(cmdbuf);
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    if (device->data[1] == 0x02 && device->data[10] == BLACKSHARK_PARAM_THX) {
        val = device->data[13];
        device->cached_v3_thx = val;
    } else {
        val = device->cached_v3_thx;
    }
    mutex_unlock(&device->lock);

    return sprintf(buf, "%d\n", val);
}

static ssize_t razer_attr_write_thx_spatial_audio(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;

    razer_blackshark_build_set(cmdbuf, BLACKSHARK_PARAM_THX | 0x80, val ? 1 : 0,
                               razer_blackshark_set_dir(device->usb_pid));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_v3_thx = val ? 1 : 0;
    mutex_unlock(&device->lock);

    return count;
}

/* ---- end BlackShark V3 sysfs functions ---- */

/* ---- BlackShark V3 Pro sysfs functions (untested — third-party RE) ---- */

/*
 * Replay the Synapse startup wake sequence on V3 wireless. Without these
 * three priming SETs the dongle silently drops every subsequent wireless
 * query on Linux. Decoded from Synapse usbmon captures.
 *
 *   1. cls=0x02 dir=0x00   (init step 1)
 *   2. cls=0x2A dir=0x00   (init step 2)
 *   3. cls=0x2A dir=0x80   (init step 3)
 *
 * Must NOT hold device->lock — this issues 3 control transfers and sleeps.
 */
static void razer_blackshark_v3_handshake(struct razer_kraken_device *device)
{
    static const struct {
        u8 cls;
        u8 dir;
    } primers[] = {
        { 0x02, 0x00 },
        { 0x2A, 0x00 },
        { 0x2A, 0x80 },
    };
    u8 prime[RAZER_BLACKSHARK_REPORT_LEN];
    u8 *kp;
    int idx, j, ret;
    u8 crc;

    for (idx = 0; idx < (int)ARRAY_SIZE(primers); idx++) {
        razer_blackshark_v3pro_build(prime, primers[idx].cls, 0x00, NULL, 0);
        prime[9] = primers[idx].dir;
        crc = 0;
        for (j = 0; j < 62; j++) crc ^= prime[j];
        prime[62] = crc;

        kp = kmemdup(prime, RAZER_BLACKSHARK_REPORT_LEN, GFP_KERNEL);
        if (!kp)
            return;
        ret = hid_hw_raw_request(device->hdev, 0x02, kp,
                                 RAZER_BLACKSHARK_REPORT_LEN,
                                 HID_OUTPUT_REPORT, HID_REQ_SET_REPORT);
        kfree(kp);
        hid_dbg(device->hdev, "v3 handshake cls=%02x dir=%02x: %d\n",
                primers[idx].cls, primers[idx].dir, ret);
        /* 2ms gap mirrors Windows-with-driver pacing. Earlier 30ms was wrong
         * — Synapse fires SETs back-to-back at ~3ms intervals; a longer gap
         * may let the firmware's handshake state machine reset between
         * primers. */
        usleep_range(2000, 3000);
    }
}

/*
 * Issue cls=0x21 battery query and poll GET_REPORT for the reply.
 *
 * The standard send_cmd → raw_event → completion path times out for V3
 * wireless on Linux (the dongle never pushes int-IN on EP 0x84). The
 * firmware does answer via control GET_REPORT once the handshake has
 * primed it, so we drive that directly here.
 *
 * Caller must hold device->lock. Returns 0 if a reply landed in
 * device->data, -ETIMEDOUT otherwise.
 */
static int razer_blackshark_v3_battery_query(struct razer_kraken_device *device)
{
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    bool is_v3_pro = (device->usb_pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO ||
                      device->usb_pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO_WIRED);

    /* V3 Pro firmware needs cls=0x21 with args=[0x00] (data_size=5).
     * V3 wireless firmware needs cls=0x21 with NO args (data_size=4) —
     * exactly what Synapse sends per the cold-plug pcap analysis.
     * This branch matches the 2026-05-02 working logic (commit a5d74fd5). */
    if (is_v3_pro) {
        const u8 args[1] = { 0x00 };
        razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_BATTERY_CLASS,
                                     BLACKSHARK_V3_PRO_BATTERY_ID, args, sizeof(args));
    } else {
        razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_BATTERY_CLASS,
                                     BLACKSHARK_V3_PRO_BATTERY_ID, NULL, 0);
    }
    /* send_cmd issues the SET_REPORT through hid-core and waits up to 1s on
     * dev->vendor_response, which raw_event() fires when the firmware pushes
     * the int-IN reply. V3 Pro replies fast (~40ms); V3 wireless never
     * replies and times out (firmware-side gate). */
    return razer_blackshark_send_cmd(device, cmdbuf);
}

/*
 * Both V3 and V3 Pro return battery as a 0..100 byte. The standard openrazer
 * convention for charge_level is a 0..255 byte (mamba.py scales by 255/100
 * to display percent). Multiply by 255/100 here so the daemon's existing
 * mamba.get_battery does the right thing.
 */
static ssize_t razer_attr_read_charge_level(struct device *dev, struct device_attribute *attr, char *buf)
{
    /* Pure cache read (see charge_status). Battery arrives as a cls=0x21 push
     * cached into pushed_battery_pct; the cache is primed once at probe. A live
     * query on every read reset the wireless link under the daemon's ~2s
     * polling, which power-cycled the headset. */
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    int level = -1;

    if (device->pushed_battery_pct >= 0)
        level = (device->pushed_battery_pct * 255) / 100;

    return sprintf(buf, "%d\n", level);
}

static ssize_t razer_attr_read_charge_status(struct device *dev, struct device_attribute *attr, char *buf)
{
    /* Pure cache read. Charging arrives on cls=0x2a (data[13]=0/1) as a push
     * that raw_event() / the ep 0x84 URB caches into pushed_charging. Issuing a
     * live query here on every read (as this used to) meant the daemon polling
     * charge_status every ~2s hammered the wireless link with SET_REPORTs and
     * reset the dongle — which showed up as the headset power-cycling. */
    struct razer_kraken_device *device = dev_get_drvdata(dev);

    return sprintf(buf, "%d\n", device->pushed_charging);
}

/*
 * Query the firmware-stored EQ bands for one slot. cls=0x15 GET arg=[slot];
 * the reply is cls=0x15 sub=0x01 cnt=0x0b with data[13]=slot and
 * data[14..23]=b0..b9 (sign-magnitude). Caller must NOT hold device->lock.
 * On success fills device->eq_query_{slot,bands} and returns 0.
 */
static int razer_blackshark_v3_eq_query(struct razer_kraken_device *device, u8 slot)
{
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    const u8 args[1] = { slot };
    int i;

    /* Mark pending. The 0x15 reply arrives asynchronously on ep 0x84 and is
     * cached by razer_blackshark_v3_cache() into eq_query_{slot,bands}; we do
     * not read it off device->data because send_cmd's completion also fires
     * for the keepalive and unrelated pushes, so device->data can hold a
     * different frame by the time send_cmd returns. */
    device->eq_query_slot = -1;

    razer_blackshark_v3_handshake(device);

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_PARAM_EQ_BANDS, 0x01, args, sizeof(args));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    mutex_unlock(&device->lock);

    /* Wait (up to ~300ms) for the callback to cache the matching slot. */
    for (i = 0; i < 30 && device->eq_query_slot != (s8)slot; i++)
        msleep(10);

    return (device->eq_query_slot == (s8)slot) ? 0 : -ETIMEDOUT;
}

/*
 * eq_slot: write a slot index (0..8) to read that slot's firmware-stored EQ
 * bands; the read returns "<slot> b0 b1 .. b9" from the last query. Lets the
 * control app load the headset's real per-slot bands (including custom slots
 * edited on-device or in Synapse) instead of assuming factory values.
 */
static ssize_t razer_attr_write_eq_slot(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    unsigned long slot;

    if (kstrtoul(buf, 10, &slot) || slot >= BLACKSHARK_V3_PRO_EQ_PRESET_COUNT)
        return -EINVAL;
    razer_blackshark_v3_eq_query(device, (u8)slot);
    return count;
}

static ssize_t razer_attr_read_eq_slot(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    int len = 0, i;

    mutex_lock(&device->lock);
    len += sprintf(buf + len, "%d", (int)device->eq_query_slot);
    for (i = 0; i < 10; i++)
        len += sprintf(buf + len, " %d", (int)device->eq_query_bands[i]);
    mutex_unlock(&device->lock);
    len += sprintf(buf + len, "\n");
    return len;
}

static ssize_t razer_attr_write_v3pro_sidetone(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;
    u8 args[2];

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    if (val > BLACKSHARK_V3_PRO_SIDETONE_MAX)
        val = BLACKSHARK_V3_PRO_SIDETONE_MAX;
    args[0] = (u8)val;
    args[1] = 0x00;

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_SIDETONE_SET_CL,
                                 BLACKSHARK_V3_PRO_SIDETONE_ID, args, sizeof(args));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_v3pro_sidetone = (s8)val;
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_v3pro_sidetone(struct device *dev, struct device_attribute *attr, char *buf)
{
    /* Pure cache read, like every other on-board-push-backed attr (game_chat,
     * in_call_mix, audio_fn_button, ...). raw_event caches sidetone from the
     * headset's own pushes (BLACKSHARK_PARAM_SIDETONE_VOLUME 0x19). The old
     * version issued a live blocking query on every read — same anti-pattern
     * as the charge_status bug; besides being slow, polling it every 700ms
     * from the control app contended with the push channel enough to make
     * on-board sidetone changes unreliable to observe. */
    struct razer_kraken_device *device = dev_get_drvdata(dev);

    return sprintf(buf, "%d\n", device->cached_v3pro_sidetone);
}

static ssize_t razer_attr_write_v3pro_thx(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;
    u8 args[2];

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    args[0] = val ? 1 : 0;
    args[1] = 0x00;

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_THX_CLASS,
                                 BLACKSHARK_V3_PRO_THX_ID, args, sizeof(args));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_v3pro_thx = args[0];
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_v3pro_thx(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->cached_v3pro_thx);
}

/* ANC: write "mode level" — mode 0=off, 1=ANC, 2=ambient. Level 1..4 (ANC only).
 * Examples: "1 3" = ANC level 3, "2 1" = ambient, "0 0" = off. */
static ssize_t razer_attr_write_v3pro_anc(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned int mode_in, level;
    u8 args[2];

    if (sscanf(buf, "%u %u", &mode_in, &level) != 2)
        return -EINVAL;
    if (level < BLACKSHARK_V3_PRO_ANC_LEVEL_MIN) level = BLACKSHARK_V3_PRO_ANC_LEVEL_MIN;
    if (level > BLACKSHARK_V3_PRO_ANC_LEVEL_MAX) level = BLACKSHARK_V3_PRO_ANC_LEVEL_MAX;

    switch (mode_in) {
    case 1:
        args[0] = BLACKSHARK_V3_PRO_ANC_MODE_ANC;
        break;
    case 2:
        args[0] = BLACKSHARK_V3_PRO_ANC_MODE_AMBIENT;
        break;
    default:
        args[0] = BLACKSHARK_V3_PRO_ANC_MODE_OFF;
        break;
    }
    args[1] = (u8)level;

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_ANC_CLASS,
                                 BLACKSHARK_V3_PRO_ANC_ID, args, sizeof(args));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_v3pro_anc_mode = mode_in > 2 ? 0 : (s8)mode_in;
    device->cached_v3pro_anc_level = (s8)level;
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_v3pro_anc(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d %d\n", device->cached_v3pro_anc_mode,
                   device->cached_v3pro_anc_level);
}

/* Ultra-Low Latency toggle (V3 Pro). Args: [on/off, 0x00]. */
static ssize_t razer_attr_write_v3pro_ull(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;
    u8 args[2];

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    args[0] = val ? 1 : 0;
    args[1] = 0x00;

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_ULL_CLASS,
                                 BLACKSHARK_V3_PRO_ULL_ID, args, sizeof(args));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_v3pro_ull = args[0];
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_v3pro_ull(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->cached_v3pro_ull);
}

/* Game/Chat balance. Args: [balance 0..20, 0x00]. 0=full game, 10=center, 20=full chat. */
static ssize_t razer_attr_write_game_chat_balance(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;
    u8 args[2];

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    if (val > BLACKSHARK_V3_PRO_GAME_CHAT_MAX)
        val = BLACKSHARK_V3_PRO_GAME_CHAT_MAX;
    args[0] = (u8)val;
    args[1] = 0x00;

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_GAME_CHAT_CLASS,
                                 BLACKSHARK_V3_PRO_GAME_CHAT_ID, args, sizeof(args));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_game_chat_balance = args[0];
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_game_chat_balance(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->cached_game_chat_balance);
}

/* In-call audio mix. Args: [mode, 0x00]; 0=combine, 1=lower, 2=mute. */
static ssize_t razer_attr_write_in_call_audio_mix(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;
    u8 args[2];

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    if (val > 2) val = 2;
    args[0] = (u8)val;
    args[1] = 0x00;

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_INCALL_MIX_CLASS,
                                 BLACKSHARK_V3_PRO_INCALL_MIX_ID, args, sizeof(args));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_in_call_audio_mix = args[0];
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_in_call_audio_mix(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->cached_in_call_audio_mix);
}

/* Audio prompts toggle. Args: [0x00, on, 0x00]. */
static ssize_t razer_attr_write_audio_prompts(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;
    u8 args[3];

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    args[0] = 0x00;
    args[1] = val ? 1 : 0;
    args[2] = 0x00;

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_AUDIO_PROMPTS_CL,
                                 BLACKSHARK_V3_PRO_AUDIO_PROMPTS_ID, args, sizeof(args));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_audio_prompts = args[1];
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_audio_prompts(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->cached_audio_prompts);
}

/* Power save timeout in minutes (0=disabled, 15/30/45/60). */
static ssize_t razer_attr_write_v3pro_power_save(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    u8 cmdbuf[RAZER_BLACKSHARK_REPORT_LEN];
    unsigned long val;
    u8 args[2];

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    if (val > 60) val = 60;
    args[0] = (u8)val;
    args[1] = 0x00;

    razer_blackshark_v3pro_build(cmdbuf, BLACKSHARK_V3_PRO_POWER_SAVE_CLASS,
                                 BLACKSHARK_V3_PRO_POWER_SAVE_ID, args, sizeof(args));
    mutex_lock(&device->lock);
    razer_blackshark_send_cmd(device, cmdbuf);
    device->cached_v3pro_power_save = args[0];
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_attr_read_v3pro_power_save(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->cached_v3pro_power_save);
}

/* Headphone EQ: writes a single byte = preset slot index (0..8). */
static ssize_t razer_attr_write_v3pro_headphone_eq(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    unsigned long val;
    int ret;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    if (val >= BLACKSHARK_V3_PRO_EQ_PRESET_COUNT)
        return -EINVAL;

    mutex_lock(&device->lock);
    ret = razer_blackshark_v3pro_apply_eq(device, (u8)val);
    if (ret >= 0)
        device->cached_v3pro_eq_profile = (s8)val;
    mutex_unlock(&device->lock);

    return ret < 0 ? ret : count;
}

static ssize_t razer_attr_read_v3pro_headphone_eq(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->cached_v3pro_eq_profile);
}

/* ---- end BlackShark V3 Pro sysfs functions ---- */

static DEVICE_ATTR(test,                    0660, razer_attr_read_test,                       razer_attr_write_test);
static DEVICE_ATTR(version,                 0440, razer_attr_read_version,                    NULL);
static DEVICE_ATTR(device_type,             0440, razer_attr_read_device_type,                NULL);
static DEVICE_ATTR(device_serial,           0440, razer_attr_read_device_serial,              NULL);
static DEVICE_ATTR(device_mode,             0660, razer_attr_read_device_mode,                razer_attr_write_device_mode);
static DEVICE_ATTR(firmware_version,        0440, razer_attr_read_firmware_version,           NULL);

static DEVICE_ATTR(matrix_current_effect,   0440, razer_attr_read_matrix_current_effect,      NULL);
static DEVICE_ATTR(matrix_effect_none,      0220, NULL,                                       razer_attr_write_matrix_effect_none);
static DEVICE_ATTR(matrix_effect_spectrum,  0220, NULL,                                       razer_attr_write_matrix_effect_spectrum);
static DEVICE_ATTR(matrix_effect_static,    0660, razer_attr_read_matrix_effect_static,       razer_attr_write_matrix_effect_static);
static DEVICE_ATTR(matrix_effect_custom,    0660, razer_attr_read_matrix_effect_custom,       razer_attr_write_matrix_effect_custom);
static DEVICE_ATTR(matrix_effect_breath,    0660, razer_attr_read_matrix_effect_breath,       razer_attr_write_matrix_effect_breath);
static DEVICE_ATTR(mic_volume,              0660, razer_attr_read_mic_volume,              razer_attr_write_mic_volume);
static DEVICE_ATTR(wireless_power_save,     0660, razer_attr_read_wireless_power_save,     razer_attr_write_wireless_power_save);
static DEVICE_ATTR(ultra_low_latency,       0660, razer_attr_read_ultra_low_latency,       razer_attr_write_ultra_low_latency);
static DEVICE_ATTR(headphone_eq,            0660, razer_attr_read_headphone_eq,            razer_attr_write_headphone_eq);
static DEVICE_ATTR(thx_spatial_audio,       0660, razer_attr_read_thx_spatial_audio,       razer_attr_write_thx_spatial_audio);
static DEVICE_ATTR(sidetone,                0660, razer_attr_read_sidetone,                razer_attr_write_sidetone);
static DEVICE_ATTR(mic_eq,                  0220, NULL,                                    razer_attr_write_mic_eq);
static ssize_t razer_attr_read_mic_mute(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->cached_mic_muted);
}
static DEVICE_ATTR(mic_eq_preset,           0660, razer_attr_read_mic_eq_preset,           razer_attr_write_mic_eq_preset);
static DEVICE_ATTR(mic_mute,                0440, razer_attr_read_mic_mute,                NULL);
static DEVICE_ATTR(eq_slot,                 0660, razer_attr_read_eq_slot,                 razer_attr_write_eq_slot);
static DEVICE_ATTR(audio_function_button,   0660, razer_attr_read_audio_function_button,   razer_attr_write_audio_function_button);
/* BlackShark V3 Pro (PID 0x0577) — distinct attribute names so they don't collide
 * with V3's headphone_eq (which has 10-band write semantics). */
static DEVICE_ATTR(charge_level,               0440, razer_attr_read_charge_level,            NULL);
static DEVICE_ATTR(charge_status,              0440, razer_attr_read_charge_status,           NULL);
static DEVICE_ATTR(v3pro_sidetone,             0660, razer_attr_read_v3pro_sidetone,          razer_attr_write_v3pro_sidetone);
static DEVICE_ATTR(v3pro_thx_spatial_audio,    0660, razer_attr_read_v3pro_thx,               razer_attr_write_v3pro_thx);
static DEVICE_ATTR(v3pro_anc,                  0660, razer_attr_read_v3pro_anc,               razer_attr_write_v3pro_anc);
static DEVICE_ATTR(v3pro_ultra_low_latency,    0660, razer_attr_read_v3pro_ull,               razer_attr_write_v3pro_ull);
static DEVICE_ATTR(v3pro_power_save,           0660, razer_attr_read_v3pro_power_save,        razer_attr_write_v3pro_power_save);
static DEVICE_ATTR(v3pro_headphone_eq,         0660, razer_attr_read_v3pro_headphone_eq,      razer_attr_write_v3pro_headphone_eq);
/* Shared between V3 (both PIDs) and V3 Pro — same wire envelope, same bytes. */
static DEVICE_ATTR(game_chat_balance,          0660, razer_attr_read_game_chat_balance,       razer_attr_write_game_chat_balance);
static DEVICE_ATTR(in_call_audio_mix,          0660, razer_attr_read_in_call_audio_mix,       razer_attr_write_in_call_audio_mix);
static DEVICE_ATTR(audio_prompts,              0660, razer_attr_read_audio_prompts,           razer_attr_write_audio_prompts);

static void razer_kraken_init(struct razer_kraken_device *dev, struct usb_interface *intf, struct hid_device *hdev)
{
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned int rand_serial = 0;

    // Initialise mutex
    mutex_init(&dev->lock);
    // Setup values
    dev->hdev = hdev;
    dev->usb_dev = usb_dev;
    dev->usb_interface_protocol = intf->cur_altsetting->desc.bInterfaceProtocol;
    dev->usb_vid = usb_dev->descriptor.idVendor;
    dev->usb_pid = usb_dev->descriptor.idProduct;

    /* -1 = "unknown" (no SET seen this session). GUI falls back to JSON cache. */
    dev->cached_v3_power_save      = -1;
    dev->cached_v3_ull             = -1;
    dev->cached_v3_thx             = -1;
    dev->cached_v3_eq_active       = -1;
    dev->cached_v3_sidetone        = -1;
    dev->cached_v3_mic_eq_preset   = -1;
    dev->cached_v3_fn_button       = -1;
    dev->cached_v3pro_thx          = -1;
    dev->cached_v3pro_anc_mode     = -1;
    dev->cached_v3pro_anc_level    = -1;
    dev->cached_v3pro_ull          = -1;
    dev->cached_v3pro_power_save   = -1;
    dev->cached_v3pro_eq_profile   = -1;
    dev->cached_v3pro_sidetone     = -1;
    dev->cached_game_chat_balance  = -1;
    dev->cached_in_call_audio_mix  = -1;
    dev->cached_audio_prompts      = -1;
    dev->cached_mic_muted          = -1;
    dev->eq_query_slot             = -1;
    dev->pushed_battery_pct        = -1;
    dev->pushed_charging           = -1;

    switch(dev->usb_pid) {
    case USB_DEVICE_ID_RAZER_KRAKEN_V2:
    case USB_DEVICE_ID_RAZER_KRAKEN_TE:
    case USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE:
    case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2:
        dev->led_mode_address = KYLIE_SET_LED_ADDRESS;
        dev->custom_address = KYLIE_CUSTOM_ADDRESS_START;
        dev->breathing_address[0] = KYLIE_BREATHING1_ADDRESS_START;
        dev->breathing_address[1] = KYLIE_BREATHING2_ADDRESS_START;
        dev->breathing_address[2] = KYLIE_BREATHING3_ADDRESS_START;
        break;
    case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC:
    case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC_ALT:
    case USB_DEVICE_ID_RAZER_KRAKEN:
        dev->led_mode_address = RAINIE_SET_LED_ADDRESS;
        dev->custom_address = RAINIE_CUSTOM_ADDRESS_START;
        dev->breathing_address[0] = RAINIE_BREATHING1_ADDRESS_START;

        // Get a "random" integer
        get_random_bytes(&rand_serial, sizeof(unsigned int));
        sprintf(dev->serial, "HN%015u", rand_serial);
        break;
    case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_WIRED:
    case USB_DEVICE_ID_RAZER_BLACKSHARK_V3:
    case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO_WIRED:
    case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO:
        // No Chroma RGB — no LED memory addresses required
        break;
    }
}

/**
 * Probe method is ran whenever a device is binded to the driver
 */
static inline bool razer_blackshark_is_v3(u16 pid)
{
    return pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3 ||
           pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_WIRED ||
           pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO ||
           pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO_WIRED;
}

/* Only the V3 Pro gets the private ep 0x84 URB, the RF_WAKE keep-alive and the
 * battery prime query. The plain V3 (0579/057a) is served by the normal usbhid
 * path: binding our own URB there competed with usbhid for the interface's
 * consumer (volume/scroll dial) reports, and its firmware treats the battery
 * GET as link-disrupting (it drops the RF link and needs a replug). So on the
 * plain V3 we never query — charge_level/charge_status are pure cache reads fed
 * only by the headset's own pushes. */
static inline bool razer_blackshark_is_v3pro(u16 pid)
{
    return pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO ||
           pid == USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO_WIRED;
}

/*
 * Cache one 64-byte V3/V3 Pro frame into device state. Shared by raw_event()
 * (when the HID stack delivers a report, e.g. wired) and the private ep 0x84
 * URB callback (the wireless path, where usbhid never polls the endpoint).
 * Copies the frame to device->data, updates the push/on-board caches, and
 * wakes any send_cmd() waiting on vendor_response. Runs in softirq / URB
 * completion context — must not sleep or take device->lock.
 */
static void razer_blackshark_v3_cache(struct razer_kraken_device *device, u8 *data, int size)
{
    memcpy(&device->data[0], &data[0], size);

    /* Capture BOTH replies (sub=0x01) and unsolicited pushes (sub=0x02).
     * data[13]==0xff on a class marks a capability/handshake reply, not a
     * real value, so each case rejects it. */
    if ((data[11] == 0x01 || data[11] == 0x02) && data[12] >= 1) {
        switch (data[10]) {
        case 0x21: /* battery percent */
            if (data[13] <= 100)
                device->pushed_battery_pct = data[13];
            break;
        case 0x2a: /* charging flag (real 0/1 state only; 0xff = capability) */
            if (data[13] <= 1)
                device->pushed_charging = data[13] ? 1 : 0;
            break;
        case 0x20: /* RF link re-established — cached battery/charging from
                    * before the drop may be stale, so invalidate them. */
            if (data[13] == 0x01) {
                device->pushed_charging = -1;
                device->pushed_battery_pct = -1;
            }
            break;
        case BLACKSHARK_PARAM_SIDETONE_VOLUME: /* 0x19 */
            if (data[13] <= 15) {
                device->cached_v3_sidetone = data[13];
                device->cached_v3pro_sidetone = data[13];
            }
            break;
        case BLACKSHARK_PARAM_GAME_CHAT_BAL_PRO: /* 0x5c */
        case BLACKSHARK_PARAM_GAME_CHAT_BAL_V3:  /* 0x65 */
            if (data[13] != 0xff)
                device->cached_game_chat_balance = data[13];
            break;
        case BLACKSHARK_PARAM_IN_CALL_AUDIO_MIX: /* 0x5d */
            if (data[13] != 0xff)
                device->cached_in_call_audio_mix = data[13];
            break;
        case BLACKSHARK_PARAM_ULTRA_LOW_LATENCY: /* 0x5f */
            if (data[13] <= 1) {
                device->cached_v3_ull = data[13];
                device->cached_v3pro_ull = data[13];
            }
            break;
        case BLACKSHARK_PARAM_AUDIO_PROMPTS_GET: /* 0x66 */
            if (data[13] <= 1)
                device->cached_audio_prompts = data[13];
            break;
        case BLACKSHARK_PARAM_AUDIO_FN_GET: /* 0x6a */
            if (data[13] <= 3)
                device->cached_v3_fn_button = data[13];
            break;
        case BLACKSHARK_PARAM_THX: /* 0x9e */
            if (data[13] <= 1) {
                device->cached_v3_thx = data[13];
                device->cached_v3pro_thx = data[13];
            }
            break;
        case BLACKSHARK_PARAM_MIC_EQ_PRESET: /* 0x16 */
            if (data[13] != 0xff)
                device->cached_v3_mic_eq_preset = data[13];
            break;
        case BLACKSHARK_PARAM_MIC_STATUS: /* 0x55 mic mute */
            if (data[13] <= 1)
                device->cached_mic_muted = data[13];
            break;
        case BLACKSHARK_V3_PRO_ANC_POLL_CLASS: /* 0x12 on-board ANC button —
                    * verified on hardware 2026-07-21: cycling Off/ANC/Ambient
                    * pushed data[13]=0/1/0x50 and data[14]=level. The push
                    * arrives on the GET/poll class, not the 0x92 SET class.
                    * Translate the raw mode back to the driver's 0/1/2 scheme
                    * (matching razer_attr_write_v3pro_anc's forward mapping). */
            if (data[13] == BLACKSHARK_V3_PRO_ANC_MODE_OFF)
                device->cached_v3pro_anc_mode = 0;
            else if (data[13] == BLACKSHARK_V3_PRO_ANC_MODE_ANC)
                device->cached_v3pro_anc_mode = 1;
            else if (data[13] == BLACKSHARK_V3_PRO_ANC_MODE_AMBIENT)
                device->cached_v3pro_anc_mode = 2;
            if (data[14] >= BLACKSHARK_V3_PRO_ANC_LEVEL_MIN &&
                data[14] <= BLACKSHARK_V3_PRO_ANC_LEVEL_MAX)
                device->cached_v3pro_anc_level = data[14];
            break;
        case BLACKSHARK_PARAM_EQ_SLOT_META: /* 0x60 on-board EQ preset */
            if (data[13] < BLACKSHARK_V3_PRO_EQ_PRESET_COUNT) {
                device->cached_v3pro_eq_profile = data[13];
                device->cached_v3_eq_active = data[13];
            }
            break;
        case BLACKSHARK_PARAM_EQ_BANDS: /* 0x15 EQ band readback reply:
                    * cnt=0x0b [slot, b0..b9] sign-magnitude. Cache it here (not
                    * off device->data after send_cmd) because the completion
                    * also fires for the keepalive and other pushes. */
            if (data[12] >= 11) {
                int i;

                for (i = 0; i < 10; i++)
                    device->eq_query_bands[i] = bs_gain_decode(data[14 + i]);
                device->eq_query_slot = data[13];  /* set last: readers gate on it */
            }
            break;
        default:
            break;
        }
    }

    if (device->vendor_response_inited)
        complete(&device->vendor_response);
}

/*
 * RF_WAKE keep-alive: Output Report 5, payload [0x05, 0x00]. Sending it
 * ~every 3.5s keeps the dongle's RF telemetry channel from going idle and
 * dropping pushes. Ported from the standalone V3 Pro driver.
 */
static void razer_blackshark_v3_rf_wake(struct razer_kraken_device *device)
{
    static const u8 rpid5[2] = { 0x05, 0x00 };
    struct usb_interface *intf = to_usb_interface(device->hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int intf_num = intf->cur_altsetting->desc.bInterfaceNumber;
    u8 *r5 = kmemdup(rpid5, sizeof(rpid5), GFP_KERNEL);

    if (!r5)
        return;
    usb_control_msg(usb_dev, usb_sndctrlpipe(usb_dev, 0),
                    HID_REQ_SET_REPORT,
                    USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                    (HID_OUTPUT_REPORT + 1) << 8 | 0x05, /* Output Report, ID 5 */
                    intf_num, r5, sizeof(rpid5), USB_CTRL_SET_TIMEOUT);
    kfree(r5);
}

static void razer_blackshark_v3_rf_wake_keepalive(struct work_struct *work)
{
    struct razer_kraken_device *dev =
        container_of(work, struct razer_kraken_device, rf_wake_work.work);

    razer_blackshark_v3_rf_wake(dev);
    schedule_delayed_work(&dev->rf_wake_work, msecs_to_jiffies(3500));
}

/*
 * -EPROTO recovery: usb_clear_halt resets the ep 0x84 data toggle so the next
 * submission re-syncs. Must run in process context (not the URB callback).
 */
static void razer_blackshark_v3_intr_recover(struct work_struct *work)
{
    struct razer_kraken_device *dev =
        container_of(work, struct razer_kraken_device, intr_recover_work);
    struct usb_interface *intf = to_usb_interface(dev->hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    if (atomic_inc_return(&dev->intr_eproto_count) > 5)
        return;   /* cap CLEAR_FEATURE floods during hot-unplug */
    usb_clear_halt(usb_dev, usb_rcvintpipe(usb_dev, 4));
    if (dev->intr_urb)
        usb_submit_urb(dev->intr_urb, GFP_KERNEL);
}

/*
 * Completion callback for the private ep 0x84 interrupt-IN URB. Feeds the
 * received frame through the shared cache path (same as raw_event) and
 * resubmits so the endpoint stays polled.
 */
static void razer_blackshark_v3_intr_callback(struct urb *urb)
{
    struct razer_kraken_device *dev = urb->context;
    int status = urb->status;

    if (status == 0) {
        u8 *data = urb->transfer_buffer;
        int len = urb->actual_length;

        atomic_set(&dev->intr_eproto_count, 0);
        if (len == RAZER_BLACKSHARK_REPORT_LEN && data[0] == 0x02) {
            /* Razer vendor report (report id 0x02): telemetry + on-board
             * pushes + query replies. */
            razer_blackshark_v3_cache(dev, data, len);
        } else if (len > 0) {
            /* Consumer-control / other input reports (the volume/scroll dial,
             * media keys) also arrive on ep 0x84. Our private URB would
             * otherwise swallow them, breaking the dial — feed them into the
             * HID input stack exactly as usbhid's own interrupt handler would
             * so they keep working. */
            hid_input_report(dev->hdev, HID_INPUT_REPORT, data, len, 1);
        }
        /* Always resubmit on status 0 — short reports (button/consumer
         * events) must not permanently kill the URB. */
        usb_submit_urb(urb, GFP_ATOMIC);
    } else if (status == -EPROTO) {
        schedule_work(&dev->intr_recover_work);
    } else if (status != -ENOENT && status != -ESHUTDOWN && status != -ENODEV) {
        usb_submit_urb(urb, GFP_ATOMIC);
    }
    /* -ENOENT/-ESHUTDOWN/-ENODEV: device gone, do not resubmit. */
}

/*
 * Allocate and pre-submit the private ep 0x84 URB. Called before hid_hw_start
 * so the endpoint is polled the moment usbhid_start's SET_IDLE activates it.
 * Best-effort: on failure the HID path still delivers whatever it can.
 */
static void razer_blackshark_v3_intr_start(struct razer_kraken_device *dev,
        struct usb_device *usb_dev)
{
    INIT_WORK(&dev->intr_recover_work, razer_blackshark_v3_intr_recover);
    INIT_DELAYED_WORK(&dev->rf_wake_work, razer_blackshark_v3_rf_wake_keepalive);
    atomic_set(&dev->intr_eproto_count, 0);

    dev->intr_buf = usb_alloc_coherent(usb_dev, RAZER_BLACKSHARK_REPORT_LEN,
                                       GFP_KERNEL, &dev->intr_dma);
    dev->intr_urb = usb_alloc_urb(0, GFP_KERNEL);
    if (dev->intr_buf && dev->intr_urb) {
        usb_fill_int_urb(dev->intr_urb, usb_dev,
                         usb_rcvintpipe(usb_dev, 4), /* ep 0x84 */
                         dev->intr_buf, RAZER_BLACKSHARK_REPORT_LEN,
                         razer_blackshark_v3_intr_callback, dev, 1);
        dev->intr_urb->transfer_dma = dev->intr_dma;
        dev->intr_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
        usb_submit_urb(dev->intr_urb, GFP_KERNEL);
    }
}

static int razer_kraken_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int retval = 0;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_kraken_device *dev = NULL;

    dev = kzalloc_obj(*dev);
    if(dev == NULL) {
        hid_err(hdev, "out of memory\n");
        return -ENOMEM;
    }

    // Init data
    razer_kraken_init(dev, intf, hdev);

    /* Arm the V3/V3 Pro response completion before any send_cmd can fire.
     * Initialise unconditionally — older Kraken devices won't take this
     * path (they use razer_kraken_send_control_msg, not the V3 helpers). */
    init_completion(&dev->vendor_response);
    dev->vendor_response_inited = true;

    if(dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE) {
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);                               // Get driver version
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_test);                                  // Test mode
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);                           // Get string of device type
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);                         // Get string of device serial
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_firmware_version);                      // Get string of device fw version
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_mode);                           // Get device mode

        switch(dev->usb_pid) {
        case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC:
        case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC_ALT:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_current_effect);         // Get current effect
            break;
        case USB_DEVICE_ID_RAZER_KRAKEN:
        case USB_DEVICE_ID_RAZER_KRAKEN_V2:
        case USB_DEVICE_ID_RAZER_KRAKEN_TE:
        case USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE:
        case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_current_effect);         // Get current effect
            break;
        case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_WIRED:
        case USB_DEVICE_ID_RAZER_BLACKSHARK_V3:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mic_volume);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_wireless_power_save);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_ultra_low_latency);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_headphone_eq);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_thx_spatial_audio);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_sidetone);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mic_eq);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mic_eq_preset);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mic_mute);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_eq_slot);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_audio_function_button);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_game_chat_balance);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_in_call_audio_mix);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_audio_prompts);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_level);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_status);
            break;
        case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO_WIRED:
        case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_level);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_status);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_v3pro_sidetone);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_v3pro_thx_spatial_audio);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_v3pro_anc);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_v3pro_ultra_low_latency);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_v3pro_power_save);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_v3pro_headphone_eq);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mic_eq);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mic_eq_preset);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mic_mute);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_eq_slot);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_audio_function_button);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_game_chat_balance);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_in_call_audio_mix);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_audio_prompts);
            break;
        }
    }

    dev_set_drvdata(&hdev->dev, dev);

    /* Bring up the private ep 0x84 URB BEFORE hid_hw_start so it is already on
     * the endpoint when usbhid_start's SET_IDLE activates it (the dongle
     * starts sending on ep 0x84 ~2.7ms after that SET_IDLE). */
    if (razer_blackshark_is_v3pro(dev->usb_pid))
        razer_blackshark_v3_intr_start(dev, usb_dev);

    if(hid_parse(hdev)) {
        hid_err(hdev, "parse failed\n");
        goto exit_free;
    }

    if (hid_hw_start(hdev, HID_CONNECT_DEFAULT)) {
        hid_err(hdev, "hw start failed\n");
        goto exit_free;
    }

    /* hid_hw_start's usb_set_interface killed our pre-submitted URB; resubmit
     * (kill first so its giveback has run and the resubmit doesn't race
     * -EBUSY). Keep the HID open count up, and start the RF_WAKE keep-alive so
     * the wireless telemetry channel doesn't go idle. */
    if (razer_blackshark_is_v3pro(dev->usb_pid)) {
        if (hid_hw_open(hdev))
            hid_warn(hdev, "hid_hw_open failed; ep 0x84 URB still drives telemetry\n");
        if (dev->intr_urb) {
            usb_kill_urb(dev->intr_urb);
            usb_submit_urb(dev->intr_urb, GFP_KERNEL);
        }
        schedule_delayed_work(&dev->rf_wake_work, msecs_to_jiffies(750));
    }

    /* V3 / V3 Pro: prime the telemetry cache once at probe. The handshake
     * replays Synapse's wake sequence (cls=0x02, then cls=0x2a dir=0x00/0x80)
     * that the firmware needs before it answers GETs, and which pushes back the
     * charging state; a single battery query then fills pushed_battery_pct.
     * After this the charge_level/charge_status handlers are pure cache reads
     * fed by ep 0x84 pushes — there is no per-read query to reset the wireless
     * link (the daemon's ~2s polling of that reset path power-cycled the
     * headset). Best-effort: if the channel is not up yet the queries just time
     * out and later pushes fill the cache. */
    if (razer_blackshark_is_v3pro(dev->usb_pid)) {
        razer_blackshark_v3_handshake(dev);
        mutex_lock(&dev->lock);
        razer_blackshark_v3_battery_query(dev);
        mutex_unlock(&dev->lock);
    }

    usb_disable_autosuspend(usb_dev);

    return 0;

exit_free:
    /* On a probe failure after the private URB was set up, tear it (and its
     * work) down before freeing dev — the URB callback holds a dev pointer. */
    if (razer_blackshark_is_v3pro(dev->usb_pid)) {
        cancel_delayed_work_sync(&dev->rf_wake_work);
        cancel_work_sync(&dev->intr_recover_work);
        if (dev->intr_urb) {
            usb_kill_urb(dev->intr_urb);
            usb_free_urb(dev->intr_urb);
        }
        if (dev->intr_buf)
            usb_free_coherent(usb_dev, RAZER_BLACKSHARK_REPORT_LEN,
                              dev->intr_buf, dev->intr_dma);
    }
    kfree(dev);
    return retval;
}

/**
 * Unbind function
 */
static void razer_kraken_disconnect(struct hid_device *hdev)
{
    struct razer_kraken_device *dev;

    dev = hid_get_drvdata(hdev);

    if(dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE) {
        device_remove_file(&hdev->dev, &dev_attr_version);                               // Get driver version
        device_remove_file(&hdev->dev, &dev_attr_test);                                  // Test mode
        device_remove_file(&hdev->dev, &dev_attr_device_type);                           // Get string of device type
        device_remove_file(&hdev->dev, &dev_attr_device_serial);                         // Get string of device serial
        device_remove_file(&hdev->dev, &dev_attr_firmware_version);                      // Get string of device fw version
        device_remove_file(&hdev->dev, &dev_attr_device_mode);                           // Get device mode

        switch(dev->usb_pid) {
        case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC:
        case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC_ALT:
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_current_effect);         // Get current effect
            break;

        case USB_DEVICE_ID_RAZER_KRAKEN:
        case USB_DEVICE_ID_RAZER_KRAKEN_V2:
        case USB_DEVICE_ID_RAZER_KRAKEN_TE:
        case USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE:
        case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2:
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_current_effect);         // Get current effect
            break;
        case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_WIRED:
        case USB_DEVICE_ID_RAZER_BLACKSHARK_V3:
            device_remove_file(&hdev->dev, &dev_attr_mic_volume);
            device_remove_file(&hdev->dev, &dev_attr_wireless_power_save);
            device_remove_file(&hdev->dev, &dev_attr_ultra_low_latency);
            device_remove_file(&hdev->dev, &dev_attr_headphone_eq);
            device_remove_file(&hdev->dev, &dev_attr_thx_spatial_audio);
            device_remove_file(&hdev->dev, &dev_attr_sidetone);
            device_remove_file(&hdev->dev, &dev_attr_mic_eq);
            device_remove_file(&hdev->dev, &dev_attr_mic_eq_preset);
            device_remove_file(&hdev->dev, &dev_attr_mic_mute);
            device_remove_file(&hdev->dev, &dev_attr_eq_slot);
            device_remove_file(&hdev->dev, &dev_attr_audio_function_button);
            device_remove_file(&hdev->dev, &dev_attr_game_chat_balance);
            device_remove_file(&hdev->dev, &dev_attr_in_call_audio_mix);
            device_remove_file(&hdev->dev, &dev_attr_audio_prompts);
            device_remove_file(&hdev->dev, &dev_attr_charge_level);
            device_remove_file(&hdev->dev, &dev_attr_charge_status);
            break;
        case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO_WIRED:
        case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO:
            device_remove_file(&hdev->dev, &dev_attr_charge_level);
            device_remove_file(&hdev->dev, &dev_attr_charge_status);
            device_remove_file(&hdev->dev, &dev_attr_v3pro_sidetone);
            device_remove_file(&hdev->dev, &dev_attr_v3pro_thx_spatial_audio);
            device_remove_file(&hdev->dev, &dev_attr_v3pro_anc);
            device_remove_file(&hdev->dev, &dev_attr_v3pro_ultra_low_latency);
            device_remove_file(&hdev->dev, &dev_attr_v3pro_power_save);
            device_remove_file(&hdev->dev, &dev_attr_v3pro_headphone_eq);
            device_remove_file(&hdev->dev, &dev_attr_mic_eq);
            device_remove_file(&hdev->dev, &dev_attr_mic_eq_preset);
            device_remove_file(&hdev->dev, &dev_attr_mic_mute);
            device_remove_file(&hdev->dev, &dev_attr_eq_slot);
            device_remove_file(&hdev->dev, &dev_attr_audio_function_button);
            device_remove_file(&hdev->dev, &dev_attr_game_chat_balance);
            device_remove_file(&hdev->dev, &dev_attr_in_call_audio_mix);
            device_remove_file(&hdev->dev, &dev_attr_audio_prompts);
            break;
        }
    }

    /* Tear down the private ep 0x84 URB and its keep-alive/recovery work.
     * Guard on the PID: the work items are INIT'd and rf_wake scheduled for
     * every V3 in probe (even if URB allocation later failed), so they must be
     * cancelled here before dev is freed regardless of intr_urb. */
    if (razer_blackshark_is_v3pro(dev->usb_pid)) {
        cancel_delayed_work_sync(&dev->rf_wake_work);
        cancel_work_sync(&dev->intr_recover_work);
    }
    if (dev->intr_urb) {
        usb_kill_urb(dev->intr_urb);
        usb_free_urb(dev->intr_urb);
        dev->intr_urb = NULL;
    }
    if (dev->intr_buf) {
        struct usb_interface *intf2 = to_usb_interface(hdev->dev.parent);
        struct usb_device *usb_dev2 = interface_to_usbdev(intf2);

        usb_free_coherent(usb_dev2, RAZER_BLACKSHARK_REPORT_LEN,
                          dev->intr_buf, dev->intr_dma);
        dev->intr_buf = NULL;
    }

    hid_hw_stop(hdev);
    kfree(dev);
    hid_info(hdev, "Razer Device disconnected\n");
}

static int razer_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size)
{
    struct razer_kraken_device *device = dev_get_drvdata(&hdev->dev);

    if (size == 33) { // Should be a response to a Control packet
        memcpy(&device->data[0], &data[0], size);
    } else if (size == 64 && razer_blackshark_is_v3(device->usb_pid)) {
        /* HID stack delivered a V3 report (wired, or whenever usbhid does
         * poll ep 0x84). On wireless the private ep 0x84 URB is the primary
         * source; both feed the same cache path. */
        razer_blackshark_v3_cache(device, data, size);
    } else {
        hid_warn(hdev, "razerkraken: Got raw message, length: %d\n", size);
    }

    return 0;
}

/**
 * Device ID mapping table
 */
static const struct hid_device_id razer_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC_ALT) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_V2) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_TE) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKSHARK_V3_WIRED) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKSHARK_V3) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO_WIRED) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO) },
    { 0 }
};

MODULE_DEVICE_TABLE(hid, razer_devices);

/**
 * Describes the contents of the driver
 */
static struct hid_driver razer_kraken_driver = {
    .name = "razerkraken",
    .id_table = razer_devices,
    .probe = razer_kraken_probe,
    .remove = razer_kraken_disconnect,
    .raw_event = razer_raw_event
};

module_hid_driver(razer_kraken_driver);
