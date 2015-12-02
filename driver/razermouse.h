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

/* Each keyboard report has 90 bytes*/
#define RAZER_REPORT_LEN 0x5A



#define RAZER_WAIT_MS 1
#define RAZER_WAIT_MIN_US 600
#define RAZER_WAIT_MAX_US 800

struct razer_rgb {
	unsigned char r,g,b;
};

struct razer_mouse_device {
	//struct input_dev *dev;
	struct usb_device *usbdev;
	struct hid_device *hiddev;
	unsigned char effect;
	char name[128];
	char phys[64];
};

struct razer_report {
	unsigned char report_start_marker; /*0x0*/
	unsigned char id; /*always 0xFF maybe it's an i2c id or some range*/
	unsigned char reserved1[3];
	unsigned char parameter_bytes_num;
	unsigned char reserved2;/*always 0x03 maybe some command class id*/
	unsigned char command;
	unsigned char sub_command;/*named first parameter*/
	unsigned char command_parameters[90-11];
	unsigned char crc;/*xor'ed bytes of report*/
	unsigned char report_end_marker; /*0x0*/
};

#endif
