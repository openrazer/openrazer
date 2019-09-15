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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 *
 * Should you need to contact me, the author, you can do so by
 * e-mail - mail your message to Terry Cain <terry@terrys-home.co.uk>
 */


#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/random.h>

#include "razermouse_driver.h"
#include "razercommon.h"
#include "razerchromacommon.h"

/*
 * Version Information
 */
#define DRIVER_DESC "Razer Mouse Device Driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE(DRIVER_LICENSE);


/**
 * Send report to the mouse
 */
int razer_get_report(struct usb_device *usb_dev, struct razer_report *request_report, struct razer_report *response_report)
{
    return razer_get_usb_response(usb_dev, 0x00, request_report, 0x00, response_report, RAZER_MOUSE_WAIT_MIN_US, RAZER_MOUSE_WAIT_MAX_US);
}

/**
 * Function to send to device, get response, and actually check the response
 */
struct razer_report razer_send_payload(struct usb_device *usb_dev, struct razer_report *request_report)
{
    int retval = -1;
    struct razer_report response_report = {0};

    request_report->crc = razer_calculate_crc(request_report);

    retval = razer_get_report(usb_dev, request_report, &response_report);

    if(retval == 0) {
        // Check the packet number, class and command are the same
        if(response_report.remaining_packets != request_report->remaining_packets ||
           response_report.command_class != request_report->command_class ||
           response_report.command_id.id != request_report->command_id.id) {
            print_erroneous_report(&response_report, "razermouse", "Response doesn't match request");
//        } else if (response_report.status == RAZER_CMD_BUSY) {
//            print_erroneous_report(&response_report, "razermouse", "Device is busy");
        } else if (response_report.status == RAZER_CMD_FAILURE) {
            print_erroneous_report(&response_report, "razermouse", "Command failed");
        } else if (response_report.status == RAZER_CMD_NOT_SUPPORTED) {
            print_erroneous_report(&response_report, "razermouse", "Command not supported");
        } else if (response_report.status == RAZER_CMD_TIMEOUT) {
            print_erroneous_report(&response_report, "razermouse", "Command timed out");
        }
    } else {
        print_erroneous_report(&response_report, "razermouse", "Invalid Report Length");
    }

    return response_report;
}

/*
 * Specific functions for ancient devices
 *
 */
void deathadder3_5g_set_scroll_led_state(struct razer_mouse_device *device, unsigned int enabled)
{
    if (enabled == 1) {
        device->da3_5g.leds |= 0x02;
    } else {
        device->da3_5g.leds &= ~(0x02);
    }

    mutex_lock(&device->lock);
    razer_send_control_msg_old_device(device->usb_dev, &device->da3_5g, 0x10, 0x00, 4, 3000, 3000);
    mutex_unlock(&device->lock);
}

void deathadder3_5g_set_logo_led_state(struct razer_mouse_device *device, unsigned int enabled)
{
    if (enabled == 1) {
        device->da3_5g.leds |= 0x01;
    } else {
        device->da3_5g.leds &= ~(0x01);
    }

    mutex_lock(&device->lock);
    razer_send_control_msg_old_device(device->usb_dev, &device->da3_5g, 0x10, 0x00, 4, 3000, 3000);
    mutex_unlock(&device->lock);
}

void deathadder3_5g_set_poll_rate(struct razer_mouse_device *device, unsigned short poll_rate)
{
    switch(poll_rate) {
    case 1000:
        device->da3_5g.poll = 1;
        break;
    case 500:
        device->da3_5g.poll = 2;
        break;
    case 125:
        device->da3_5g.poll = 3;
        break;
    default: // 500
        device->da3_5g.poll = 2;
        break;
    }

    mutex_lock(&device->lock);
    razer_send_control_msg_old_device(device->usb_dev, &device->da3_5g, 0x10, 0x00, 4, 3000, 3000);
    mutex_unlock(&device->lock);
}

void deathadder3_5g_set_dpi(struct razer_mouse_device *device, unsigned short dpi)
{
    switch(dpi) {
    case 450:
        device->da3_5g.dpi = 4;
        break;
    case 900:
        device->da3_5g.dpi = 3;
        break;
    case 1800:
        device->da3_5g.dpi = 2;
        break;
    case 3500:
    default:
        device->da3_5g.dpi = 1;
        break;
    }

    mutex_lock(&device->lock);
    razer_send_control_msg_old_device(device->usb_dev, &device->da3_5g, 0x10, 0x00, 4, 3000, 3000);
    mutex_unlock(&device->lock);
}


/*
 * New functions
 */


/**
 * Read device file "version"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", DRIVER_VERSION);
}

/**
 * Read device file "device_type"
 *
 * Returns friendly string of device type
 */
static ssize_t razer_attr_read_device_type(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    char *device_type;

    switch (usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G:
        device_type = "Razer DeathAdder 3.5G\n";
        break;

    case USB_DEVICE_ID_RAZER_MAMBA_2012_WIRED:
        device_type = "Razer Mamba 2012 (Wired)\n";
        break;

    case USB_DEVICE_ID_RAZER_MAMBA_2012_WIRELESS:
        device_type = "Razer Mamba 2012 (Wireless)\n";
        break;

    case USB_DEVICE_ID_RAZER_MAMBA_WIRED:
        device_type = "Razer Mamba (Wired)\n";
        break;

    case USB_DEVICE_ID_RAZER_MAMBA_WIRELESS:
        device_type = "Razer Mamba (Wireless)\n";
        break;

    case USB_DEVICE_ID_RAZER_MAMBA_TE_WIRED:
        device_type = "Razer Mamba Tournament Edition\n";
        break;

    case USB_DEVICE_ID_RAZER_ABYSSUS:
        device_type = "Razer Abyssus 2014\n";
        break;

    case USB_DEVICE_ID_RAZER_ABYSSUS_1800:
        device_type = "Razer Abyssus 1800\n";
        break;

    case USB_DEVICE_ID_RAZER_ABYSSUS_2000:
        device_type = "Razer Abyssus 2000\n";
        break;

    case USB_DEVICE_ID_RAZER_IMPERATOR:
        device_type = "Razer Imperator 2012\n";
        break;

    case USB_DEVICE_ID_RAZER_OUROBOROS:
        device_type = "Razer Ouroboros\n";
        break;

    case USB_DEVICE_ID_RAZER_OROCHI_2011:
        device_type = "Razer Orochi 2011\n";
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_2013:
        device_type = "Razer DeathAdder 2013\n";
        break;

    case USB_DEVICE_ID_RAZER_OROCHI_2013:
        device_type = "Razer Orochi 2013\n";
        break;

    case USB_DEVICE_ID_RAZER_OROCHI_CHROMA:
        device_type = "Razer Orochi (Wired)\n";
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_CHROMA:
        device_type = "Razer DeathAdder Chroma\n";
        break;

    case USB_DEVICE_ID_RAZER_NAGA_HEX_RED:
        device_type = "Razer Naga Hex (Red)\n";
        break;

    case USB_DEVICE_ID_RAZER_NAGA_HEX:
        device_type = "Razer Naga Hex\n";
        break;

    case USB_DEVICE_ID_RAZER_NAGA_2014:
        device_type = "Razer Naga 2014\n";
        break;

    case USB_DEVICE_ID_RAZER_TAIPAN:
        device_type = "Razer Taipan\n";
        break;

    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        device_type = "Razer Naga Hex V2\n";
        break;

    case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
        device_type = "Razer Naga Chroma\n";
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
        device_type = "Razer DeathAdder Elite\n";
        break;

    case USB_DEVICE_ID_RAZER_ABYSSUS_V2:
        device_type = "Razer Abyssus V2\n";
        break;

    case USB_DEVICE_ID_RAZER_DIAMONDBACK_CHROMA:
        device_type = "Razer Diamondback Chroma\n";
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_3500:
        device_type = "Razer DeathAdder 3500\n";
        break;

    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        device_type = "Razer Lancehead Tournament Edition\n";
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
        device_type = "Razer Deathadder Essential\n";
        break;

    case USB_DEVICE_ID_RAZER_NAGA_TRINITY:
        device_type = "Razer Naga Trinity\n";
        break;

    default:
        device_type = "Unknown Device\n";
    }

    return sprintf(buf, device_type);
}

/**
 * Read device file "get_firmware_version"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_get_firmware_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    struct razer_report report = razer_chroma_standard_get_firmware_version();
    struct razer_report response_report = {0};

    switch(device->usb_pid) {
    case USB_DEVICE_ID_RAZER_OROCHI_2011:  // Orochi 2011 doesn't have FW
        return sprintf(buf, "v%d.%d\n", 9, 99);
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G: // DA don't think supports fw, its proper old
        return sprintf(buf, "v%d.%d\n", 0x01, 0x00);
        break;

    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report.transaction_id.id = 0x3f;
        break;
    }

    mutex_lock(&device->lock);
    response_report = razer_send_payload(device->usb_dev, &report);
    mutex_unlock(&device->lock);

    return sprintf(buf, "v%d.%d\n", response_report.arguments[0], response_report.arguments[1]);
}

/**
 * Write device file "test"
 *
 * Writes the colour segments on the mouse.
 */
static ssize_t razer_attr_write_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_standard_set_led_state(VARSTORE, LOGO_LED, enabled);

    razer_send_payload(usb_dev, &report);

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
    struct razer_report report = {0};

    switch (usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
    case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
        report = razer_chroma_mouse_extended_matrix_effect_none(VARSTORE, BACKLIGHT_LED);
        break;

    default:
        report = razer_chroma_standard_matrix_effect_none(VARSTORE, BACKLIGHT_LED);
        break;
    }

    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "mode_custom"
 *
 * Sets the mouse to custom mode whenever the file is written to
 */
static ssize_t razer_attr_write_mode_custom(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = razer_chroma_standard_matrix_effect_custom_frame(NOSTORE);

    switch (usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2: // TODO look into this think its extended effects
        report = razer_chroma_standard_matrix_effect_custom_frame(NOSTORE);
        report.transaction_id.id = 0x3f;
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_effect_custom_frame();
        break;

    case USB_DEVICE_ID_RAZER_MAMBA_WIRELESS:
        report = razer_chroma_standard_matrix_effect_custom_frame(NOSTORE);
        report.transaction_id.id = 0x80;
        break;
    }

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "mode_static"
 *
 * Set the mouse to static mode when 3 RGB bytes are written
 */
static ssize_t razer_attr_write_mode_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    if(count == 3) {
        switch (usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
            report = razer_chroma_mouse_extended_matrix_effect_static(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0]);
            break;

        case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
            report = razer_chroma_mouse_extended_matrix_effect_static(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0]);
            report.transaction_id.id = 0xff;
            break;

        default:
            report = razer_chroma_standard_matrix_effect_static(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0]);
            break;
        }

        razer_send_payload(usb_dev, &report);
    } else {
        printk(KERN_WARNING "razermouse: Static mode only accepts RGB (3byte)");
    }

    return count;
}

/**
 * Write device file "mode_wave"
 *
 * When 1 is written (as a character, 0x31) the wave effect is displayed moving up the mouse
 * if 2 is written (0x32) then the wave effect goes down
 */
static ssize_t razer_attr_write_mode_wave(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char direction = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_standard_matrix_effect_wave(VARSTORE, BACKLIGHT_LED, direction);

    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "mode_spectrum"
 *
 * Spectrum effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_mode_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    switch (usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
    case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
        report = razer_chroma_mouse_extended_matrix_effect_spectrum(VARSTORE, BACKLIGHT_LED);
        break;

    default:
        report = razer_chroma_standard_matrix_effect_spectrum(VARSTORE, BACKLIGHT_LED);
        break;
    }

    razer_send_payload(usb_dev, &report);

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
    struct razer_report report = {0};

    if(count == 4) {
        unsigned char speed = (unsigned char)buf[0];

        switch (usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
            report = razer_chroma_mouse_extended_matrix_effect_reactive(VARSTORE, BACKLIGHT_LED, speed, (struct razer_rgb*)&buf[1]);
            break;

        default:
            report = razer_chroma_standard_matrix_effect_reactive(VARSTORE, BACKLIGHT_LED, speed, (struct razer_rgb*)&buf[1]);
            break;
        }

        razer_send_payload(usb_dev, &report);

    } else {
        printk(KERN_WARNING "razermouse: Reactive only accepts Speed, RGB (4byte)");
    }
    return count;
}

/**
 * Write device file "mode_breath"
 *
 * Sets breathing mode by writing 1, 3 or 6 bytes
 */
static ssize_t razer_attr_write_mode_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    switch (usb_dev->descriptor.idProduct) { // TODO refactor to have 2 methods to split out the breathing crap
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
    case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
        switch(count) {
        case 3: // Single colour mode
            report = razer_chroma_mouse_extended_matrix_effect_breathing_single(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0]);
            break;

        case 6: // Dual colour mode
            report = razer_chroma_mouse_extended_matrix_effect_breathing_dual(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
            break;

        default: // "Random" colour mode
            report = razer_chroma_mouse_extended_matrix_effect_breathing_random(VARSTORE, BACKLIGHT_LED);
            break;
        }
        break;

    default:
        switch(count) {
        case 3: // Single colour mode
            report = razer_chroma_standard_matrix_effect_breathing_single(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0]);
            break;

        case 6: // Dual colour mode
            report = razer_chroma_standard_matrix_effect_breathing_dual(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
            break;

        default: // "Random" colour mode
            report = razer_chroma_standard_matrix_effect_breathing_random(VARSTORE, BACKLIGHT_LED);
            break;
        }
        break;
    }

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Read device file "get_serial"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_get_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    char serial_string[23];
    struct razer_report report = razer_chroma_standard_get_serial();
    struct razer_report response_report = {0};

    switch(device->usb_pid) {
    case USB_DEVICE_ID_RAZER_OROCHI_2011:
    case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G:
    case USB_DEVICE_ID_RAZER_MAMBA_2012_WIRED: // Doesn't have proper serial
    case USB_DEVICE_ID_RAZER_MAMBA_2012_WIRELESS:
        return sprintf(buf, "%s\n", &device->serial[0]);
        break;

    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report.transaction_id.id = 0x3f;
        break;
    }

    mutex_lock(&device->lock);
    response_report = razer_send_payload(device->usb_dev, &report);
    strncpy(&serial_string[0], &response_report.arguments[0], 22);
    serial_string[22] = '\0';
    mutex_unlock(&device->lock);

    return sprintf(buf, "%s\n", &serial_string[0]);
}

/**
 * Read device file "get_battery"
 *
 * Returns an integer which needs to be scaled from 0-255 -> 0-100
 */
static ssize_t razer_attr_read_get_battery(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = razer_chroma_misc_get_battery_level();
    struct razer_report response_report = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response_report.arguments[1]);
}

/**
 * Read device file "is_charging"
 *
 * Returns 0 when not charging, 1 when charging
 */
static ssize_t razer_attr_read_is_charging(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = razer_chroma_misc_get_charging_status();
    struct razer_report response_report = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response_report.arguments[1]);
}

/**
 * Write device file "set_charging_effect"
 *
 * Sets charging effect.
 */
static ssize_t razer_attr_write_set_charging_effect(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    if(count == 1) {
        report = razer_chroma_misc_set_dock_charge_type(buf[0]);
        razer_send_payload(usb_dev, &report);
    } else {
        printk(KERN_WARNING "razermouse: Incorrect number of bytes for setting the charging effect\n");
    }
    return count;
}

/**
 * Write device file "set_charging_colour"
 *
 * Sets charging colour using 3 RGB bytes
 */
static ssize_t razer_attr_write_set_charging_colour(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    // First enable static charging effect
    struct razer_report report = razer_chroma_misc_set_dock_charge_type(0x01);
    razer_send_payload(usb_dev, &report);


    if(count == 3) {
        report = razer_chroma_standard_set_led_rgb(NOSTORE, BATTERY_LED, (struct razer_rgb*)&buf[0]);
        razer_send_payload(usb_dev, &report);
    } else {
        printk(KERN_WARNING "razermouse: Charging colour mode only accepts RGB (3byte)");
    }

    return count;
}

/**
 * Read device file "poll_rate"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_poll_rate(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    struct razer_report report = razer_chroma_misc_get_polling_rate();
    struct razer_report response_report = {0};
    unsigned short polling_rate = 0;

    switch(device->usb_pid) {
    case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G:
        switch(device->da3_5g.poll) {
        case 0x01:
            polling_rate = 1000;
            break;
        case 0x02:
            polling_rate = 500;
            break;
        case 0x03:
            polling_rate = 125;
            break;
        }
        return sprintf(buf, "%d\n", polling_rate);
        break;

    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report.transaction_id.id = 0x3f;
        break;
    }

    if(device->usb_pid == USB_DEVICE_ID_RAZER_OROCHI_2011) {
        response_report.arguments[0] = device->orochi2011_poll;
    } else {
        mutex_lock(&device->lock);
        response_report = razer_send_payload(device->usb_dev, &report);
        mutex_unlock(&device->lock);
    }

    switch(response_report.arguments[0]) {
    case 0x01:
        polling_rate = 1000;
        break;
    case  0x02:
        polling_rate = 500;
        break;
    case  0x08:
        polling_rate = 125;
        break;
    }

    return sprintf(buf, "%d\n", polling_rate);
}

/**
 * Write device file "poll_rate"
 *
 * Sets the poll rate
 */
static ssize_t razer_attr_write_poll_rate(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    unsigned short polling_rate = (unsigned short)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_misc_set_polling_rate(polling_rate);

    switch(device->usb_pid) {
    case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G:
        deathadder3_5g_set_poll_rate(device, polling_rate);
        return count;

    case USB_DEVICE_ID_RAZER_OROCHI_2011:
        device->orochi2011_poll = polling_rate;
        report = razer_chroma_misc_set_orochi2011_poll_dpi(device->orochi2011_poll, device->orochi2011_dpi, device->orochi2011_dpi);
        break;

    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
    case USB_DEVICE_ID_RAZER_MAMBA_WIRED:
    case USB_DEVICE_ID_RAZER_MAMBA_WIRELESS:
        report.transaction_id.id = 0x3f;
        break;
    }

    mutex_lock(&device->lock);
    razer_send_payload(device->usb_dev, &report);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Write device file "matrix_brightness"
 *
 * Sets the brightness to the ASCII number written to this file.
 */

static ssize_t razer_attr_write_matrix_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char brightness = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_MAMBA_WIRELESS:
        report = razer_chroma_misc_set_dock_brightness(brightness);
        break;

    case USB_DEVICE_ID_RAZER_OROCHI_CHROMA:
        // Orochi sets brightness of scroll wheel apparently
        report = razer_chroma_standard_set_led_brightness(VARSTORE, SCROLL_WHEEL_LED, brightness);
        break;

    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        report = razer_chroma_standard_set_led_brightness(VARSTORE, BACKLIGHT_LED, brightness);
        report.transaction_id.id = 0x3f;
        break;

    case USB_DEVICE_ID_RAZER_NAGA_TRINITY:
        // Naga Trinity uses the LED 0x00 and Matrix Brightness
        report = razer_chroma_extended_matrix_brightness(VARSTORE, 0x00, brightness);
        break;

    default:
        report = razer_chroma_standard_set_led_brightness(VARSTORE, BACKLIGHT_LED, brightness);
        break;
    }
    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Read device file "matrix_brightness"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_matrix_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};
    struct razer_report response = {0};
    unsigned char brightness_index = 0x02;

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_MAMBA_WIRELESS:
        report = razer_chroma_misc_get_dock_brightness();
        brightness_index = 0x00;
        break;

    case USB_DEVICE_ID_RAZER_OROCHI_CHROMA:
        // Orochi sets brightness of scroll wheel apparently
        report = razer_chroma_standard_get_led_brightness(VARSTORE, SCROLL_WHEEL_LED);
        break;

    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        // Orochi sets brightness of scroll wheel apparently
        report = razer_chroma_standard_get_led_brightness(VARSTORE, BACKLIGHT_LED);
        report.transaction_id.id = 0x3f;
        break;

    case USB_DEVICE_ID_RAZER_NAGA_TRINITY:
        // Naga Trinity uses the LED 0x00 and Matrix Brightness
        report = razer_chroma_extended_matrix_get_brightness(VARSTORE, 0x00);
        break;

    default:
        report = razer_chroma_standard_get_led_brightness(VARSTORE, BACKLIGHT_LED);
        break;
    }
    response = razer_send_payload(usb_dev, &report);

    // Brightness is at arg[0] for dock and arg[1] for led_brightness
    return sprintf(buf, "%d\n", response.arguments[brightness_index]);
}

/**
 * Write device file "set_mouse_dpi"
 *
 * Sets the mouse DPI to the unsigned short integer written to this file.
 */
static ssize_t razer_attr_write_mouse_dpi(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    struct razer_report report = {0};
    unsigned short dpi_x;
    unsigned short dpi_y;
    unsigned char dpi_x_byte;
    unsigned char dpi_y_byte;
    unsigned char varstore;


    // So far I think imperator uses varstore
    switch(device->usb_pid) {
    case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G:
        if(count == 2) {
            dpi_x = (buf[0] << 8) | (buf[1] & 0xFF); // TODO make convenience function
            deathadder3_5g_set_dpi(device, dpi_x);
        } else {
            printk(KERN_WARNING "razermouse: DPI requires 2 bytes\n");
        }
        return count;
        break;

    // Damn naga hex only uses 1 byte per x, y dpi
    case USB_DEVICE_ID_RAZER_NAGA_HEX_RED:
    case USB_DEVICE_ID_RAZER_NAGA_HEX:
    case USB_DEVICE_ID_RAZER_ABYSSUS_1800:
    case USB_DEVICE_ID_RAZER_DEATHADDER_2013:
        if(count == 1) {
            dpi_x_byte = buf[0];
            dpi_y_byte = buf[0];
        } else if (count == 2) {
            dpi_x_byte = buf[0];
            dpi_y_byte = buf[1];
        } else {
            printk(KERN_WARNING "razermouse: DPI requires 1 byte or 2 bytes\n");
            return count;
        }

        report = razer_chroma_misc_set_dpi_xy_byte(dpi_x_byte, dpi_y_byte);
        razer_send_payload(device->usb_dev, &report);
        return count;
        break;

    case USB_DEVICE_ID_RAZER_OROCHI_2011:
        if(count == 1) {
            dpi_x_byte = buf[0];
            dpi_y_byte = buf[0];
        } else if (count == 2) {
            dpi_x_byte = buf[0];
            dpi_y_byte = buf[1];
        } else {
            printk(KERN_WARNING "razermouse: DPI requires 1 byte or 2 bytes\n");
            return count;
        }
        device->orochi2011_dpi = dpi_x_byte;

        report = razer_chroma_misc_set_orochi2011_poll_dpi(device->orochi2011_poll, dpi_x_byte, dpi_y_byte);
        razer_send_payload(device->usb_dev, &report);
        return count;
        break;

    case USB_DEVICE_ID_RAZER_OROCHI_2013:
    case USB_DEVICE_ID_RAZER_IMPERATOR:
        varstore = VARSTORE;
        break;
    default:
        varstore = NOSTORE;
        break;
    }


    if(count != 2 && count != 4) {
        printk(KERN_WARNING "razermouse: DPI requires 2 bytes or 4 bytes\n");
    } else {

        if(count == 2) {
            dpi_x = (buf[0] << 8) | (buf[1] & 0xFF); // TODO make convenience function
            report = razer_chroma_misc_set_dpi_xy(varstore, dpi_x, dpi_x);

        } else if(count == 4) {
            dpi_x = (buf[0] << 8) | (buf[1] & 0xFF); // Apparently the char buffer is rubbish, as buf[1] somehow can equal FFFFFF80????
            dpi_y = (buf[2] << 8) | (buf[3] & 0xFF);

            report = razer_chroma_misc_set_dpi_xy(varstore, dpi_x, dpi_y);
        }

        switch(device->usb_pid) { // New devices set the device ID properly
        case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
            report.transaction_id.id = 0x3f;
            break;
        }

        razer_send_payload(device->usb_dev, &report);
    }

    return count;
}

/**
 * Read device file "dpi"
 *
 * Gets the mouse DPI to the unsigned short integer written to this file.
 */
static ssize_t razer_attr_read_mouse_dpi(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    struct razer_report report = {0};
    struct razer_report response = {0};
    unsigned short dpi_x;
    unsigned short dpi_y;

    // So far I think imperator uses varstore
    switch(device->usb_pid) {
    case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G:
        switch(device->da3_5g.dpi) {
        case 0x04:
            dpi_x = 450;
            break;
        case 0x03:
            dpi_x = 900;
            break;
        case 0x02:
            dpi_x = 1800;
            break;
        case 0x01:
        default:
            dpi_x = 3500;
            break;
        }
        return sprintf(buf, "%u\n", dpi_x);
        break;

    case USB_DEVICE_ID_RAZER_OROCHI_2011:
        return sprintf(buf, "%u:%u\n", device->orochi2011_dpi, device->orochi2011_dpi);
        break;

    case USB_DEVICE_ID_RAZER_NAGA_HEX_RED:
    case USB_DEVICE_ID_RAZER_NAGA_HEX:
    case USB_DEVICE_ID_RAZER_ABYSSUS_1800:
    case USB_DEVICE_ID_RAZER_DEATHADDER_2013:
        report = razer_chroma_misc_get_dpi_xy_byte();
        break;

    case USB_DEVICE_ID_RAZER_OROCHI_2013:
    case USB_DEVICE_ID_RAZER_IMPERATOR:
        report = razer_chroma_misc_get_dpi_xy(VARSTORE);
        break;

    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_misc_get_dpi_xy(NOSTORE);
        report.transaction_id.id = 0x3f;

    default:
        report = razer_chroma_misc_get_dpi_xy(NOSTORE);
        break;
    }

    response = razer_send_payload(device->usb_dev, &report);

    // Byte, Byte for DPI not Short, Short
    if (device->usb_pid == USB_DEVICE_ID_RAZER_NAGA_HEX ||
        device->usb_pid == USB_DEVICE_ID_RAZER_NAGA_HEX_RED ||
        device->usb_pid == USB_DEVICE_ID_RAZER_DEATHADDER_2013 ||
        device->usb_pid == USB_DEVICE_ID_RAZER_ABYSSUS_1800) { // NagaHex is crap uses only byte for dpi
        dpi_x = response.arguments[0];
        dpi_y = response.arguments[1];
    } else {
        dpi_x = (response.arguments[1] << 8) | (response.arguments[2] & 0xFF); // Apparently the char buffer is rubbish, as buf[1] somehow can equal FFFFFF80????
        dpi_y = (response.arguments[3] << 8) | (response.arguments[4] & 0xFF);
    }

    return sprintf(buf, "%u:%u\n", dpi_x, dpi_y);
}

/**
 * Write device file "set_idle_time"
 *
 * Sets the idle time to the ASCII number written to this file.
 */
static ssize_t razer_attr_write_set_idle_time(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned short idle_time = (unsigned short)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_misc_set_idle_time(idle_time);

    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "set_low_battery_threshold"
 *
 * Sets the low battery blink threshold to the ASCII number written to this file.
 */
static ssize_t razer_attr_write_set_low_battery_threshold(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char threshold = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_misc_set_low_battery_threshold(threshold);

    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "set_key_row"
 *
 * Writes the colour segments on the mouse.
 */
static ssize_t razer_attr_write_set_key_row(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    struct razer_report report = {0};
    size_t offset = 0;
    unsigned char row_id;
    unsigned char start_col;
    unsigned char stop_col;
    unsigned char row_length;

    //printk(KERN_ALERT "razermouse: Total count: %d\n", (unsigned char)count);

    while(offset < count) {
        if(offset + 3 > count) {
            printk(KERN_ALERT "razermouse: Wrong Amount of data provided: Should be ROW_ID, START_COL, STOP_COL, N_RGB\n");
            break;
        }

        row_id = buf[offset++];
        start_col = buf[offset++];
        stop_col = buf[offset++];
        row_length = ((stop_col+1) - start_col) * 3;

        // printk(KERN_ALERT "razermouse: Row ID: %d, Start: %d, Stop: %d, row length: %d\n", row_id, start_col, stop_col, row_length);

        // Mouse only has 1 row, row0 (pseudo row as the command actually doesn't take rows)
        if(row_id != 0) {
            printk(KERN_ALERT "razermouse: Row ID must be 0\n");
            break;
        }

        if(start_col > stop_col) {
            printk(KERN_ALERT "razermouse: Start column is greater than end column\n");
            break;
        }

        if(offset + row_length > count) {
            printk(KERN_ALERT "razermouse: Not enough RGB to fill row\n");
            break;
        }

        // Offset now at beginning of RGB data
        switch(usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
            report = razer_chroma_standard_matrix_set_custom_frame(row_id, start_col, stop_col, (unsigned char*)&buf[offset]);
            report.transaction_id.id = 0x3f;
            break;

        case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
        case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
            report = razer_chroma_extended_matrix_set_custom_frame(row_id, start_col, stop_col, (unsigned char*)&buf[offset]);
            break;

        case USB_DEVICE_ID_RAZER_MAMBA_WIRED:
        case USB_DEVICE_ID_RAZER_MAMBA_WIRELESS:
            report = razer_chroma_misc_one_row_set_custom_frame(start_col, stop_col, (unsigned char*)&buf[offset]);
            report.transaction_id.id = 0x80;
            break;

        case USB_DEVICE_ID_RAZER_MAMBA_TE_WIRED:
        case USB_DEVICE_ID_RAZER_DIAMONDBACK_CHROMA:
            report = razer_chroma_misc_one_row_set_custom_frame(start_col, stop_col, (unsigned char*)&buf[offset]);
            break;
        }
        razer_send_payload(usb_dev, &report);

        // *3 as its 3 bytes per col (RGB)
        offset += row_length;
    }


    return count;
}

/**
 * Write device file "device_mode"
 */
static ssize_t razer_attr_write_device_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    struct razer_report report = {0};

    if(count == 2) {
        report = razer_chroma_standard_set_device_mode(buf[0], buf[1]);

        switch(device->usb_pid) {
        case USB_DEVICE_ID_RAZER_OROCHI_2011:  // Doesn't have device mode
        case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G: // Doesn't support device mode, exit early
            return count;
            break;
        case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
            report.transaction_id.id = 0x3f;
            break;
        }

        mutex_lock(&device->lock);
        razer_send_payload(device->usb_dev, &report);
        mutex_unlock(&device->lock);
    } else {
        printk(KERN_WARNING "razerkbd: Device mode only takes 2 bytes.");
    }

    return count;
}

/**
 * Read device file "device_mode"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_device_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    struct razer_report report = razer_chroma_standard_get_device_mode();
    struct razer_report response = {0};

    switch(device->usb_pid) {
    case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G: // Doesn't support device mode, exit early
    case USB_DEVICE_ID_RAZER_OROCHI_2011:
        return sprintf(buf, "%d:%d\n", 0, 0);
        break;

    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report.transaction_id.id = 0x3f;
        break;
    }

    mutex_lock(&device->lock);
    response = razer_send_payload(device->usb_dev, &report);
    mutex_unlock(&device->lock);

    return sprintf(buf, "%d:%d\n", response.arguments[0], response.arguments[1]);
}

/**
 * Read device file "scroll_led_brightness"
 */
static ssize_t razer_attr_read_scroll_led_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = razer_chroma_standard_get_led_brightness(VARSTORE, SCROLL_WHEEL_LED);
    struct razer_report response = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        report = razer_chroma_standard_get_led_brightness(VARSTORE, SCROLL_WHEEL_LED);
        report.transaction_id.id = 0x3F;
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
        report = razer_chroma_extended_matrix_get_brightness(VARSTORE, SCROLL_WHEEL_LED);
        break;

    default:
        report = razer_chroma_standard_get_led_brightness(VARSTORE, SCROLL_WHEEL_LED);
        break;
    }

    response = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "scroll_led_brightness"
 */
static ssize_t razer_attr_write_scroll_led_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char brightness = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        report = razer_chroma_standard_set_led_brightness(VARSTORE, SCROLL_WHEEL_LED, brightness);
        report.transaction_id.id = 0x3F;
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
        report = razer_chroma_extended_matrix_brightness(VARSTORE, SCROLL_WHEEL_LED, brightness);
        break;

    default:
        report = razer_chroma_standard_set_led_brightness(VARSTORE, SCROLL_WHEEL_LED, brightness);
        break;
    }

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Read device file "logo_led_brightness"
 */
static ssize_t razer_attr_read_logo_led_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};
    struct razer_report response = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        report = razer_chroma_standard_get_led_brightness(VARSTORE, LOGO_LED);
        report.transaction_id.id = 0x3F;
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
        report = razer_chroma_extended_matrix_get_brightness(VARSTORE, LOGO_LED);
        break;

    default:
        report = razer_chroma_standard_get_led_brightness(VARSTORE, LOGO_LED);
        break;
    }

    response = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "logo_led_brightness"
 */
static ssize_t razer_attr_write_logo_led_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char brightness = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        report = razer_chroma_standard_set_led_brightness(VARSTORE, LOGO_LED, brightness);
        report.transaction_id.id = 0x3F;
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
        report = razer_chroma_extended_matrix_brightness(VARSTORE, LOGO_LED, brightness);
        break;

    default:
        report = razer_chroma_standard_set_led_brightness(VARSTORE, LOGO_LED, brightness);
        break;
    }

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Read device file "left_led_brightness"
 */
static ssize_t razer_attr_read_left_led_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};
    struct razer_report response = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_get_brightness(VARSTORE, LEFT_SIDE_LED);
        break;

    default:
        report = razer_chroma_standard_get_led_brightness(VARSTORE, LEFT_SIDE_LED);
        break;
    }

    response = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "left_led_brightness"
 */
static ssize_t razer_attr_write_left_led_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char brightness = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_brightness(VARSTORE, LEFT_SIDE_LED, brightness);
        break;

    default:
        report = razer_chroma_standard_set_led_brightness(VARSTORE, LEFT_SIDE_LED, brightness);
        break;
    }

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Read device file "right_led_brightness"
 */
static ssize_t razer_attr_read_right_led_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};
    struct razer_report response = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_get_brightness(VARSTORE, RIGHT_SIDE_LED);
        break;

    default:
        report = razer_chroma_standard_get_led_brightness(VARSTORE, RIGHT_SIDE_LED);
        break;
    }

    response = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "right_led_brightness"
 */
static ssize_t razer_attr_write_right_led_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char brightness = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_brightness(VARSTORE, RIGHT_SIDE_LED, brightness);
        break;

    default:
        report = razer_chroma_standard_set_led_brightness(VARSTORE, RIGHT_SIDE_LED, brightness);
        break;
    }

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "scroll_led_state"
 */
static ssize_t razer_attr_write_scroll_led_state(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_standard_set_led_state(VARSTORE, SCROLL_WHEEL_LED, enabled);
    report.transaction_id.id = 0x3F;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_OROCHI_2011:
        if(enabled) {
            device->orochi2011_led |= 0b00000001;
        } else {
            device->orochi2011_led &= 0b11111110;
        }
        report = razer_chroma_misc_set_orochi2011_led(device->orochi2011_led);
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G:
        deathadder3_5g_set_scroll_led_state(device, enabled);
        return count;
        break;
    }

    mutex_lock(&device->lock);
    razer_send_payload(device->usb_dev, &report);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Read device file "scroll_led_state"
 */
static ssize_t razer_attr_read_scroll_led_state(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    struct razer_report report = razer_chroma_standard_get_led_state(VARSTORE, SCROLL_WHEEL_LED);
    struct razer_report response = {0};
    report.transaction_id.id = 0x3F;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_OROCHI_2011:
        return sprintf(buf, "%d\n", device->orochi2011_led & 0b00000001);
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G:
        if((device->da3_5g.leds & 0x02) == 0x02) {
            return sprintf(buf, "1\n");
        }
        return sprintf(buf, "0\n");
        break;

    default:
        mutex_lock(&device->lock);
        response = razer_send_payload(device->usb_dev, &report);
        mutex_unlock(&device->lock);
        break;
    }

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "logo_led_state"
 */
static ssize_t razer_attr_write_logo_led_state(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_standard_set_led_state(VARSTORE, LOGO_LED, enabled);
    report.transaction_id.id = 0x3F;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_OROCHI_2011:
        if(enabled) {
            device->orochi2011_led |= 0b00000010;
        } else {
            device->orochi2011_led &= 0b11111101;
        }
        report = razer_chroma_misc_set_orochi2011_led(device->orochi2011_led);
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G:
        deathadder3_5g_set_logo_led_state(device, enabled);
        return count;
        break;
    }

    mutex_lock(&device->lock);
    razer_send_payload(device->usb_dev, &report);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Read device file "logo_led_state"
 */
static ssize_t razer_attr_read_logo_led_state(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    struct razer_report report = razer_chroma_standard_get_led_state(VARSTORE, LOGO_LED);
    struct razer_report response = {0};
    report.transaction_id.id = 0x3F;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_OROCHI_2011:
        return sprintf(buf, "%d\n", (device->orochi2011_led & 0b00000010) >> 1);
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G:
        if((device->da3_5g.leds & 0x01) == 0x01) {
            return sprintf(buf, "1\n");
        }
        return sprintf(buf, "0\n");
        break;

    default:
        mutex_lock(&device->lock);
        response = razer_send_payload(device->usb_dev, &report);
        mutex_unlock(&device->lock);
        break;
    }

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "scroll_led_rgb"
 */
static ssize_t razer_attr_write_scroll_led_rgb(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    if(count == 3) {
        report = razer_chroma_standard_set_led_rgb(VARSTORE, SCROLL_WHEEL_LED, (struct razer_rgb*)&buf[0]);
        report.transaction_id.id = 0x3F;
        razer_send_payload(usb_dev, &report);
    } else {
        printk(KERN_WARNING "razermouse: Scroll wheel LED mode only accepts RGB (3byte)");
    }

    return count;
}

/**
 * Read device file "scroll_led_rgb"
 */
static ssize_t razer_attr_read_scroll_led_rgb(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = razer_chroma_standard_get_led_rgb(VARSTORE, SCROLL_WHEEL_LED);
    struct razer_report response = {0};
    report.transaction_id.id = 0x3F;
    response = razer_send_payload(usb_dev, &report);


    return sprintf(buf, "%u%u%u\n", response.arguments[2], response.arguments[3], response.arguments[4]);
}

/**
 * Write device file "logo_led_rgb"
 */
static ssize_t razer_attr_write_logo_led_rgb(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    if(count == 3) {
        report = razer_chroma_standard_set_led_rgb(VARSTORE, LOGO_LED, (struct razer_rgb*)&buf[0]);
        report.transaction_id.id = 0x3F;
        razer_send_payload(usb_dev, &report);
    } else {
        printk(KERN_WARNING "razermouse: Logo LED mode only accepts RGB (3byte)");
    }

    return count;
}

/**
 * Read device file "logo_led_rgb"
 */
static ssize_t razer_attr_read_logo_led_rgb(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = razer_chroma_standard_get_led_rgb(VARSTORE, LOGO_LED);
    struct razer_report response = {0};

    report.transaction_id.id = 0x3F;
    response = razer_send_payload(usb_dev, &report);


    return sprintf(buf, "%u%u%u\n", response.arguments[2], response.arguments[3], response.arguments[4]);
}

/**
 * Write device file "scroll_led_effect"
 */
static ssize_t razer_attr_write_scroll_led_effect(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char effect = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_standard_set_led_effect(VARSTORE, SCROLL_WHEEL_LED, effect);
    report.transaction_id.id = 0x3F;

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Read device file "scroll_led_effect"
 */
static ssize_t razer_attr_read_scroll_led_effect(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = razer_chroma_standard_get_led_effect(VARSTORE, SCROLL_WHEEL_LED);
    struct razer_report response = {0};
    report.transaction_id.id = 0x3F;
    response = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "logo_led_effect"
 */
static ssize_t razer_attr_write_logo_led_effect(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char effect = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_standard_set_led_effect(VARSTORE, LOGO_LED, effect);
    report.transaction_id.id = 0x3F;

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Read device file "logo_led_effect"
 */
static ssize_t razer_attr_read_logo_led_effect(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = razer_chroma_standard_get_led_effect(VARSTORE, LOGO_LED);
    struct razer_report response = {0};

    report.transaction_id.id = 0x3F;
    response = razer_send_payload(usb_dev, &report);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "scroll_mode_wave" (for extended mouse matrix effects)
 *
 * Wave effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_scroll_mode_wave(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char direction = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_effect_wave(VARSTORE, SCROLL_WHEEL_LED, direction);
        break;
    }

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "scroll_mode_spectrum" (for extended mouse matrix effects)
 *
 * Spectrum effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_scroll_mode_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
    case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
        report = razer_chroma_mouse_extended_matrix_effect_spectrum(VARSTORE, SCROLL_WHEEL_LED);
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_effect_spectrum(VARSTORE, SCROLL_WHEEL_LED);
        break;
    }

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "scroll_mode_reactive" (for extended mouse matrix effects)
 *
 * Sets reactive mode when this file is written to. A speed byte and 3 RGB bytes should be written
 */
static ssize_t razer_attr_write_scroll_mode_reactive(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    if(count == 4) {
        unsigned char speed = (unsigned char)buf[0];

        switch(usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
            report = razer_chroma_mouse_extended_matrix_effect_reactive(VARSTORE, SCROLL_WHEEL_LED, speed, (struct razer_rgb*)&buf[1]);
            break;

        case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
            report = razer_chroma_extended_matrix_effect_reactive(VARSTORE, SCROLL_WHEEL_LED, speed, (struct razer_rgb*)&buf[1]);
            break;
        }

        razer_send_payload(usb_dev, &report);

    } else {
        printk(KERN_WARNING "razermouse: Reactive only accepts Speed, RGB (4byte)");
    }
    return count;
}

/**
 * Write device file "scroll_mode_breath" (for extended mouse matrix effects)
 *
 * Sets breathing mode by writing 1, 3 or 6 bytes
 */
static ssize_t razer_attr_write_scroll_mode_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};
    // TODO refactor main breathing matrix function, add in LED ID field and this nastiness goes away too!

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        switch(count) {
        case 3: // Single colour mode
            report = razer_chroma_mouse_extended_matrix_effect_breathing_single(VARSTORE, SCROLL_WHEEL_LED, (struct razer_rgb*)&buf[0]);
            break;

        case 6: // Dual colour mode
            report = razer_chroma_mouse_extended_matrix_effect_breathing_dual(VARSTORE, SCROLL_WHEEL_LED, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
            break;

        default: // "Random" colour mode
            report = razer_chroma_mouse_extended_matrix_effect_breathing_random(VARSTORE, SCROLL_WHEEL_LED);
            break;
        }
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
        switch(count) {
        case 3: // Single colour mode
            report = razer_chroma_extended_matrix_effect_breathing_single(VARSTORE, SCROLL_WHEEL_LED, (struct razer_rgb*)&buf[0]);
            break;

        case 6: // Dual colour mode
            report = razer_chroma_extended_matrix_effect_breathing_dual(VARSTORE, SCROLL_WHEEL_LED, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
            break;

        default: // "Random" colour mode
            report = razer_chroma_extended_matrix_effect_breathing_random(VARSTORE, SCROLL_WHEEL_LED);
            break;
        }
        break;
    }

    report.transaction_id.id = 0x3f;
    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "scroll_mode_static" (for extended mouse matrix effects)
 *
 * Set the mouse to static mode when 3 RGB bytes are written
 */
static ssize_t razer_attr_write_scroll_mode_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    if(count == 3) {

        switch(usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
        case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
            report = razer_chroma_mouse_extended_matrix_effect_static(VARSTORE, SCROLL_WHEEL_LED, (struct razer_rgb*)&buf[0]);
            break;

        case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
            report = razer_chroma_extended_matrix_effect_static(VARSTORE, SCROLL_WHEEL_LED, (struct razer_rgb*)&buf[0]);
            break;
        }

        razer_send_payload(usb_dev, &report);
    } else {
        printk(KERN_WARNING "razermouse: Static mode only accepts RGB (3byte)");
    }

    return count;
}

/**
 * Write device file "scroll_mode_none" (for extended mouse matrix effects)
 *
 * No effect is activated whenever this file is written to
 */
static ssize_t razer_attr_write_scroll_mode_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        report = razer_chroma_mouse_extended_matrix_effect_none(VARSTORE, SCROLL_WHEEL_LED);
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
        report = razer_chroma_extended_matrix_effect_none(VARSTORE, SCROLL_WHEEL_LED);
        break;
    }

    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "logo_mode_wave" (for extended mouse matrix effects)
 *
 * Wave effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_logo_mode_wave(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char direction = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_effect_wave(VARSTORE, LOGO_LED, direction);
        break;
    }

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "logo_mode_spectrum" (for extended mouse matrix effects)
 *
 * Spectrum effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_logo_mode_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        report = razer_chroma_mouse_extended_matrix_effect_spectrum(VARSTORE, LOGO_LED);
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_effect_spectrum(VARSTORE, LOGO_LED);
        break;
    }

    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "logo_mode_reactive" (for extended mouse matrix effects)
 *
 * Sets reactive mode when this file is written to. A speed byte and 3 RGB bytes should be written
 */
static ssize_t razer_attr_write_logo_mode_reactive(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    if(count == 4) {
        unsigned char speed = (unsigned char)buf[0];

        switch(usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
        case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
            report = razer_chroma_mouse_extended_matrix_effect_reactive(VARSTORE, LOGO_LED, speed, (struct razer_rgb*)&buf[1]);
            break;

        case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
            report = razer_chroma_extended_matrix_effect_reactive(VARSTORE, LOGO_LED, speed, (struct razer_rgb*)&buf[1]);
            break;
        }

        razer_send_payload(usb_dev, &report);

    } else {
        printk(KERN_WARNING "razermouse: Reactive only accepts Speed, RGB (4byte)");
    }
    return count;
}

/**
 * Write device file "logo_mode_breath" (for extended mouse matrix effects)
 *
 * Sets breathing mode by writing 1, 3 or 6 bytes
 */
static ssize_t razer_attr_write_logo_mode_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};
    // TODO refactor main breathing matrix function, add in LED ID field and this nastiness goes away too!

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        switch(count) {
        case 3: // Single colour mode
            report = razer_chroma_mouse_extended_matrix_effect_breathing_single(VARSTORE, LOGO_LED, (struct razer_rgb*)&buf[0]);
            break;

        case 6: // Dual colour mode
            report = razer_chroma_mouse_extended_matrix_effect_breathing_dual(VARSTORE, LOGO_LED, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
            break;

        default: // "Random" colour mode
            report = razer_chroma_mouse_extended_matrix_effect_breathing_random(VARSTORE, LOGO_LED);
            break;
        }
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
        switch(count) {
        case 3: // Single colour mode
            report = razer_chroma_extended_matrix_effect_breathing_single(VARSTORE, LOGO_LED, (struct razer_rgb*)&buf[0]);
            break;

        case 6: // Dual colour mode
            report = razer_chroma_extended_matrix_effect_breathing_dual(VARSTORE, LOGO_LED, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
            break;

        default: // "Random" colour mode
            report = razer_chroma_extended_matrix_effect_breathing_random(VARSTORE, LOGO_LED);
            break;
        }
        break;
    }

    report.transaction_id.id = 0x3f;
    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "logo_mode_static" (for extended mouse matrix effects)
 *
 * Set the mouse to static mode when 3 RGB bytes are written
 */
static ssize_t razer_attr_write_logo_mode_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    if(count == 3) {
        switch(usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
        case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
            report = razer_chroma_mouse_extended_matrix_effect_static(VARSTORE, LOGO_LED, (struct razer_rgb*)&buf[0]);
            break;

        case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
            report = razer_chroma_extended_matrix_effect_static(VARSTORE, LOGO_LED, (struct razer_rgb*)&buf[0]);
            break;
        }

        razer_send_payload(usb_dev, &report);
    } else {
        printk(KERN_WARNING "razermouse: Static mode only accepts RGB (3byte)");
    }

    return count;
}

/**
 * Write device file "logo_mode_none" (for extended mouse matrix effects)
 *
 * No effect is activated whenever this file is written to
 */
static ssize_t razer_attr_write_logo_mode_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
    case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        report = razer_chroma_mouse_extended_matrix_effect_none(VARSTORE, LOGO_LED);
        break;

    case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
    case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
        report = razer_chroma_extended_matrix_effect_none(VARSTORE, LOGO_LED);
        break;
    }

    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "left_mode_wave" (for extended mouse matrix effects)
 *
 * Wave effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_left_mode_wave(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char direction = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_effect_wave(VARSTORE, LEFT_SIDE_LED, direction);
        break;
    }

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "left_mode_spectrum" (for extended mouse matrix effects)
 *
 * Spectrum effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_left_mode_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_effect_spectrum(VARSTORE, LEFT_SIDE_LED);
        break;
    }

    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "left_mode_reactive" (for extended mouse matrix effects)
 *
 * Sets reactive mode when this file is written to. A speed byte and 3 RGB bytes should be written
 */
static ssize_t razer_attr_write_left_mode_reactive(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    if(count == 4) {
        unsigned char speed = (unsigned char)buf[0];

        switch(usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
            report = razer_chroma_extended_matrix_effect_reactive(VARSTORE, LEFT_SIDE_LED, speed, (struct razer_rgb*)&buf[1]);
            break;
        }

        razer_send_payload(usb_dev, &report);

    } else {
        printk(KERN_WARNING "razermouse: Reactive only accepts Speed, RGB (4byte)");
    }
    return count;
}

/**
 * Write device file "left_mode_breath" (for extended mouse matrix effects)
 *
 * Sets breathing mode by writing 1, 3 or 6 bytes
 */
static ssize_t razer_attr_write_left_mode_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        switch(count) {
        case 3: // Single colour mode
            report = razer_chroma_extended_matrix_effect_breathing_single(VARSTORE, LEFT_SIDE_LED, (struct razer_rgb*)&buf[0]);
            break;

        case 6: // Dual colour mode
            report = razer_chroma_extended_matrix_effect_breathing_dual(VARSTORE, LEFT_SIDE_LED, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
            break;

        default: // "Random" colour mode
            report = razer_chroma_extended_matrix_effect_breathing_random(VARSTORE, LEFT_SIDE_LED);
            break;
        }
        break;
    }

    report.transaction_id.id = 0x3f;
    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "left_mode_static" (for extended mouse matrix effects)
 *
 * Set the mouse to static mode when 3 RGB bytes are written
 */
static ssize_t razer_attr_write_left_mode_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    if(count == 3) {
        switch(usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
            report = razer_chroma_extended_matrix_effect_static(VARSTORE, LEFT_SIDE_LED, (struct razer_rgb*)&buf[0]);
            break;
        }

        razer_send_payload(usb_dev, &report);
    } else {
        printk(KERN_WARNING "razermouse: Static mode only accepts RGB (3byte)");
    }

    return count;
}

/**
 * Write device file "left_mode_none" (for extended mouse matrix effects)
 *
 * No effect is activated whenever this file is written to
 */
static ssize_t razer_attr_write_left_mode_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_effect_none(VARSTORE, LEFT_SIDE_LED);
        break;
    }

    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "right_mode_wave" (for extended mouse matrix effects)
 *
 * Wave effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_right_mode_wave(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char direction = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_effect_wave(VARSTORE, RIGHT_SIDE_LED, direction);
        break;
    }

    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "right_mode_spectrum" (for extended mouse matrix effects)
 *
 * Spectrum effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_right_mode_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_effect_spectrum(VARSTORE, RIGHT_SIDE_LED);
        break;
    }

    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "right_mode_reactive" (for extended mouse matrix effects)
 *
 * Sets reactive mode when this file is written to. A speed byte and 3 RGB bytes should be written
 */
static ssize_t razer_attr_write_right_mode_reactive(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    if(count == 4) {
        unsigned char speed = (unsigned char)buf[0];

        switch(usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
            report = razer_chroma_extended_matrix_effect_reactive(VARSTORE, RIGHT_SIDE_LED, speed, (struct razer_rgb*)&buf[1]);
            break;
        }

        razer_send_payload(usb_dev, &report);

    } else {
        printk(KERN_WARNING "razermouse: Reactive only accepts Speed, RGB (4byte)");
    }
    return count;
}

/**
 * Write device file "right_mode_breath" (for extended mouse matrix effects)
 *
 * Sets breathing mode by writing 1, 3 or 6 bytes
 */
static ssize_t razer_attr_write_right_mode_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        switch(count) {
        case 3: // Single colour mode
            report = razer_chroma_extended_matrix_effect_breathing_single(VARSTORE, RIGHT_SIDE_LED, (struct razer_rgb*)&buf[0]);
            break;

        case 6: // Dual colour mode
            report = razer_chroma_extended_matrix_effect_breathing_dual(VARSTORE, RIGHT_SIDE_LED, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
            break;

        default: // "Random" colour mode
            report = razer_chroma_extended_matrix_effect_breathing_random(VARSTORE, RIGHT_SIDE_LED);
            break;
        }
        break;
    }

    report.transaction_id.id = 0x3f;
    razer_send_payload(usb_dev, &report);

    return count;
}

/**
 * Write device file "right_mode_static" (for extended mouse matrix effects)
 *
 * Set the mouse to static mode when 3 RGB bytes are written
 */
static ssize_t razer_attr_write_right_mode_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    if(count == 3) {
        switch(usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
            report = razer_chroma_extended_matrix_effect_static(VARSTORE, RIGHT_SIDE_LED, (struct razer_rgb*)&buf[0]);
            break;
        }

        razer_send_payload(usb_dev, &report);
    } else {
        printk(KERN_WARNING "razermouse: Static mode only accepts RGB (3byte)");
    }

    return count;
}

/**
 * Write device file "right_mode_none" (for extended mouse matrix effects)
 *
 * No effect is activated whenever this file is written to
 */
static ssize_t razer_attr_write_right_mode_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct usb_interface *intf = to_usb_interface(dev->parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_report report = {0};

    switch(usb_dev->descriptor.idProduct) {
    case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
        report = razer_chroma_extended_matrix_effect_none(VARSTORE, RIGHT_SIDE_LED);
        break;
    }

    razer_send_payload(usb_dev, &report);
    return count;
}

/**
 * Write device file "backlight_led_state"
 */
static ssize_t razer_attr_write_backlight_led_state(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report report = razer_chroma_standard_set_led_state(VARSTORE, BACKLIGHT_LED, enabled);
    report.transaction_id.id = 0x3F;

    mutex_lock(&device->lock);
    razer_send_payload(device->usb_dev, &report);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Read device file "backlight_led_state"
 */
static ssize_t razer_attr_read_backlight_led_state(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_mouse_device *device = dev_get_drvdata(dev);
    struct razer_report report = razer_chroma_standard_get_led_state(VARSTORE, BACKLIGHT_LED);
    struct razer_report response = {0};
    report.transaction_id.id = 0x3F;

    mutex_lock(&device->lock);
    response = razer_send_payload(device->usb_dev, &report);
    mutex_unlock(&device->lock);

    return sprintf(buf, "%d\n", response.arguments[2]);
}


/**
 * Set up the device driver files
 *
 * Read-only is  0444
 * Write-only is 0220
 * Read/write is 0664
 */

static DEVICE_ATTR(version,                   0440, razer_attr_read_version,               NULL);
static DEVICE_ATTR(firmware_version,          0440, razer_attr_read_get_firmware_version,  NULL);
static DEVICE_ATTR(test,                      0220, NULL,                                  razer_attr_write_test);
static DEVICE_ATTR(poll_rate,                 0660, razer_attr_read_poll_rate,             razer_attr_write_poll_rate);
static DEVICE_ATTR(dpi,                       0660, razer_attr_read_mouse_dpi,             razer_attr_write_mouse_dpi);

static DEVICE_ATTR(device_type,               0440, razer_attr_read_device_type,           NULL);
static DEVICE_ATTR(device_mode,               0660, razer_attr_read_device_mode,           razer_attr_write_device_mode);
static DEVICE_ATTR(device_serial,             0440, razer_attr_read_get_serial,            NULL);
static DEVICE_ATTR(device_idle_time,          0220, NULL,                                  razer_attr_write_set_idle_time);

static DEVICE_ATTR(charge_level,              0440, razer_attr_read_get_battery,           NULL);
static DEVICE_ATTR(charge_status,             0440, razer_attr_read_is_charging,           NULL);
static DEVICE_ATTR(charge_effect,             0220, NULL,                                  razer_attr_write_set_charging_effect);
static DEVICE_ATTR(charge_colour,             0220, NULL,                                  razer_attr_write_set_charging_colour);
static DEVICE_ATTR(charge_low_threshold,      0220, NULL,                                  razer_attr_write_set_low_battery_threshold);

static DEVICE_ATTR(matrix_brightness,         0660, razer_attr_read_matrix_brightness,     razer_attr_write_matrix_brightness);
static DEVICE_ATTR(matrix_custom_frame,       0220, NULL,                                  razer_attr_write_set_key_row);
static DEVICE_ATTR(matrix_effect_none,        0220, NULL,                                  razer_attr_write_mode_none);   // Matrix
static DEVICE_ATTR(matrix_effect_custom,      0220, NULL,                                  razer_attr_write_mode_custom);   // Matrix
static DEVICE_ATTR(matrix_effect_static,      0220, NULL,                                  razer_attr_write_mode_static);   // Matrix
static DEVICE_ATTR(matrix_effect_wave,        0220, NULL,                                  razer_attr_write_mode_wave);   // Matrix
static DEVICE_ATTR(matrix_effect_spectrum,    0220, NULL,                                  razer_attr_write_mode_spectrum);   // Matrix
static DEVICE_ATTR(matrix_effect_reactive,    0220, NULL,                                  razer_attr_write_mode_reactive);   // Matrix
static DEVICE_ATTR(matrix_effect_breath,      0220, NULL,                                  razer_attr_write_mode_breath);   // Matrix


static DEVICE_ATTR(scroll_led_brightness,     0660, razer_attr_read_scroll_led_brightness, razer_attr_write_scroll_led_brightness);
// For old-school led commands
static DEVICE_ATTR(scroll_led_state,          0660, razer_attr_read_scroll_led_state,      razer_attr_write_scroll_led_state);
static DEVICE_ATTR(scroll_led_rgb,            0660, razer_attr_read_scroll_led_rgb,        razer_attr_write_scroll_led_rgb);
static DEVICE_ATTR(scroll_led_effect,         0660, razer_attr_read_scroll_led_effect,     razer_attr_write_scroll_led_effect);
// For "extended" matrix effects
static DEVICE_ATTR(scroll_matrix_effect_wave,        0220, NULL,                           razer_attr_write_scroll_mode_wave);
static DEVICE_ATTR(scroll_matrix_effect_spectrum,    0220, NULL,                           razer_attr_write_scroll_mode_spectrum);
static DEVICE_ATTR(scroll_matrix_effect_reactive,    0220, NULL,                           razer_attr_write_scroll_mode_reactive);
static DEVICE_ATTR(scroll_matrix_effect_breath,      0220, NULL,                           razer_attr_write_scroll_mode_breath);
static DEVICE_ATTR(scroll_matrix_effect_static,      0220, NULL,                           razer_attr_write_scroll_mode_static);
static DEVICE_ATTR(scroll_matrix_effect_none,        0220, NULL,                           razer_attr_write_scroll_mode_none);

static DEVICE_ATTR(logo_led_brightness,       0660, razer_attr_read_logo_led_brightness,   razer_attr_write_logo_led_brightness);
// For old-school led commands
static DEVICE_ATTR(logo_led_state,            0660, razer_attr_read_logo_led_state,        razer_attr_write_logo_led_state);
static DEVICE_ATTR(logo_led_rgb,              0660, razer_attr_read_logo_led_rgb,          razer_attr_write_logo_led_rgb);
static DEVICE_ATTR(logo_led_effect,           0660, razer_attr_read_logo_led_effect,       razer_attr_write_logo_led_effect);
// For "extended" matrix effects
static DEVICE_ATTR(logo_matrix_effect_wave,        0220, NULL,                             razer_attr_write_logo_mode_wave);
static DEVICE_ATTR(logo_matrix_effect_spectrum,    0220, NULL,                             razer_attr_write_logo_mode_spectrum);
static DEVICE_ATTR(logo_matrix_effect_reactive,    0220, NULL,                             razer_attr_write_logo_mode_reactive);
static DEVICE_ATTR(logo_matrix_effect_breath,      0220, NULL,                             razer_attr_write_logo_mode_breath);
static DEVICE_ATTR(logo_matrix_effect_static,      0220, NULL,                             razer_attr_write_logo_mode_static);
static DEVICE_ATTR(logo_matrix_effect_none,        0220, NULL,                             razer_attr_write_logo_mode_none);

static DEVICE_ATTR(left_led_brightness,       0660, razer_attr_read_left_led_brightness,   razer_attr_write_left_led_brightness);
// For "extended" matrix effects
static DEVICE_ATTR(left_matrix_effect_wave,        0220, NULL,                             razer_attr_write_left_mode_wave);
static DEVICE_ATTR(left_matrix_effect_spectrum,    0220, NULL,                             razer_attr_write_left_mode_spectrum);
static DEVICE_ATTR(left_matrix_effect_reactive,    0220, NULL,                             razer_attr_write_left_mode_reactive);
static DEVICE_ATTR(left_matrix_effect_breath,      0220, NULL,                             razer_attr_write_left_mode_breath);
static DEVICE_ATTR(left_matrix_effect_static,      0220, NULL,                             razer_attr_write_left_mode_static);
static DEVICE_ATTR(left_matrix_effect_none,        0220, NULL,                             razer_attr_write_left_mode_none);

static DEVICE_ATTR(right_led_brightness,       0660, razer_attr_read_right_led_brightness,   razer_attr_write_right_led_brightness);
// For "extended" matrix effects
static DEVICE_ATTR(right_matrix_effect_wave,        0220, NULL,                             razer_attr_write_right_mode_wave);
static DEVICE_ATTR(right_matrix_effect_spectrum,    0220, NULL,                             razer_attr_write_right_mode_spectrum);
static DEVICE_ATTR(right_matrix_effect_reactive,    0220, NULL,                             razer_attr_write_right_mode_reactive);
static DEVICE_ATTR(right_matrix_effect_breath,      0220, NULL,                             razer_attr_write_right_mode_breath);
static DEVICE_ATTR(right_matrix_effect_static,      0220, NULL,                             razer_attr_write_right_mode_static);
static DEVICE_ATTR(right_matrix_effect_none,        0220, NULL,                             razer_attr_write_right_mode_none);

// For old-school led commands
static DEVICE_ATTR(backlight_led_state,            0660, razer_attr_read_backlight_led_state, razer_attr_write_backlight_led_state);



/**
 * Raw event function
 */
static int razer_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size)
{
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);

    // The event were looking for is 16 bytes long and starts with 0x04
    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD && size == 16 && data[0] == 0x04) {
        // Convert 04... to 0100...
        int index = size-1; // This way we start at 2nd last value, does subtract 1 from the 15key rollover though (not an issue cmon)
        u8 cur_value = 0x00;

        while(--index > 0) {
            cur_value = data[index];
            if(cur_value == 0x00) { // Skip 0x00
                continue;
            }

            switch(cur_value) {
            case 0x20: // DPI Up
                cur_value = 0x68; // F13
                break;
            case 0x21: // DPI Down
                cur_value = 0x69; // F14
                break;
            case 0x22: // Wheel Left
                cur_value = 0x6A; // F15
                break;
            case 0x23: // Wheel Right
                cur_value = 0x6B; // F16
                break;
            }

            data[index+1] = cur_value;
        }


        data[0] = 0x01;
        data[1] = 0x00;
        return 1;
    }

    return 0;
}

/**
 * Mouse init function
 */
void razer_mouse_init(struct razer_mouse_device *dev, struct usb_interface *intf, struct hid_device *hdev)
{
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned int rand_serial = 0;

    // Initialise mutex
    mutex_init(&dev->lock);
    // Setup values
    dev->usb_dev = usb_dev;
    dev->usb_vid = usb_dev->descriptor.idVendor;
    dev->usb_pid = usb_dev->descriptor.idProduct;
    dev->usb_interface_protocol = intf->cur_altsetting->desc.bInterfaceProtocol;

    // Get a "random" integer
    get_random_bytes(&rand_serial, sizeof(unsigned int));
    sprintf(&dev->serial[0], "PM%012u", rand_serial);

    // Setup orochi2011
    dev->orochi2011_dpi = 0x4c;
    dev->orochi2011_poll = 500;

    // Setup default values for DeathAdder 3.5G
    dev->da3_5g.leds = 3; // Lights up all lights
    dev->da3_5g.dpi = 1; // 3500 DPI
    dev->da3_5g.profile = 1; // Profile 1
    dev->da3_5g.poll = 1; // Poll rate 1000
}

/**
 * Probe method is ran whenever a device is binded to the driver
 */
static int razer_mouse_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int retval = 0;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct razer_mouse_device *dev = NULL;

    dev = kzalloc(sizeof(struct razer_mouse_device), GFP_KERNEL);

    if(dev == NULL) {
        dev_err(&intf->dev, "out of memory\n");
        retval = -ENOMEM;
        goto exit;
    }

    // Init data
    razer_mouse_init(dev, intf, hdev);

    if(dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_MOUSE) {
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_test);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_firmware_version);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_mode);

        switch(dev->usb_pid) {
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_wave);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_wave);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_matrix_effect_wave);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_matrix_effect_wave);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);
            break;

        case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);
            break;

        case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);
            break;

        case USB_DEVICE_ID_RAZER_MAMBA_2012_WIRELESS:
        case USB_DEVICE_ID_RAZER_MAMBA_2012_WIRED:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_level);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_status);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_low_threshold);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_idle_time);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_rgb);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_effect);
            break;

        case USB_DEVICE_ID_RAZER_MAMBA_WIRELESS:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_effect);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_colour);
        case USB_DEVICE_ID_RAZER_MAMBA_WIRED:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_level);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_status);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_low_threshold);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_idle_time);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);
            break;

        case USB_DEVICE_ID_RAZER_MAMBA_TE_WIRED:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);
            break;

        case USB_DEVICE_ID_RAZER_OROCHI_2011:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            break;

        case USB_DEVICE_ID_RAZER_ABYSSUS:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            break;

        case USB_DEVICE_ID_RAZER_IMPERATOR:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            break;

        case USB_DEVICE_ID_RAZER_OUROBOROS:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_low_threshold);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_idle_time);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_level);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_status);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            break;

        case USB_DEVICE_ID_RAZER_DEATHADDER_2013:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_rgb);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_effect);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_rgb);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_effect);
            break;

        case USB_DEVICE_ID_RAZER_OROCHI_2013:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            break;

        case USB_DEVICE_ID_RAZER_OROCHI_CHROMA:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_low_threshold);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_idle_time);
            break;

        case USB_DEVICE_ID_RAZER_NAGA_HEX_RED:
        case USB_DEVICE_ID_RAZER_NAGA_HEX:
        case USB_DEVICE_ID_RAZER_TAIPAN:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            break;

        case USB_DEVICE_ID_RAZER_NAGA_2014:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_backlight_led_state);
            break;

        case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            break;

        case USB_DEVICE_ID_RAZER_ABYSSUS_V2:
        case USB_DEVICE_ID_RAZER_DEATHADDER_3500:
        case USB_DEVICE_ID_RAZER_DEATHADDER_CHROMA:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_rgb);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_effect);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_rgb);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_effect);
            break;

        case USB_DEVICE_ID_RAZER_DIAMONDBACK_CHROMA:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);
            break;

        case USB_DEVICE_ID_RAZER_ABYSSUS_1800:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            break;

        case USB_DEVICE_ID_RAZER_ABYSSUS_2000:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            break;

        case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_scroll_matrix_effect_none);
            break;

        case USB_DEVICE_ID_RAZER_NAGA_TRINITY:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dpi);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_poll_rate);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_brightness);
            break;
        }

    }

    hid_set_drvdata(hdev, dev);
    dev_set_drvdata(&hdev->dev, dev);

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

    //razer_reset(usb_dev);
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
static void razer_mouse_disconnect(struct hid_device *hdev)
{
    struct razer_mouse_device *dev;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    dev = hid_get_drvdata(hdev);

    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE) {
        device_remove_file(&hdev->dev, &dev_attr_version);
        device_remove_file(&hdev->dev, &dev_attr_test);
        device_remove_file(&hdev->dev, &dev_attr_firmware_version);
        device_remove_file(&hdev->dev, &dev_attr_device_type);
        device_remove_file(&hdev->dev, &dev_attr_device_serial);
        device_remove_file(&hdev->dev, &dev_attr_device_mode);

        switch(usb_dev->descriptor.idProduct) {
        case USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED:
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_left_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_left_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_left_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_left_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_left_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_left_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_left_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_right_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_right_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_right_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_right_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_right_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_right_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_right_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);
            device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);
            break;

        case USB_DEVICE_ID_RAZER_DEATHADDER_ELITE:
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);
            device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);
            break;

        case USB_DEVICE_ID_RAZER_NAGA_HEX_V2:
        case USB_DEVICE_ID_RAZER_NAGA_CHROMA:
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_matrix_brightness);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);
            device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);
            break;

        case USB_DEVICE_ID_RAZER_MAMBA_2012_WIRELESS:
        case USB_DEVICE_ID_RAZER_MAMBA_2012_WIRED:
            device_remove_file(&hdev->dev, &dev_attr_charge_level);
            device_remove_file(&hdev->dev, &dev_attr_charge_status);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_charge_low_threshold);
            device_remove_file(&hdev->dev, &dev_attr_device_idle_time);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_state);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_rgb);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_effect);
            break;

        case USB_DEVICE_ID_RAZER_MAMBA_WIRELESS:
            device_remove_file(&hdev->dev, &dev_attr_charge_effect);
            device_remove_file(&hdev->dev, &dev_attr_charge_colour);
        case USB_DEVICE_ID_RAZER_MAMBA_WIRED:
            device_remove_file(&hdev->dev, &dev_attr_charge_level);
            device_remove_file(&hdev->dev, &dev_attr_charge_status);
            device_remove_file(&hdev->dev, &dev_attr_charge_low_threshold);
            device_remove_file(&hdev->dev, &dev_attr_device_idle_time);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);
            device_remove_file(&hdev->dev, &dev_attr_matrix_brightness);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            break;

        case USB_DEVICE_ID_RAZER_MAMBA_TE_WIRED:
            device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);
            device_remove_file(&hdev->dev, &dev_attr_matrix_brightness);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            break;

        case USB_DEVICE_ID_RAZER_OROCHI_2011:
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_state);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            break;

        case USB_DEVICE_ID_RAZER_ABYSSUS:
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            break;

        case USB_DEVICE_ID_RAZER_IMPERATOR:
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_state);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            break;

        case USB_DEVICE_ID_RAZER_OUROBOROS:
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_charge_low_threshold);
            device_remove_file(&hdev->dev, &dev_attr_device_idle_time);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_state);
            device_remove_file(&hdev->dev, &dev_attr_charge_level);
            device_remove_file(&hdev->dev, &dev_attr_charge_status);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            break;

        case USB_DEVICE_ID_RAZER_OROCHI_2013:
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_state);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            break;

        case USB_DEVICE_ID_RAZER_OROCHI_CHROMA:
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_state);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_matrix_brightness);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_charge_low_threshold);
            device_remove_file(&hdev->dev, &dev_attr_device_idle_time);
            break;

        case USB_DEVICE_ID_RAZER_NAGA_HEX:
        case USB_DEVICE_ID_RAZER_NAGA_HEX_RED:
        case USB_DEVICE_ID_RAZER_TAIPAN:
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_state);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            break;

        case USB_DEVICE_ID_RAZER_NAGA_2014:
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_state);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            device_remove_file(&hdev->dev, &dev_attr_backlight_led_state);
            break;

        case USB_DEVICE_ID_RAZER_DEATHADDER_3_5G:
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_state);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            break;

        case USB_DEVICE_ID_RAZER_ABYSSUS_V2:
        case USB_DEVICE_ID_RAZER_DEATHADDER_3500:
        case USB_DEVICE_ID_RAZER_DEATHADDER_CHROMA:
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_state);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_rgb);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_effect);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_rgb);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_effect);
            break;

        case USB_DEVICE_ID_RAZER_DIAMONDBACK_CHROMA:
            device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);
            device_remove_file(&hdev->dev, &dev_attr_matrix_brightness);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            break;

        case USB_DEVICE_ID_RAZER_ABYSSUS_1800:
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            break;

        case USB_DEVICE_ID_RAZER_ABYSSUS_2000:
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            break;

        case USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL:
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_scroll_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_scroll_matrix_effect_none);
            break;

        case USB_DEVICE_ID_RAZER_NAGA_TRINITY:
            device_remove_file(&hdev->dev, &dev_attr_dpi);
            device_remove_file(&hdev->dev, &dev_attr_poll_rate);
            device_remove_file(&hdev->dev, &dev_attr_matrix_brightness);
            break;
        }

    }


    hid_hw_stop(hdev);
    kfree(dev);
    dev_info(&intf->dev, "Razer Device disconnected\n");
}


/**
 * Device ID mapping table
 */
static const struct hid_device_id razer_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_OROCHI_2011) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_ABYSSUS_1800) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_ABYSSUS_2000) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_DEATHADDER_3_5G) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_NAGA_HEX_RED) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_NAGA_2014) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_NAGA_HEX) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_MAMBA_2012_WIRED) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_MAMBA_2012_WIRELESS) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_MAMBA_WIRED) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_MAMBA_WIRELESS) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_MAMBA_TE_WIRED) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_ABYSSUS) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_TAIPAN) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_IMPERATOR) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_OUROBOROS) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_DEATHADDER_2013) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_OROCHI_2013) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_OROCHI_CHROMA) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_DEATHADDER_CHROMA) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_NAGA_HEX_V2) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_NAGA_CHROMA) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_DEATHADDER_ELITE) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_DIAMONDBACK_CHROMA) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_ABYSSUS_V2) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_DEATHADDER_3500) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_LANCEHEAD_TE_WIRED) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_NAGA_TRINITY) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_DEATHADDER_ESSENTIAL) },
    { }
};

MODULE_DEVICE_TABLE(hid, razer_devices);


/**
 * Describes the contents of the driver
 */
static struct hid_driver razer_mouse_driver = {
    .name      = "razermouse",
    .id_table  = razer_devices,
    .probe     = razer_mouse_probe,
    .remove    = razer_mouse_disconnect,

    .raw_event = razer_raw_event,
};

module_hid_driver(razer_mouse_driver);
