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

#define USB_INTERFACE_PROTOCOL_NONE 0

/* BlackShark V3 HID command protocol (Report ID 0x02, interface 5) */
#define RAZER_BLACKSHARK_REPORT_LEN  64
#define RAZER_BLACKSHARK_IFACE        5

/* GET params (class byte at buf[10] with direction byte 0x80).
 * Synapse 4 webapp parser source defines the canonical names — see
 * project memory v3 audio enum decode 2026-05-03. */
#define BLACKSHARK_PARAM_SERIAL            0x00
#define BLACKSHARK_PARAM_SIDETONE_VOLUME   0x19  /* SIDETONE_VOLUME=25 in Synapse enum */
#define BLACKSHARK_PARAM_MIC_VOLUME        0x21  /* DEPRECATED alias — mic vol is UAC2 not Razer HID */
#define BLACKSHARK_PARAM_AUTO_POWER_OFF    0x2c  /* AUTO_POWER_OFF_STATUS=44; was misnamed POWER_SAVE */
#define BLACKSHARK_PARAM_ULTRA_LOW_LATENCY 0x5f
#define BLACKSHARK_PARAM_THX               0x9e
#define BLACKSHARK_PARAM_EQ                0x15
/* Back-compat alias — older code used POWER_SAVE for what's actually AUTO_POWER_OFF. */
#define BLACKSHARK_PARAM_POWER_SAVE        BLACKSHARK_PARAM_AUTO_POWER_OFF

/* SET commands (verified from pcap captures) */
#define BLACKSHARK_SET_EQ                  0x95  /* Headphone EQ data — buf[14..23]=10 bands, buf[13]=profile_idx */
#define BLACKSHARK_SET_MIC_EQ_PRESET       0x96  /* Mic EQ preset — buf[13]: 0x20=Default 0x21=Esports 0x22=Broadcast 0x23=MicBoost */
#define BLACKSHARK_SET_MIC_EQ_DATA         0x97  /* Mic EQ band data — buf[13..22]=10 bands sign-magnitude */
#define BLACKSHARK_SET_SIDETONE_INIT       0x98  /* Sidetone enable — buf[13]=0x01 */
#define BLACKSHARK_SET_SIDETONE_LEVEL      0x99  /* Sidetone level — buf[13]=0x00..0x0f (0..15) */
#define BLACKSHARK_SET_POWER_SAVE          0xac  /* Wireless inactivity timeout — buf[13]=minutes (0/15/30/45/60) */
#define BLACKSHARK_SET_GAME_CHAT_BALANCE   0xdc  /* Game/chat balance — buf[13]: 0..20, 0=full game, 10=center, 20=full chat */
#define BLACKSHARK_SET_IN_CALL_AUDIO_MIX   0xdd  /* Behaviour when BT call interrupts 2.4 GHz — buf[13]: 0=combine, 1=lower, 2=mute */
#define BLACKSHARK_SET_ULTRA_LOW_LATENCY   0xdf  /* Ultra-Low Latency toggle — buf[13]: 0/1 */
#define BLACKSHARK_SET_EQ_APPLY            0xe0  /* Headphone EQ apply — profile-specific */
#define BLACKSHARK_SET_EQ_BEGIN            0xe1  /* Headphone EQ begin/end — buf[13]=0x01 begin, 0x02 end */
#define BLACKSHARK_SET_AUDIO_PROMPTS       0xe5  /* Voice-prompts toggle — buf[13]=0x00, buf[14]=0/1 (count=2) */
#define BLACKSHARK_SET_EQ_COMMIT           0xeb  /* Headphone EQ commit */
#define BLACKSHARK_SET_FN_BUTTON           0xea  /* Audio FN button mode — buf[13]: 0x00=GameChat (default), 0x01=Sidetone, 0x02=Footsteps, 0x03=BluetoothVolume. All four verified on-device 2026-05-03 after widening write_audio_function_button clamp from 1..2 to 0..255. */
#define BLACKSHARK_SET_MIC_EQ_BEGIN        0x16  /* Mic EQ begin marker */
#define BLACKSHARK_SET_MIC_EQ_END          0x17  /* Mic EQ end marker */

/* DEPRECATED — mic volume is UAC2 standard (Report 0x44 Feature, Interface 0),
 * handled by ALSA/PipeWire — not a Razer HID command. */
#define BLACKSHARK_SET_MIC_VOLUME          0xa1  /* DEPRECATED: mic vol is UAC2 not Razer HID */

// #define RAZER_KRAKEN_V2_REPORT_LEN ?

struct razer_kraken_device {
    struct usb_device *usb_dev;
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

    /* Last-written cache for V3 attrs. -1 means "no SET this session" so a
     * userspace reader can fall back to its own cache. The kernel's GET
     * doesn't always get a reply within the 150 ms post-SET wait, so we also
     * fall back to these on read failure rather than returning a hardcoded
     * default that masks "we don't know". */
    s8 cached_v3_power_save;
    s8 cached_v3_ull;
    s8 cached_v3_thx;
    s8 cached_v3_sidetone;
    s8 cached_v3_mic_eq_preset;
    s8 cached_v3_fn_button;
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

struct razer_kraken_response_report {
    unsigned char report_id;
    unsigned char arguments[36];
};

#endif
