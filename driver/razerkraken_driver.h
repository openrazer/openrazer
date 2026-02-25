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
#define USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE 0x0527
#define USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2 0x0560
#define USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2_PRO 0x0554

#define USB_INTERFACE_PROTOCOL_NONE 0

/*
 * Kraken Kitty V2 Pro specific definitions
 * This device uses a completely different protocol from other Kraken devices.
 * Instead of memory-mapped addresses (Kylie/Rainie protocol), it uses direct
 * commands with a 15-byte HID report format.
 *
 * Protocol:
 *   Report ID: 0x40
 *   USB Value: 0x0240 (Report Type 2, Report ID 0x40)
 *   USB Index: 0x0003 (Interface 3)
 *   Report size: 15 bytes
 *
 * Commands:
 *   0x01 - Mode control (init/enable)
 *   0x02 - Brightness control
 *   0x03 - Static color (4 zones RGB)
 *
 * Zones: LeftEar, RightEar, LogoLeft, LogoRight
 */
#define KRAKEN_V2_PRO_REPORT_ID 0x40
#define KRAKEN_V2_PRO_REPORT_LEN 15
#define KRAKEN_V2_PRO_USB_INTERFACE 0x0003
#define KRAKEN_V2_PRO_USB_VALUE 0x0240

// V2 Pro Commands
#define KRAKEN_V2_PRO_CMD_MODE 0x01
#define KRAKEN_V2_PRO_CMD_BRIGHTNESS 0x02
#define KRAKEN_V2_PRO_CMD_STATIC_COLOR 0x03

// V2 Pro Mode/Effect values
#define KRAKEN_V2_PRO_MODE_DIRECT 0x08
#define KRAKEN_V2_PRO_EFFECT_SPECTRUM 0x03

// V2 Pro Zone mask (all 4 zones)
#define KRAKEN_V2_PRO_ZONES 4
#define KRAKEN_V2_PRO_ZONE_MASK_ALL 0x0f

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

    u8 data[33];

    // Kraken Kitty V2 Pro uses different protocol
    unsigned char is_v2_pro;

    // V2 Pro state tracking
    unsigned char v2pro_effect_none;     // 1 if "none" effect is active (brightness=0)
    unsigned char v2pro_brightness;      // Cached brightness for restore
    unsigned char v2pro_current_effect;  // Current effect: 0=none, 1=static, 2=custom, 3=spectrum
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

/*
 * Kraken Kitty V2 Pro report structure (15 bytes)
 * Format: [ReportID][Command][Subcommand][Data x 12]
 *
 * Brightness command (0x02):
 *   40 02 01 0f XX 00 00 00 00 00 00 00 00 00 00
 *   XX = brightness (0x00-0xff)
 *
 * Static color command (0x03):
 *   40 03 00 R1 G1 B1 R2 G2 B2 R3 G3 B3 R4 G4 B4
 *   Zone1=LeftEar, Zone2=RightEar, Zone3=LogoLeft, Zone4=LogoRight
 */
struct razer_kraken_v2pro_report {
    unsigned char report_id;   // 0x40
    unsigned char command;     // 0x01, 0x02, 0x03
    unsigned char subcommand;
    unsigned char data[12];
};

#endif
