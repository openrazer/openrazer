/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2026 Memoraike
 */

#ifndef __HID_RAZER_KRAKEN_V2PRO_H
#define __HID_RAZER_KRAKEN_V2PRO_H

#define USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2_PRO 0x0554

// Not provided by razercommon.h; the other Kraken header defines its own copy
#define USB_INTERFACE_PROTOCOL_NONE 0

/*
 * The Kraken Kitty V2 Pro does not use the memory-mapped protocol of the
 * other Kraken headsets. It takes 15-byte HID reports of the form
 *
 *   [0x40][command][subcommand][data x12]
 *
 * sent as SET_REPORT on interface 3. The device answers no reads at all:
 * GET_REPORT for report id 0x40 stalls for every report type and length,
 * and report id 0x00 returns zero bytes. Razer's own software never reads
 * the state back either, so brightness and the current effect are cached
 * here.
 *
 * Zones, in both frame order and mask-bit order:
 *   bit 0 left cat ear, bit 1 right cat ear,
 *   bit 2 left ear cup, bit 3 right ear cup.
 *
 * A colour frame is discarded unless the device is in direct mode, and pylib
 * writes the frame before switching, so matrix_custom_frame switches by
 * itself. The mode command also discards a frame that arrived immediately
 * before it, so it is only ever sent when the mode actually has to change.
 */
#define KRAKEN_V2_PRO_REPORT_ID 0x40
#define KRAKEN_V2_PRO_REPORT_LEN 15
#define KRAKEN_V2_PRO_USB_VALUE 0x0240
#define KRAKEN_V2_PRO_USB_INDEX 0x0003

// Commands
#define KRAKEN_V2_PRO_CMD_MODE 0x01
#define KRAKEN_V2_PRO_CMD_BRIGHTNESS 0x02
#define KRAKEN_V2_PRO_CMD_FRAME 0x03

// Subcommands of KRAKEN_V2_PRO_CMD_MODE
#define KRAKEN_V2_PRO_SUB_MODE 0x00
#define KRAKEN_V2_PRO_SUB_EFFECT 0x01

// Subcommand of KRAKEN_V2_PRO_CMD_BRIGHTNESS
#define KRAKEN_V2_PRO_SUB_BRIGHTNESS 0x01

/*
 * Hardware effects, as sent with KRAKEN_V2_PRO_SUB_EFFECT. "none" is static
 * with a colour count of zero; 0x00 is never sent, it is only the cached
 * value matrix_current_effect reports for that state.
 */
#define KRAKEN_V2_PRO_EFFECT_NONE 0x00
#define KRAKEN_V2_PRO_EFFECT_STATIC 0x01
#define KRAKEN_V2_PRO_EFFECT_BREATHING 0x02
#define KRAKEN_V2_PRO_EFFECT_SPECTRUM 0x03

/*
 * Modes, as sent with KRAKEN_V2_PRO_SUB_MODE. Spectrum is reachable through
 * either register; the effect register is used so that all four effects go
 * through one place.
 */
#define KRAKEN_V2_PRO_MODE_SPECTRUM 0x03
#define KRAKEN_V2_PRO_MODE_DIRECT 0x08

#define KRAKEN_V2_PRO_ZONES 4
#define KRAKEN_V2_PRO_FRAME_LEN (KRAKEN_V2_PRO_ZONES * 3)

// Zone mask. Bit N of the mask and triple N of the colour frame are the same
// physical zone
#define KRAKEN_V2_PRO_ZONE_LEFT_EAR 0x01
#define KRAKEN_V2_PRO_ZONE_RIGHT_EAR 0x02
#define KRAKEN_V2_PRO_ZONE_LEFT_CUP 0x04
#define KRAKEN_V2_PRO_ZONE_RIGHT_CUP 0x08
#define KRAKEN_V2_PRO_ZONE_MASK_ALL 0x0f

struct razer_krakenv2pro_device {
    struct hid_device *hdev;
    struct mutex lock;
    unsigned short usb_pid;
    unsigned short usb_vid;

    char serial[23];

    // The device cannot report either of these back, so they are cached
    u8 brightness;
    u8 effect;
    // Tracks whether direct mode is currently in force: a colour frame is
    // discarded unless it is, and pylib writes the frame first
    bool direct;

    // Shadow of the four zones: the wire format has no partial frame
    u8 frame[KRAKEN_V2_PRO_FRAME_LEN];
};

/*
 * Report layout
 *
 *   40 02 01 0f XX 00 ...                brightness XX
 *   40 03 00 R G B R G B R G B R G B     one colour per zone
 */
struct razer_krakenv2pro_report {
    unsigned char report_id;
    unsigned char command;
    unsigned char subcommand;
    unsigned char data[12];
};

#endif
