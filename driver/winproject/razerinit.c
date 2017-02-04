/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hid.h>

DLL_INTERNAL void init() {
	// Initialize the library.
	usb_init();

	// Find all busses.
	usb_find_busses();

	// Find all connected devices.
	usb_find_devices();
}

DLL_INTERNAL void close(struct device *dev) {
	struct usb_interface *intf = to_usb_interface(dev->parent);
	struct usb_device *usb_dev = interface_to_usbdev(intf);
	usb_close((struct usb_dev_handle*)usb_dev->dev);
	//usb_close(dev->parent->dev);
}
