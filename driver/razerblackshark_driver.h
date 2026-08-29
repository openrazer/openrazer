/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __HID_RAZER_BLACKSHARK_H
#define __HID_RAZER_BLACKSHARK_H

#define USB_DEVICE_ID_RAZER_BLACKSHARK_V3_X_USB 0x057C
#define USB_DEVICE_ID_RAZER_BLACKSHARK_V3_X 0x057D

#ifndef USB_INTERFACE_PROTOCOL_NONE
#define USB_INTERFACE_PROTOCOL_NONE 0
#endif

#define RAZER_BLACKSHARK_REPORT_ID 0x07
#define RAZER_BLACKSHARK_REPORT_LEN 64

struct razer_blackshark_device {
    struct hid_device *hdev;
    struct mutex lock;
    u16 usb_pid;
    char serial[24];
    u8 usb_interface_protocol;
    u8 transaction_id;
};

#endif
