/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Razer Wolverine Pro 8K USB Gamepad Driver - Header
 */

#ifndef __RAZERWOLVERINE_DRIVER_H
#define __RAZERWOLVERINE_DRIVER_H

#include <linux/workqueue.h>
#include <linux/mutex.h>

/* USB Vendor/Device IDs */
#define USB_VENDOR_ID_RAZER             0x1532
#define USB_DEVICE_ID_RAZER_WOLVERINE_V3_PRO_WIRED    0x0A57  /* Wired USB-C mode */
#define USB_DEVICE_ID_RAZER_WOLVERINE_V3_PRO_WIRELESS 0x0A59  /* 2.4GHz wireless dongle */

/* Xbox 360 controller IDs - reported to input subsystem for Steam compatibility */
#define USB_VENDOR_ID_MICROSOFT         0x045e
#define USB_DEVICE_ID_XBOX360_CONTROLLER 0x028e

/* Timeout for detecting controller disconnect (in jiffies) - 30 seconds
 * The controller sends periodic packets when on, so this only triggers
 * when the controller is actually powered off. */
#define WOLVERINE_DISCONNECT_TIMEOUT (30 * HZ)

/**
 * Device structure for Wolverine Pro 8K
 */
struct wolverine_device {
	struct usb_device *usbdev;      /* USB device */
	struct usb_interface *intf;     /* USB interface */
	struct input_dev *input;        /* Input device */
	struct urb *irq;                /* URB for interrupt transfers */
	
	unsigned char *data;            /* Input data buffer */
	dma_addr_t data_dma;            /* DMA address for data buffer */
	size_t data_size;               /* Size of data buffer */
	
	char phys[64];                  /* Physical device path */
	
	/* Controller connection tracking (for wireless dongle) */
	atomic_t controller_connected;  /* True if controller is actively sending data */
	bool input_registered;          /* True if input device is registered */
	unsigned long last_packet_time; /* jiffies timestamp of last packet */
	struct delayed_work disconnect_work; /* Work item for disconnect detection */
	struct work_struct connect_work;     /* Work item for registration */
	struct mutex reg_lock;          /* Protects input registration state */
	bool shutting_down;             /* True when driver is being removed */
};

#endif /* __RAZERWOLVERINE_DRIVER_H */
