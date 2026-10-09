// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Tim Theede <pez2001@voyagerproject.de>
 *               2015 Terri Cain <terri@dolphincorp.co.uk>
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/hid.h>

#include "razercommon.h"

/**
 * Send USB control report to the keyboard
 * USUALLY index = 0x02
 * FIREFLY is 0
 */
/* Leviathan V2 X uses Feature Report ID 7 (0x0307) and 91-byte packets */
static inline bool razer_is_leviathan_v2x(struct usb_device *usb_dev)
{
    return usb_dev->descriptor.idProduct == 0x054a;
}

int razer_send_control_msg(struct hid_device *hdev, const void *data, u16 size, u16 index, ulong wait)
{
    struct usb_device *usb_dev = hid_to_usb_dev(hdev);
    bool leviathan = razer_is_leviathan_v2x(usb_dev);
    u16 value = leviathan ? 0x0307 : 0x0300;
    u16 send_size = leviathan ? size + 1 : size;
    u8 *buf = NULL;
    int ret;

    if (leviathan) {
        buf = kzalloc(send_size, GFP_KERNEL);
        if (!buf)
            return -ENOMEM;
        buf[0] = 0x07;
        memcpy(buf + 1, data, size);
        data = buf;
    }

    // Send usb control message
    ret = usb_control_msg_send(usb_dev,
                               0, // endpoint to send the message to
                               HID_REQ_SET_REPORT, // USB message request value (0x09)
                               USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT, // USB message request type value (0x21)
                               value, // USB message value
                               index, // USB message index value
                               data, // pointer to the data to send
                               send_size, // length in bytes of the data to send
                               USB_CTRL_SET_TIMEOUT, // time in msecs to wait for the message to complete before timing out
                               GFP_KERNEL);

    // Wait
    fsleep(wait);

    kfree(buf);

    if (ret)
        hid_warn(hdev, "Failed to send USB control message: %d\n", ret);

    return ret;
}

/**
 * Get a response from the razer device
 *
 * Makes a request like normal, this must change a variable in the device as then we
 * tell it give us data and it gives us a report.
 *
 * Supported Devices:
 *   Razer Chroma
 *   Razer Mamba
 *   Razer BlackWidow Ultimate 2013*
 *   Razer Firefly*
 *
 * Request report is the report sent to the device specifying what response we want
 * Response report will get populated with a response
 *
 * Returns 0 when successful, 1 if the report length is invalid.
 */
int razer_get_usb_response(struct hid_device *hdev, uint report_index, struct razer_report* request_report, uint response_index, struct razer_report* response_report, ulong wait)
{
    struct usb_device *usb_dev = hid_to_usb_dev(hdev);
    bool leviathan = razer_is_leviathan_v2x(usb_dev);
    int err;

    if (WARN_ON(request_report->transaction_id.id == 0x00)) {
        request_report->transaction_id.id = 0xFF;
    }

    // Send the request to the device.
    err = razer_send_control_msg(hdev, request_report, sizeof(*request_report), report_index, wait);
    if (err)
        return err;

    if (leviathan) {
        /* Leviathan returns 91 bytes: report ID (0x07) + 90-byte razer_report */
        u8 buf[sizeof(struct razer_report) + 1] = {0};
        err = usb_control_msg_recv(usb_dev,
                                   0,
                                   HID_REQ_GET_REPORT,
                                   USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
                                   0x0307,
                                   response_index,
                                   buf, sizeof(buf),
                                   USB_CTRL_SET_TIMEOUT, GFP_KERNEL);
        if (err) {
            hid_warn(hdev, "Failed to receive USB control message: %d\n", err);
            return err;
        }
        if (buf[0] == 0x07)
            memcpy(response_report, buf + 1, sizeof(struct razer_report));
        else
            memset(response_report, 0, sizeof(struct razer_report));
    } else {
        // Now ask for response
        err = usb_control_msg_recv(usb_dev,
                                   0, // endpoint to send the message to
                                   HID_REQ_GET_REPORT, // USB message request value (0x01)
                                   USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN, // USB message request type value (0xA1)
                                   0x300, // USB message value
                                   response_index, // USB message index value
                                   response_report, // pointer to the data to be filled in by the message
                                   sizeof(*response_report), // length in bytes of the data to be received
                                   USB_CTRL_SET_TIMEOUT, // time in msecs to wait for the message to complete before timing out
                                   GFP_KERNEL);
        if (err) {
            hid_warn(hdev, "Failed to receive USB control message: %d\n", err);
            return err;
        }
    }

    if (WARN_ONCE(response_report->data_size > ARRAY_SIZE(response_report->arguments),
                  "Field data_size %d in response is bigger than arguments\n",
                  response_report->data_size)) {
        /* Sanitize the value since at the moment callers don't respect the return code */
        response_report->data_size = ARRAY_SIZE(response_report->arguments);
        return -EINVAL;
    }

    return err;
}

/**
 * Calculate the checksum for the usb message
 *
 * Checksum byte is stored in the 2nd last byte in the messages payload.
 * The checksum is generated by XORing all the bytes in the report starting
 * at byte number 2 (0 based) and ending at byte 88.
 */
unsigned char razer_calculate_crc(struct razer_report *report)
{
    /*second to last byte of report is a simple checksum*/
    /*just xor all bytes up with overflow and you are done*/
    unsigned char crc = 0;
    unsigned char *_report = (unsigned char*)report;

    unsigned int i;
    for(i = 2; i < 88; i++) {
        crc ^= _report[i];
    }

    return crc;
}

/**
 * Get initialised razer report
 */
struct razer_report get_razer_report(unsigned char command_class, unsigned char command_id, unsigned char data_size)
{
    struct razer_report new_report = {0};
    memset(&new_report, 0, sizeof(struct razer_report));

    new_report.status = 0x00;
    new_report.transaction_id.id = 0x00;
    new_report.remaining_packets = 0x00;
    new_report.protocol_type = 0x00;
    new_report.command_class = command_class;
    new_report.command_id.id = command_id;
    new_report.data_size = data_size;

    return new_report;
}

/**
 * Print report to syslog
 */
void print_erroneous_report(struct hid_device *hdev, struct razer_report* report, const char *message)
{
    hid_warn(hdev, "%s. status: %02x transaction_id.id: %02x remaining_packets: %02x protocol_type: %02x data_size: %02x, command_class: %02x, command_id.id: %02x Params: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x .\n",
             message,
             report->status,
             report->transaction_id.id,
             report->remaining_packets,
             report->protocol_type,
             report->data_size,
             report->command_class,
             report->command_id.id,
             report->arguments[0], report->arguments[1], report->arguments[2], report->arguments[3], report->arguments[4], report->arguments[5],
             report->arguments[6], report->arguments[7], report->arguments[8], report->arguments[9], report->arguments[10], report->arguments[11],
             report->arguments[12], report->arguments[13], report->arguments[14], report->arguments[15]);
}

int razer_send_control_msg_old_device(struct hid_device *hdev, const void *data, uint value, uint index, uint size, ulong wait)
{
    struct usb_device *usb_dev = hid_to_usb_dev(hdev);
    int ret;

    // Send usb control message
    ret = usb_control_msg_send(usb_dev,
                               0, // endpoint to send the message to
                               HID_REQ_SET_REPORT, // USB message request value (0x09)
                               USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT, // USB message request type value (0x21)
                               value, // USB message value
                               index, // USB message index value
                               data, // pointer to the data to send
                               size, // length in bytes of the data to send
                               USB_CTRL_SET_TIMEOUT, // time in msecs to wait for the message to complete before timing out
                               GFP_KERNEL);

    // Wait
    fsleep(wait);

    if (ret)
        hid_warn(hdev, "Failed to send USB control message: %d\n", ret);

    return ret;
}

int razer_send_argb_msg(struct hid_device* hdev, unsigned char channel, size_t size, void const* data)
{
    struct usb_device *usb_dev = hid_to_usb_dev(hdev);
    struct razer_argb_report report = {0};
    int ret;

    if (channel < 5) {
        report.report_id = 0x04;
    } else {
        report.report_id = 0x84;
    }

    report.channel_1 = channel;
    report.channel_2 = channel;

    report.pad = 0;

    report.last_idx = size - 1;

    if (size * 3 > ARRAY_SIZE(report.color_data)) {
        hid_err(hdev, "razer driver: size too big\n");
        return -EINVAL;
    }

    memcpy(report.color_data, data, size * 3);

    // Send usb control message
    ret = usb_control_msg_send(usb_dev,
                               0, // endpoint to send the message to
                               HID_REQ_SET_REPORT, // USB message request value (0x09)
                               USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT, // USB message request type value (0x21)
                               0x300, // USB message value
                               0x01, // USB message index value
                               &report, // pointer to the data to send
                               sizeof(report), // length in bytes of the data to send
                               USB_CTRL_SET_TIMEOUT, // time in msecs to wait for the message to complete before timing out
                               GFP_KERNEL);

    if (ret)
        hid_warn(hdev, "Failed to send USB control message: %d\n", ret);

    return ret;
}
