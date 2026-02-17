/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2024 OpenRazer Team
 *
 * Razer Wolverine V3 Pro 8K PC USB Gamepad Driver - Header
 */

#ifndef __HID_RAZER_WOLVERINE_H
#define __HID_RAZER_WOLVERINE_H

#include <linux/workqueue.h>
#include <linux/mutex.h>

/* USB Vendor/Device IDs */
#define USB_VENDOR_ID_RAZER                           0x1532
#define USB_DEVICE_ID_RAZER_WOLVERINE_V3_PRO_WIRED    0x0A57
#define USB_DEVICE_ID_RAZER_WOLVERINE_V3_PRO_WIRELESS 0x0A59

/* Timeout for detecting controller disconnect (in jiffies) */
#define WOLVERINE_DISCONNECT_TIMEOUT (60 * HZ)

/**
 * struct wolverine_device - Device structure for Wolverine V3 Pro 8K PC
 */
struct wolverine_device {
    struct usb_device *usbdev;
    struct usb_interface *intf;
    struct input_dev *input;
    struct urb *irq;

    unsigned char *data;
    dma_addr_t data_dma;
    size_t data_size;

    char phys[64];

    /* Controller connection tracking (for wireless dongle) */
    atomic_t controller_connected;
    bool input_registered;
    unsigned long last_packet_time;
    struct delayed_work disconnect_work;
    struct work_struct connect_work;
    struct mutex reg_lock;
    bool shutting_down;
};

#endif /* __HID_RAZER_WOLVERINE_H */
