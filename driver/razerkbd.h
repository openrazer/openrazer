/*
 * Copyright (c) 2015 Tim Theede <pez2001@voyagerproject.de>
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef __HID_RAZER_KEYBOARD_BLACKWIDOW_CHROMA_H
#define __HID_RAZER_KEYBOARD_BLACKWIDOW_CHROMA_H

#ifndef USB_VENDOR_ID_RAZER
 #define USB_VENDOR_ID_RAZER 0x1532
#endif

#ifndef USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA
 #define USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA 0x0203
#endif

#ifndef USB_DEVICE_ID_RAZER_FIREFLY
 #define USB_DEVICE_ID_RAZER_FIREFLY 0x0c00
#endif


/*each keyboard report has 90 bytes*/
#define RAZER_BLACKWIDOW_REPORT_LEN 0x5a 

#define RAZER_BLACKWIDOW_CHROMA_WAVE_DIRECTION_LEFT 2
#define RAZER_BLACKWIDOW_CHROMA_WAVE_DIRECTION_RIGHT 1

#define RAZER_BLACKWIDOW_CHROMA_CHANGE_EFFECT 0x0A

#define RAZER_BLACKWIDOW_CHROMA_EFFECT_NONE 0
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_WAVE 1
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_REACTIVE 2
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_BREATH 3
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_SPECTRUM 4
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_CUSTOM 5 //update profile data 
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_STATIC 6
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_CLEAR_ROW 8


#define RAZER_BLACKWIDOW_CHROMA_EFFECT_SET_KEYS 9 //update profile needs to be called after setting keys to reflect changes
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_RESET 10
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN 11
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN2 12
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN3 13
#define RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN4 14


#define RAZER_BLACKWIDOW_CHROMA_ROW_LEN 0x15
#define RAZER_BLACKWIDOW_CHROMA_ROWS_NUM 6

//#define RAZER_BLACKWIDOW_CHROMA_ROW_LEN 6
//#define RAZER_BLACKWIDOW_CHROMA_ROWS_NUM 4



#define RAZER_BLACKWIDOW_CHROMA_WAIT_MS 1
#define RAZER_BLACKWIDOW_CHROMA_WAIT_MIN_US 600
#define RAZER_BLACKWIDOW_CHROMA_WAIT_MAX_US 800



struct razer_rgb {
	unsigned char r,g,b;
};

struct razer_row_rgb {
	struct razer_rgb cols[RAZER_BLACKWIDOW_CHROMA_ROW_LEN+1];
};


struct razer_kbd_device {
	//struct input_dev *dev;
	struct usb_device *usbdev;
	struct hid_device *hiddev;
	unsigned char effect;
	char name[128];
	char phys[64];
	struct razer_row_rgb matrix[RAZER_BLACKWIDOW_CHROMA_ROWS_NUM];
	bool effect_submitted;
};

//TODO speed parameter ?

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