/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2015 Terry Cain <terrys-home.co.uk>
 */

#ifndef __HID_RAZER_NARI_H
#define __HID_RAZER_NARI_H

#include <linux/workqueue.h>
#include <linux/ktime.h>

#define USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS 0x051A
#define USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRED 0x051B
#define USB_DEVICE_ID_RAZER_NARI_WIRELESS 0x051C
#define USB_DEVICE_ID_RAZER_NARI_WIRED 0x051D

#define USB_INTERFACE_PROTOCOL_NONE 0

#define RAZER_NARI_REPORT_LEN 64
#define RAZER_NARI_INTERFACE_INDEX 5
#define RAZER_NARI_REPORT_ID 0xFF
#define RAZER_NARI_VALUE 0x03FF

#define RAZER_NARI_SERIAL_LEN 15
#define RAZER_NARI_DEFAULT_IDLE_TIME 900

enum razer_nari_effect_mode {
    NARI_EFFECT_NONE = 0,
    NARI_EFFECT_STATIC,
    NARI_EFFECT_SPECTRUM,
    NARI_EFFECT_WAVE,
    NARI_EFFECT_BREATH_SINGLE,
    NARI_EFFECT_BREATH_DUAL,
    NARI_EFFECT_BREATH_TRIPLE,
    NARI_EFFECT_BREATH_RANDOM,
    NARI_EFFECT_REACTIVE,
    NARI_EFFECT_STARLIGHT_SINGLE,
    NARI_EFFECT_STARLIGHT_DUAL,
    NARI_EFFECT_STARLIGHT_RANDOM,
    NARI_EFFECT_CUSTOM
};

struct razer_nari_device {
    struct usb_device *usb_dev;
    struct hid_device *hid_dev;
    struct mutex lock;
    struct delayed_work effect_work;

    unsigned char usb_interface_protocol;
    unsigned short usb_pid;
    unsigned short usb_vid;

    char serial[RAZER_NARI_SERIAL_LEN + 1];
    unsigned char fw_major;
    unsigned char fw_minor;
    bool fw_loaded;

    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char led_brightness;
    unsigned char led_on;

    enum razer_nari_effect_mode effect_mode;
    unsigned char colors[9];
    unsigned char custom_rgb[3];
    unsigned char wave_dir;
    unsigned char speed;
    unsigned int anim_step;
    ktime_t effect_start_ktime;
    ktime_t reactive_trigger_ktime;
    bool reactive_active;
    unsigned char breath_color_idx;
    unsigned char random_rgb[3];

    unsigned char haptic_enabled;
    unsigned char haptic_intensity;

    unsigned char charge_level;
    unsigned char charge_status;
    unsigned short idle_time;
};

/*
 * Razer Nari family uses 64-byte HID Feature Reports (Report ID 0xFF)
 * on USB Interface 5 (SET_REPORT / GET_REPORT, wValue=0x03FF, wIndex=5).
 * No CRC is used.
 */
struct razer_nari_report {
    unsigned char data[RAZER_NARI_REPORT_LEN];
};

#endif
