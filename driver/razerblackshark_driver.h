/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024 Openrazer contributors
 */

#ifndef __HID_RAZER_BLACKSHARK_H
#define __HID_RAZER_BLACKSHARK_H

#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/workqueue.h>

#define USB_DEVICE_ID_RAZER_BLACKSHARK_V2_PRO_2_4 0x0555

/*
 * The BlackShark V2 Pro exposes its vendor control interface as a HID
 * interface with no boot protocol (bInterfaceProtocol == 0).
 */
#define USB_INTERFACE_PROTOCOL_NONE 0

/*
 * These headsets do not speak the classic 90-byte Razer report protocol. They
 * use 64-byte HID reports with report id 0x02 (1 id byte + 63 payload) carrying
 * a vendor command protocol: commands go out via the interrupt OUT endpoint and
 * replies arrive as input reports on the interrupt IN endpoint (handled in
 * .raw_event).
 *
 * Protocol reverse-engineered from Ashesh3/razer-device-control and a hidraw
 * capture of this exact device (1532:0555).
 */
#define BLACKSHARK_REPORT_LEN 64
#define BLACKSHARK_REPORT_ID  0x02

/*
 * Command sub-frame, common to every BlackShark model:
 *
 *   [10] = command id
 *   [11] = flag byte (0 in a request, 0x01 = ACK in a reply)
 *   [12] = payload length
 *   [13..] = payload
 *
 * Only the surrounding envelope (bytes [0..9], and on some models a trailer)
 * differs between models; see struct blackshark_model.
 */
#define BS_OFF_CMD_ID    10
#define BS_OFF_FLAG      11
#define BS_OFF_DATA_LEN  12
#define BS_OFF_DATA      13

/*
 * V2 Pro 2.4 ("PA") envelope:
 *   [1]=0x80 direction  [2]=total_len  [5]='P'  [6]='A'  [7]=inner_len
 *   [9]=cmd_type
 * total_len is 8 + payload length; inner_len is 0x08 for the standard
 * sub-frame and 0x0E for the remote-mode frame.
 */
#define BS_OFF_DIR       1
#define BS_OFF_TOTAL_LEN 2
#define BS_OFF_MARK_P    5
#define BS_OFF_MARK_A    6
#define BS_OFF_INNER_LEN 7
#define BS_OFF_CMD_TYPE  9

#define BS_DIR_OUT       0x80
#define BS_MARK_P        0x50
#define BS_MARK_A        0x41
#define BS_TOTAL_LEN_BASE 8
#define BS_INNER_LEN_STD  0x08
#define BS_INNER_LEN_REMOTE 0x0E

/* Command type / ids */
#define BS_TYPE_REMOTE   0x02
#define BS_CMD_REMOTE    0xE1  /* set_remote_mode; param 1=on, 0=off */
#define BS_TYPE_QUERY    0x03
#define BS_CMD_SERIAL    0x00  /* get serial number (ASCII, 15 bytes) */
#define BS_CMD_FW_VER    0x02  /* get firmware version ([0]=major, [1]=minor) */
#define BS_CMD_HW_MODEL  0x03  /* get the headset's own product id, high byte
                                * first. Over a 2.4GHz dongle this identifies
                                * the paired headset, so it differs from the
                                * dongle's USB product id (0x0555 -> 0x0556) */
#define BS_CMD_MIC_MUTE  0x55  /* get mic mute state (hardware button, 1=muted) */
#define BS_CMD_BATTERY   0x21  /* get battery level (0-100) */
#define BS_CMD_CHARGING  0x2a  /* get charging state: 0=on battery, nonzero=on cable */
#define BS_CMD_PREP      0x1e  /* query sent before audio writes (type 0x03) */

#define BS_CMD_IDLE_GET  0x2c  /* get auto power-off timeout in minutes (type 0x03) */
#define BS_CMD_PRESET_GET 0x13 /* get current EQ preset; also sent unsolicited
                                * by the device when the headset's EQ button
                                * cycles presets */

/*
 * Getter ids follow a fixed rule: get_id = set_id - 0x80 (preset 0x93->0x13,
 * power-off 0xac->0x2c, and the three below, all verified on hardware).
 */
#define BS_CMD_EQ_GET       0x15 /* get stored custom EQ bands (10 bytes)   */
#define BS_CMD_SIDETONE_GET 0x18 /* get sidetone (mic monitoring) state     */
#define BS_CMD_SIDETONE_LVL_GET 0x19 /* get sidetone level                  */
#define BS_CMD_DND_GET      0x27 /* get do-not-disturb state                */

/*
 * Audio / config writes (type 0x04 / 0x0d). Decoded from a Razer Synapse USB
 * capture of this exact device. Each write is wrapped by set_remote(on) and
 * (for the equalizer) a 0x1e prep query, mirroring Synapse's apply sequence.
 */
#define BS_TYPE_AUDIO    0x04
#define BS_CMD_PRESET    0x93  /* EQ preset selector (see BS_PRESET_*) */
#define BS_CMD_SIDETONE  0x98  /* sidetone (mic monitoring) on/off */
#define BS_CMD_SIDETONE_LVL 0x99 /* sidetone level (observed 0-10 scale) */
#define BS_CMD_ENHANCE   0x9d  /* "audio enhancement" flag; part of Synapse's
                                * captured EQ apply sequence but the device
                                * neither stores nor acts on it (Synapse-
                                * software-only), so it is not exposed */
#define BS_CMD_DND       0xa7  /* "Do Not Disturb" toggle; stored on-device,
                                * read back via 0x27 */
#define BS_CMD_IDLE_SET  0xac  /* set auto power-off timeout in minutes; 0=off */
#define BS_TYPE_EQ       0x0d
#define BS_CMD_EQ        0x95  /* set 10-band equalizer (signed dB per band) */

#define BS_EQ_BANDS       10   /* 31Hz 63 125 250 500 1k 2k 4k 8k 16k */

/*
 * Registers this model answers but that are deliberately not exposed:
 *
 *   0x20  wireless connection status (returns 1 when the headset is linked).
 *         No openrazer convention for reporting a dongle's link state.
 *   0x1e  BS_CMD_PREP above, sent as a query inside the apply sequences. The
 *         BlackShark V2 HyperSpeed work (issue #2316) identifies this register
 *         (setter 0x9e) as the equalizer engine master switch, gating whether
 *         a stored preset or curve reaches the audio path at all.
 *
 *         It is NOT that on this model. Measured here: it rests at 1, the 0x9e
 *         setter works and the value sticks in both directions - but with it
 *         cleared, preset switches stay audible, a custom curve of all -9 dB
 *         stays just as quiet, and neither the active preset nor the curve
 *         returned by 0x15 changes. Tested from both preset families (classic
 *         0x08 and game-EQ 0xfa), so it is not a family selector either.
 *
 *         So it is left read-only-in-passing: the apply sequences send it
 *         because Synapse does, and nothing here depends on its value. Not
 *         exposed, because nothing observable responds to it on this model.
 *   0x66  status-indicator LED mode (returns 1 = connection status). Every
 *         write to the matching setter 0xe6 is acknowledged - all four modes,
 *         all three command types, with and without the 0x1e frame - yet the
 *         mode never changes, so no write has been made to take effect from
 *         here and a read-only attribute alone would be useless.
 *
 *         The BlackShark V2 HyperSpeed work (issue #2316) reports this
 *         register as living on the receiver rather than the headset, reached
 *         with a distinct command class, and that the same opcode relayed to
 *         the headset lands on an unrelated register which merely echoes what
 *         was written. That would explain the ACK-without-effect seen here;
 *         the 'PA' envelope has no equivalent class byte, so how to address
 *         the receiver on this model is unresolved.
 *
 *         Either way: an ACK from this device does not on its own mean the
 *         setting took.
 */

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

/*
 * Sidetone level: Synapse sends 0-10 on the wire for the V2 Pro. Other models
 * use a different range, so the limit lives in struct blackshark_model and
 * this is only that model's entry.
 */
#define BS_V2PRO_SIDETONE_MAX 10

/* Sidetone level the V2 Pro rests at in Synapse; used until the device is read. */
#define BS_V2PRO_SIDETONE_DEFAULT 7

#define BLACKSHARK_RESPONSE_TIMEOUT_MS 1000

/*
 * Per-model descriptor.
 *
 * Every BlackShark shares the command ids and the command sub-frame above; the
 * models differ only in the envelope they wrap it in, in whether writes need
 * the remote-mode handshake, in a few hardware ranges, and in which attributes
 * they expose. Keeping all of that behind one descriptor means a new model is
 * a table entry rather than a second code path (or a second driver).
 */
struct blackshark_model {
    unsigned short usb_pid;

    /* Serial reported until the real one is read off the device. */
    const char *serial_fallback;

    /* --- framing ------------------------------------------------------ */

    /*
     * Write the model-specific envelope bytes into a zeroed frame.
     * @cmd_type is the V2 Pro's type selector; models without the concept
     * ignore it. @data_len is the payload length ([12]).
     */
    void (*write_header)(u8 *buf, u8 cmd_type, u8 data_len);

    /* Optional trailer once the frame is complete (e.g. a checksum). */
    void (*finalize)(u8 *buf);

    /*
     * Offset of the echoed command id in a reply. The reply repeats the
     * request sub-frame layout from there: [+1] ack, [+2] length,
     * [+3..] payload. Verified on 1532:0555, where a reply reads
     * [12]=cmd [13]=0x01 ack [14]=len [15..]=payload.
     */
    u8 reply_cmd_off;

    /* Writes must be wrapped in remote-mode (0xE1) handshakes. */
    bool needs_remote_mode;

    /* --- hardware ranges ---------------------------------------------- */

    /* Highest sidetone level this model accepts on the wire. */
    u8 sidetone_max;

    /* Sidetone level assumed until the device has been read. */
    u8 sidetone_default;

    /*
     * Bias applied to each band on an equalizer write: the driver sends
     * gain + eq_write_offset and the device stores the plain gain, so reads
     * need no correction. 0 on models that round-trip exactly.
     */
    s8 eq_write_offset;
};

#define BS_REPLY_ACK_OFF(m)  ((m)->reply_cmd_off + 1)
#define BS_REPLY_LEN_OFF(m)  ((m)->reply_cmd_off + 2)
#define BS_REPLY_DATA_OFF(m) ((m)->reply_cmd_off + 3)

/* Longest reply payload we keep: the serial is 15 bytes, the firmware blob 24. */
#define BS_RESP_BUF_LEN 32

struct razer_blackshark_device {
    struct hid_device *hdev;

    unsigned char usb_interface_protocol;
    unsigned short usb_pid;

    const struct blackshark_model *model;

    /*
     * Identity read back from the device (cmds 0x00 / 0x02). Both are fetched
     * once, on the first successful read, and latched by @ids_read: a query
     * costs two remote-mode handshakes, and neither value changes at runtime.
     *
     * @ids_lock serialises the fetch-and-publish: without it two concurrent
     * sysfs readers can both pass the @ids_read check, and one can observe
     * @serial spliced between the probe-time fallback and a shorter real
     * serial. Always taken before req_lock, never the other way round.
     */
    struct mutex ids_lock;

    /*
     * Fetches the identity in the background just after probe. Probe itself
     * must not do it: with the headset asleep both queries time out, which
     * blocked the hotplug path for ~2.3s and still produced only the
     * placeholder. Cancelled before the device is torn down.
     */
    struct delayed_work ids_work;

    char serial[64];
    u8 fw_major;
    u8 fw_minor;
    bool ids_read;

    /* request/response synchronisation */
    struct mutex req_lock;        /* serialises requests */
    struct completion resp_done;  /* signalled by raw_event on matching reply */
    u8 cmd_buf[BLACKSHARK_REPORT_LEN]; /* DMA-safe (lives in heap struct) */
    /*
     * A reply is outstanding. This has to be a separate flag rather than
     * "expected_cmd != 0": the serial number query is command 0x00, so a
     * zero command id is a legitimate request, not a sentinel.
     */
    bool resp_pending;
    u8 expected_cmd;              /* cmd id raw_event should match */
    u8 resp_buf[BS_RESP_BUF_LEN]; /* reply payload */
    u8 resp_len;                  /* payload length the device reported */

    /* Cached audio config: last written / last read values, used as
     * fallbacks when the headset is unreachable. */
    s8 eq_bands[BS_EQ_BANDS];     /* per-band gain in dB (signed) */
    u8 eq_preset;                 /* BS_PRESET_*: active EQ preset */
    u8 idle_minutes;              /* auto power-off timeout, 0 = off */
    u8 sidetone;                  /* sidetone level, 0 = off */
    u8 dnd;                       /* do-not-disturb: 0/1 */
};

#endif
