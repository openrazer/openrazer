/*
 * Copyright (c) 2015 Terry Cain <terrys-home.co.uk>
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef __HID_RAZER_ACCESSORY_H
#define __HID_RAZER_ACCESSORY_H

#define USB_DEVICE_ID_RAZER_NOMMO_CHROMA 0x0517
#define USB_DEVICE_ID_RAZER_NOMMO_PRO 0x0518
#define USB_DEVICE_ID_RAZER_CHROMA_MUG 0x0F07
#define USB_DEVICE_ID_RAZER_CHROMA_BASE 0x0F08
#define USB_DEVICE_ID_RAZER_CHROMA_HDK 0x0F09

struct razer_accessory_device {
    struct usb_device *usb_dev;
    struct input_dev *input;
    struct mutex lock;
    unsigned char usb_interface_protocol;

    unsigned short usb_vid;
    unsigned short usb_pid;

    unsigned char mug_present_poll;

    char serial[23];
    // 3 Bytes, first byte is whether fw version is collected, 2nd byte is major version, 3rd is minor, should be printed out in hex form as are bcd
    unsigned char firmware_version[3];
};

/*
 * USB INTERRUPT
 *
 * */

#endif
