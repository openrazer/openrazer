/*
 * Copyright (c) 2015 Terry Cain <terry@terrys-home.co.uk>
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef __HID_RAZER_MOUSE_H
#define __HID_RAZER_MOUSE_H

#ifndef USB_VENDOR_ID_RAZER
#define USB_VENDOR_ID_RAZER 0x1532
#endif

#ifndef USB_DEVICE_ID_RAZER_MAMBA
 #define USB_DEVICE_ID_RAZER_MAMBA 0x0045
#endif

#ifndef USB_DEVICE_ID_RAZER_ABYSSUS
 #define USB_DEVICE_ID_RAZER_ABYSSUS 0x0042
#endif

/* Each keyboard report has 90 bytes*/
#define RAZER_REPORT_LEN 0x5A
#define RAZER_MAMBA_ROW_LEN 15


#define RAZER_MOUSE_WAIT_MIN_US 600
#define RAZER_MOUSE_WAIT_MAX_US 800

struct razer_mouse_device {
    //struct input_dev *dev;
    struct usb_device *usbdev;
    struct hid_device *hiddev;
    unsigned char effect;
    char name[128];
    char phys[64];
};

#endif
