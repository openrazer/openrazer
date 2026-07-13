/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024 Openrazer contributors
 */

#ifndef __HID_RAZER_BLACKSHARK_H
#define __HID_RAZER_BLACKSHARK_H

#include <linux/mutex.h>
#include <linux/completion.h>

#define USB_DEVICE_ID_RAZER_BLACKSHARK_V2_PRO_2_4 0x0555

/*
 * The BlackShark V2 Pro exposes its vendor control interface as a HID
 * interface with no boot protocol (bInterfaceProtocol == 0).
 */
#define USB_INTERFACE_PROTOCOL_NONE 0

/*
 * The headset uses the "MXIC" vendor protocol: 64-byte HID reports with
 * report id 0x02 (1 id byte + 63 payload). Commands go out via the interrupt
 * OUT endpoint and replies arrive as input reports on the interrupt IN
 * endpoint (handled in .raw_event). Protocol reverse-engineered from
 * Ashesh3/razer-device-control and a local hidraw capture of this exact device
 * (1532:0555). See BLACKSHARK_NOTES.md for the full protocol.
 */
#define BLACKSHARK_REPORT_LEN 64
#define BLACKSHARK_REPORT_ID  0x02

/* Command builder field offsets (host -> device) */
#define BS_OFF_DIR       1  /* 0x80 = host->device */
#define BS_OFF_TOTAL_LEN 2
#define BS_OFF_MARK_P    5  /* 'P' (0x50) */
#define BS_OFF_MARK_A    6  /* 'A' (0x41) for commands */
#define BS_OFF_INNER_LEN 7
#define BS_OFF_CMD_TYPE  9
#define BS_OFF_CMD_ID    10
#define BS_OFF_PARAMS    11

#define BS_DIR_OUT       0x80
#define BS_MARK_P        0x50
#define BS_MARK_A        0x41

/* Command type / ids */
#define BS_TYPE_REMOTE   0x02
#define BS_CMD_REMOTE    0xE1  /* set_remote_mode; param 1=on, 0=off */
#define BS_TYPE_QUERY    0x03
#define BS_CMD_WIRELESS  0x20  /* get wireless connection status */
#define BS_CMD_BATTERY   0x21  /* get battery level (0-100) */
#define BS_CMD_CHARGING  0x2a  /* get charging state: 0=on battery, nonzero=on cable */
#define BS_CMD_PREP      0x1e  /* query sent before audio writes (type 0x03) */
#define BS_CMD_IDLE_GET  0x2c  /* get auto power-off timeout in minutes (type 0x03) */
#define BS_CMD_PRESET_GET 0x13 /* get current EQ preset; also sent unsolicited
                                * by the device when the headset's EQ button
                                * cycles presets */

/*
 * Getter ids follow a fixed rule: get_id = set_id - 0x80 (preset 0x93->0x13,
 * power-off 0xac->0x2c, and the four below, all verified on hardware).
 */
#define BS_CMD_EQ_GET      0x15 /* get stored custom EQ bands (10 bytes)  */
#define BS_CMD_MIC_MON_GET 0x18 /* get mic monitoring state              */
#define BS_CMD_MIC_LVL_GET 0x19 /* get mic monitoring level              */
#define BS_CMD_BT_DND_GET  0x27 /* get Bluetooth do-not-disturb state    */

/*
 * Audio / config writes (type 0x04 / 0x0d). Decoded from a Razer Synapse USB
 * capture of this exact device; see BLACKSHARK_NOTES.md. Each write is wrapped
 * by set_remote(on) and (for the equalizer) a 0x1e prep query, mirroring
 * Synapse's apply sequence. Params are [00 01 <value>] for the 0x04 commands
 * and [00 0a <b0..b9>] for the equalizer.
 */
#define BS_TYPE_AUDIO    0x04
#define BS_CMD_PRESET    0x93  /* EQ preset selector (see BS_PRESET_*) */
#define BS_CMD_MIC_MON   0x98  /* mic monitoring (sidetone) on/off */
#define BS_CMD_MIC_LVL   0x99  /* mic monitoring level (observed 0-10 scale) */
#define BS_CMD_ENHANCE   0x9d  /* "audio enhancement" flag; part of Synapse's
                                * captured EQ apply sequence but the device
                                * neither stores nor acts on it (Synapse-
                                * software-only), so it is not exposed */
#define BS_CMD_BT_DND    0xa7  /* Bluetooth "Do Not Disturb" (Synapse's toggle
                                * for this dual-mode 2.4GHz+BT headset); stored
                                * on-device, read back via 0x27 */
#define BS_CMD_IDLE_SET  0xac  /* set auto power-off timeout in minutes; 0=off */
#define BS_TYPE_EQ       0x0d
#define BS_CMD_EQ        0x95  /* set 10-band equalizer (signed dB per band) */

#define BS_EQ_BANDS       10   /* 31Hz 63 125 250 500 1k 2k 4k 8k 16k */

/*
 * EQ presets (cmd 0x93). Game/Music/Movie select curves stored on the device
 * (Synapse sends only the selector, no band data); CUSTOM activates the
 * host-supplied 10-band curve (cmd 0x95).
 */
#define BS_PRESET_GAME   0x07
#define BS_PRESET_MUSIC  0x08
#define BS_PRESET_MOVIE  0x09
#define BS_PRESET_CUSTOM 0xff

/*
 * Synapse also offers five fixed game-specific EQ profiles, numbered here in
 * the order they appear in Synapse. Like CUSTOM they are host-curve presets:
 * Synapse sends the selector plus a fixed 10-band curve (and mode flag 2 on
 * cmd 0x9d instead of 1); only the selector is stored on the device.
 */
#define BS_PRESET_GAME1  0xfa
#define BS_PRESET_GAME2  0xfe
#define BS_PRESET_GAME3  0xfb
#define BS_PRESET_GAME4  0xfd
#define BS_PRESET_GAME5  0xfc
#define BS_PRESET_IS_GAME_EQ(p) ((p) >= 0xfa && (p) <= 0xfe)

/* Auto power-off timeout: device offers 15-60 minutes (Synapse range); 0=off. */
#define BS_IDLE_MIN_MINUTES 15
#define BS_IDLE_MAX_MINUTES 60

/* Mic monitoring level: Synapse sends 0-10 on the wire. */
#define BS_MIC_LEVEL_MAX 10

/* In a reply (device->host) the echoed cmd id sits at offset 12 and its value
 * at offset 15 (confirmed for the battery 0x21 and charging 0x2a replies on
 * 1532:0555). The continuous 'PI' telemetry frames carry other ids here, so an
 * exact-offset match avoids false positives. */
#define BS_REPLY_CMD_OFF   12
#define BS_REPLY_VAL_OFF   15

#define BLACKSHARK_RESPONSE_TIMEOUT_MS 1000

struct razer_blackshark_device {
    struct hid_device *hdev;

    unsigned char usb_interface_protocol;
    unsigned short usb_vid;
    unsigned short usb_pid;

    char serial[64];

    /* MXIC request/response synchronisation */
    struct mutex req_lock;        /* serialises requests */
    struct completion resp_done;  /* signalled by raw_event on matching reply */
    u8 cmd_buf[BLACKSHARK_REPORT_LEN]; /* DMA-safe (lives in heap struct) */
    u8 expected_cmd;              /* cmd id raw_event should match (0 = none) */
    u8 resp_buf[BS_EQ_BANDS];     /* reply payload (1 byte for scalar getters,
                                   * 10 for the EQ bands getter) */

    /* Cached audio config: last written / last read values, used as
     * fallbacks when the headset is unreachable. */
    s8 eq_bands[BS_EQ_BANDS];     /* per-band gain in dB (signed) */
    u8 eq_preset;                 /* BS_PRESET_*: active EQ preset */
    u8 idle_minutes;              /* auto power-off timeout, 0 = off */
    u8 mic_monitoring;            /* mic monitoring (sidetone): 0/1 */
    u8 mic_level;                 /* mic monitoring level */
    u8 bt_dnd;                    /* Bluetooth do-not-disturb: 0/1 */
};

#endif
