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

#ifndef USB_DEVICE_ID_RAZER_OROCHI_2011
#define USB_DEVICE_ID_RAZER_OROCHI_2011 0x0013
#endif

#ifndef USB_DEVICE_ID_RAZER_ABYSSUS_1800
#define USB_DEVICE_ID_RAZER_ABYSSUS_1800 0x0020
#endif

#ifndef USB_DEVICE_ID_RAZER_MAMBA_2012_WIRED
#define USB_DEVICE_ID_RAZER_MAMBA_2012_WIRED 0x0024
#endif

#ifndef USB_DEVICE_ID_RAZER_MAMBA_2012_WIRELESS
#define USB_DEVICE_ID_RAZER_MAMBA_2012_WIRELESS 0x0025
#endif

#ifndef USB_DEVICE_ID_RAZER_IMPERATOR // 2012
#define USB_DEVICE_ID_RAZER_IMPERATOR 0x002F
#endif

#ifndef USB_DEVICE_ID_RAZER_OUROBOROS // 2012
#define USB_DEVICE_ID_RAZER_OUROBOROS 0x0032
#endif

#ifndef USB_DEVICE_ID_RAZER_TAIPAN // 2016?
#define USB_DEVICE_ID_RAZER_TAIPAN 0x0034
#endif

#ifndef USB_DEVICE_ID_RAZER_NAGA_HEX_RED
#define USB_DEVICE_ID_RAZER_NAGA_HEX_RED 0x0036
#endif

#ifndef USB_DEVICE_ID_RAZER_OROCHI_2013
#define USB_DEVICE_ID_RAZER_OROCHI_2013 0x0039
#endif

#ifndef USB_DEVICE_ID_RAZER_NAGA_2014
#define USB_DEVICE_ID_RAZER_NAGA_2014 0x0040
#endif

#ifndef USB_DEVICE_ID_RAZER_NAGA_HEX
#define USB_DEVICE_ID_RAZER_NAGA_HEX 0x0041
#endif

#ifndef USB_DEVICE_ID_RAZER_ABYSSUS
#define USB_DEVICE_ID_RAZER_ABYSSUS 0x0042
#endif

#ifndef USB_DEVICE_ID_RAZER_DEATHADDER_CHROMA
#define USB_DEVICE_ID_RAZER_DEATHADDER_CHROMA 0x0043
#endif

#ifndef USB_DEVICE_ID_RAZER_MAMBA_WIRED
#define USB_DEVICE_ID_RAZER_MAMBA_WIRED 0x0044
#endif

#ifndef USB_DEVICE_ID_RAZER_MAMBA_WIRELESS
#define USB_DEVICE_ID_RAZER_MAMBA_WIRELESS 0x0045
#endif

#ifndef USB_DEVICE_ID_RAZER_MAMBA_TE_WIRED
#define USB_DEVICE_ID_RAZER_MAMBA_TE_WIRED 0x0046
#endif

#ifndef USB_DEVICE_ID_RAZER_OROCHI_CHROMA
#define USB_DEVICE_ID_RAZER_OROCHI_CHROMA 0x0048
#endif

#ifndef USB_DEVICE_ID_RAZER_DIAMONDBACK_CHROMA
#define USB_DEVICE_ID_RAZER_DIAMONDBACK_CHROMA 0x004C
#endif

#ifndef USB_DEVICE_ID_RAZER_NAGA_HEX_V2
#define USB_DEVICE_ID_RAZER_NAGA_HEX_V2 0x0050
#endif

#ifndef USB_DEVICE_ID_RAZER_NAGA_CHROMA
#define USB_DEVICE_ID_RAZER_NAGA_CHROMA 0x0053
#endif

#ifndef USB_DEVICE_ID_RAZER_ABYSSUS_V2
#define USB_DEVICE_ID_RAZER_ABYSSUS_V2 0x005B
#endif

#ifndef USB_DEVICE_ID_RAZER_DEATHADDER_ELITE
#define USB_DEVICE_ID_RAZER_DEATHADDER_ELITE 0x005C
#endif

#ifndef USB_DEVICE_ID_RAZER_NAGA_TRINITY
#define USB_DEVICE_ID_RAZER_NAGA_TRINITY 0x0067
#endif


/* Each keyboard report has 90 bytes*/
#define RAZER_REPORT_LEN 0x5A

#define RAZER_MAMBA_ROW_LEN 15          // 0 => 14
#define RAZER_MAMBA_TE_ROW_LEN 16       // 0 => 15
#define RAZER_DIAMONDBACK_ROW_LEN 21    // 0 => 20

#define RAZER_MOUSE_WAIT_MIN_US 600
#define RAZER_MOUSE_WAIT_MAX_US 800

struct razer_mouse_device {
    //struct input_dev *dev;
    struct usb_device *usbdev;
    struct hid_device *hiddev;
    unsigned char effect;
    char name[128];
    char phys[64];

    struct usb_device *usb_dev;
    struct mutex lock;
    unsigned char usb_interface_protocol;

    unsigned short usb_vid;
    unsigned short usb_pid;

    char serial[23];

    unsigned char orochi2011_led;
    unsigned char orochi2011_dpi;
    unsigned short orochi2011_poll;
};

// Mamba Key Location
// 0 => 6  --->  top left  => bottom left
// 7 => 13 --->  top right => bottom right
// 14      --->  Scroll LED

// Mamba TE Key Location
// 0 => 6  --->  top left  => bottom left
// 7 => 13 --->  top right => bottom right
// 14      --->  Logo LED
// 15      --->  Scroll LED

#endif
