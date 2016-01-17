/*
 * Copyright (c) 2015 Terry Cain <terry@terrys-home.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#ifndef DRIVER_RAZERCOMMON_H_
#define DRIVER_RAZERCOMMON_H_


/* Each USB report has 90 bytes*/
#define RAZER_USB_REPORT_LEN 0x5A


struct razer_rgb {
    unsigned char r,g,b;
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



int razer_send_control_msg(struct usb_device *usb_dev,void const *data, uint report_index, ulong wait_min, ulong wait_max);
int razer_get_usb_response(struct usb_device *usb_dev, uint report_index, struct razer_report* request_report, struct razer_report* response_report, ulong wait_min, ulong wait_max);
unsigned char razer_calculate_crc(struct razer_report *report);
void razer_prepare_report(struct razer_report *report);
void print_erroneous_report(struct razer_report* report, char* driver_name, char* message);










#endif /* DRIVER_RAZERCOMMON_H_ */
