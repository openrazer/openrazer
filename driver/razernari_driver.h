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

    //store color in device. -> hardware does not report set color. At least Synapse can't get it either.
    unsigned short red;
    unsigned short green;
    unsigned short blue;
    unsigned short brigthness;

    //don't know yet what  is in the reports.. keep looking.
    u8 data[64];
};

/*
 * TBH I don't really have a clue how to use the reports.
 * Somewhere in there seems to be the battery information. But I can't decipher it.
 * Maybe there is more in there, but not too much. Synapse can not tell a lot
 * about the device either. For example the don't know the set color or brightness.
 * */
struct razer_nari_request_report {
    unsigned char arguments[64];
    unsigned char length;
};

struct razer_nari_response_report {
    unsigned char report_id;
    unsigned char arguments[64];
};

#endif
