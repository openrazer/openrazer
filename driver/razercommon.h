/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2015 Tim Theede <pez2001@voyagerproject.de>
 *               2015 Terri Cain <terri@dolphincorp.co.uk>
 */

#ifndef DRIVER_RAZERCOMMON_H_
#define DRIVER_RAZERCOMMON_H_

#include <linux/usb/input.h>
#include "compat.h"

#define DRIVER_VERSION "3.12.1"
#define DRIVER_LICENSE "GPL v2"
#define DRIVER_AUTHOR "Terri Cain <terri@dolphincorp.co.uk>"

// Macro to create device files
#define CREATE_DEVICE_FILE(dev, type) \
do { \
    if(device_create_file(dev, type)) { \
        goto exit_free; \
    } \
} while (0)

#define USB_VENDOR_ID_RAZER 0x1532

/* Each USB report has 90 bytes*/
#define RAZER_USB_REPORT_LEN 0x5A

// LED STATE
#define OFF 0x00
#define ON  0x01

// LED STORAGE Options
#define NOSTORE          0x00
#define VARSTORE         0x01

// LED definitions
#define ZERO_LED          0x00
#define SCROLL_WHEEL_LED  0x01
#define BATTERY_LED       0x03
#define LOGO_LED          0x04
#define BACKLIGHT_LED     0x05
#define MACRO_LED         0x07
#define GAME_LED          0x08
#define RED_PROFILE_LED   0x0C
#define GREEN_PROFILE_LED 0x0D
#define BLUE_PROFILE_LED  0x0E
#define RIGHT_SIDE_LED    0x10
#define LEFT_SIDE_LED     0x11
#define ARGB_CH_1_LED     0x1A
#define ARGB_CH_2_LED     0x1B
#define ARGB_CH_3_LED     0x1C
#define ARGB_CH_4_LED     0x1D
#define ARGB_CH_5_LED     0x1E
#define ARGB_CH_6_LED     0x1F
#define CHARGING_LED      0x20
#define FAST_CHARGING_LED 0x21
#define FULLY_CHARGED_LED 0x22

// LED Effect definitions
enum razer_classic_effect_id {
    CLASSIC_EFFECT_STATIC = 0x00,
    CLASSIC_EFFECT_BLINKING = 0x01,
    CLASSIC_EFFECT_BREATHING = 0x02, // also called pulsating
    CLASSIC_EFFECT_SPECTRUM = 0x04,
};

enum razer_matrix_effect_id {
    MATRIX_EFFECT_OFF = 0x00,
    MATRIX_EFFECT_WAVE = 0x01,
    MATRIX_EFFECT_REACTIVE = 0x02, // afterglow
    MATRIX_EFFECT_BREATHING = 0x03,
    MATRIX_EFFECT_SPECTRUM = 0x04,
    MATRIX_EFFECT_CUSTOMFRAME = 0x05,
    MATRIX_EFFECT_STATIC = 0x06,
    MATRIX_EFFECT_STARLIGHT = 0x19
};

// Report Responses
#define RAZER_CMD_BUSY          0x01
#define RAZER_CMD_SUCCESSFUL    0x02
#define RAZER_CMD_FAILURE       0x03
#define RAZER_CMD_TIMEOUT       0x04
#define RAZER_CMD_NOT_SUPPORTED 0x05

struct razer_rgb {
    u8 r;
    u8 g;
    u8 b;
};

union transaction_id_union {
    u8 id;
    struct transaction_parts {
        u8 device : 3;
        u8 id : 5;
    } parts;
};

union command_id_union {
    u8 id;
    struct command_id_parts {
        u8 direction : 1;
        u8 id : 7;
    } parts;
};

/* Status:
 * 0x00 New Command
 * 0x01 Command Busy
 * 0x02 Command Successful
 * 0x03 Command Failure
 * 0x04 Command No Response / Command Timeout
 * 0x05 Command Not Support
 *
 * Transaction ID used to group request-response, device useful when multiple devices are on one usb
 * Remaining Packets is the number of remaining packets in the sequence
 * Protocol Type is always 0x00
 * Data Size is the size of payload, cannot be greater than 80. 90 = header (8B) + data + CRC (1B) + Reserved (1B)
 * Command Class is the type of command being issued
 * Command ID is the type of command being send. Direction 0 is Host->Device, Direction 1 is Device->Host. AKA Get LED 0x80, Set LED 0x00
 *
 * */

struct razer_report {
    u8 status;
    union transaction_id_union transaction_id; /* */
    __be16 remaining_packets; /* Big Endian */
    u8 protocol_type; /*0x0*/
    u8 data_size;
    u8 command_class;
    union command_id_union command_id;
    u8 arguments[80];
    u8 crc;/*xor'ed bytes of report*/
    u8 reserved; /*0x0*/
};
static_assert(sizeof(struct razer_report) == 90);

struct razer_argb_report {
    u8 report_id;
    u8 channel_1;
    u8 channel_2;
    u8 pad;
    u8 last_idx;
    u8 color_data[315];
};
static_assert(sizeof(struct razer_argb_report) == 320);

int razer_send_control_msg(struct usb_device *usb_dev,void const *data, unsigned int report_index, unsigned long wait);
int razer_send_control_msg_old_device(struct usb_device *usb_dev,void const *data, uint report_value, uint report_index, uint report_size, ulong wait);
int razer_get_usb_response(struct usb_device *usb_dev, unsigned int report_index, struct razer_report* request_report, unsigned int response_index, struct razer_report* response_report, unsigned long wait);
int razer_send_argb_msg(struct usb_device* usb_dev, unsigned char channel, size_t size, void const* data);
unsigned char razer_calculate_crc(struct razer_report *report);
struct razer_report get_razer_report(unsigned char command_class, unsigned char command_id, unsigned char data_size);
void print_erroneous_report(struct razer_report* report, char* driver_name, char* message);

/* Borrowed from drivers/hid/usbhid/usbhid.h */
#define	hid_to_usb_dev(hid_dev) \
	to_usb_device(hid_dev->dev.parent->parent)

#endif /* DRIVER_RAZERCOMMON_H_ */
