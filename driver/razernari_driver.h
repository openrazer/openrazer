/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2015 Terry Cain <terrys-home.co.uk>
 */

#ifndef __HID_RAZER_NARI_H
#define __HID_RAZER_NARI_H

// Codenames Unknown
// a lot discussion here: https://github.com/openrazer/openrazer/issues/974
#define USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS 0x051A
#define USB_DEVICE_ID_RAZER_NARI_ULTIMATE_USB 0x051B
//is this correct? -> derived from https://github.com/openrazer/openrazer/issues/724
#define USB_DEVICE_ID_RAZER_NARI_WIRELESS 0x051C
#define USB_DEVICE_ID_RAZER_NARI_USB 0x051D
// there might be another one for the xbox version. But I don't know the id.
// this might also work for the manowar. Are there still people out there that have the hardware and want to test it?

#define USB_INTERFACE_PROTOCOL_NONE 0

#define RAZER_NARI_REPORT_LEN 64

struct razer_nari_device {
    struct usb_device *usb_dev;
    struct hid_device *hid_dev; //needed for getting reports.
    struct mutex lock;
    unsigned char usb_interface_protocol;
    unsigned short usb_pid;
    unsigned short usb_vid;
    unsigned char* name;

    // LED logo state. Hardware does not report set color back, Synapse can't
    // read it either, so we store it in the device struct for read-back.
    unsigned short red;
    unsigned short green;
    unsigned short blue;
    unsigned short led_brightness;

    // Haptic state: enable flag and intensity 0..100. The Nari Ultimate has
    // two bass-response haptic motors in the ear cups; Nari (non-Ultimate)
    // lacks the motors and will ignore the haptic commands.
    unsigned char haptic_enabled;
    unsigned char haptic_intensity;

    // Raw response data from the device, kept for future battery decoding.
    u8 data[64];
};

/*
 * Razer Nari uses HID Feature Report 0xff on interface 5 (SET_REPORT,
 * wValue=0x03ff, wIndex=0x0005, wLength=64). The report is 64 bytes
 * including the Report ID as byte 0. No CRC is used.
 *
 * Layout (decoded from pcap captures in felixZmn/razer-nari-driver):
 *
 *     offset  meaning
 *     ------  -------
 *          0  Report ID, always 0xff
 *          1  magic, always 0x0a (0xfd for some rare requests)
 *          2  magic, always 0x00
 *          3  magic, always 0xff
 *          4  common header byte, always 0x04
 *          5  command class
 *          6  protocol byte, always 0xf1
 *          7  command sub-id
 *          8  command target
 *          9  enable / state byte
 *         10  value (haptic intensity, etc.)
 *      11-63  command-specific arguments or zero padding
 *
 * Known commands:
 *
 *   LED logo on/off:
 *       bytes[5..8] = { 0x12, 0xf1, 0x03, 0x71 }
 *       bytes[9]    = 0x00 (off) or 0xff (on)
 *
 *   Haptic intensity:
 *       bytes[5..8] = { 0x02, 0xf1, 0x06, 0x20 }
 *       bytes[9]    = 0x00 (disable) or 0x01 (enable)
 *       bytes[10]   = intensity 0x00..0x64 (0..100 decimal, linear)
 */
struct razer_nari_request_report {
    unsigned char arguments[RAZER_NARI_REPORT_LEN];
    unsigned char length;
};

struct razer_nari_response_report {
    unsigned char report_id;
    unsigned char arguments[64];
};

#endif
