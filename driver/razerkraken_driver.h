/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2015 Terri Cain <terri@dolphincorp.co.uk>
 */

#ifndef __HID_RAZER_KRAKEN_H
#define __HID_RAZER_KRAKEN_H

#define USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC 0x0501
#define USB_DEVICE_ID_RAZER_KRAKEN 0x0504 // Codename Rainie
#define USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC_ALT 0x0506
#define USB_DEVICE_ID_RAZER_KRAKEN_V2 0x0510 // Codename Kylie
#define USB_DEVICE_ID_RAZER_KRAKEN_TE 0x0520
#define USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE 0x0527
#define USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2 0x0560
#define USB_DEVICE_ID_RAZER_BLACKSHARK_V3_WIRED 0x0579
#define USB_DEVICE_ID_RAZER_BLACKSHARK_V3 0x057A
#define USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO_WIRED 0x0576
#define USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO 0x0577

#define USB_INTERFACE_PROTOCOL_NONE 0

/* BlackShark V3 HID command protocol (Report ID 0x02, interface 5) */
#define RAZER_BLACKSHARK_REPORT_LEN  64
#define RAZER_BLACKSHARK_IFACE        5

/* GET params (class byte at buf[10]; buf[9] picks 0x80 wireless / 0x00 wired,
 * not request type, see the envelope notes below). Getter and setter are
 * separate classes related by get = set - 0x80, so a GET is just its own class
 * with no args. Verified from Synapse responses in the 2026-05-02 V3 wired +
 * 2.4GHz pcaps, and re-swept live on a V3 dongle 2026-07-22. */
#define BLACKSHARK_PARAM_SERIAL            0x00  /* GET; cnt=15 ASCII serial */
#define BLACKSHARK_PARAM_INIT              0x02  /* GET; capability handshake — Synapse sends FIRST before any other GET */
#define BLACKSHARK_PARAM_EQ_BANDS          0x15  /* GET; cnt=11 [slot, b0..b9] sign-magnitude — paired SET 0x95 */
#define BLACKSHARK_PARAM_MIC_EQ_PRESET     0x16  /* GET; cnt=1 [slot] 0x20=Default..0x23=MicBoost (was misnamed BLACKSHARK_SET_MIC_EQ_BEGIN) */
#define BLACKSHARK_PARAM_MIC_EQ_BANDS      0x17  /* GET; cnt=10 mic EQ bands sign-magnitude (was misnamed BLACKSHARK_SET_MIC_EQ_END) */
#define BLACKSHARK_PARAM_BATTERY_LEVEL     0x21  /* GET; cnt=1 [percent 0..100] */
#define BLACKSHARK_PARAM_MIC_VOLUME        0x21  /* DEPRECATED alias for BATTERY_LEVEL — earlier wrong guess; mic volume is UAC2 not Razer HID */
#define BLACKSHARK_PARAM_SIDETONE_VOLUME   0x19  /* GET; cnt=1 [level 0..15]. Synapse 4 enum: SIDETONE_VOLUME=25 (0x19). */
#define BLACKSHARK_PARAM_CHARGE_STATE      0x2a  /* GET; cnt=1 [charging? 0/1] */
#define BLACKSHARK_PARAM_AUTO_POWER_OFF    0x2c  /* GET; cnt=1 [minutes]. Synapse 4 enum: AUTO_POWER_OFF_STATUS=44 (0x2c). Previously misnamed SIDETONE_LEVEL; both reply 0..15 so the bug was masked until 2026-05-03 Synapse webapp source decode. */
#define BLACKSHARK_PARAM_MIC_STATUS        0x55  /* GET/push; cnt=1 [1=muted, 0=live]. Synapse MIC_STATUS=85. */
#define BLACKSHARK_PARAM_GAME_CHAT_BAL_PRO 0x5c  /* GET; cnt=1 [balance]. V3 Pro variant (Synapse Eu enum: 92). Pair: SET 0xdc. */
#define BLACKSHARK_PARAM_IN_CALL_AUDIO_MIX 0x5d  /* GET; cnt=1 [mode]. Pair: SET 0xdd. */
#define BLACKSHARK_PARAM_ULTRA_LOW_LATENCY 0x5f  /* GET; cnt=1 [on/off]. Confirmed via readback. */
#define BLACKSHARK_PARAM_EQ_SLOT_META      0x60  /* GET; cnt=6 [slot, ?, enabled?, 00, ?, 00] */
#define BLACKSHARK_PARAM_EQ_PRESET         0x13  /* GET; cnt=1 [slot]. Pair: SET 0x93 (get = set - 0x80).
                                                  * Verified on a V3 dongle 2026-07-22: returns the same
                                                  * slot as the cnt=6 0x60 reply, in one byte. */
#define BLACKSHARK_PARAM_GAME_CHAT_BAL_V3  0x65  /* GET; cnt=1 [balance]. V3 wireless variant (Synapse Eu enum: 229=0xe5). Pair: SET 0xe5 sub=0x01. */
#define BLACKSHARK_PARAM_AUDIO_PROMPTS_GET 0x66  /* GET; cnt=1 [on/off]. Pair: SET 0xe5 sub=0x02 (cls byte multiplexes by sub). */
#define BLACKSHARK_PARAM_AUDIO_FN_GET      0x6a  /* GET; cnt=1 [mode 0..3]. Pair: SET 0xea. */
#define BLACKSHARK_PARAM_THX               0x9e  /* GET; cnt=1 [0=stereo, 1=THX-spatial] */
#define BLACKSHARK_PARAM_AUDIO_PROMPTS     0xe5  /* GET (V3 wireless variant — note 0xe5 cls is multiplexed: sub=0x01 = GAME_CHAT_BALANCE, sub=0x02 = AUDIO_PROMPTS). */
#define BLACKSHARK_PARAM_AUDIO_FN_BUTTON   0xea  /* GET; cnt=1 [mode 0..3] */
#define BLACKSHARK_PARAM_EQ                BLACKSHARK_PARAM_EQ_BANDS  /* legacy alias */

/* SET commands (verified from pcap captures) */
#define BLACKSHARK_SET_EQ                  0x95  /* Headphone EQ data — buf[14..23]=10 bands, buf[13]=profile_idx */
#define BLACKSHARK_SET_MIC_EQ_PRESET       0x96  /* Mic EQ preset — buf[13]: 0x20=Default 0x21=Esports 0x22=Broadcast 0x23=MicBoost */
#define BLACKSHARK_SET_MIC_EQ_DATA         0x97  /* Mic EQ band data — buf[13..22]=10 bands sign-magnitude */
#define BLACKSHARK_SET_SIDETONE_INIT       0x98  /* Sidetone enable — buf[13]=0x01 */
#define BLACKSHARK_SET_SIDETONE_LEVEL      0x99  /* Sidetone level — buf[13]=0x00..0x0f (0..15) */
#define BLACKSHARK_SET_EQ_APPLY            0xe0  /* Headphone EQ apply — profile-specific */
#define BLACKSHARK_SET_EQ_BEGIN            0xe1  /* Headphone EQ begin/end — buf[13]=0x01 begin, 0x02 end */
#define BLACKSHARK_SET_EQ_COMMIT           0xeb  /* Headphone EQ commit */
#define BLACKSHARK_SET_FN_BUTTON           0xea  /* Audio FN button mode — buf[13]: 0x00=GameChat (default), 0x01=Sidetone, 0x02=Footsteps, 0x03=BluetoothVolume. All four verified on-device 2026-05-03 after widening the write_audio_function_button clamp from 1..2 to 0..255 — the earlier "0x00/0x03 alias to 0x01" reading was the driver returning -EINVAL before the bytes ever reached the device. */

/* ---- BlackShark V3 Pro (PID 0x0577) command set ----
 *
 * Decoded from Razer Synapse 4 USBPcap traces on Windows (initial seed from
 * RiskRunner0's blackshark-linux + dedicated pcaps Apr 2026). Full protocol
 * spec at docs/v3_pro_protocol.md.
 *
 * Same envelope as V3 (Report 0x02, transaction_id 0x60, CRC=XOR[0..61]).
 * V3 and V3 Pro share most commands — only ANC/Ambient is V3 Pro–exclusive.
 *
 * Layout:
 *   buf[6]   = data_size (4 + args_len: direction + class + sub + argc + args)
 *   buf[9]   = transport (0x80 wireless, 0x00 wired), NOT a request type
 *   buf[10]  = command class
 *   buf[11]  = subclass in a request; in a reply 0x01 = ACK, 0x02 = unsolicited push
 *   buf[12]  = arg count (number of meaningful bytes at buf[13..])
 *   buf[13..]= args
 *
 * buf[9] does not distinguish GET from SET. Both use the transport byte for
 * the link the device is on; the class alone selects the operation, with
 * getters at (setter - 0x80): 0x93/0x13 preset, 0xac/0x2c power save,
 * 0x9e/0x1e audio mode. Swept on a V3 dongle 2026-07-22: all 19 known getter
 * classes answered with buf[9]=0x80 and flag=0x01. The 0x00-then-0x80 pair the
 * handshake sends on 0x2a is a firmware wake quirk, not a direction switch.
 *
 * (The older "0x80 SET / 0x00 GET / 0x84 ACK" and "data_size = 3 + args_len"
 * readings were both wrong; the size formula was corrected in code in the ANC
 * fix, where a trailing arg byte was being dropped, but this comment lagged.)
 */
#define BLACKSHARK_V3_PRO_BATTERY_CLASS    0x21  /* GET; resp[0]=%, resp[1]=charging */
#define BLACKSHARK_V3_PRO_BATTERY_ID       0x00
#define BLACKSHARK_V3_PRO_CHARGING_CLASS   0x2a  /* GET; resp[0]=charging? */
#define BLACKSHARK_V3_PRO_CHARGING_GET     0x00
#define BLACKSHARK_V3_PRO_SIDETONE_GET_CL  0x98  /* SET precursor; args=[0x01] */
#define BLACKSHARK_V3_PRO_SIDETONE_SET_CL  0x99  /* SET level; args=[level 0..15] */
#define BLACKSHARK_V3_PRO_SIDETONE_READ_CL 0x19  /* GET; resp[0]=level. Was 0x2c (AUTO_POWER_OFF) — fixed 2026-05-03 from Synapse 4 source enum SIDETONE_VOLUME=25. */
#define BLACKSHARK_V3_PRO_SIDETONE_ID      0x01
#define BLACKSHARK_V3_PRO_SIDETONE_MAX     0x0f
#define BLACKSHARK_V3_PRO_THX_CLASS        0x9e  /* SET; args=[mode]; 0=stereo 1=THX-spatial */
#define BLACKSHARK_V3_PRO_THX_ID           0x01
#define BLACKSHARK_V3_PRO_ULL_CLASS        0xdf  /* SET; args=[on/off]; Ultra-Low Latency */
#define BLACKSHARK_V3_PRO_ULL_ID           0x01
#define BLACKSHARK_V3_PRO_ANC_CLASS        0x92  /* SET; args=[mode, level]; mode 0=off 1=ANC 0x50=ambient */
#define BLACKSHARK_V3_PRO_ANC_POLL_CLASS   0x12  /* GET/poll class; the on-board ANC button pushes state back here (data[13]=mode, data[14]=level), not the 0x92 SET class — same pattern as battery/charging (0x21/0x2a). */
#define BLACKSHARK_V3_PRO_ANC_ID           0x02
#define BLACKSHARK_V3_PRO_ANC_MODE_OFF     0x00
#define BLACKSHARK_V3_PRO_ANC_MODE_ANC     0x01
#define BLACKSHARK_V3_PRO_ANC_MODE_AMBIENT 0x50
#define BLACKSHARK_V3_PRO_ANC_LEVEL_MIN    1
#define BLACKSHARK_V3_PRO_ANC_LEVEL_MAX    4
#define BLACKSHARK_V3_PRO_POWER_SAVE_CLASS 0xac  /* SET; args=[minutes]; 0/15/30/45/60 */
#define BLACKSHARK_V3_PRO_POWER_SAVE_ID    0x01
#define BLACKSHARK_V3_PRO_GAME_CHAT_CLASS  0xdc  /* SET; args=[balance 0..20]; 0=full game, 10=center, 20=full chat */
#define BLACKSHARK_V3_PRO_GAME_CHAT_ID     0x01
#define BLACKSHARK_V3_PRO_GAME_CHAT_MAX    0x14
#define BLACKSHARK_V3_PRO_INCALL_MIX_CLASS 0xdd  /* SET; args=[mode]; 0=combine 1=lower 2=mute */
#define BLACKSHARK_V3_PRO_INCALL_MIX_ID    0x01
#define BLACKSHARK_V3_PRO_AUDIO_PROMPTS_CL 0xe5  /* SET; args=[0x00, on]; mic mute/unmute prompt */
#define BLACKSHARK_V3_PRO_AUDIO_PROMPTS_ID 0x02

/* EQ — 5-step sequence per preset switch.
 * 10 bands at 31/63/125/250/500/1k/2k/4k/8k/16k Hz; 9 preset slots (0..8).
 * Slots 0..4 are factory (Default/Game/Movie/Music/Esports), 5..8 are user
 * custom slots from Synapse's "EDIT EQ LIST" UI.
 * Sign-magnitude gain (same as V3): 0x00=0dB, 0x01=+1dB, 0x81=-1dB; range ±6dB. */
#define BLACKSHARK_V3_PRO_EQ_STATE_CLASS   0xe1  /* SET; args=[profile_idx] activates slot */
#define BLACKSHARK_V3_PRO_EQ_STATE_ID      0x01
#define BLACKSHARK_V3_PRO_EQ_BANDS_CLASS   0x95  /* SET; args=[profile_idx, b0..b9] (11 bytes) */
#define BLACKSHARK_V3_PRO_EQ_BANDS_ID      0x0b
#define BLACKSHARK_V3_PRO_EQ_META_CLASS    0xe0  /* SET; args=[idx, ...] (7 bytes) */
#define BLACKSHARK_V3_PRO_EQ_META_ID       0x06
#define BLACKSHARK_V3_PRO_EQ_COMMIT_CLASS  0xeb  /* SET; args=[idx, ...] (11 bytes) */
#define BLACKSHARK_V3_PRO_EQ_COMMIT_ID     0x0b
#define BLACKSHARK_V3_PRO_EQ_PRESET_COUNT  9

/* DEPRECATED — these were guessed and verified WRONG/unverified.
 * Mic volume is UAC2 standard (Report 0x44 Feature, Interface 0), handled by ALSA/PipeWire.
 * Power save and ULL command bytes have NOT been found yet (0x99 is sidetone, not ULL). */
#define BLACKSHARK_SET_MIC_VOLUME          0xa1  /* DEPRECATED: mic vol is UAC2 not Razer HID */
#define BLACKSHARK_SET_POWER_SAVE          0xac  /* UNVERIFIED: never seen in captures */
#define BLACKSHARK_SET_ULTRA_LOW_LATENCY   0xdf  /* UNVERIFIED: never seen in captures */

// #define RAZER_KRAKEN_V2_REPORT_LEN ?

struct razer_kraken_device {
    struct hid_device *hdev;
    struct usb_device *usb_dev; // TODO: remove usages, replace with hdev
    struct mutex lock;
    unsigned char usb_interface_protocol;
    unsigned short usb_pid;
    unsigned short usb_vid;

    // Will be set with the correct address for setting LED mode for each device
    unsigned short led_mode_address;
    unsigned short custom_address;
    unsigned short breathing_address[3];

    char serial[23];
    // 3 Bytes, first byte is whether fw version is collected, 2nd byte is major version, 3rd is minor, should be printed out in hex form as are bcd
    unsigned char firmware_version[3];

    u8 data[64];
    s8 eq_bands[10];

    /* V3 / V3 Pro vendor-response synchronization.
     * razer_blackshark_send_cmd sends a SET_REPORT via hid_hw_raw_request,
     * then waits on this completion. razer_raw_event signals it when the
     * device's interrupt-IN reply (64 bytes) arrives. Without this, the
     * kernel's old 150 ms msleep often misses the reply window. */
    struct completion vendor_response;
    bool vendor_response_inited;

    /* Cache of the most recent UNSOLICITED push value (sub=0x02) the
     * device sent on its own initiative — separate from the request-reply
     * path because send_cmd clears device->data[1] on each call. Updated
     * by raw_event whenever a push arrives. -1 = no push received yet.
     * Used as a fallback by the charge_level/charge_status readers when
     * the synchronous GET reply doesn't arrive (the V3 firmware on some
     * setups silently drops query responses but still emits state-change
     * pushes — the 96→97 battery transition in the wired capture was an
     * unsolicited push, not a reply). */
    s8 pushed_battery_pct;
    s8 pushed_charging;

    /* Last-written cache for write-only V3/V3 Pro attrs.
     * -1 = not yet written this session; read handler returns "-1" so the GUI
     * can fall back to its JSON cache. Updated on every successful SET.
     * V3 reads also fall back to these when the device GET reply doesn't
     * arrive in time (the kernel's 150 ms wait often misses the response). */
    s8 cached_v3_power_save;
    s8 cached_v3_ull;
    s8 cached_v3_thx;
    s8 cached_v3_eq_active;
    s8 cached_v3_sidetone;
    s8 cached_v3_mic_eq_preset;
    s8 cached_v3_fn_button;
    s8 cached_v3pro_thx;
    s8 cached_v3pro_anc_mode;
    s8 cached_v3pro_anc_level;
    s8 cached_v3pro_ull;
    s8 cached_v3pro_power_save;
    s8 cached_v3pro_eq_profile;
    s8 cached_v3pro_sidetone;
    s8 cached_game_chat_balance;
    s8 cached_in_call_audio_mix;
    s8 cached_audio_prompts;
    s8 cached_mic_muted;
    /* Last firmware EQ-slot readback (cls=0x15 GET): eq_query_slot is the slot
     * index the bands were read from, eq_query_bands the decoded -6..+6 gains. */
    s8 eq_query_slot;
    s8 eq_query_bands[10];

    /* Private interrupt-IN URB on ep 0x84. usbhid does not submit the
     * interrupt-IN URB on kernel 7.x (the hid_hw_open -> usbhid_open path is
     * decoupled), so we drive ep 0x84 ourselves to receive the firmware's
     * spontaneous pushes and its query replies. Without this the whole
     * request/reply and on-board-push telemetry silently stalls. */
    struct urb  *intr_urb;
    u8          *intr_buf;
    dma_addr_t   intr_dma;
    /* On -EPROTO (data-toggle mismatch after hot-plug) the callback defers
     * usb_clear_halt + resubmit to this work (can't run in atomic context). */
    struct work_struct intr_recover_work;
    atomic_t intr_eproto_count;   /* consecutive -EPROTO count, reset on success */
    /* Periodic RF_WAKE (Output Report 5, [0x05,0x00]) keep-alive. Sending it
     * ~every 3.5s is what keeps the dongle's RF telemetry channel from going
     * idle and dropping pushes over a long session. */
    struct delayed_work rf_wake_work;
    /* One-shot cache prime, deferred off probe(): frames sent from probe get
     * answered but the replies are dropped, because usbhid is not polling the
     * interrupt IN endpoint yet. Runs once, a beat after the HID stack has
     * settled. */
    struct delayed_work prime_work;
};

union razer_kraken_effect_byte {
    unsigned char value;
    struct razer_kraken_effect_byte_bits {
        unsigned char on_off_static :1;
        unsigned char single_colour_breathing :1;
        unsigned char spectrum_cycling :1;
        unsigned char sync :1;
        unsigned char two_colour_breathing :1;
        unsigned char three_colour_breathing :1;
    } bits;
};

/*
 * Should wait 15ms per write to EEPROM
 *
 * Report ID:
 *   0x04 - Output ID for memory access
 *   0x05 - Input ID for memory access result
 *
 * Destination:
 *   0x20 - Read data from EEPROM
 *   0x40 - Write data to RAM
 *   0x00 - Read data from RAM
 *
 * Address:
 *   RAM - Both
 *   0x1189 - Custom effect Colour1 Red
 *   0x118A - Custom effect Colour1 Green
 *   0x118B - Custom effect Colour1 Blue
 *   0x118C - Custom effect Colour1 Intensity
 *
 *   RAM - Kylie
 *   0x172D - Set LED Effect, see note 1
 *   0x1741 - Static/Breathing1 Colour1 Red
 *   0x1742 - Static/Breathing1 Colour1 Green
 *   0x1743 - Static/Breathing1 Colour1 Blue
 *   0x1744 - Static/Breathing1 Colour1 Intensity
 *
 *   0x1745 - Breathing2 Colour1 Red
 *   0x1746 - Breathing2 Colour1 Green
 *   0x1747 - Breathing2 Colour1 Blue
 *   0x1748 - Breathing2 Colour1 Intensity
 *   0x1749 - Breathing2 Colour2 Red
 *   0x174A - Breathing2 Colour2 Green
 *   0x174B - Breathing2 Colour2 Blue
 *   0x174C - Breathing2 Colour2 Intensity
 *
 *   0x174D - Breathing3 Colour1 Red
 *   0x174E - Breathing3 Colour1 Green
 *   0x174F - Breathing3 Colour1 Blue
 *   0x1750 - Breathing3 Colour1 Intensity
 *   0x1751 - Breathing3 Colour2 Red
 *   0x1752 - Breathing3 Colour2 Green
 *   0x1753 - Breathing3 Colour2 Blue
 *   0x1754 - Breathing3 Colour2 Intensity
 *   0x1755 - Breathing3 Colour3 Red
 *   0x1756 - Breathing3 Colour3 Green
 *   0x1757 - Breathing3 Colour3 Blue
 *   0x1758 - Breathing3 Colour3 Intensity
 *
 *   RAM - Rainie
 *   0x1008 - Set LED Effect, see note 1
 *   0x15DE - Static/Breathing1 Colour1 Red
 *   0x15DF - Static/Breathing1 Colour1 Green
 *   0x15E0 - Static/Breathing1 Colour1 Blue
 *   0x15E1 - Static/Breathing1 Colour1 Intensity
 *
 *   EEPROM
 *   0x0030 - Firmware version, 2 byted BCD
 *   0x7f00 - Serial Number - 22 Bytes
 *
 *
 * Note 1:
 *   Takes one byte which is a bitfield (0 being the rightmost byte 76543210)
 *     - Bit 0 = LED ON/OFF = 1/0 Static
 *     - Bit 1 = Single Colour Breathing ON/OFF, 1/0
 *     - Bit 2 = Spectrum Cycling
 *     - Bit 3 = Sync = 1
 *     - Bit 4 = 2 Colour breathing ON/OFF = 1/0
 *     - Bit 5 = 3 Colour breathing ON/OFF = 1/0
 *   E.g.
 *    7   6  5  4  3  2  1  0
 *    128 64 32 16 8  4  2  1
 *    =====================================================
 *    0   0  0  0  0  1  0  1 0x05 Spectrum Cycling on
 *
 * Note 2:
 *   Razer Kraken Classic uses 0x1008 for Logo LED on off.
 * */

#define KYLIE_SET_LED_ADDRESS 0x172D
#define RAINIE_SET_LED_ADDRESS 0x1008

#define KYLIE_CUSTOM_ADDRESS_START 0x1189
#define RAINIE_CUSTOM_ADDRESS_START 0x1189

#define KYLIE_BREATHING1_ADDRESS_START 0x1741
#define RAINIE_BREATHING1_ADDRESS_START 0x15DE

#define KYLIE_BREATHING2_ADDRESS_START 0x1745
#define KYLIE_BREATHING3_ADDRESS_START 0x174D

struct razer_kraken_request_report {
    unsigned char report_id;
    unsigned char destination;
    unsigned char length;
    unsigned char addr_h;
    unsigned char addr_l;
    unsigned char arguments[32];
};
static_assert(sizeof(struct razer_kraken_request_report) == 37);

struct razer_kraken_response_report {
    unsigned char report_id;
    unsigned char arguments[36];
};

#endif
