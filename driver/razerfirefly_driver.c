/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <pez2001@voyagerproject.de>, or by paper mail:
 * Tim Theede, Am See 22, 24790 Schuelldorf, Germany
 * Terry Cain <terrys-home.co.uk>
 */


#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

#include "razercommon.h"
#include "razerfirefly_driver.h"


/*
 * Version Information
 */
#define DRIVER_VERSION "0.2"
#define DRIVER_AUTHOR "Tim Theede <pez2001@voyagerproject.de>"
#define DRIVER_DESC "USB HID Razer BlackWidow Chroma"
#define DRIVER_LICENSE "GPL v2"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);

/*

    TODO:

        restore store rgb profile (helpful for event-animations etc)
        #coloritup update

    future todos:

        read keystroke stats etc.

*/

/**
 * Send report to the firefly
 */
int razer_set_report(struct usb_device *usb_dev,void const *data) {
    return razer_send_control_msg(usb_dev, data, 0x00, RAZER_FIREFLY_WAIT_MIN_US, RAZER_FIREFLY_WAIT_MAX_US);
}

int razer_get_report(struct usb_device *usb_dev, struct razer_report *request_report, struct razer_report *response_report) {
    return razer_get_usb_response(usb_dev, 0x00, request_report, 0x00, response_report, RAZER_FIREFLY_WAIT_MIN_US, RAZER_FIREFLY_WAIT_MAX_US);
}

/**
 * Get firmware version
 *
 * Supported by:
 *   Razer BlackWidow Chroma
 *   Razer BlackWidow Ultimate 2013?
 *   Razer BlackWidow Ultimate 2016?
 */
int razer_get_firmware_version(struct usb_device *usb_dev, unsigned char* fw_string)
{
    int retval = -1;
    struct razer_report response_report;
    struct razer_report request_report = get_razer_report(0x00, 0x81, 0x02);
    request_report.crc = razer_calculate_crc(&request_report);

    retval = razer_get_report(usb_dev, &request_report, &response_report);

    if(retval == 0)
    {
        if(response_report.status == 0x02 && response_report.command_class == 0x00 && response_report.command_id.id == 0x81)
        {
            sprintf(fw_string, "v%d.%d", response_report.arguments[0], response_report.arguments[1]);
            retval = response_report.arguments[2];
        } else
        {
            print_erroneous_report(&response_report, "razerkbd", "Invalid Report Type");
        }
    } else
    {
      print_erroneous_report(&response_report, "razerkbd", "Invalid Report Length");
    }

    return retval;
}

/**
 * Get the devices serial number
 *
 * Makes a request like normal, this must change a variable in the mouse as then we
 * tell it give us data (same request for get_battery in the mouse driver) and it 
 * gives us a report.
 */
void razer_get_serial(struct usb_device *usb_dev, unsigned char* serial_string)
{
    struct razer_report response_report;
    struct razer_report request_report = get_razer_report(0x00, 0x82, 0x16);
    int retval;
    int i;

    request_report.crc = razer_calculate_crc(&request_report);
    retval = razer_get_report(usb_dev, &request_report, &response_report);

    if(retval == 0)
    {
        if(response_report.status == 0x02 && response_report.command_class == 0x00 && response_report.command_id.id == 0x82)
        {
            unsigned char* pointer = &response_report.arguments[0];
            for(i = 0; i < 20; ++i)
            {
                serial_string[i] = *pointer;
                ++pointer;
            }
        } else
        {
            print_erroneous_report(&response_report, "razerfirefly", "Invalid Report Type");
        }
    } else
    {
      print_erroneous_report(&response_report, "razerfirefly", "Invalid Report Length");
    }
}

/**
 * Set the wave effect on the firefly
 */
int razer_set_wave_mode(struct usb_device *usb_dev, unsigned char direction)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x02);
    report.arguments[0] = 0x01; // Effect ID
    report.arguments[1] = direction; // Direction
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set no effect on the firefly
 */
int razer_set_none_mode(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);
    report.arguments[0] = 0x00; // Effect ID
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set reactive effect on the firefly
 *
 * The speed must be within 01-03
 *
 * 1 Short, 2 Medium, 3 Long
 *
 * Need to test to see if this actually works. I think the synapse program simulates this.
 */
int razer_set_reactive_mode(struct usb_device *usb_dev, struct razer_rgb *color, unsigned char speed)
{
    int retval = 0;
    if(speed > 0 && speed < 4)
    {
        struct razer_report report = get_razer_report(0x03, 0x0A, 0x05);
        report.arguments[0] = 0x02; // Effect ID
        report.arguments[1] = speed; // Time
        report.arguments[2] = color->r; /*rgb color definition*/
        report.arguments[3] = color->g;
        report.arguments[4] = color->b;
        report.crc = razer_calculate_crc(&report);
        retval = razer_set_report(usb_dev, &report);
    } else
    {
        printk(KERN_WARNING "razerfirefly: Reactive mode, Speed must be within 1-3. Got: %d", speed);
    }
    return retval;
}

/**
 * Set breath effect on the firefly
 *
 * Breathing types
 * 1: Only 1 Colour
 * 2: 2 Colours
 * 3: Random
 */
int razer_set_breath_mode(struct usb_device *usb_dev, unsigned char breathing_type, struct razer_rgb *color1, struct razer_rgb *color2)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x08);
    report.arguments[0] = 0x03; // Effect ID

    report.arguments[1] = breathing_type;

    if(breathing_type == 1 || breathing_type == 2)
    {
        // Colour 1
        report.arguments[2] = color1->r;
        report.arguments[3] = color1->g;
        report.arguments[4] = color1->b;
    }

    if(breathing_type == 2)
    {
        // Colour 2
        report.arguments[5] = color2->r;
        report.arguments[6] = color2->g;
        report.arguments[7] = color2->b;
    }

    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set spectrum effect on the firefly
 */
int razer_set_spectrum_mode(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x01);
    report.arguments[0] = 0x04; // Effect ID
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set custom effect on the keyboard
 */
int razer_set_custom_mode(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x02);
    report.arguments[0] = 0x05; // Effect ID
    report.arguments[1] = 0x00; /*Data frame ID ?*/
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set static effect on the firefly
 */
int razer_set_static_mode(struct usb_device *usb_dev, struct razer_rgb *color)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x04);
    report.arguments[0] = 0x06; // Effect ID
    report.arguments[1] = color->r; /*rgb color definition*/
    report.arguments[2] = color->g;
    report.arguments[3] = color->b;
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Clear row on the firefly
 *
 * Clears a row's colour on the firefly.
 *
 * TODO test and document properly
 */
int razer_temp_clear_row(struct usb_device *usb_dev, unsigned char row_index)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0A, 0x02);
    report.arguments[0] = 0x08; // Clear Row Effect
    report.arguments[1] = row_index; // Row ID
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set colour of LEDs on the firefly
 *
 * This sets the colour of LEDs on the firefly. Takes in an array of RGB bytes.
 *
 * TODO test and document properly
 */
int razer_set_key_row(struct usb_device *usb_dev, unsigned char row_index, unsigned char *row_cols)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x0C, RAZER_FIREFLY_ROW_LEN * 3 + 5);
    report.arguments[0] = 0x00;
    report.arguments[1] = RAZER_FIREFLY_ROW_LEN -1;

    memcpy(&report.arguments[2], row_cols, RAZER_FIREFLY_ROW_LEN * 3);

    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Reset the keyboard
 *
 * TODO test
 */
int razer_reset(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x00, 0x03);
    report.arguments[0] = 0x01; // LED Class, profile?
    report.arguments[1] = 0x08; // LED ID Game mode
    report.arguments[2] = 0x00; // Off
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Set the firefly brightness
 */
int razer_set_brightness(struct usb_device *usb_dev, unsigned char brightness)
{
    int retval;
    struct razer_report report = get_razer_report(0x03, 0x03, 0x03);
    report.arguments[0] = 0x01;/* LED Class, profile*/
    report.arguments[1] = 0x05; // LED ID Backlight LED
    report.arguments[2] = brightness;
    report.crc = razer_calculate_crc(&report);
    retval = razer_set_report(usb_dev, &report);
    return retval;
}

/**
 * Get raw event from the firefly
 *
 * Useful if the keyboard's 2 keyboard devices are binded then keypress's can be
 * monitored and used.
 */
static int razer_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size)
{
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    //struct razer_kbd_device *widow = hid_get_drvdata(hdev);

    if (intf->cur_altsetting->desc.bInterfaceProtocol != USB_INTERFACE_PROTOCOL_MOUSE)
    {
        return 0;
    }

    return 0;
}




/**
 * Write device file "reset"
 *
 * Resets the firefly whenever anything is written
 */
static ssize_t razer_attr_write_reset(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_reset(usb_dev);
    return count;
}

/**
 * Write device file "mode_wave"
 *
 * When 1 is written (as a character, 0x31) the wave effect is displayed moving anti clockwise
 * if 2 is written (0x32) then the wave effect goes clockwise
 */
static ssize_t razer_attr_write_mode_wave(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int temp = simple_strtoul(buf, NULL, 10);
    razer_set_wave_mode(usb_dev, temp);
    return count;
}

/**
 * Write device file "mode_spectrum"
 *
 * Specrum effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_mode_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_set_spectrum_mode(usb_dev);
    return count;
}

/**
 * Write device file "mode_none"
 *
 * No effect is activated whenever this file is written to
 */
static ssize_t razer_attr_write_mode_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_set_none_mode(usb_dev);
    return count;
}

/**
 * Read device file "set_brightness"
 *
 * Returns brightness or -1 if the initial brightness is not known
 */
static ssize_t razer_attr_read_set_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct razer_firefly_device *firefly = usb_get_intfdata(intf);

    return sprintf(buf, "%d\n", firefly->brightness);
}

/**
 * Write device file "set_brightness"
 *
 * Sets the brightness to the ASCII number written to this file.
 */
static ssize_t razer_attr_write_set_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct razer_firefly_device *firefly = usb_get_intfdata(intf);
    struct usb_device *usb_dev = interface_to_usbdev(intf);


    int brightness = simple_strtoul(buf, NULL, 10);
    razer_set_brightness(usb_dev, (unsigned char)brightness);
    firefly->brightness = brightness;

    return count;
}

/**
 * Write device file "mode_reactive"
 *
 * Sets reactive mode when this file is written to. A speed byte and 3 RGB bytes should be written
 */
static ssize_t razer_attr_write_mode_reactive(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    if(count == 4)
    {
        unsigned char speed = (unsigned char)buf[0];
        razer_set_reactive_mode(usb_dev, (struct razer_rgb*)&buf[1], speed);
    }
    return count;
}

/**
 * Write device file "mode_breath"
 */
static ssize_t razer_attr_write_mode_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    const char *alt_buf[6] = { 0 };

    if(count == 3)
    {
        // Single colour mode
        razer_set_breath_mode(usb_dev, 0x01, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&alt_buf[3]);
    } else if(count == 6)
    {
        // Dual colour mode
        razer_set_breath_mode(usb_dev, 0x02, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
    } else
    {
        // "Random" colour mode
        razer_set_breath_mode(usb_dev, 0x03, (struct razer_rgb*)&alt_buf[0], (struct razer_rgb*)&alt_buf[3]);
    }
    return count;
}

/**
 * Write device file "mode_custom"
 *
 * Sets the firefly to custom mode whenever the file is written to
 */
static ssize_t razer_attr_write_mode_custom(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_reset(usb_dev);
    razer_set_custom_mode(usb_dev);
    return count;
}

/**
 * Write device file "mode_static"
 *
 * Set the firefly to static mode when 3 RGB bytes are written
 */
static ssize_t razer_attr_write_mode_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    // Set firefly to static colour
    if(count == 3)
    {
        razer_set_static_mode(usb_dev, (struct razer_rgb*)&buf[0]);
    } else
    {
        printk(KERN_WARNING "razerfirefly: Cannot set static mode for this device, wrong number of bytes recieved");
    }
    return count;
}

/**
 * Write device file "temp_clear_row"
 *
 * Clears a row when an ASCII number is written
 */
static ssize_t razer_attr_write_temp_clear_row(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int temp = simple_strtoul(buf, NULL, 10);
    razer_temp_clear_row(usb_dev, temp);
    return count;
}

/**
 * Write device file "set_key_row"
 *
 * Writes the colour to the LEDs of the firefly
 */
static ssize_t razer_attr_write_set_key_row(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    size_t offset = 0;
    unsigned char row_index;

    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    size_t buf_size = RAZER_FIREFLY_ROW_LEN * 3 + 1;

    while(offset < count) {
        if((count-offset) < buf_size) {
            printk(KERN_ALERT "Wrong Amount of RGB data provided: %d of %d\n",(int)(count-offset), (int)buf_size);
            return -EINVAL;
        }
        row_index = (unsigned char)buf[offset];
        razer_set_key_row(usb_dev, row_index, (unsigned char*)&buf[offset + 1]);
        offset += buf_size;
    }
    return count;
}

/**
 * Read device file "device_type"
 *
 * Returns friendly string of device type
 */
static ssize_t razer_attr_read_device_type(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "Razer Firefly\n");
}

/**
 * Read device file "get_serial"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_get_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    char serial_string[100] = ""; // Can't be longer than this as report length is 90

    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    razer_get_serial(usb_dev, &serial_string[0]);
    return sprintf(buf, "%s\n", &serial_string[0]);
}

/**
 * Read device file "get_firmware_version"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_get_firmware_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    char fw_string[100] = ""; // Cant be longer than this as report length is 90
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    razer_get_firmware_version(usb_dev, &fw_string[0]);
    return sprintf(buf, "%s\n", &fw_string[0]);
}

/**
 * Set up the device driver files
 *
 * Read only is 0444
 * Write only is 0220
 * Read and write is 0664
 */
static DEVICE_ATTR(set_brightness, 0664, razer_attr_read_set_brightness, razer_attr_write_set_brightness);

static DEVICE_ATTR(get_firmware_version, 0444, razer_attr_read_get_firmware_version, NULL);
static DEVICE_ATTR(device_type,          0444, razer_attr_read_device_type,          NULL);
static DEVICE_ATTR(get_serial,           0444, razer_attr_read_get_serial,           NULL);

static DEVICE_ATTR(mode_wave,      0220, NULL, razer_attr_write_mode_wave);
static DEVICE_ATTR(mode_spectrum,  0220, NULL, razer_attr_write_mode_spectrum);
static DEVICE_ATTR(mode_none,      0220, NULL, razer_attr_write_mode_none);
static DEVICE_ATTR(mode_reactive,  0220, NULL, razer_attr_write_mode_reactive);
static DEVICE_ATTR(mode_breath,    0220, NULL, razer_attr_write_mode_breath);
static DEVICE_ATTR(mode_custom,    0220, NULL, razer_attr_write_mode_custom);
static DEVICE_ATTR(mode_static,    0220, NULL, razer_attr_write_mode_static);
static DEVICE_ATTR(temp_clear_row, 0220, NULL, razer_attr_write_temp_clear_row);
static DEVICE_ATTR(set_key_row,    0220, NULL, razer_attr_write_set_key_row);
static DEVICE_ATTR(reset,          0220, NULL, razer_attr_write_reset);



/**
 * Probe method is ran whenever a device is binded to the driver
 *
 * TODO remove goto's
 */
static int razer_firefly_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int retval;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_firefly_device *dev = NULL;

    dev = kzalloc(sizeof(struct razer_firefly_device), GFP_KERNEL);
    if(dev == NULL) {
        dev_err(&intf->dev, "out of memory\n");
        retval = -ENOMEM;
        goto exit;
    }

    dev->brightness = -1;

    retval = device_create_file(&hdev->dev, &dev_attr_mode_wave);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_mode_spectrum);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_mode_none);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_mode_reactive);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_mode_breath);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_mode_custom);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_temp_clear_row);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_set_key_row);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_get_serial);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_get_firmware_version);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_device_type);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_mode_static);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_reset);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_set_brightness);
    if (retval)
        goto exit_free;

    hid_set_drvdata(hdev, dev);


    retval = hid_parse(hdev);
    if(retval)    {
        hid_err(hdev, "parse failed\n");
       goto exit_free;
    }
    retval = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (retval) {
        hid_err(hdev, "hw start failed\n");
       goto exit_free;
    }


    razer_reset(usb_dev);
    usb_disable_autosuspend(usb_dev);
    //razer_activate_macro_keys(usb_dev);
    //msleep(3000);
    return 0;
exit:
    return retval;
exit_free:
    kfree(dev);
    return retval;
}

/**
 * Unbind function
 */
static void razer_firefly_disconnect(struct hid_device *hdev)
{
    struct razer_kbd_device *dev;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    //struct usb_device *usb_dev = interface_to_usbdev(intf);

    dev = hid_get_drvdata(hdev);

    device_remove_file(&hdev->dev, &dev_attr_mode_wave);
    device_remove_file(&hdev->dev, &dev_attr_mode_spectrum);
    device_remove_file(&hdev->dev, &dev_attr_mode_none);
    device_remove_file(&hdev->dev, &dev_attr_mode_reactive);
    device_remove_file(&hdev->dev, &dev_attr_mode_breath);
    device_remove_file(&hdev->dev, &dev_attr_mode_custom);
    device_remove_file(&hdev->dev, &dev_attr_temp_clear_row);
    device_remove_file(&hdev->dev, &dev_attr_set_key_row);
    device_remove_file(&hdev->dev, &dev_attr_get_serial);
    device_remove_file(&hdev->dev, &dev_attr_get_firmware_version);
    device_remove_file(&hdev->dev, &dev_attr_mode_static);
    device_remove_file(&hdev->dev, &dev_attr_reset);
    device_remove_file(&hdev->dev, &dev_attr_set_brightness);
    device_remove_file(&hdev->dev, &dev_attr_device_type);

    hid_hw_stop(hdev);
    kfree(dev);
    dev_info(&intf->dev, "Razer Device disconnected\n");
}

/**
 * Device ID mapping table
 */
static const struct hid_device_id razer_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_FIREFLY) },
    { }
};

MODULE_DEVICE_TABLE(hid, razer_devices);

/**
 * Describes the contents of the driver
 */
static struct hid_driver razer_firefly_driver = {
    .name =        "razerfirefly",
    .id_table =    razer_devices,
    .probe =    razer_firefly_probe,
    .remove =    razer_firefly_disconnect,
    .raw_event = razer_raw_event
};

module_hid_driver(razer_firefly_driver);
