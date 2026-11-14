/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2015 Terri Cain <terri@dolphincorp.co.uk>
 */

#ifndef __HID_RAZER_ACCESSORY_H
#define __HID_RAZER_ACCESSORY_H

#define USB_DEVICE_ID_RAZER_FIREFLY_HYPERFLUX 0x0068
#define USB_DEVICE_ID_RAZER_MOUSE_DOCK 0x007E
#define USB_DEVICE_ID_RAZER_MOUSE_DOCK_PRO 0x00A4
#define USB_DEVICE_ID_RAZER_CORE 0x0215
#define USB_DEVICE_ID_RAZER_NOMMO_CHROMA 0x0517
#define USB_DEVICE_ID_RAZER_NOMMO_PRO 0x0518
#define USB_DEVICE_ID_RAZER_FIREFLY 0x0C00
#define USB_DEVICE_ID_RAZER_GOLIATHUS_CHROMA 0x0C01
#define USB_DEVICE_ID_RAZER_GOLIATHUS_CHROMA_EXTENDED 0x0C02
#define USB_DEVICE_ID_RAZER_FIREFLY_V2 0x0C04
#define USB_DEVICE_ID_RAZER_STRIDER_CHROMA 0x0C05
#define USB_DEVICE_ID_RAZER_GOLIATHUS_CHROMA_3XL 0x0C06
#define USB_DEVICE_ID_RAZER_FIREFLY_V2_PRO 0x0C08
#define USB_DEVICE_ID_RAZER_CHROMA_MUG 0x0F07
#define USB_DEVICE_ID_RAZER_CHROMA_BASE 0x0F08
#define USB_DEVICE_ID_RAZER_CHROMA_HDK 0x0F09
#define USB_DEVICE_ID_RAZER_LAPTOP_STAND_CHROMA 0x0F0D
#define USB_DEVICE_ID_RAZER_RAPTOR_27 0x0F12
#define USB_DEVICE_ID_RAZER_LIANLI_O11_DYNAMIC 0x0F13
#define USB_DEVICE_ID_RAZER_TOMAHAWK_ATX 0x0F17
#define USB_DEVICE_ID_RAZER_KRAKEN_KITTY_EDITION 0x0F19
#define USB_DEVICE_ID_RAZER_CORE_X_CHROMA 0x0F1A
#define USB_DEVICE_ID_RAZER_MOUSE_BUNGEE_V3_CHROMA 0x0F1D
#define USB_DEVICE_ID_RAZER_CHROMA_ADDRESSABLE_RGB_CONTROLLER 0x0F1F
#define USB_DEVICE_ID_RAZER_BASE_STATION_V2_CHROMA 0x0F20
#define USB_DEVICE_ID_RAZER_THUNDERBOLT_4_DOCK_CHROMA 0x0F21
#define USB_DEVICE_ID_RAZER_CHARGING_PAD_CHROMA 0x0F26
#define USB_DEVICE_ID_RAZER_LAPTOP_STAND_CHROMA_V2 0x0F2B

#include <linux/atomic.h>
#include <linux/kref.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

#define RAZER_ACCESSORY_WAIT_MIN_US 600

#define RAZER_NEW_DEVICE_WAIT_MIN_US 31000

/* Maximum number of nearby Razer mice the dock can announce at once.
 * Per the v2 capture format (16-byte HID input report on EP 0x82, "05 37 ..."),
 * the slot count is small; 8 covers any realistic environment. */
#define RAZER_DOCK_PRO_MAX_NEARBY 8

/* Paired-mouse RF link state, from the dock's '05 09 <state>' input report. */
#define RAZER_DOCK_PRO_LINK_UNKNOWN (-1)
#define RAZER_DOCK_PRO_LINK_DOWN 0
#define RAZER_DOCK_PRO_LINK_UP 1

/*
 * State shared between the two HID interfaces the Mouse Dock Pro exposes.
 * Interface 0 carries the control-transfer feature reports (matrix LEDs,
 * pair/unpair, paired-mouse passthrough) and owns the user-visible sysfs
 * tree.  Interface 1 carries unsolicited HID input reports on EP 0x82,
 * including the dock's announcements of nearby Razer mice.  They need to
 * share the nearby-mice cache, so each per-interface razer_accessory_device
 * carries a pointer to the same razer_dock_pro_shared.
 */
struct razer_dock_pro_shared {
    struct list_head list;       /* linked into dock_pro_shared_list */
    struct kref ref;             /* released when both interfaces detach */
    struct usb_device *usb_dev;  /* key for find-or-create lookup */

    spinlock_t nearby_lock;      /* IRQ-safe: written from raw_event */
    unsigned short nearby_pids[RAZER_DOCK_PRO_MAX_NEARBY];  /* 0 = empty */
    unsigned long nearby_jiffies;

    /*
     * PID of the currently paired mouse, set at pair_step2 and cleared at
     * unpair.  Used as a fallback by razer_attr_read_paired_pid when the
     * 0xbf heartbeat response does not carry the PID (e.g. older firmware).
     * Protected by nearby_lock.
     */
    unsigned short paired_pid;

    /*
     * Last DPI the paired mouse announced with '05 02' (cycle button, wake
     * from sleep).  Cleared when the link drops, since the mouse sends none
     * on power-on and a stale value must not be taken for the current one.
     * Protected by nearby_lock.
     */
    unsigned short reported_dpi_x;
    unsigned short reported_dpi_y;
    bool reported_dpi_valid;

    /*
     * RAZER_DOCK_PRO_LINK_* as last reported on interface 1; UNKNOWN until the
     * first report, and again from the start of a pair/unpair until the next
     * one.  A change to it or to the reported DPI queues link_work, which
     * wakes pollers of mouse_connected and mouse_reported_dpi on the
     * interface that owns the sysfs tree (ctrl_hdev, NULL once it detaches).
     */
    atomic_t link_state;
    struct work_struct link_work;
    struct mutex ctrl_lock;      /* protects ctrl_hdev */
    struct hid_device *ctrl_hdev;
};

struct razer_accessory_device {
    struct hid_device *hdev;
    struct usb_device *usb_dev; // TODO: remove usages, replace with hdev
    struct input_dev *input;
    struct mutex lock;
    unsigned char usb_interface_protocol;

    unsigned short usb_vid;
    unsigned short usb_pid;

    unsigned char saved_brightness;

    char serial[23];

    /*
     * Set to 1 while a pair or unpair sequence is in progress.  The
     * mouse_connected sysfs handler returns 0 immediately when this flag is
     * set so the daemon's monitor loop cannot send RF-relay battery GETs that
     * would disrupt the dock's RF state machine mid-sequence.
     */
    atomic_t pairing_busy;

    /* Non-NULL only on Mouse Dock Pro interfaces.  Lazily allocated by the
     * first interface to probe; released when both interfaces detach. */
    struct razer_dock_pro_shared *shared;

    /* Set when probe called hid_hw_open() to keep the interrupt endpoint
     * polled without a userspace reader. */
    bool hw_opened;
};

/*
 * USB INTERRUPT
 *
 * */

#endif
