/*
 * Copyright (c) 2015 Tim Theede <pez2001@voyagerproject.de>
 *               2015 Terry Cain <terry@terrys-home.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef __HID_RAZER_CHROMAHDK_H
#define __HID_RAZER_CHROMAHDK_H

#define USB_DEVICE_ID_RAZER_CHROMA_HDK 0x0F09

#define RAZER_CHROMA_HDK_WAIT_MIN_US 900
#define RAZER_CHROMA_HDK_WAIT_MAX_US 1000

struct razer_chromahdk_device {
    struct usb_device *usbdev;
    struct hid_device *hiddev;
    unsigned char effect;
    char name[128];
    char phys[64];
};


#endif
