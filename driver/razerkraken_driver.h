/*
 * Copyright (c) 2015 Terry Cain <terrys-home.co.uk>
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef __HID_RAZER_KRAKEN_H
#define __HID_RAZER_KRAKEN_H

#ifndef USB_VENDOR_ID_RAZER
    #define USB_VENDOR_ID_RAZER 0x1532
#endif

#ifndef USB_DEVICE_ID_RAZER_KRAKEN_V2 // Codename Kylie
    #define USB_DEVICE_ID_RAZER_KRAKEN_V2 0x0510
#endif

#define USB_INTERFACE_PROTOCOL_NONE 0

// #define RAZER_KRAKEN_V2_REPORT_LEN ?

struct razer_kraken_device {
    struct usb_device *usb_dev;
    struct mutex lock;
    unsigned char usb_interface_protocol;
    unsigned short usb_pid;
    unsigned short usb_vid;
    
    char serial[23];
    // 3 Bytes, first byte is wether fw version is collected, 2nd byte is major version, 3rd is minor, should be printed out in hex form as are bcd
    unsigned char firmware_version[3];
    
    u8 data[33];
    
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
 *
 * Destination:
 *   0x40 - Write data to RAM
 * 
 * Address:
 *   RAM
 *   0x172D - Set LED Effect, see note 1
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
 * */

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
