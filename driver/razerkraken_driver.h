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
#define USB_DEVICE_ID_RAZER_BLACKSHARK_V3_PRO 0x0577

#define USB_INTERFACE_PROTOCOL_NONE 0

/* BlackShark V3 HID command protocol (Report ID 0x02, interface 5) */
#define RAZER_BLACKSHARK_REPORT_LEN  64
#define RAZER_BLACKSHARK_IFACE        5

/* GET params (arg[1] with direction byte 0x80) */
#define BLACKSHARK_PARAM_SERIAL            0x00
#define BLACKSHARK_PARAM_MIC_VOLUME        0x21
#define BLACKSHARK_PARAM_POWER_SAVE        0x2c
#define BLACKSHARK_PARAM_ULTRA_LOW_LATENCY 0x5f
#define BLACKSHARK_PARAM_THX               0x9e
#define BLACKSHARK_PARAM_EQ                0x15

/* SET commands (verified from pcap captures) */
#define BLACKSHARK_SET_EQ                  0x95  /* Headphone EQ data — buf[14..23]=10 bands, buf[13]=profile_idx */
#define BLACKSHARK_SET_MIC_EQ_PRESET       0x96  /* Mic EQ preset — buf[13]: 0x20=Default 0x21=Esports 0x22=Broadcast 0x23=MicBoost */
#define BLACKSHARK_SET_MIC_EQ_DATA         0x97  /* Mic EQ band data — buf[13..22]=10 bands sign-magnitude */
#define BLACKSHARK_SET_SIDETONE_INIT       0x98  /* Sidetone enable — buf[13]=0x01 */
#define BLACKSHARK_SET_SIDETONE_LEVEL      0x99  /* Sidetone level — buf[13]=0x00..0x0f (0..15) */
#define BLACKSHARK_SET_EQ_APPLY            0xe0  /* Headphone EQ apply — profile-specific */
#define BLACKSHARK_SET_EQ_BEGIN            0xe1  /* Headphone EQ begin/end — buf[13]=0x01 begin, 0x02 end */
#define BLACKSHARK_SET_EQ_COMMIT           0xeb  /* Headphone EQ commit */
#define BLACKSHARK_SET_FN_BUTTON           0xea  /* Audio function button mode — buf[13]: 0x01=sidetone save, 0x02=footsteps */
#define BLACKSHARK_SET_MIC_EQ_BEGIN        0x16  /* Mic EQ begin marker */
#define BLACKSHARK_SET_MIC_EQ_END          0x17  /* Mic EQ end marker */

/* ---- BlackShark V3 Pro (PID 0x0577) command set ----
 *
 * Protocol decoded by RiskRunner0 (https://github.com/RiskRunner0/blackshark-linux,
 * GPL-2.0-or-later) from Synapse usbmon captures on Windows. Verified against
 * a startup pcap from a V3 Pro user on the openrazer fork PR.
 *
 * Same envelope as V3 (Report 0x02, transaction_id 0x60, CRC=XOR[0..61]),
 * but with a substantially different command vocabulary.
 *
 * Layout:
 *   buf[6]   = data_size (3 + args_len)
 *   buf[9]   = flags (0x80 = SET/GET, 0x00 = init handshake)
 *   buf[10]  = command class
 *   buf[11]  = sub (always 0x00)
 *   buf[12]  = command id
 *   buf[13..]= args
 */
#define BLACKSHARK_V3_PRO_BATTERY_CLASS    0x21  /* args=[0x00]; resp[0]=%, resp[1]=charging */
#define BLACKSHARK_V3_PRO_BATTERY_ID       0x00
#define BLACKSHARK_V3_PRO_CHARGING_CLASS   0x2a  /* args=[0x00] GET state */
#define BLACKSHARK_V3_PRO_CHARGING_GET     0x00
#define BLACKSHARK_V3_PRO_SIDETONE_GET_CL  0x98  /* args=[0x01, 0x00] */
#define BLACKSHARK_V3_PRO_SIDETONE_SET_CL  0x99  /* args=[level, 0x00] */
#define BLACKSHARK_V3_PRO_SIDETONE_READ_CL 0x2c  /* args=[0x00]; resp[0]=level (0..15) */
#define BLACKSHARK_V3_PRO_SIDETONE_ID      0x01
#define BLACKSHARK_V3_PRO_SIDETONE_MAX     0x0f
#define BLACKSHARK_V3_PRO_THX_CLASS        0xdf  /* args=[mode, 0x00]; 0=stereo 1=spatial */
#define BLACKSHARK_V3_PRO_THX_ID           0x01
#define BLACKSHARK_V3_PRO_ANC_CLASS        0x92  /* args=[on, level, 0x00]; level 1..4 */
#define BLACKSHARK_V3_PRO_ANC_ID           0x02
#define BLACKSHARK_V3_PRO_ANC_LEVEL_MIN    1
#define BLACKSHARK_V3_PRO_ANC_LEVEL_MAX    4
#define BLACKSHARK_V3_PRO_POWER_SAVE_CLASS 0xac  /* args=[minutes, 0x00]; 0/15/30/45/60 */
#define BLACKSHARK_V3_PRO_POWER_SAVE_ID    0x01

/* EQ — 5-step sequence per preset switch.
 * 9 bands at 60/170/310/600/1k/3k/6k/12k/16k Hz, 9 preset slots (0..8).
 * Sign-magnitude encoding (same as V3): 0x00=0dB, 0x01=+1dB, 0x81=-1dB. */
#define BLACKSHARK_V3_PRO_EQ_STATE_CLASS   0xe1  /* args=[0x01,0x00] gate, [0x02,0x00] apply */
#define BLACKSHARK_V3_PRO_EQ_STATE_ID      0x01
#define BLACKSHARK_V3_PRO_EQ_BANDS_CLASS   0x95  /* args=[idx, b0..b8, 0x00] (12 bytes) */
#define BLACKSHARK_V3_PRO_EQ_BANDS_ID      0x0b
#define BLACKSHARK_V3_PRO_EQ_META_CLASS    0xe0  /* args=[idx, ...] (7 bytes) */
#define BLACKSHARK_V3_PRO_EQ_META_ID       0x06
#define BLACKSHARK_V3_PRO_EQ_COMMIT_CLASS  0xeb  /* args=[idx, ...] (12 bytes) */
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
