/*
 * Copyright (c) 2015 Tim Theede <pez2001@voyagerproject.de>
 *               2015 Terry Cain <terry@terrys-home.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef __HID_RAZER_FIREFLY_H
#define __HID_RAZER_FIREFLY_H

// TODO MOVE TO COMMON
#ifndef USB_VENDOR_ID_RAZER
#define USB_VENDOR_ID_RAZER 0x1532
#endif

#ifndef USB_DEVICE_ID_RAZER_FIREFLY
#define USB_DEVICE_ID_RAZER_FIREFLY 0x0C00
#endif


/* Each keyboard report has 90 bytes*/
#define RAZER_FIREFLY_REPORT_LEN 0x5A

#define RAZER_FIREFLY_WAVE_DIRECTION_ACW 2
#define RAZER_FIREFLY_WAVE_DIRECTION_CW 1

#define RAZER_FIREFLY_CHANGE_EFFECT 0x0A

#define RAZER_FIREFLY_EFFECT_NONE 0
#define RAZER_FIREFLY_EFFECT_WAVE 1
#define RAZER_FIREFLY_EFFECT_REACTIVE 2 // Didn't get this working
#define RAZER_FIREFLY_EFFECT_BREATH 3
#define RAZER_FIREFLY_EFFECT_SPECTRUM 4
#define RAZER_FIREFLY_EFFECT_CUSTOM 5
#define RAZER_FIREFLY_EFFECT_STATIC 6
#define RAZER_FIREFLY_EFFECT_CLEAR_ROW 8

#define RAZER_FIREFLY_ROW_LEN 0x0F
#define RAZER_FIREFLY_ROWS_NUM 1

#define RAZER_FIREFLY_WAIT_MIN_US 900
#define RAZER_FIREFLY_WAIT_MAX_US 1000

struct razer_firefly_device {
    struct usb_device *usbdev;
    struct hid_device *hiddev;
    unsigned char effect;
    char name[128];
    char phys[64];
};


#endif
