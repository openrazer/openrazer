/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Razer Wolverine Pro 8K USB Gamepad Driver - Header
 */

#ifndef __RAZERWOLVERINE_DRIVER_H
#define __RAZERWOLVERINE_DRIVER_H

/* USB Vendor/Device IDs */
#define USB_VENDOR_ID_RAZER             0x1532
#define USB_DEVICE_ID_RAZER_WOLVERINE_V3_PRO_WIRED    0x0A57  /* Wired USB-C mode */
#define USB_DEVICE_ID_RAZER_WOLVERINE_V3_PRO_WIRELESS 0x0A59  /* 2.4GHz wireless dongle */

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
};

#endif /* __RAZERWOLVERINE_DRIVER_H */
