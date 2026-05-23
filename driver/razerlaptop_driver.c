// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Terri Cain <terri@dolphincorp.co.uk>
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/dmi.h>
#include <linux/input-event-codes.h>
#include <linux/version.h>
#ifdef CONFIG_HWMON
#include <linux/hwmon.h>
#endif

#include "usb_hid_keys.h"

#include "razerkbd_driver.h"
#include "razercommon.h"
#include "razerchromacommon.h"

/*
 * Version Information
 */
#define DRIVER_DESC "Razer Laptop Device Driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE(DRIVER_LICENSE);

// KEY_MACRO* has been added in Linux 5.5, so define ourselves for older kernels.
// See also https://git.kernel.org/torvalds/c/b5625db
#ifndef KEY_MACRO1
#define KEY_MACRO1  0x290
#define KEY_MACRO2  0x291
#define KEY_MACRO3  0x292
#define KEY_MACRO4  0x293
#define KEY_MACRO5  0x294
#define KEY_MACRO6  0x295
#define KEY_MACRO7  0x296
#define KEY_MACRO8  0x297
#define KEY_MACRO9  0x298
#define KEY_MACRO10 0x299
#define KEY_MACRO11 0x2a0
#define KEY_MACRO12 0x2a1
// ...
#define KEY_MACRO27 0x2aa
#define KEY_MACRO28 0x2ab
#define KEY_MACRO29 0x2ac
#define KEY_MACRO30 0x2ad
#endif

// These are evdev key codes, not HID key codes.
// Lower macro key codes are intended for the actual macro keys
// Higher macro key codes are inteded for Chroma functions
#define RAZER_MACRO_KEY KEY_MACRO30 // TODO maybe KEY_MACRO_RECORD_START?
#define RAZER_GAME_KEY KEY_MACRO29 // TODO maybe KEY_GAMES?
#define RAZER_BRIGHTNESS_DOWN KEY_MACRO28
#define RAZER_BRIGHTNESS_UP KEY_MACRO27

/**
 * List of keys to swap
 */
static bool is_blade_laptop(struct razer_kbd_device *device)
{
    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_2018:
    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_2018_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
    case USB_DEVICE_ID_RAZER_BLADE_2019_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BOOK_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
    case USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        return true;
    }
    return false;
}

/**
 * Get request/response indices and timing parameters for the device
 */
static void razer_get_report_params(struct usb_device *usb_dev, uint *report_index, uint *response_index, ulong *wait_min, ulong *wait_max)
{
    *report_index   = 0x01;
    *response_index = 0x01;
    *wait_min = RAZER_BLACKWIDOW_CHROMA_WAIT_MIN_US;
    *wait_max = RAZER_BLACKWIDOW_CHROMA_WAIT_MAX_US;
}
/**
 * Send report to the keyboard
 */
static int razer_get_report(struct usb_device *usb_dev, struct razer_report *request, struct razer_report *response)
{
    uint report_index, response_index;
    ulong wait_min, wait_max;
    razer_get_report_params(usb_dev, &report_index, &response_index, &wait_min, &wait_max);
    return razer_get_usb_response(usb_dev, report_index, request, response_index, response, wait_min, wait_max);
}

/**
 * Send report to the keyboard, but without even reading the response
 */
static int razer_send_payload_no_response(struct razer_kbd_device *device, struct razer_report *request)
{
    uint report_index, response_index;
    ulong wait_min, wait_max;

    /* Except the caller to have set the transaction_id */
    WARN_ON(request->transaction_id.id == 0x00);

    razer_get_report_params(device->usb_dev, &report_index, &response_index, &wait_min, &wait_max);
    return razer_send_control_msg(device->usb_dev, request, report_index, wait_min, wait_max);
}

/**
 * Function to send to device, get response, and actually check the response
 */
static int razer_send_payload(struct razer_kbd_device *device, struct razer_report *request, struct razer_report *response)
{
    int err;

    request->crc = razer_calculate_crc(request);

    mutex_lock(&device->lock);
    err = razer_get_report(device->usb_dev, request, response);
    mutex_unlock(&device->lock);
    if (err) {
        print_erroneous_report(response, "razerkbd", "Invalid Report Length");
        return err;
    }

    /* Check the packet number, class and command are the same */
    if (response->remaining_packets != request->remaining_packets ||
        response->command_class != request->command_class ||
        response->command_id.id != request->command_id.id) {
        print_erroneous_report(response, "razerkbd", "Response doesn't match request");
        return -EIO;
    }

    switch (response->status) {
    case RAZER_CMD_BUSY:
        // TODO: Check if this should be an error.
        // print_erroneous_report(&response, "razermouse", "Device is busy");
        break;
    case RAZER_CMD_FAILURE:
        print_erroneous_report(response, "razerkbd", "Command failed");
        return -EIO;
    case RAZER_CMD_NOT_SUPPORTED:
        print_erroneous_report(response, "razerkbd", "Command not supported");
        return -EIO;
    case RAZER_CMD_TIMEOUT:
        print_erroneous_report(response, "razerkbd", "Command timed out");
        return -EIO;
    }

    return 0;
}

/**
 * Reads the physical layout of the keyboard.
 *
 * Returns a string
 */
static ssize_t razer_attr_read_kbd_layout(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = get_razer_report(0x00, 0x86, 0x02);
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);

    return sprintf(buf, "%02x\n", response.arguments[0]);
}

/**
 * Device mode function
 */
static void razer_set_device_mode(struct razer_kbd_device *device, unsigned char mode, unsigned char param)
{
    struct razer_report request = {0};
    struct razer_report response = {0};

    if (is_blade_laptop(device)) {
        return;
    }

    request = razer_chroma_standard_set_device_mode(mode, param);

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
    case USB_DEVICE_ID_RAZER_BLADE_2018:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019:
    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
    case USB_DEVICE_ID_RAZER_BLADE_2018_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_2019_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
    case USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE:
    case USB_DEVICE_ID_RAZER_BOOK_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        request.transaction_id.id = 0xFF;
        break;

    default:
        printk(KERN_WARNING "razerkbd: device_mode not supported for this model\n");
        return;
    }

    razer_send_payload(device, &request, &response);
}

/**
 * Read device file "charge_level"
 *
 * Returns an integer which needs to be scaled from 0-255 -> 0-100
 */
static ssize_t razer_attr_read_charge_level(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = razer_chroma_misc_get_battery_level();

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: charge_level not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return sprintf(buf, "%d\n", response.arguments[1]);
}

/**
 * Read device file "charge_status"
 *
 * Returns 0 when not charging, 1 when charging
 */
static ssize_t razer_attr_read_charge_status(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = razer_chroma_misc_get_charging_status();

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: charge_status not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return sprintf(buf, "%d\n", response.arguments[1]);
}

/**
 * Write device file "charge_effect"
 *
 * Sets charging effect.
 */
static ssize_t razer_attr_write_charge_effect(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    if (count != 1) {
        printk(KERN_WARNING "razerkbd: Incorrect number of bytes for setting the charging effect\n");
        return -EINVAL;
    }

    request = razer_chroma_misc_set_dock_charge_type(buf[0]);
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Write device file "charge_colour"
 *
 * Sets charging colour using 3 RGB bytes
 */
static ssize_t razer_attr_write_charge_colour(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    // First enable static charging effect
    request = razer_chroma_misc_set_dock_charge_type(0x01);
    request.transaction_id.id = 0xFF;
    razer_send_payload(device, &request, &response);

    if (count != 3) {
        printk(KERN_WARNING "razerkbd: Charging colour mode only accepts RGB (3byte)\n");
        return -EINVAL;
    }

    request = razer_chroma_standard_set_led_rgb(NOSTORE, BATTERY_LED, (struct razer_rgb*)&buf[0]);
    request.transaction_id.id = 0xFF;
    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Read device file "charge_low_threshold"
 */
static ssize_t razer_attr_read_charge_low_threshold(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = razer_chroma_misc_get_low_battery_threshold();
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);

    return sprintf(buf, "%d\n", response.arguments[0]);
}

/**
 * Write device file "charge_low_threshold"
 *
 * Sets the low battery blink threshold to the ASCII number written to this file.
 */
static ssize_t razer_attr_write_charge_low_threshold(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    unsigned char threshold = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = razer_chroma_misc_set_low_battery_threshold(threshold);
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);
    return count;
}

/**
 * Write device file "game_led_state"
 *
 * When 1 is written (as a character, 0x31) Game mode will be enabled, if 0 is written (0x30)
 * then game mode will be disabled
 *
 * The reason the keyboard appears as 2 keyboard devices is that one of those devices is used by
 * game mode as that keyboard device is missing a super key. A hacky and over-the-top way to disable
 * the super key if you ask me.
 */
static ssize_t razer_attr_write_game_led_state(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: game_led_state not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Read device file "game_led_state"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_game_led_state(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: game_led_state not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);
    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "keyswitch_optimization"
 */
static ssize_t razer_attr_write_keyswitch_optimization(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};
    unsigned char mode = (unsigned char)simple_strtoul(buf, NULL, 10);

    // Toggle Keyswitch Optimization
    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: keyswitch_optimization not supported for this model\n");
        return -EINVAL;
    }

    return count;
}

/**
 * Read device file "keyswitch_optimization"
 */
static ssize_t razer_attr_read_keyswitch_optimization(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};
    int state;

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: keyswitch_optimization not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    if(response.arguments[1] == 0x14) { // Either 0x00 or 0x14
        state = 0; // Typing
    } else {
        state = 1; // Gaming
    }

    return sprintf(buf, "%d\n", state);
}

/**
 * Write device file "macro_led_state"
 *
 * When 1 is written (as a character, 0x31) Macro mode will be enabled, if 0 is written (0x30)
 * then game mode will be disabled
 */
static ssize_t razer_attr_write_macro_led_state(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = razer_chroma_standard_set_led_state(VARSTORE, MACRO_LED, enabled);
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Read device file "macro_led_state"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_macro_led_state(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = razer_chroma_standard_get_led_state(VARSTORE, MACRO_LED);
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);
    return sprintf(buf, "%d\n", response.arguments[2]);
}

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
    struct razer_kbd_device *device = dev_get_drvdata(dev);

    char *device_type;

    switch (device->usb_pid) {

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
        device_type = "Razer Blade Stealth";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019:
        device_type = "Razer Blade Stealth (Late 2019)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020:
        device_type = "Razer Blade Stealth (Early 2020)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020:
        device_type = "Razer Blade Stealth (Late 2020)";
        break;

    case USB_DEVICE_ID_RAZER_BOOK_2020:
        device_type = "Razer Book 13 (2020)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
        device_type = "Razer Blade Stealth (Late 2016)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
        device_type = "Razer Blade Stealth (Mid 2017)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_QHD:
        device_type = "Razer Blade Stealth (QHD)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
        device_type = "Razer Blade Pro (Late 2016)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_2018:
        device_type = "Razer Blade 15 (2018)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
        device_type = "Razer Blade 15 (2018) Mercury";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_2018_BASE:
        device_type = "Razer Blade 15 (2018) Base Model";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
        device_type = "Razer Blade 15 (2019) Advanced";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_2019_BASE:
        device_type = "Razer Blade 15 (2019) Base Model";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE:
        device_type = "Razer Blade 15 Base (Early 2020)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE:
        device_type = "Razer Blade 15 Base (Late 2020)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
        device_type = "Razer Blade 15 (Mid 2019) Mercury White";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
        device_type = "Razer Blade 15 Studio Edition (2019)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
        device_type = "Razer Blade (Late 2016)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
        device_type = "Razer Blade Pro (2017)";
        break;
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
        device_type = "Razer Blade Pro FullHD (2017)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
        device_type = "Razer Blade Pro (2019)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
        device_type = "Razer Blade Pro (Late 2019)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
        device_type = "Razer Blade Advanced (Late 2019)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
        device_type = "Razer Blade Pro (Early 2020)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
        device_type = "Razer Blade Stealth (Late 2017)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019:
        device_type = "Razer Blade Stealth (2019)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
        device_type = "Razer Blade 15 Advanced (2020)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
        device_type = "Razer Blade 15 Advanced (Early 2021)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
        device_type = "Razer Blade 15 Advanced (Mid 2021)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021:
        device_type = "Razer Blade 15 Base (Early 2021)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022:
        device_type = "Razer Blade 15 Base (2022)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
        device_type = "Razer Blade 17 Pro (Early 2021)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
        device_type = "Razer Blade 17 Pro (Mid 2021)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
        device_type = "Razer Blade 14 (2021)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
        device_type = "Razer Blade 17 (2022)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
        device_type = "Razer Blade 14 (2022)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
        device_type = "Razer Blade 15 Advanced (Early 2022)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
        device_type = "Razer Blade 14 (2023)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
        device_type = "Razer Blade 14 (2024)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
        device_type = "Razer Blade 14 (2025)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
        device_type = "Razer Blade 15 (2023)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
        device_type = "Razer Blade 16 (2023)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
        device_type = "Razer Blade 16 (2025)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
        device_type = "Razer Blade 18 (2023)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
        device_type = "Razer Blade 18 (2024)";
        break;

    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        device_type = "Razer Blade 18 (2025)";
        break;

    default:
        device_type = "Unknown Device";
    }

    return sprintf(buf, "%s\n", device_type);
}

/**
 * Write device file "macro_led_effect"
 *
 * When 1 is written the LED will blink, 0 will static
 */
static ssize_t razer_attr_write_macro_led_effect(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: macro_led_effect not supported for this model\n");
        return -EINVAL;
    }
    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Read device file "macro_led_effect"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_macro_led_effect(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = razer_chroma_standard_get_led_effect(VARSTORE, MACRO_LED);
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "matrix_effect_pulsate"
 *
 * The brightness oscillates between fully on and fully off generating a pulsing effect
 */
static ssize_t razer_attr_write_matrix_effect_pulsate(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: matrix_effect_pulsate not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Read device file "matrix_effect_pulsate"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_matrix_effect_pulsate(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = razer_chroma_standard_get_led_effect(VARSTORE, LOGO_LED);
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Read device file "profile_led_red"
 *
 * Actually a Yellow LED
 *
 * Returns a string
 */
static ssize_t razer_attr_read_profile_led_red(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: profile_led_red not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Read device file "profile_led_green"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_profile_led_green(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: profile_led_green not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Read device file "profile_led_blue"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_profile_led_blue(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: profile_led_blue not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return sprintf(buf, "%d\n", response.arguments[2]);
}

/**
 * Write device file "profile_led_red"
 */
static ssize_t razer_attr_write_profile_led_red(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: profile_led_red not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Write device file "profile_led_green"
 */
static ssize_t razer_attr_write_profile_led_green(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: profile_led_green not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);
    return count;
}

/**
 * Write device file "profile_led_blue"
 */
static ssize_t razer_attr_write_profile_led_blue(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    unsigned char enabled = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: profile_led_blue not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);
    return count;
}

/**
 * Read device file "device_serial"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_device_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    char serial_string[51];
    struct razer_report request = {0};
    struct razer_report response = {0};

    /* For Blade laptops we get the serial number from DMI */
    if (is_blade_laptop(device)) {
        strncpy(&serial_string[0], dmi_get_system_info(DMI_PRODUCT_SERIAL), 50);
        goto exit;
    }

    request = razer_chroma_standard_get_serial();
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);

    strncpy(&serial_string[0], &response.arguments[0], 22);
    serial_string[22] = '\0';

exit:
    return sprintf(buf, "%s\n", &serial_string[0]);
}

/**
 * Read device file "firmware_version"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_firmware_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = razer_chroma_standard_get_firmware_version();
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);

    return sprintf(buf, "v%d.%d\n", response.arguments[0], response.arguments[1]);
}

/**
 * Write device file "matrix_effect_none"
 *
 * No keyboard effect is activated whenever this file is written to
 */
static ssize_t razer_attr_write_matrix_effect_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
    case USB_DEVICE_ID_RAZER_BLADE_2018:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019:
    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
    case USB_DEVICE_ID_RAZER_BLADE_2018_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_2019_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
    case USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE:
    case USB_DEVICE_ID_RAZER_BOOK_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        request = razer_chroma_standard_matrix_effect_none();
        request.transaction_id.id = 0xFF;
        break;

    default:
        printk(KERN_WARNING "razerkbd: matrix_effect_none not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Write device file "matrix_effect_wave"
 *
 * When 1 is written (as a character, 0x31) the wave effect is displayed moving left across the keyboard
 * if 2 is written (0x32) then the wave effect goes right
 *
 * For the extended its 0x00 and 0x01
 */
static ssize_t razer_attr_write_matrix_effect_wave(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    unsigned char direction = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
    case USB_DEVICE_ID_RAZER_BLADE_2018:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        request = razer_chroma_standard_matrix_effect_wave(direction);
        request.transaction_id.id = 0xFF;
        break;

    default:
        printk(KERN_WARNING "razerkbd: matrix_effect_wave not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Write device file "matrix_effect_wheel"
 *
 * When 1 is written (as a character, 0x31) the wheel effect is turning right
 * if 2 is written (0x32) then the wheel effect goes left.
 */
static ssize_t razer_attr_write_matrix_effect_wheel(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    unsigned char direction = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch(device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: matrix_effect_wheel not supported for this model\n");
        return -EINVAL;
    }
    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Write device file "matrix_effect_spectrum"
 *
 * Spectrum effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_matrix_effect_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022:
        request = razer_chroma_extended_matrix_effect_spectrum(VARSTORE, BACKLIGHT_LED);
        request.transaction_id.id = 0x1F;
        break;

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
    case USB_DEVICE_ID_RAZER_BLADE_2018:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019:
    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
    case USB_DEVICE_ID_RAZER_BLADE_2018_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_2019_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
    case USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE:
    case USB_DEVICE_ID_RAZER_BOOK_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        request = razer_chroma_standard_matrix_effect_spectrum();
        request.transaction_id.id = 0xFF;
        break;

    default:
        printk(KERN_WARNING "razerkbd: matrix_effect_spectrum not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Write device file "matrix_effect_reactive"
 *
 * Sets reactive mode when this file is written to. A speed byte and 3 RGB bytes should be written
 */
static ssize_t razer_attr_write_matrix_effect_reactive(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};
    unsigned char speed;

    if (count != 4) {
        printk(KERN_WARNING "razerkbd: Reactive only accepts Speed, RGB (4byte)\n");
        return -EINVAL;
    }

    speed = (unsigned char)buf[0];

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022:
        request = razer_chroma_extended_matrix_effect_reactive(VARSTORE, BACKLIGHT_LED, speed, (struct razer_rgb*)&buf[1]);
        request.transaction_id.id = 0x1F;
        break;

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
    case USB_DEVICE_ID_RAZER_BLADE_2018:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019:
    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
    case USB_DEVICE_ID_RAZER_BLADE_2018_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_2019_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
    case USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE:
    case USB_DEVICE_ID_RAZER_BOOK_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        request = razer_chroma_standard_matrix_effect_reactive(speed, (struct razer_rgb*)&buf[1]);
        request.transaction_id.id = 0xFF;
        break;

    default:
        printk(KERN_WARNING "razerkbd: matrix_effect_reactive not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Write device file "matrix_effect_static"
 *
 * Set the keyboard to static mode when 3 RGB bytes are written
 */
static ssize_t razer_attr_write_matrix_effect_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020:
    case USB_DEVICE_ID_RAZER_BOOK_2020:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_2018:
    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_2018_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
    case USB_DEVICE_ID_RAZER_BLADE_2019_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        if (count != 3) {
            printk(KERN_WARNING "razerkbd: Static mode only accepts RGB (3byte)\n");
            return -EINVAL;
        }
        request = razer_chroma_standard_matrix_effect_static((struct razer_rgb*)&buf[0]);
        request.transaction_id.id = 0xFF;
        razer_send_payload(device, &request, &response);
        break;

    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022:
        if (count != 3) {
            printk(KERN_WARNING "razerkbd: Static mode only accepts RGB (3byte)\n");
            return -EINVAL;
        }
        request = razer_chroma_extended_matrix_effect_static(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0]);
        request.transaction_id.id = 0x1F;
        razer_send_payload(device, &request, &response);
        break;

    default:
        printk(KERN_WARNING "razerkbd: Cannot set static mode for this device\n");
        return -EINVAL;
    }

    return count;
}

/**
 * Write device file "matrix_effect_starlight"
 *
 * Starlight keyboard effect is activated whenever this file is written to (for bw2016)
 *
 * Or if an Ornata
 * 7 bytes, speed, rgb, rgb
 * 4 bytes, speed, rgb
 * 1 byte, speed
 */
static ssize_t razer_attr_write_matrix_effect_starlight(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_rgb rgb1 = {.r = 0x00, .g = 0xFF, .b = 0x00};
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
        if(count == 7) {
            request = razer_chroma_standard_matrix_effect_starlight_dual(buf[0], (struct razer_rgb*)&buf[1], (struct razer_rgb*)&buf[4]);
            request.transaction_id.id = 0xFF;
            razer_send_payload(device, &request, &response);
        } else if(count == 4) {
            request = razer_chroma_standard_matrix_effect_starlight_single(buf[0], (struct razer_rgb*)&buf[1]);
            request.transaction_id.id = 0xFF;
            razer_send_payload(device, &request, &response);
        } else if(count == 1) {
            request = razer_chroma_standard_matrix_effect_starlight_random(buf[0]);
            request.transaction_id.id = 0xFF;
            razer_send_payload(device, &request, &response);
        } else {
            printk(KERN_WARNING "razerkbd: Starlight only accepts Speed (1byte). Speed, RGB (4byte). Speed, RGB, RGB (7byte)\n");
            return -EINVAL;
        }
        break;

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
    case USB_DEVICE_ID_RAZER_BLADE_2018:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        request = razer_chroma_standard_matrix_effect_starlight_single(0x01, &rgb1);
        request.transaction_id.id = 0xFF;
        razer_send_payload(device, &request, &response);
        break;

    default:
        printk(KERN_WARNING "razerkbd: matrix_effect_starlight not supported for this model\n");
        return -EINVAL;
    }

    return count;
}

/**
 * Write device file "matrix_effect_breath"
 */
static ssize_t razer_attr_write_matrix_effect_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022:
        switch(count) {
        case 3: // Single colour mode
            request = razer_chroma_extended_matrix_effect_breathing_single(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0]);
            request.transaction_id.id = 0x3F;
            razer_send_payload(device, &request, &response);
            request.transaction_id.id = 0x1F;
            break;

        case 6: // Dual colour mode
            request = razer_chroma_extended_matrix_effect_breathing_dual(VARSTORE, BACKLIGHT_LED, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
            request.transaction_id.id = 0x3F;
            razer_send_payload(device, &request, &response);
            request.transaction_id.id = 0x1F;
            break;

        case 1: // "Random" colour mode
            request = razer_chroma_extended_matrix_effect_breathing_random(VARSTORE, BACKLIGHT_LED);
            request.transaction_id.id = 0x3F;
            razer_send_payload(device, &request, &response);
            request.transaction_id.id = 0x1F;
            break;

        default:
            printk(KERN_WARNING "razerkbd: Breathing only accepts '1' (1byte). RGB (3byte). RGB, RGB (6byte)\n");
            return -EINVAL;
        }
        break;

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
    case USB_DEVICE_ID_RAZER_BLADE_2018:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019:
    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
    case USB_DEVICE_ID_RAZER_BLADE_2018_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_2019_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
    case USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE:
    case USB_DEVICE_ID_RAZER_BOOK_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        switch(count) {
        case 3: // Single colour mode
            request = razer_chroma_standard_matrix_effect_breathing_single((struct razer_rgb*)&buf[0]);
            request.transaction_id.id = 0xFF;
            razer_send_payload(device, &request, &response);
            break;

        case 6: // Dual colour mode
            request = razer_chroma_standard_matrix_effect_breathing_dual((struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
            request.transaction_id.id = 0xFF;
            razer_send_payload(device, &request, &response);
            break;

        default: // "Random" colour mode
            request = razer_chroma_standard_matrix_effect_breathing_random();
            request.transaction_id.id = 0xFF;
            razer_send_payload(device, &request, &response);
            break;
            // TODO move default to case 1:. Then default: printk(warning). Also remove pointless buffer
        }
        break;

    default:
        printk(KERN_WARNING "razerkbd: matrix_effect_breath not supported for this model\n");
        return -EINVAL;
    }

    return count;
}

static int has_inverted_led_state(struct device *dev)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
        return 1;
    default:
        return 0;
    }
}

/**
 * Reads device file "logo_led_state"
 *
 * Reads the logo lighting state (the ASCII number) written to this file.
 */
static ssize_t razer_attr_read_logo_led_state(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};
    int state;

    request = razer_chroma_standard_get_led_effect(VARSTORE, LOGO_LED);
    request.transaction_id.id = 0xFF;

    // Blade laptops don't use effect for logo on/off, and mode 2 ("blink") is technically unsupported.
    if (is_blade_laptop(device)) {
        request = razer_chroma_standard_get_led_state(VARSTORE, LOGO_LED);
        request.transaction_id.id = 0xFF;
    }

    razer_send_payload(device, &request, &response);
    state = response.arguments[2];

    if (has_inverted_led_state(dev) && (state == 0 || state == 1))
        state = !state;

    return sprintf(buf, "%d\n", state);
}

/**
 * Write device file "logo_led_state"
 *
 * Sets the logo lighting state to the ASCII number written to this file.
 */
static ssize_t razer_attr_write_logo_led_state(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    unsigned char state = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report request = {0};
    struct razer_report response = {0};

    if (has_inverted_led_state(dev) && (state == 0 || state == 1))
        state = !state;

    // Blade laptops are... different. They use state instead of effect.
    // Note: This does allow setting of mode 2 ("blink"), but this is an undocumented feature.
    if (is_blade_laptop(device) && (state == 0 || state == 1)) {
        request = razer_chroma_standard_set_led_state(VARSTORE, LOGO_LED, state);
        request.transaction_id.id = 0xFF;
    } else {
        request = razer_chroma_standard_set_led_effect(VARSTORE, LOGO_LED, state);
        request.transaction_id.id = 0xFF;
    }

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Write device file "matrix_effect_custom"
 *
 * Sets the keyboard to custom mode whenever the file is written to
 */
static ssize_t razer_attr_write_matrix_effect_custom(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};
    bool want_response = true;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
    case USB_DEVICE_ID_RAZER_BLADE_2018:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        request = razer_chroma_standard_matrix_effect_custom_frame(NOSTORE);
        request.transaction_id.id = 0xFF;
        break;

    default:
        printk(KERN_WARNING "razerkbd: matrix_effect_custom not supported for this model\n");
        return -EINVAL;
    }

    /* See comment in razer_attr_write_matrix_custom_frame for want_response */
    if (want_response)
        razer_send_payload(device, &request, &response);
    else
        razer_send_payload_no_response(device, &request);

    return count;
}

/**
 * Write device file "fn_toggle"
 *
 * Sets the logo lighting state to the ASCII number written to this file.
 */
static ssize_t razer_attr_write_fn_toggle(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    unsigned char state = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = razer_chroma_misc_fn_key_toggle(state);
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Write device file "test"
 *
 * Does nothing
 */
static ssize_t razer_attr_write_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return count;
}

/**
 * Read device file "test"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_test(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = get_razer_report(0x00, 0x86, 0x02);
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);

    print_erroneous_report(&response, "razerkbd", "Test");
    return sprintf(buf, "%02x%02x%02x\n", response.arguments[0], response.arguments[1], response.arguments[2]);
}

/**
 * Write device file "matrix_brightness"
 *
 * Sets the brightness to the ASCII number written to this file.
 */
static ssize_t razer_attr_write_matrix_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    unsigned char brightness = (unsigned char)simple_strtoul(buf, NULL, 10);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
    case USB_DEVICE_ID_RAZER_BLADE_2018:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019:
    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
    case USB_DEVICE_ID_RAZER_BLADE_2018_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_2019_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
    case USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE:
    case USB_DEVICE_ID_RAZER_BOOK_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        if (is_blade_laptop(device)) {
            request = razer_chroma_misc_set_blade_brightness(brightness);
        } else {
            request = razer_chroma_standard_set_led_brightness(VARSTORE, BACKLIGHT_LED, brightness);
        }
        request.transaction_id.id = 0xFF;
        break;

    default:
        printk(KERN_WARNING "razerkbd: matrix_brightness not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Read device file "matrix_brightness"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_matrix_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    unsigned char brightness = 0;
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {

    case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
    case USB_DEVICE_ID_RAZER_BLADE_QHD:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
    case USB_DEVICE_ID_RAZER_BLADE_2018:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019:
    case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
    case USB_DEVICE_ID_RAZER_BLADE_2018_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
    case USB_DEVICE_ID_RAZER_BLADE_2019_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
    case USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE:
    case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
    case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020:
    case USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE:
    case USB_DEVICE_ID_RAZER_BOOK_2020:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021:
    case USB_DEVICE_ID_RAZER_BLADE_14_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
    case USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022:
    case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
    case USB_DEVICE_ID_RAZER_BLADE_17_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2022:
    case USB_DEVICE_ID_RAZER_BLADE_14_2023:
    case USB_DEVICE_ID_RAZER_BLADE_15_2023:
    case USB_DEVICE_ID_RAZER_BLADE_16_2023:
    case USB_DEVICE_ID_RAZER_BLADE_18_2023:
    case USB_DEVICE_ID_RAZER_BLADE_14_2024:
    case USB_DEVICE_ID_RAZER_BLADE_14_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2024:
    case USB_DEVICE_ID_RAZER_BLADE_16_2025:
    case USB_DEVICE_ID_RAZER_BLADE_18_2025:
        if (is_blade_laptop(device)) {
            request = razer_chroma_misc_get_blade_brightness();
        } else {
            request = razer_chroma_standard_get_led_brightness(VARSTORE, BACKLIGHT_LED);
        }
        request.transaction_id.id = 0xFF;
        break;

    default:
        printk(KERN_WARNING "razerkbd: matrix_brightness not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    // Brightness is stored elsewhere for the stealth cmds
    if (is_blade_laptop(device)) {
        brightness = response.arguments[1];
    } else {
        brightness = response.arguments[2];
    }

    return sprintf(buf, "%d\n", brightness);
}

/**
 * Write device file "device_mode"
 */
static ssize_t razer_attr_write_device_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    if (count != 2) {
        printk(KERN_WARNING "razerkbd: Device mode only takes 2 bytes.\n");
        return -EINVAL;
    }

    // No-op on Blades
    if (is_blade_laptop(device)) {
        return count;
    }

    request = razer_chroma_standard_set_device_mode(buf[0], buf[1]);
    request.transaction_id.id = 0xFF;
    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Read device file "device_mode"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_device_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};

    request = razer_chroma_standard_get_device_mode();
    request.transaction_id.id = 0xFF;

    razer_send_payload(device, &request, &response);

    buf[0] = response.arguments[0];
    buf[1] = response.arguments[1];

    return 2;
}

/**
 * Write device file "matrix_custom_frame"
 *
 * Format
 * ROW_ID START_COL STOP_COL RGB...
 */
static ssize_t razer_attr_write_matrix_custom_frame(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};
    size_t offset = 0;
    unsigned char row_id, start_col, stop_col;
    size_t row_length;
    bool want_response = true;

    while(offset < count) {
        if(offset + 3 > count) {
            printk(KERN_ALERT "razerkbd: Wrong Amount of data provided: Should be ROW_ID, START_COL, STOP_COL, N_RGB\n");
            return -EINVAL;
        }

        row_id = buf[offset++];
        start_col = buf[offset++];
        stop_col = buf[offset++];

        // Validate parameters
        if(start_col > stop_col) {
            printk(KERN_ALERT "razerkbd: Start column (%u) is greater than end column (%u)\n", start_col, stop_col);
            return -EINVAL;
        }

        row_length = ((stop_col + 1) - start_col) * 3;

        // Make sure we actually got the data that was promised to us
        if(count < offset + row_length) {
            printk(KERN_ALERT "razerkbd: Not enough RGB to fill row (expecting %lu bytes of RGB data, got %lu)\n", row_length, (count - 3));
            return -EINVAL;
        }

        // printk(KERN_INFO "razerkbd: Row ID: %u, Start: %u, Stop: %u, row length: %lu\n", row_id, start_col, stop_col, row_length);

        // Offset now at beginning of RGB data

        switch (device->usb_pid) {
        case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
            request = razer_chroma_standard_matrix_set_custom_frame(row_id, start_col, stop_col, (unsigned char*)&buf[offset]);
            request.transaction_id.id = 0x3F;
            break;

        case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
        case USB_DEVICE_ID_RAZER_BLADE_QHD:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
        case USB_DEVICE_ID_RAZER_BLADE_2018:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
        case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
        case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
        case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
        case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
        case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
        case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
        case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
        case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
        case USB_DEVICE_ID_RAZER_BLADE_14_2021:
        case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
        case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
        case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
        case USB_DEVICE_ID_RAZER_BLADE_17_2022:
        case USB_DEVICE_ID_RAZER_BLADE_14_2022:
        case USB_DEVICE_ID_RAZER_BLADE_14_2023:
        case USB_DEVICE_ID_RAZER_BLADE_15_2023:
        case USB_DEVICE_ID_RAZER_BLADE_16_2023:
        case USB_DEVICE_ID_RAZER_BLADE_18_2023:
        case USB_DEVICE_ID_RAZER_BLADE_14_2024:
        case USB_DEVICE_ID_RAZER_BLADE_14_2025:
        case USB_DEVICE_ID_RAZER_BLADE_18_2024:
        case USB_DEVICE_ID_RAZER_BLADE_16_2025:
        case USB_DEVICE_ID_RAZER_BLADE_18_2025:
            request = razer_chroma_standard_matrix_set_custom_frame(row_id, start_col, stop_col, (unsigned char*)&buf[offset]);
            request.transaction_id.id = 0xFF;
            break;

        default:
            printk(KERN_WARNING "razerkbd: matrix_custom_frame not supported for this model\n");
            return -EINVAL;
        }

        /*
         * Some devices don't like us asking for responses for custom frame
         * requests. And in any case it shouldn't be necessary for most devices
         * but let's keep it enabled by default for now to not potentially
         * break anything.
         */
        if (want_response)
            razer_send_payload(device, &request, &response);
        else
            razer_send_payload_no_response(device, &request);

        // *3 as its 3 bytes per col (RGB)
        offset += row_length;
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
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    struct razer_report request = {0};
    struct razer_report response = {0};
    unsigned short polling_rate = 0;

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: poll_rate not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    switch(response.arguments[1]) {
    case 0x01:
        polling_rate = 8000;
        break;
    case 0x02:
        polling_rate = 4000;
        break;
    case 0x04:
        polling_rate = 2000;
        break;
    case 0x08:
        polling_rate = 1000;
        break;
    case 0x10:
        polling_rate = 500;
        break;
    case 0x20:
        polling_rate = 250;
        break;
    case 0x40:
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
    struct razer_kbd_device *device = dev_get_drvdata(dev);
    unsigned short polling_rate = (unsigned short)simple_strtoul(buf, NULL, 10);
    struct razer_report request = {0};
    struct razer_report response = {0};

    switch (device->usb_pid) {
    default:
        printk(KERN_WARNING "razerkbd: poll_rate not supported for this model\n");
        return -EINVAL;
    }

    razer_send_payload(device, &request, &response);

    return count;
}

/**
 * Write device file "key_super"
 */
static ssize_t razer_attr_write_key_super(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);

    if (count < 1) {
        printk(KERN_ALERT "razerkbd: Failed to provide argument\n");
        return -EINVAL;
    }

    device->block_keys[0] = buf[0];

    return count;
}

/**
 * Read device file "key_super"
 */
static ssize_t razer_attr_read_key_super(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);

    buf[0] = device->block_keys[0];

    return 1;
}

/**
 * Write device file "key_alt_tab"
 */
static ssize_t razer_attr_write_key_alt_tab(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);

    if (count < 1) {
        printk(KERN_ALERT "razerkbd: Failed to provide argument\n");
        return -EINVAL;
    }

    printk(KERN_WARNING "razerkbd: Settings block_keys[1] to %u\n", buf[0]);
    device->block_keys[1] = buf[0];

    return count;
}

/**
 * Read device file "read_key_alt_tab"
 */
static ssize_t razer_attr_read_key_alt_tab(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);

    buf[0] = device->block_keys[1];

    return 1;
}

/**
 * Write device file "write_key_alt_f4"
 */
static ssize_t razer_attr_write_key_alt_f4(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);

    if (count < 1) {
        printk(KERN_ALERT "razerkbd: Failed to provide argument\n");
        return -EINVAL;
    }

    device->block_keys[2] = buf[0];

    return count;
}

/**
 * Read device file "read_key_alt_f4"
 */
static ssize_t razer_attr_read_key_alt_f4(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kbd_device *device = dev_get_drvdata(dev);

    buf[0] = device->block_keys[2];

    return 1;
}

/**
 * Set up the device driver files

 *
 * Read only is 0444
 * Write only is 0220
 * Read and write is 0664
 */
static DEVICE_ATTR(game_led_state,          0660, razer_attr_read_game_led_state,             razer_attr_write_game_led_state);
static DEVICE_ATTR(macro_led_state,         0660, razer_attr_read_macro_led_state,            razer_attr_write_macro_led_state);
static DEVICE_ATTR(macro_led_effect,        0660, razer_attr_read_macro_led_effect,           razer_attr_write_macro_led_effect);
static DEVICE_ATTR(logo_led_state,          0660, razer_attr_read_logo_led_state,             razer_attr_write_logo_led_state);
static DEVICE_ATTR(profile_led_red,         0660, razer_attr_read_profile_led_red,            razer_attr_write_profile_led_red);
static DEVICE_ATTR(profile_led_green,       0660, razer_attr_read_profile_led_green,          razer_attr_write_profile_led_green);
static DEVICE_ATTR(profile_led_blue,        0660, razer_attr_read_profile_led_blue,           razer_attr_write_profile_led_blue);

static DEVICE_ATTR(test,                    0660, razer_attr_read_test,                       razer_attr_write_test);
static DEVICE_ATTR(version,                 0440, razer_attr_read_version,                    NULL);
static DEVICE_ATTR(kbd_layout,              0440, razer_attr_read_kbd_layout,                 NULL);

static DEVICE_ATTR(firmware_version,        0440, razer_attr_read_firmware_version,           NULL);
static DEVICE_ATTR(fn_toggle,               0220, NULL,                                       razer_attr_write_fn_toggle);
static DEVICE_ATTR(poll_rate,               0660, razer_attr_read_poll_rate,                  razer_attr_write_poll_rate);
static DEVICE_ATTR(keyswitch_optimization,  0660, razer_attr_read_keyswitch_optimization,     razer_attr_write_keyswitch_optimization);

static DEVICE_ATTR(device_type,             0440, razer_attr_read_device_type,                NULL);
static DEVICE_ATTR(device_mode,             0660, razer_attr_read_device_mode,                razer_attr_write_device_mode);
static DEVICE_ATTR(device_serial,           0440, razer_attr_read_device_serial,              NULL);

static DEVICE_ATTR(matrix_effect_none,      0220, NULL,                                       razer_attr_write_matrix_effect_none);
static DEVICE_ATTR(matrix_effect_wave,      0220, NULL,                                       razer_attr_write_matrix_effect_wave);
static DEVICE_ATTR(matrix_effect_wheel,     0220, NULL,                                       razer_attr_write_matrix_effect_wheel);
static DEVICE_ATTR(matrix_effect_spectrum,  0220, NULL,                                       razer_attr_write_matrix_effect_spectrum);
static DEVICE_ATTR(matrix_effect_reactive,  0220, NULL,                                       razer_attr_write_matrix_effect_reactive);
static DEVICE_ATTR(matrix_effect_static,    0220, NULL,                                       razer_attr_write_matrix_effect_static);
static DEVICE_ATTR(matrix_effect_starlight, 0220, NULL,                                       razer_attr_write_matrix_effect_starlight);
static DEVICE_ATTR(matrix_effect_breath,    0220, NULL,                                       razer_attr_write_matrix_effect_breath);
static DEVICE_ATTR(matrix_effect_pulsate,   0660, razer_attr_read_matrix_effect_pulsate,      razer_attr_write_matrix_effect_pulsate);
static DEVICE_ATTR(matrix_brightness,       0660, razer_attr_read_matrix_brightness,          razer_attr_write_matrix_brightness);
static DEVICE_ATTR(matrix_effect_custom,    0220, NULL,                                       razer_attr_write_matrix_effect_custom);
static DEVICE_ATTR(matrix_custom_frame,     0220, NULL,                                       razer_attr_write_matrix_custom_frame);

static DEVICE_ATTR(key_super,               0660, razer_attr_read_key_super,                  razer_attr_write_key_super);
static DEVICE_ATTR(key_alt_tab,             0660, razer_attr_read_key_alt_tab,                razer_attr_write_key_alt_tab);
static DEVICE_ATTR(key_alt_f4,              0660, razer_attr_read_key_alt_f4,                 razer_attr_write_key_alt_f4);

static DEVICE_ATTR(charge_level,            0440, razer_attr_read_charge_level,               NULL);
static DEVICE_ATTR(charge_status,           0440, razer_attr_read_charge_status,              NULL);
static DEVICE_ATTR(charge_effect,           0220, NULL,                                       razer_attr_write_charge_effect);
static DEVICE_ATTR(charge_colour,           0220, NULL,                                       razer_attr_write_charge_colour);
static DEVICE_ATTR(charge_low_threshold,    0660, razer_attr_read_charge_low_threshold,       razer_attr_write_charge_low_threshold);

/* ── Fan hwmon integration ────────────────────────────────────────────────── */

#define RAZER_FAN_PWM_MANUAL 0
#define RAZER_FAN_PWM_AUTO   1

struct razer_fan_spec {
    u16 pid;
    u16 fan_min;
    u16 fan_max;
};

static const struct razer_fan_spec razer_fan_specs[] = {
    { USB_DEVICE_ID_RAZER_BLADE_LATE_2016,           3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_2018,                3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_2018_BASE,           3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY,        3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_2019_BASE,           3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_2019_ADV,            3500, 5300 },
    { USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY,    3500, 5300 },
    { USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE,     3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE,      3600, 5200 },
    { USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020,         3500, 5300 },
    { USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017,    3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017,   3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019,        3500, 5300 },
    { USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019,   3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020,  3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020,   3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020,      3500, 5300 },
    { USB_DEVICE_ID_RAZER_BLADE_PRO_2019,            3500, 5300 },
    { USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD,     3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_PRO_2017,            3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016,       3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_QHD,                 3500, 5000 },
    { USB_DEVICE_ID_RAZER_BOOK_2020,                 3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021,  3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_14_2021,             3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021,     3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021,   3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022,        3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022,   3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_17_2022,             3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_14_2022,             3500, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_16_2023,             2200, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_14_2023,             2200, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021,   2300, 4300 },
    { USB_DEVICE_ID_RAZER_BLADE_14_2024,             2200, 5000 },
    { USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021,     2300, 4300 },
    { USB_DEVICE_ID_RAZER_BLADE_18_2023,             2200, 5000 },
};

static const struct razer_fan_spec *razer_find_fan_spec(u16 pid)
{
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(razer_fan_specs); i++) {
        if (razer_fan_specs[i].pid == pid)
            return &razer_fan_specs[i];
    }
    return NULL;
}

struct razer_fan_data {
    struct razer_kbd_device *device;
    u16  fan_min;
    u16  fan_max;
    u8   fan_power_mode;
    u8   fan_pwm;
    u8   fan_pwm_enable;
    struct mutex fan_lock;
};

static void razer_fan_send(struct razer_kbd_device *device,
                           u8 cmd_class, u8 cmd_id, u8 data_size,
                           u8 arg0, u8 arg1, u8 arg2, u8 arg3)
{
    struct razer_report request  = {0};
    struct razer_report response = {0};

    request = get_razer_report(cmd_class, cmd_id, data_size);
    request.transaction_id.id = 0x1F;
    request.arguments[0] = arg0;
    request.arguments[1] = arg1;
    request.arguments[2] = arg2;
    request.arguments[3] = arg3;
    razer_send_payload(device, &request, &response);
}


static u8 razer_fan_pwm_to_rpm100(struct razer_fan_data *fan, u8 pwm)
{
    u32 range = fan->fan_max - fan->fan_min;

    return (u8)((fan->fan_min + range * pwm / 255) / 100);
}

static void razer_fan_apply(struct razer_fan_data *fan)
{
    u8 rpm100;
    bool manual;
    u8 zone;

    switch (fan->fan_pwm_enable) {
    case RAZER_FAN_PWM_MANUAL:
        rpm100 = razer_fan_pwm_to_rpm100(fan, fan->fan_pwm);
        break;
    default:
        rpm100 = 0;
        break;
    }

    manual = rpm100 != 0;

    for (zone = 0x01; zone <= 0x02; zone++) {
        /* get_power_mode */
        razer_fan_send(fan->device, 0x0d, 0x82, 0x04, 0x00, zone, 0x00, 0x00);
        /* set_power_zone */
        razer_fan_send(fan->device, 0x0d, 0x02, 0x04, 0x00, zone,
                       fan->fan_power_mode, manual ? 1 : 0);
        if (manual)
            /* set_rpm */
            razer_fan_send(fan->device, 0x0d, 0x01, 0x03, 0x00, zone, rpm100, 0x00);
    }
}

#ifdef CONFIG_HWMON

static umode_t razer_laptop_fan_is_visible(const void *data,
        enum hwmon_sensor_types type,
        u32 attr, int channel)
{
    if (type == hwmon_pwm &&
        (attr == hwmon_pwm_enable || attr == hwmon_pwm_input))
        return 0644;
    return 0;
}

static int razer_laptop_fan_read(struct device *dev,
                                 enum hwmon_sensor_types type,
                                 u32 attr, int channel, long *val)
{
    struct razer_fan_data *fan = dev_get_drvdata(dev);

    mutex_lock(&fan->fan_lock);
    switch (attr) {
    case hwmon_pwm_input:
        *val = fan->fan_pwm;
        break;
    case hwmon_pwm_enable:
        *val = fan->fan_pwm_enable;
        break;
    default:
        mutex_unlock(&fan->fan_lock);
        return -EOPNOTSUPP;
    }
    mutex_unlock(&fan->fan_lock);
    return 0;
}

static int razer_laptop_fan_write(struct device *dev,
                                  enum hwmon_sensor_types type,
                                  u32 attr, int channel, long val)
{
    struct razer_fan_data *fan = dev_get_drvdata(dev);
    int ret = 0;

    mutex_lock(&fan->fan_lock);
    switch (attr) {
    case hwmon_pwm_input:
        if (val < 0 || val > 255) {
            ret = -EINVAL;
            break;
        }
        fan->fan_pwm        = (u8)val;
        fan->fan_pwm_enable = RAZER_FAN_PWM_MANUAL;
        razer_fan_apply(fan);
        break;
    case hwmon_pwm_enable:
        switch (val) {
        case 0:
            fan->fan_pwm_enable = RAZER_FAN_PWM_AUTO;
            break;
        case 1:
            fan->fan_pwm_enable = RAZER_FAN_PWM_MANUAL;
            break;
        case 2:
            fan->fan_pwm_enable = RAZER_FAN_PWM_AUTO;
            break;
        default:
            ret = -EINVAL;
            break;
        }
        if (!ret)
            razer_fan_apply(fan);
        break;
    default:
        ret = -EOPNOTSUPP;
    }
    mutex_unlock(&fan->fan_lock);
    return ret;
}

static const struct hwmon_ops razer_laptop_fan_hwmon_ops = {
    .is_visible = razer_laptop_fan_is_visible,
    .read       = razer_laptop_fan_read,
    .write      = razer_laptop_fan_write,
};

static const u32 razer_laptop_fan_pwm_config[] = {
    HWMON_PWM_ENABLE | HWMON_PWM_INPUT,
    0
};

static const struct hwmon_channel_info razer_laptop_fan_pwm_chan = {
    .type   = hwmon_pwm,
    .config = razer_laptop_fan_pwm_config,
};


static const struct hwmon_channel_info * const razer_laptop_fan_hwmon_info[] = {
    &razer_laptop_fan_pwm_chan,
    NULL
};

static const struct hwmon_chip_info razer_laptop_fan_chip_info = {
    .ops  = &razer_laptop_fan_hwmon_ops,
    .info = razer_laptop_fan_hwmon_info,
};

#endif /* CONFIG_HWMON */

static int razer_event(struct hid_device *hdev, struct hid_field *field, struct hid_usage *usage, __s32 value)
{
    return 0;
}

#define RAW_EVENT_BITFIELD_BYTES (20)
#define RAW_EVENT_BITFIELD_BITS (RAW_EVENT_BITFIELD_BYTES * BITS_PER_BYTE)

static int razer_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size)
{
    return 0;
}

/**
 * Set static hid-events translation map
 *
 * Some keyboards generates wheel-events for volume control knob
 */
static int razer_kbd_input_mapping(struct hid_device *hdev, struct hid_input *hidinput, struct hid_field *field,
                                   struct hid_usage *usage, unsigned long **bit, int *max)
{
    switch (hdev->product) {
    default:
        return 0;
    }
}

static void razer_kbd_init(struct razer_kbd_device *dev, struct usb_interface *intf, struct hid_device *hdev)
{
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    // Initialise mutex
    mutex_init(&dev->lock);
    // Setup values
    dev->usb_dev = usb_dev;
    dev->usb_vid = usb_dev->descriptor.idVendor;
    dev->usb_pid = usb_dev->descriptor.idProduct;
    dev->usb_interface_protocol = intf->cur_altsetting->desc.bInterfaceProtocol;
}

/**
 * Probe method is ran whenever a device is binded to the driver
 */
static int razer_laptop_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int retval = 0;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_kbd_device *dev = NULL;

    dev = kzalloc(sizeof(struct razer_kbd_device), GFP_KERNEL);
    if(dev == NULL) {
        dev_err(&intf->dev, "out of memory\n");
        return -ENOMEM;
    }

    // Init data
    razer_kbd_init(dev, intf, hdev);

    // Other interfaces are actual key-emitting devices
    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE) {
        // If the currently bound device is the control (mouse) interface
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_firmware_version);                      // Get the firmware version
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);                         // Get serial number
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_brightness);                     // Gets and sets the brightness
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_test);                                  // Test mode
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);                           // Get string of device type
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_mode);                           // Get device mode
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_kbd_layout);                            // Gets the physical layout

        switch(usb_dev->descriptor.idProduct) {

        case USB_DEVICE_ID_RAZER_BLADE_2018_BASE:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020:
        case USB_DEVICE_ID_RAZER_BOOK_2020:
        case USB_DEVICE_ID_RAZER_BLADE_2019_BASE:
        case USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE:
        case USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE:
        case USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021:
        case USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
            break;

        case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
        case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
        case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_starlight);       // Starlight effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
            break;

        case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_starlight);       // Starlight effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_fn_toggle);                     // Sets whether FN is requires for F-Keys
            break;

        case USB_DEVICE_ID_RAZER_BLADE_QHD:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_starlight);       // Starlight effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_fn_toggle);                     // Sets whether FN is requires for F-Keys
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);                // Enable/Disable the logo
            break;

        case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
        case USB_DEVICE_ID_RAZER_BLADE_2018:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
        case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
        case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
        case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
        case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
        case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
        case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
        case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
        case USB_DEVICE_ID_RAZER_BLADE_14_2021:
        case USB_DEVICE_ID_RAZER_BLADE_17_2022:
        case USB_DEVICE_ID_RAZER_BLADE_14_2022:
        case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
        case USB_DEVICE_ID_RAZER_BLADE_14_2023:
        case USB_DEVICE_ID_RAZER_BLADE_15_2023:
        case USB_DEVICE_ID_RAZER_BLADE_16_2023:
        case USB_DEVICE_ID_RAZER_BLADE_16_2025:
        case USB_DEVICE_ID_RAZER_BLADE_18_2023:
        case USB_DEVICE_ID_RAZER_BLADE_14_2024:
        case USB_DEVICE_ID_RAZER_BLADE_14_2025:
        case USB_DEVICE_ID_RAZER_BLADE_18_2024:
        case USB_DEVICE_ID_RAZER_BLADE_18_2025:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_starlight);       // Starlight effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);                // Enable/Disable the logo
            break;

        case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);            // Wave effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);        // Reactive effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);           // Set LED matrix
            break;
        }

    } else if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD) {
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_key_super);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_key_alt_tab);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_key_alt_f4);
    }

    hid_set_drvdata(hdev, dev);
    dev_set_drvdata(&hdev->dev, dev);

#ifdef CONFIG_HWMON
    if (intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE) {
        const struct razer_fan_spec *fan_spec = razer_find_fan_spec(dev->usb_pid);
        if (fan_spec) {
            struct razer_fan_data *fan_data = devm_kzalloc(&hdev->dev,
                                              sizeof(*fan_data),
                                              GFP_KERNEL);
            if (fan_data) {
                fan_data->device         = dev;
                fan_data->fan_min        = fan_spec->fan_min;
                fan_data->fan_max        = fan_spec->fan_max;
                fan_data->fan_power_mode = 0;
                fan_data->fan_pwm        = 128;
                fan_data->fan_pwm_enable = RAZER_FAN_PWM_AUTO;
                mutex_init(&fan_data->fan_lock);
                devm_hwmon_device_register_with_info(&hdev->dev, "razer_fan",
                                                     fan_data, &razer_laptop_fan_chip_info, NULL);
            }
        }
    }
#endif


    if(hid_parse(hdev)) {
        hid_err(hdev, "parse failed\n");
        goto exit_free;
    }

    if (hid_hw_start(hdev, HID_CONNECT_DEFAULT)) {
        hid_err(hdev, "hw start failed\n");
        goto exit_free;
    }

    //razer_activate_macro_keys(usb_dev);
    //msleep(3000);
    return 0;

exit_free:
    kfree(dev);
    return retval;
}

/**
 * Unbind function
 */
static void razer_laptop_disconnect(struct hid_device *hdev)
{
    struct razer_kbd_device *dev;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    dev = hid_get_drvdata(hdev);

    // Other interfaces are actual key-emitting devices
    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_MOUSE) {
        // If the currently bound device is the control (mouse) interface
        device_remove_file(&hdev->dev, &dev_attr_version);
        device_remove_file(&hdev->dev, &dev_attr_firmware_version);                      // Get the firmware version
        device_remove_file(&hdev->dev, &dev_attr_device_serial);                         // Get serial number
        device_remove_file(&hdev->dev, &dev_attr_matrix_brightness);                     // Gets and sets the brightness
        device_remove_file(&hdev->dev, &dev_attr_test);                                  // Test mode
        device_remove_file(&hdev->dev, &dev_attr_device_type);                           // Get string of device type
        device_remove_file(&hdev->dev, &dev_attr_device_mode);                           // Get device mode
        device_remove_file(&hdev->dev, &dev_attr_kbd_layout);                            // Gets the physical layout

        switch(usb_dev->descriptor.idProduct) {

        case USB_DEVICE_ID_RAZER_BLADE_2018_BASE:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020:
        case USB_DEVICE_ID_RAZER_BOOK_2020:
        case USB_DEVICE_ID_RAZER_BLADE_2019_BASE:
        case USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE:
        case USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE:
        case USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021:
        case USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022:
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);
            break;

        case USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY:
        case USB_DEVICE_ID_RAZER_BLADE_2019_ADV:
        case USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019:
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_starlight);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);
            device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);
            break;

        case USB_DEVICE_ID_RAZER_BLADE_LATE_2016:
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_starlight);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);
            device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);
            device_remove_file(&hdev->dev, &dev_attr_fn_toggle);
            break;

        case USB_DEVICE_ID_RAZER_BLADE_QHD:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017:
        case USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017:
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_starlight);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);
            device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);
            device_remove_file(&hdev->dev, &dev_attr_fn_toggle);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            break;

        case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016:
        case USB_DEVICE_ID_RAZER_BLADE_2018:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_2017:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD:
        case USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_2019:
        case USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019:
        case USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019:
        case USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020:
        case USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021:
        case USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021:
        case USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021:
        case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021:
        case USB_DEVICE_ID_RAZER_BLADE_14_2021:
        case USB_DEVICE_ID_RAZER_BLADE_17_2022:
        case USB_DEVICE_ID_RAZER_BLADE_14_2022:
        case USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022:
        case USB_DEVICE_ID_RAZER_BLADE_14_2023:
        case USB_DEVICE_ID_RAZER_BLADE_15_2023:
        case USB_DEVICE_ID_RAZER_BLADE_16_2023:
        case USB_DEVICE_ID_RAZER_BLADE_16_2025:
        case USB_DEVICE_ID_RAZER_BLADE_18_2023:
        case USB_DEVICE_ID_RAZER_BLADE_14_2024:
        case USB_DEVICE_ID_RAZER_BLADE_14_2025:
        case USB_DEVICE_ID_RAZER_BLADE_18_2024:
        case USB_DEVICE_ID_RAZER_BLADE_18_2025:
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_starlight);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);
            device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            break;

        case USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020:
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);
            device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);
            break;
        }

    } else if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_KEYBOARD) {
        device_remove_file(&hdev->dev, &dev_attr_key_super);
        device_remove_file(&hdev->dev, &dev_attr_key_alt_tab);
        device_remove_file(&hdev->dev, &dev_attr_key_alt_f4);
    }

    hid_hw_stop(hdev);
    kfree(dev);
    dev_info(&intf->dev, "Razer Device disconnected\n");
}

/**
 * Setup input device keybit mask
 */
static void razer_setup_key_bits(struct input_dev *input)
{
    __set_bit(EV_KEY, input->evbit);

    // Chroma keys
    __set_bit(RAZER_MACRO_KEY, input->keybit);
    __set_bit(RAZER_GAME_KEY, input->keybit);
    __set_bit(RAZER_BRIGHTNESS_DOWN, input->keybit);
    __set_bit(RAZER_BRIGHTNESS_UP, input->keybit);
}

/**
 * Setup the input device now that its been added to our struct
 */
static int razer_input_configured(struct hid_device *hdev, struct hid_input *hi)
{
    razer_setup_key_bits(hi->input);
    return 0;
}

/**
 * Device ID mapping table
 */
/**
 * Device ID mapping table
 */
static const struct hid_device_id razer_laptop_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_STEALTH) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2016) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_QHD) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2016) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_2018) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_2018_MERCURY) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_2018_BASE) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_2019_ADV) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_2019_BASE) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_MID_2019_MERCURY) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_PRO_LATE_2019) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_ADV_LATE_2019) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_STUDIO_EDITION_2019) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_PRO_2019) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_15_ADV_2020) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_LATE_2016) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_STEALTH_MID_2017) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_PRO_2017) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BOOK_2020) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_PRO_2017_FULLHD) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2017) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_STEALTH_2019) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2019) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_STEALTH_EARLY_2020) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_STEALTH_LATE_2020) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_PRO_EARLY_2020) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_EARLY_2020_BASE) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_LATE_2020_BASE) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2021) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_15_ADV_MID_2021) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_15_BASE_EARLY_2021) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_15_BASE_2022) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_17_PRO_EARLY_2021) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_17_PRO_MID_2021) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_14_2021) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_17_2022) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_14_2022) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_15_ADV_EARLY_2022) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_14_2023) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_15_2023) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_14_2024) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_14_2025) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_16_2023) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_16_2025) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_18_2023) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_18_2024) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLADE_18_2025) },
    { 0 }
};

MODULE_DEVICE_TABLE(hid, razer_laptop_devices);

/**
 * Describes the contents of the driver
 */
static struct hid_driver razer_laptop_driver = {
    .name     = "razerlaptop",
    .id_table = razer_laptop_devices,
    .probe    = razer_laptop_probe,
    .remove   = razer_laptop_disconnect,
    .event    = razer_event,
    .raw_event = razer_raw_event,
};

module_hid_driver(razer_laptop_driver);
