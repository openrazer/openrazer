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
#include <linux/random.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

#include "razerkraken_driver.h"
#include "razercommon.h"

/*
 * Version Information
 */
#define DRIVER_DESC "Razer Keyboard Device Driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE(DRIVER_LICENSE);

/**
 * Print report to syslog
 */
/*
static void print_erroneous_kraken_request_report(struct razer_kraken_request_report* report, char* driver_name, char* message)
{
    printk(KERN_WARNING "%s: %s. Report ID: %02x dest: %02x length: %02x ADDR: %02x%02x Args: %02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x .\n",
           driver_name,
           message,
           report->report_id,
           report->destination,
           report->length,
           report->addr_h,
           report->addr_l,
           report->arguments[0], report->arguments[1], report->arguments[2], report->arguments[3], report->arguments[4], report->arguments[5],
           report->arguments[6], report->arguments[7], report->arguments[8], report->arguments[9], report->arguments[10], report->arguments[11],
           report->arguments[12], report->arguments[13], report->arguments[14], report->arguments[15]);
}
*/

static int razer_kraken_send_control_msg(struct hid_device *hdev,struct razer_kraken_request_report* report, unsigned char skip)
{
    struct usb_device *usb_dev = hid_to_usb_dev(hdev);
    int ret;

    // Send usb control message
    ret = usb_control_msg_send(usb_dev,
                               0, // endpoint to send the message to
                               HID_REQ_SET_REPORT, // USB message request value (0x09)
                               USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT, // USB message request type value (0x21)
                               0x0204, // USB message value
                               0x0003, // USB message index value
                               report, // pointer to the data to send
                               sizeof(*report), // length in bytes of the data to send
                               USB_CTRL_SET_TIMEOUT, // time in msecs to wait for the message to complete before timing out
                               GFP_KERNEL);

    // Wait
    if(skip != 1) {
        msleep(report->length * 15);
    }

    if (ret)
        hid_warn(hdev, "Failed to send USB control message: %d\n", ret);

    return ret;
}

/*
 * Kraken V3 Pro HyperSpeed dongle: lighting over CDC bulk (not HID reports).
 *
 * Protocol (Synapse USBPcap + scripts/kraken_v3_pro):
 *   CMD 0x8e brightness: 01 f3 ff 18 02 21 40 8e 00 00 LV 00 00*14 CS
 *                        CS = (0x72 - LV) & 0xff
 *   CMD 0x8c colour:     01 f3 ff 18 02 21 40 8c 00 00 08 P1 RR GG BB 00*12 CS
 *                        CS = (0x6c - RR - GG - BB - P1) & 0xff
 *
 * 28-byte URB_BULK on EP 0x06; DTR|RTS via SET_CONTROL_LINE_STATE (0x22) on if4.
 * cdc_acm is unbound from if4/if5 so we can usb_bulk_msg; it may rebind after
 * probe, so writes re-check and re-claim.
 */

#define KRAKEN_V3_PRO_CDC_CTRL_IFACE 4
#define KRAKEN_V3_PRO_SPECTRUM_MS 40
#define KRAKEN_V3_PRO_CDC_SET_CONTROL_LINE_STATE 0x22

static bool razer_kraken_v3_pro_cdc_busy(struct razer_kraken_device *device)
{
    struct usb_interface *ctrl =
        usb_ifnum_to_if(device->usb_dev, KRAKEN_V3_PRO_CDC_CTRL_IFACE);
    struct usb_interface *data =
        usb_ifnum_to_if(device->usb_dev, KRAKEN_V3_PRO_CDC_DATA_IFACE);

    return (ctrl && ctrl->dev.driver) || (data && data->dev.driver);
}

static void razer_kraken_v3_pro_unbind_cdc(struct razer_kraken_device *device)
{
    int ifnums[] = {
        KRAKEN_V3_PRO_CDC_CTRL_IFACE,
        KRAKEN_V3_PRO_CDC_DATA_IFACE,
    };
    int i;

    for (i = 0; i < ARRAY_SIZE(ifnums); i++) {
        struct usb_interface *intf = usb_ifnum_to_if(device->usb_dev, ifnums[i]);

        if (!intf || !intf->dev.driver)
            continue;

        hid_info(device->hdev, "Unbinding %s from interface %d for lighting\n",
                 intf->dev.driver->name, ifnums[i]);
        device_release_driver(&intf->dev);
    }
}

/* bit0=DTR, bit1=RTS */
static int razer_kraken_v3_pro_set_line_state(struct razer_kraken_device *device,
        u16 line)
{
    struct usb_device *udev = device->usb_dev;
    int ret;

    ret = usb_control_msg(udev,
                          usb_sndctrlpipe(udev, 0),
                          KRAKEN_V3_PRO_CDC_SET_CONTROL_LINE_STATE,
                          USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT,
                          line,
                          KRAKEN_V3_PRO_CDC_CTRL_IFACE,
                          NULL, 0,
                          USB_CTRL_SET_TIMEOUT);
    if (ret < 0) {
        hid_warn(device->hdev, "SET_CONTROL_LINE_STATE 0x%04x failed: %d\n",
                 line, ret);
        device->cdc_line_up = false;
        return ret;
    }

    device->cdc_line_up = (line & 0x03) != 0;
    return 0;
}

/* Unbind foreign drivers from if4/if5 and raise DTR|RTS. */
static int razer_kraken_v3_pro_claim_cdc(struct razer_kraken_device *device)
{
    int i;
    int ret;

    for (i = 0; i < 6; i++) {
        if (razer_kraken_v3_pro_cdc_busy(device)) {
            razer_kraken_v3_pro_unbind_cdc(device);
            device->cdc_line_up = false;
            msleep(20);
            continue;
        }
        break;
    }

    if (razer_kraken_v3_pro_cdc_busy(device)) {
        hid_err(device->hdev,
                "CDC interfaces still held by a class driver; cannot light\n");
        return -EBUSY;
    }

    ret = razer_kraken_v3_pro_set_line_state(device, 0x0003);
    if (ret)
        return ret;

    usleep_range(3000, 8000);
    return 0;
}

static int razer_kraken_v3_pro_bulk_write(struct razer_kraken_device *device,
        const unsigned char *pkt)
{
    struct usb_device *udev = device->usb_dev;
    unsigned char *buf;
    unsigned int pipe;
    int actual;
    int ret;
    int tries;

    if (razer_kraken_v3_pro_cdc_busy(device) || !device->cdc_line_up) {
        ret = razer_kraken_v3_pro_claim_cdc(device);
        if (ret)
            return ret;
    }

    buf = kmemdup(pkt, KRAKEN_V3_PRO_BULK_LEN, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    pipe = usb_sndbulkpipe(udev, KRAKEN_V3_PRO_BULK_OUT_EP & 0x7f);

    for (tries = 0; tries < 8; tries++) {
        if (razer_kraken_v3_pro_cdc_busy(device)) {
            ret = razer_kraken_v3_pro_claim_cdc(device);
            if (ret) {
                kfree(buf);
                return ret;
            }
        }

        actual = 0;
        ret = usb_bulk_msg(udev, pipe, buf, KRAKEN_V3_PRO_BULK_LEN,
                           &actual, 1000);
        if (ret == 0 && actual == KRAKEN_V3_PRO_BULK_LEN) {
            kfree(buf);
            return 0;
        }

        /* Retry soft failures (common under VM USB redirect). */
        if (ret == -EAGAIN || ret == -ETIMEDOUT || ret == -EPIPE ||
            ret == -EBUSY || ret == -ENXIO || ret == -ENODEV ||
            (ret == 0 && actual != KRAKEN_V3_PRO_BULK_LEN)) {
            if (ret == -EPIPE)
                usb_clear_halt(udev, pipe);
            device->cdc_line_up = false;
            razer_kraken_v3_pro_claim_cdc(device);
            usleep_range(5000, 15000);
            continue;
        }

        hid_warn(device->hdev, "bulk OUT failed: %d actual=%d\n", ret, actual);
        kfree(buf);
        return ret ? ret : -EIO;
    }

    hid_warn(device->hdev, "bulk OUT gave up after retries (last ret=%d actual=%d)\n",
             ret, actual);
    kfree(buf);
    return ret ? ret : -EIO;
}

static int razer_kraken_v3_pro_set_brightness(struct razer_kraken_device *device,
        unsigned char level)
{
    unsigned char pkt[KRAKEN_V3_PRO_BULK_LEN];
    int ret;

    memset(pkt, 0, sizeof(pkt));
    pkt[0] = 0x01;
    pkt[1] = 0xf3;
    pkt[2] = 0xff;
    pkt[3] = 0x18;
    pkt[4] = 0x02;
    pkt[5] = 0x21;
    pkt[6] = 0x40;
    pkt[7] = 0x8e;
    pkt[10] = level;
    pkt[27] = (unsigned char)((0x72 - level) & 0xff);

    ret = razer_kraken_v3_pro_bulk_write(device, pkt);
    if (ret)
        return ret;

    device->last_brightness = level;
    usleep_range(15000, 25000);
    return 0;
}

static int razer_kraken_v3_pro_set_color_zone(struct razer_kraken_device *device,
        unsigned char r, unsigned char g,
        unsigned char b, unsigned char zone)
{
    unsigned char pkt[KRAKEN_V3_PRO_BULK_LEN];

    memset(pkt, 0, sizeof(pkt));
    pkt[0] = 0x01;
    pkt[1] = 0xf3;
    pkt[2] = 0xff;
    pkt[3] = 0x18;
    pkt[4] = 0x02;
    pkt[5] = 0x21;
    pkt[6] = 0x40;
    pkt[7] = 0x8c;
    pkt[10] = 0x08;
    pkt[11] = zone;
    pkt[12] = r;
    pkt[13] = g;
    pkt[14] = b;
    pkt[27] = (unsigned char)((0x6c - r - g - b - zone) & 0xff);

    return razer_kraken_v3_pro_bulk_write(device, pkt);
}

/*
 * send_brightness: send CMD 0x8e first (needed after power-on / none).
 * Both zone flags match Synapse dual-zone updates. Caller holds device->lock.
 */
static int razer_kraken_v3_pro_set_color(struct razer_kraken_device *device,
        unsigned char r, unsigned char g,
        unsigned char b, bool send_brightness)
{
    int ret;

    if (send_brightness) {
        ret = razer_kraken_v3_pro_set_brightness(device, device->last_brightness);
        if (ret)
            return ret;
    }

    ret = razer_kraken_v3_pro_set_color_zone(device, r, g, b, 0x00);
    if (ret)
        return ret;

    usleep_range(5000, 10000);

    ret = razer_kraken_v3_pro_set_color_zone(device, r, g, b, 0x01);
    if (ret)
        return ret;

    device->last_rgb[0] = r;
    device->last_rgb[1] = g;
    device->last_rgb[2] = b;
    return 0;
}

/* Must not be called with device->lock held (cancel_delayed_work_sync). */
static void razer_kraken_v3_pro_stop_spectrum(struct razer_kraken_device *device)
{
    if (!device->spectrum_active)
        return;
    device->spectrum_active = false;
    cancel_delayed_work_sync(&device->spectrum_work);
}

static void razer_kraken_v3_pro_spectrum_work(struct work_struct *work)
{
    struct razer_kraken_device *device =
        container_of(to_delayed_work(work), struct razer_kraken_device, spectrum_work);
    unsigned char r, g, b;
    bool cont;

    mutex_lock(&device->lock);
    if (!device->spectrum_active) {
        mutex_unlock(&device->lock);
        return;
    }

    r = device->last_rgb[0];
    g = device->last_rgb[1];
    b = device->last_rgb[2];

    /* red -> yellow -> green -> cyan -> blue -> magenta -> red */
    if (r == 0xff && g < 0xff && b == 0x00)
        g = (g + 5 > 0xff) ? 0xff : g + 5;
    else if (g == 0xff && r > 0x00 && b == 0x00)
        r = (r < 5) ? 0 : r - 5;
    else if (g == 0xff && b < 0xff && r == 0x00)
        b = (b + 5 > 0xff) ? 0xff : b + 5;
    else if (b == 0xff && g > 0x00 && r == 0x00)
        g = (g < 5) ? 0 : g - 5;
    else if (b == 0xff && r < 0xff && g == 0x00)
        r = (r + 5 > 0xff) ? 0xff : r + 5;
    else if (r == 0xff && b > 0x00 && g == 0x00)
        b = (b < 5) ? 0 : b - 5;
    else {
        r = 0xff;
        g = 0x00;
        b = 0x00;
    }

    razer_kraken_v3_pro_set_color(device, r, g, b, false);
    cont = device->spectrum_active;
    mutex_unlock(&device->lock);

    if (cont)
        schedule_delayed_work(&device->spectrum_work,
                              msecs_to_jiffies(KRAKEN_V3_PRO_SPECTRUM_MS));
}

static void razer_kraken_v3_pro_release_cdc(struct razer_kraken_device *device)
{
    if (device->cdc_line_up)
        razer_kraken_v3_pro_set_line_state(device, 0x0000);
    device->cdc_line_up = false;
}

static int razer_kraken_v3_pro_setup_bulk(struct razer_kraken_device *device)
{
    struct usb_interface *ctrl_intf;
    struct usb_interface *data_intf;
    int ret;

    ctrl_intf = usb_ifnum_to_if(device->usb_dev, KRAKEN_V3_PRO_CDC_CTRL_IFACE);
    data_intf = usb_ifnum_to_if(device->usb_dev, KRAKEN_V3_PRO_CDC_DATA_IFACE);
    if (!ctrl_intf || !data_intf) {
        hid_err(device->hdev, "CDC interfaces %d/%d not found\n",
                KRAKEN_V3_PRO_CDC_CTRL_IFACE, KRAKEN_V3_PRO_CDC_DATA_IFACE);
        return -ENODEV;
    }

    device->use_cdc_bulk = true;
    device->last_brightness = 0xff;
    device->spectrum_active = false;
    device->cdc_line_up = false;
    INIT_DELAYED_WORK(&device->spectrum_work, razer_kraken_v3_pro_spectrum_work);

    msleep(100);
    ret = razer_kraken_v3_pro_claim_cdc(device);
    if (ret) {
        /* Keep sysfs; first effect write retries claim. */
        hid_warn(device->hdev,
                 "CDC claim at probe failed (%d); will retry on first light command\n",
                 ret);
    }

    hid_info(device->hdev,
             "Using CDC bulk EP 0x%02x for lighting (re-claim if cdc_acm binds)\n",
             KRAKEN_V3_PRO_BULK_OUT_EP);
    return 0;
}

/**
 * Get a request report
 *
 * report_id - The type of report
 * destination - where data is going (like ram)
 * length - amount of data
 * address - where to write data to
 */
static struct razer_kraken_request_report get_kraken_request_report(unsigned char report_id, unsigned char destination, unsigned char length, unsigned short address)
{
    struct razer_kraken_request_report report;
    memset(&report, 0, sizeof(struct razer_kraken_request_report));

    report.report_id = report_id;
    report.destination = destination;
    report.length = length;
    report.addr_h = (address >> 8);
    report.addr_l = (address & 0xFF);

    return report;
}

/**
 * Get a union containing the effect bitfield
 */
static union razer_kraken_effect_byte get_kraken_effect_byte(void)
{
    union razer_kraken_effect_byte effect_byte;
    memset(&effect_byte, 0, sizeof(union razer_kraken_effect_byte));

    return effect_byte;
}

/**
 * Get the current effect
 */
static unsigned char get_current_effect(struct device *dev)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report report = get_kraken_request_report(0x04, 0x00, 0x01, device->led_mode_address);
    int is_mutex_locked = mutex_is_locked(&device->lock);
    unsigned char result = 0;

    // Lock if there isn't already a lock, otherwise skip, essentially emulate a rentrant lock
    if(is_mutex_locked == 0) {
        mutex_lock(&device->lock);
    }

    device->data[0] = 0x00;
    razer_kraken_send_control_msg(device->hdev, &report, 1);
    msleep(25); // Sleep 20ms

    // Check for actual data
    if(device->data[0] == 0x05) {
        result = device->data[1];
    } else {
        dev_err(dev, "razerkraken: Did not manage to get report\n");
    }

    // Unlock if there isn't already a lock (as there would be by now), otherwise skip as reusing existing lock
    if(is_mutex_locked == 0) {
        mutex_unlock(&device->lock);
    }

    return result;
}

static unsigned int get_rgb_from_addr(struct device *dev, unsigned short address, unsigned char len, char* buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report report = get_kraken_request_report(0x04, 0x00, len, address);
    int is_mutex_locked = mutex_is_locked(&device->lock);
    unsigned char written = 0;

    // Lock if there isn't already a lock, otherwise skip, essentially emulate a rentrant lock
    if(is_mutex_locked == 0) {
        mutex_lock(&device->lock);
    }

    device->data[0] = 0x00;
    razer_kraken_send_control_msg(device->hdev, &report, 1);
    msleep(25); // Sleep 20ms

    // Check for actual data
    if(device->data[0] == 0x05) {
        //dev_err(dev, "razerkraken: Got %02x%02x%02x %02x\n", device->data[1], device->data[2], device->data[3], device->data[4]);
        memcpy(buf, &device->data[1], len);
        written = len;
    } else {
        dev_err(dev, "razerkraken: Did not manage to get report\n");
    }

    // Unlock if there isn't already a lock (as there would be by now), otherwise skip as reusing existing lock
    if(is_mutex_locked == 0) {
        mutex_unlock(&device->lock);
    }

    return written;
}

/**
 * Read device file "version"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%s\n", DRIVER_VERSION);
}

/**
 * Read device file "device_type"
 *
 * Returns friendly string of device type
 */
static ssize_t razer_attr_read_device_type(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);

    char *device_type;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC:
    case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC_ALT:
        device_type = "Razer Kraken 7.1";
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN:
        device_type = "Razer Kraken 7.1 Chroma"; // Rainie
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN_V2:
        device_type = "Razer Kraken 7.1 V2"; // Kylie
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN_TE:
        device_type = "Razer Kraken Tournament Edition";
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE:
        device_type = "Razer Kraken Ultimate";
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2:
        device_type = "Razer Kraken Kitty V2";
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN_V3_PRO:
        device_type = "Razer Kraken V3 Pro";
        break;

    default:
        device_type = "Unknown Device";
    }

    return sysfs_emit(buf, "%s\n", device_type);
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
    return sysfs_emit(buf, "\n");
}

/**
 * Write device file "mode_spectrum"
 *
 * Specrum effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_matrix_effect_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report report;
    union razer_kraken_effect_byte effect_byte;

    /* V3 Pro has no hardware spectrum; emulate with delayed_work colour steps. */
    if (device->use_cdc_bulk) {
        int ret;

        razer_kraken_v3_pro_stop_spectrum(device);
        mutex_lock(&device->lock);
        device->last_brightness = 0xff;
        device->last_rgb[0] = 0xff;
        device->last_rgb[1] = 0x00;
        device->last_rgb[2] = 0x00;
        ret = razer_kraken_v3_pro_set_color(device, 0xff, 0x00, 0x00, true);
        if (ret) {
            mutex_unlock(&device->lock);
            return ret;
        }
        device->last_effect = 0x04;
        device->spectrum_active = true;
        mutex_unlock(&device->lock);
        schedule_delayed_work(&device->spectrum_work,
                              msecs_to_jiffies(KRAKEN_V3_PRO_SPECTRUM_MS));
        return count;
    }

    report = get_kraken_request_report(0x04, 0x40, 0x01, device->led_mode_address);
    effect_byte = get_kraken_effect_byte();

    // Spectrum Cycling | ON
    effect_byte.bits.on_off_static = 1;
    effect_byte.bits.spectrum_cycling = 1;

    report.arguments[0] = effect_byte.value;

    // Lock access to sending USB as adhering to the razer len*15ms delay
    mutex_lock(&device->lock);
    razer_kraken_send_control_msg(device->hdev, &report, 0);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Write device file "mode_none"
 *
 * None effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_matrix_effect_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report report;
    union razer_kraken_effect_byte effect_byte;

    if (device->use_cdc_bulk) {
        int ret;

        razer_kraken_v3_pro_stop_spectrum(device);
        mutex_lock(&device->lock);
        device->last_brightness = 0x00;
        ret = razer_kraken_v3_pro_set_color(device, 0x00, 0x00, 0x00, true);
        if (ret) {
            mutex_unlock(&device->lock);
            return ret;
        }
        device->last_effect = 0x00;
        mutex_unlock(&device->lock);
        return count;
    }

    report = get_kraken_request_report(0x04, 0x40, 0x01, device->led_mode_address);
    effect_byte = get_kraken_effect_byte();

    // Spectrum Cycling | OFF
    effect_byte.bits.on_off_static = 0;
    effect_byte.bits.spectrum_cycling = 0;

    report.arguments[0] = effect_byte.value;

    // Lock access to sending USB as adhering to the razer len*15ms delay
    mutex_lock(&device->lock);
    razer_kraken_send_control_msg(device->hdev, &report, 0);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Write device file "mode_static"
 *
 * Static effect mode is activated whenever the file is written to with 3 bytes
 */
static ssize_t razer_attr_write_matrix_effect_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report rgb_report;
    struct razer_kraken_request_report effect_report;
    union razer_kraken_effect_byte effect_byte;

    if (count != 3 && count != 4) {
        dev_warn(dev, "razerkraken: Static mode only accepts RGB (3byte) or RGB with intensity (4byte)\n");
        return -EINVAL;
    }

    if (device->use_cdc_bulk) {
        int ret;

        razer_kraken_v3_pro_stop_spectrum(device);
        mutex_lock(&device->lock);
        /* Optional 4th byte is brightness (CMD 0x8e). */
        if (count == 4)
            device->last_brightness = buf[3];
        else if (device->last_brightness == 0)
            device->last_brightness = 0xff;
        ret = razer_kraken_v3_pro_set_color(device, buf[0], buf[1], buf[2], true);
        if (ret) {
            mutex_unlock(&device->lock);
            return ret;
        }
        device->last_effect = 0x01;
        mutex_unlock(&device->lock);
        return count;
    }

    rgb_report = get_kraken_request_report(0x04, 0x40, count, device->breathing_address[0]);
    effect_report = get_kraken_request_report(0x04, 0x40, 0x01, device->led_mode_address);
    effect_byte = get_kraken_effect_byte();

    rgb_report.arguments[0] = buf[0];
    rgb_report.arguments[1] = buf[1];
    rgb_report.arguments[2] = buf[2];

    if(count == 4) {
        rgb_report.arguments[3] = buf[3];
    }

    // ON/Static
    effect_byte.bits.on_off_static = 1;
    effect_report.arguments[0] = effect_byte.value;

    // Lock sending of the 2 commands
    mutex_lock(&device->lock);

    // Basically Kraken Classic doesn't take RGB arguments so only do it for the KrakenV1,V2,Ultimate
    switch(device->usb_pid) {
    case USB_DEVICE_ID_RAZER_KRAKEN:
    case USB_DEVICE_ID_RAZER_KRAKEN_V2:
    case USB_DEVICE_ID_RAZER_KRAKEN_TE:
    case USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE:
    case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2:
        razer_kraken_send_control_msg(device->hdev, &rgb_report, 0);
        break;
    }

    // Send Set static command
    razer_kraken_send_control_msg(device->hdev, &effect_report, 0);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Write device file "mode_custom"
 *
 * Custom effect mode is activated whenever the file is written to with 3 bytes
 */
static ssize_t razer_attr_write_matrix_effect_custom(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report rgb_report = get_kraken_request_report(0x04, 0x40, count, device->custom_address);
    struct razer_kraken_request_report effect_report = get_kraken_request_report(0x04, 0x40, 0x01, device->led_mode_address);
    union razer_kraken_effect_byte effect_byte = get_kraken_effect_byte();

    if(count != 3 && count != 4) {
        dev_warn(dev, "razerkraken: Custom mode only accepts RGB (3byte) or RGB with intensity (4byte)\n");
        return -EINVAL;
    }

    rgb_report.arguments[0] = buf[0];
    rgb_report.arguments[1] = buf[1];
    rgb_report.arguments[2] = buf[2];

    if(count == 4) {
        rgb_report.arguments[3] = buf[3];
    }

    // ON/Static
    effect_byte.bits.on_off_static = 1;
    effect_report.arguments[0] = effect_byte.value;

    // Lock sending of the 2 commands
    mutex_lock(&device->lock);
    razer_kraken_send_control_msg(device->hdev, &rgb_report, 1);

    razer_kraken_send_control_msg(device->hdev, &effect_report, 1);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Read device file "mode_static"
 *
 * Returns 4 bytes for config
 */
static ssize_t razer_attr_read_matrix_effect_static(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);

    if (device->use_cdc_bulk) {
        buf[0] = device->last_rgb[0];
        buf[1] = device->last_rgb[1];
        buf[2] = device->last_rgb[2];
        buf[3] = 0x00;
        return 4;
    }
    return get_rgb_from_addr(dev, device->breathing_address[0], 0x04, buf);
}

/**
 * Read device file "mode_custom"
 *
 * Returns 4 bytes for config
 */
static ssize_t razer_attr_read_matrix_effect_custom(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    return get_rgb_from_addr(dev, device->custom_address, 0x04, buf);
}

/**
 * Write device file "mode_breath"
 *
 * Breathing effect mode is activated whenever the file is written to with 3,6 or 9 bytes
 */
static ssize_t razer_attr_write_matrix_effect_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report effect_report = get_kraken_request_report(0x04, 0x40, 0x01, device->led_mode_address);
    union razer_kraken_effect_byte effect_byte = get_kraken_effect_byte();

    // Short circuit here as rainie only does breathing1
    if(device->usb_pid == USB_DEVICE_ID_RAZER_KRAKEN && count != 3) {
        dev_warn(dev, "razerkraken: Breathing mode only accepts RGB (3byte)\n");
        return -EINVAL;
    }

    if(count == 3) {
        struct razer_kraken_request_report rgb_report = get_kraken_request_report(0x04, 0x40, 0x03, device->breathing_address[0]);

        rgb_report.arguments[0] = buf[0];
        rgb_report.arguments[1] = buf[1];
        rgb_report.arguments[2] = buf[2];

        // ON/Static
        effect_byte.bits.on_off_static = 1;
        effect_byte.bits.single_colour_breathing = 1;
        effect_byte.bits.sync = 1;
        effect_report.arguments[0] = effect_byte.value;

        // Lock sending of the 2 commands
        mutex_lock(&device->lock);
        razer_kraken_send_control_msg(device->hdev, &rgb_report, 0);

        razer_kraken_send_control_msg(device->hdev, &effect_report, 0);
        mutex_unlock(&device->lock);
    } else if(count == 6) {
        struct razer_kraken_request_report rgb_report  = get_kraken_request_report(0x04, 0x40, 0x03, device->breathing_address[1]);
        struct razer_kraken_request_report rgb_report2 = get_kraken_request_report(0x04, 0x40, 0x03, device->breathing_address[1]+4); // Address the 2nd set of colours

        rgb_report.arguments[0] = buf[0];
        rgb_report.arguments[1] = buf[1];
        rgb_report.arguments[2] = buf[2];
        rgb_report2.arguments[0] = buf[3];
        rgb_report2.arguments[1] = buf[4];
        rgb_report2.arguments[2] = buf[5];

        // ON/Static
        effect_byte.bits.on_off_static = 1;
        effect_byte.bits.two_colour_breathing = 1;
        effect_byte.bits.sync = 1;
        effect_report.arguments[0] = effect_byte.value;

        // Lock sending of the 2 commands
        mutex_lock(&device->lock);
        razer_kraken_send_control_msg(device->hdev, &rgb_report, 0);

        razer_kraken_send_control_msg(device->hdev, &rgb_report2, 0);

        razer_kraken_send_control_msg(device->hdev, &effect_report, 0);
        mutex_unlock(&device->lock);

    } else if(count == 9) {
        struct razer_kraken_request_report rgb_report  = get_kraken_request_report(0x04, 0x40, 0x03, device->breathing_address[2]);
        struct razer_kraken_request_report rgb_report2 = get_kraken_request_report(0x04, 0x40, 0x03, device->breathing_address[2]+4); // Address the 2nd set of colours
        struct razer_kraken_request_report rgb_report3 = get_kraken_request_report(0x04, 0x40, 0x03, device->breathing_address[2]+8); // Address the 3rd set of colours

        rgb_report.arguments[0] = buf[0];
        rgb_report.arguments[1] = buf[1];
        rgb_report.arguments[2] = buf[2];
        rgb_report2.arguments[0] = buf[3];
        rgb_report2.arguments[1] = buf[4];
        rgb_report2.arguments[2] = buf[5];
        rgb_report3.arguments[0] = buf[6];
        rgb_report3.arguments[1] = buf[7];
        rgb_report3.arguments[2] = buf[8];

        // ON/Static
        effect_byte.bits.on_off_static = 1;
        effect_byte.bits.three_colour_breathing = 1;
        effect_byte.bits.sync = 1;
        effect_report.arguments[0] = effect_byte.value;

        // Lock sending of the 2 commands
        mutex_lock(&device->lock);
        razer_kraken_send_control_msg(device->hdev, &rgb_report, 0);

        razer_kraken_send_control_msg(device->hdev, &rgb_report2, 0);

        razer_kraken_send_control_msg(device->hdev, &rgb_report3, 0);

        razer_kraken_send_control_msg(device->hdev, &effect_report, 0);
        mutex_unlock(&device->lock);

    } else {
        dev_warn(dev, "razerkraken: Breathing mode only accepts RGB (3byte), RGB RGB (6byte) or RGB RGB RGB (9byte)\n");
        return -EINVAL;
    }

    return count;
}

/**
 * Read device file "mode_breath"
 *
 * Returns 4, 8, 12 bytes for config
 */
static ssize_t razer_attr_read_matrix_effect_breath(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    union razer_kraken_effect_byte effect_byte;
    unsigned char num_colours = 1;

    effect_byte.value = get_current_effect(dev);

    if(effect_byte.bits.two_colour_breathing == 1) {
        num_colours = 2;
    } else if(effect_byte.bits.three_colour_breathing == 1) {
        num_colours = 3;
    }

    switch(device->usb_pid) {
    case USB_DEVICE_ID_RAZER_KRAKEN_V2:
    case USB_DEVICE_ID_RAZER_KRAKEN_TE:
    case USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE:
    case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2:
        switch(num_colours) {
        case 3:
            return get_rgb_from_addr(dev, device->breathing_address[2], 0x0C, buf);
            break;
        case 2:
            return get_rgb_from_addr(dev, device->breathing_address[1], 0x08, buf);
            break;
        default:
            return get_rgb_from_addr(dev, device->breathing_address[0], 0x04, buf);
            break;
        }
        break;

    case USB_DEVICE_ID_RAZER_KRAKEN:
        return get_rgb_from_addr(dev, device->breathing_address[0], 0x04, buf);
        break;

    default:
        dev_warn(dev, "razerkraken: Unknown device\n");
        return -EINVAL;
    }
}

/**
 * Read device file "serial"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_device_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report report = get_kraken_request_report(0x04, 0x20, 0x16, 0x7f00);

    // Basically some simple caching
    // Also skips going to device if it doesn't contain the serial
    if(device->serial[0] == '\0') {
        if (device->use_cdc_bulk) {
            unsigned int rand_serial = 0;

            get_random_bytes(&rand_serial, sizeof(unsigned int));
            sprintf(device->serial, "KV3P%015u", rand_serial);
            return sysfs_emit(buf, "%s\n", device->serial);
        }

        mutex_lock(&device->lock);
        device->data[0] = 0x00;
        razer_kraken_send_control_msg(device->hdev, &report, 1);
        msleep(25); // Sleep 20ms

        // Check for actual data
        if(device->data[0] == 0x05) {
            // Serial is present
            memcpy(device->serial, &device->data[1], 22);
            device->serial[22] = '\0';
        } else {
            dev_err(dev, "razerkraken: Did not manage to get serial from device, using XX01 instead\n");
            device->serial[0] = 'X';
            device->serial[1] = 'X';
            device->serial[2] = '0';
            device->serial[3] = '1';
            device->serial[4] = '\0';
        }
        mutex_unlock(&device->lock);

    }

    return sysfs_emit(buf, "%s\n", device->serial);
}

/**
 * Read device file "get_firmware_version"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_firmware_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    struct razer_kraken_request_report report = get_kraken_request_report(0x04, 0x20, 0x02, 0x0030);

    // Basically some simple caching
    if(device->firmware_version[0] != 1) {
        if (device->use_cdc_bulk) {
            device->firmware_version[0] = 1;
            device->firmware_version[1] = 0x01;
            device->firmware_version[2] = 0x00;
            return sysfs_emit(buf, "v%x.%x\n", device->firmware_version[1], device->firmware_version[2]);
        }

        mutex_lock(&device->lock);
        device->data[0] = 0x00;
        razer_kraken_send_control_msg(device->hdev, &report, 1);
        msleep(25); // Sleep 20ms

        // Check for actual data
        if(device->data[0] == 0x05) {
            // Serial is present
            device->firmware_version[0] = 1;
            device->firmware_version[1] = device->data[1];
            device->firmware_version[2] = device->data[2];
        } else {
            dev_err(dev, "razerkraken: Did not manage to get firmware version from device, using v9.99 instead\n");
            device->firmware_version[0] = 1;
            device->firmware_version[1] = 0x09;
            device->firmware_version[2] = 0x99;
        }
        mutex_unlock(&device->lock);
    }

    return sysfs_emit(buf, "v%x.%x\n", device->firmware_version[1], device->firmware_version[2]);
}

/**
 * Read device file "matrix_current_effect"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_matrix_current_effect(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_kraken_device *device = dev_get_drvdata(dev);
    unsigned char current_effect;

    if (device->use_cdc_bulk)
        current_effect = device->last_effect;
    else
        current_effect = get_current_effect(dev);

    return sysfs_emit(buf, "%02x\n", current_effect);
}

/**
 * Write device file "device_mode"
 */
static ssize_t razer_attr_write_device_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return count;
}

/**
 * Read device file "device_mode"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_device_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
    buf[0] = 0x00;
    buf[1] = 0x00;

    return 2;
}

/**
 * Set up the device driver files

 *
 * Read only is 0444
 * Write only is 0220
 * Read and write is 0664
 */

static DEVICE_ATTR(test,                    0660, razer_attr_read_test,                       razer_attr_write_test);
static DEVICE_ATTR(version,                 0440, razer_attr_read_version,                    NULL);
static DEVICE_ATTR(device_type,             0440, razer_attr_read_device_type,                NULL);
static DEVICE_ATTR(device_serial,           0440, razer_attr_read_device_serial,              NULL);
static DEVICE_ATTR(device_mode,             0660, razer_attr_read_device_mode,                razer_attr_write_device_mode);
static DEVICE_ATTR(firmware_version,        0440, razer_attr_read_firmware_version,           NULL);

static DEVICE_ATTR(matrix_current_effect,   0440, razer_attr_read_matrix_current_effect,      NULL);
static DEVICE_ATTR(matrix_effect_none,      0220, NULL,                                       razer_attr_write_matrix_effect_none);
static DEVICE_ATTR(matrix_effect_spectrum,  0220, NULL,                                       razer_attr_write_matrix_effect_spectrum);
static DEVICE_ATTR(matrix_effect_static,    0660, razer_attr_read_matrix_effect_static,       razer_attr_write_matrix_effect_static);
static DEVICE_ATTR(matrix_effect_custom,    0660, razer_attr_read_matrix_effect_custom,       razer_attr_write_matrix_effect_custom);
static DEVICE_ATTR(matrix_effect_breath,    0660, razer_attr_read_matrix_effect_breath,       razer_attr_write_matrix_effect_breath);

static void razer_kraken_init(struct razer_kraken_device *dev, struct usb_interface *intf, struct hid_device *hdev)
{
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned int rand_serial = 0;

    // Initialise mutex
    mutex_init(&dev->lock);
    // Setup values
    dev->hdev = hdev;
    dev->usb_dev = usb_dev;
    dev->usb_interface_protocol = intf->cur_altsetting->desc.bInterfaceProtocol;
    dev->usb_vid = usb_dev->descriptor.idVendor;
    dev->usb_pid = usb_dev->descriptor.idProduct;

    switch(dev->usb_pid) {
    case USB_DEVICE_ID_RAZER_KRAKEN_V2:
    case USB_DEVICE_ID_RAZER_KRAKEN_TE:
    case USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE:
    case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2:
        dev->led_mode_address = KYLIE_SET_LED_ADDRESS;
        dev->custom_address = KYLIE_CUSTOM_ADDRESS_START;
        dev->breathing_address[0] = KYLIE_BREATHING1_ADDRESS_START;
        dev->breathing_address[1] = KYLIE_BREATHING2_ADDRESS_START;
        dev->breathing_address[2] = KYLIE_BREATHING3_ADDRESS_START;
        break;
    case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC:
    case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC_ALT:
    case USB_DEVICE_ID_RAZER_KRAKEN:
        dev->led_mode_address = RAINIE_SET_LED_ADDRESS;
        dev->custom_address = RAINIE_CUSTOM_ADDRESS_START;
        dev->breathing_address[0] = RAINIE_BREATHING1_ADDRESS_START;

        // Get a "random" integer
        get_random_bytes(&rand_serial, sizeof(unsigned int));
        sprintf(dev->serial, "HN%015u", rand_serial);
        break;
    case USB_DEVICE_ID_RAZER_KRAKEN_V3_PRO:
        dev->use_cdc_bulk = true;
        dev->last_brightness = 0xff;
        get_random_bytes(&rand_serial, sizeof(unsigned int));
        sprintf(dev->serial, "KV3P%015u", rand_serial);
        break;
    }
}

/**
 * Probe method is ran whenever a device is binded to the driver
 */
static int razer_kraken_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int retval = 0;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_kraken_device *dev = NULL;

    dev = kzalloc_obj(*dev);
    if(dev == NULL) {
        hid_err(hdev, "out of memory\n");
        return -ENOMEM;
    }

    // Init data
    razer_kraken_init(dev, intf, hdev);

    /* V3 Pro control HID is Boot Keyboard (protocol 1), not PROTOCOL_NONE. */
    if(dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE ||
       dev->usb_pid == USB_DEVICE_ID_RAZER_KRAKEN_V3_PRO) {
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);                               // Get driver version
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_test);                                  // Test mode
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);                           // Get string of device type
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);                         // Get string of device serial
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_firmware_version);                      // Get string of device fw version
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_mode);                           // Get device mode

        switch(dev->usb_pid) {
        case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC:
        case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC_ALT:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_current_effect);         // Get current effect
            break;
        case USB_DEVICE_ID_RAZER_KRAKEN:
        case USB_DEVICE_ID_RAZER_KRAKEN_V2:
        case USB_DEVICE_ID_RAZER_KRAKEN_TE:
        case USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE:
        case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_current_effect);         // Get current effect
            break;
        case USB_DEVICE_ID_RAZER_KRAKEN_V3_PRO:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_current_effect);
            break;
        }
    }

    dev_set_drvdata(&hdev->dev, dev);

    if(hid_parse(hdev)) {
        hid_err(hdev, "parse failed\n");
        goto exit_free;
    }

    if (hid_hw_start(hdev, HID_CONNECT_DEFAULT)) {
        hid_err(hdev, "hw start failed\n");
        goto exit_free;
    }

    if (dev->usb_pid == USB_DEVICE_ID_RAZER_KRAKEN_V3_PRO) {
        retval = razer_kraken_v3_pro_setup_bulk(dev);
        if (retval) {
            hid_err(hdev, "Failed to set up CDC bulk path: %d\n", retval);
            hid_hw_stop(hdev);
            goto exit_free_files;
        }
    }

    usb_disable_autosuspend(usb_dev);

    return 0;

exit_free_files:
    if(dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE ||
       dev->usb_pid == USB_DEVICE_ID_RAZER_KRAKEN_V3_PRO) {
        device_remove_file(&hdev->dev, &dev_attr_version);
        device_remove_file(&hdev->dev, &dev_attr_test);
        device_remove_file(&hdev->dev, &dev_attr_device_type);
        device_remove_file(&hdev->dev, &dev_attr_device_serial);
        device_remove_file(&hdev->dev, &dev_attr_firmware_version);
        device_remove_file(&hdev->dev, &dev_attr_device_mode);
        if (dev->usb_pid == USB_DEVICE_ID_RAZER_KRAKEN_V3_PRO) {
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_current_effect);
        }
    }
exit_free:
    kfree(dev);
    return retval;
}

/**
 * Unbind function
 */
static void razer_kraken_disconnect(struct hid_device *hdev)
{
    struct razer_kraken_device *dev;

    dev = hid_get_drvdata(hdev);

    if (dev->use_cdc_bulk) {
        razer_kraken_v3_pro_stop_spectrum(dev);
        razer_kraken_v3_pro_release_cdc(dev);
    }

    if(dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE ||
       dev->usb_pid == USB_DEVICE_ID_RAZER_KRAKEN_V3_PRO) {
        device_remove_file(&hdev->dev, &dev_attr_version);                               // Get driver version
        device_remove_file(&hdev->dev, &dev_attr_test);                                  // Test mode
        device_remove_file(&hdev->dev, &dev_attr_device_type);                           // Get string of device type
        device_remove_file(&hdev->dev, &dev_attr_device_serial);                         // Get string of device serial
        device_remove_file(&hdev->dev, &dev_attr_firmware_version);                      // Get string of device fw version
        device_remove_file(&hdev->dev, &dev_attr_device_mode);                           // Get device mode

        switch(dev->usb_pid) {
        case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC:
        case USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC_ALT:
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_current_effect);         // Get current effect
            break;

        case USB_DEVICE_ID_RAZER_KRAKEN:
        case USB_DEVICE_ID_RAZER_KRAKEN_V2:
        case USB_DEVICE_ID_RAZER_KRAKEN_TE:
        case USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE:
        case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2:
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);            // No effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);        // Spectrum effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);          // Static effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);          // Custom effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);          // Breathing effect
            device_remove_file(&hdev->dev, &dev_attr_matrix_current_effect);         // Get current effect
            break;

        case USB_DEVICE_ID_RAZER_KRAKEN_V3_PRO:
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_current_effect);
            break;
        }
    }

    hid_hw_stop(hdev);
    kfree(dev);
    hid_info(hdev, "Razer Device disconnected\n");
}

static int razer_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size)
{
    struct razer_kraken_device *device = dev_get_drvdata(&hdev->dev);

    //dev_warn(dev, "razerkraken: Got raw message %d\n", size);

    if(size == 33) { // Should be a response to a Control packet
        memcpy(device->data, data, size);

    } else {
        hid_warn(hdev, "razerkraken: Got raw message, length: %d\n", size);
    }

    return 0;
}

/**
 * Device ID mapping table
 */
static const struct hid_device_id razer_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_CLASSIC_ALT) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_V2) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_TE) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_ULTIMATE) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_V3_PRO) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2) },
    { 0 }
};

MODULE_DEVICE_TABLE(hid, razer_devices);

/**
 * Describes the contents of the driver
 */
static struct hid_driver razer_kraken_driver = {
    .name = "razerkraken",
    .id_table = razer_devices,
    .probe = razer_kraken_probe,
    .remove = razer_kraken_disconnect,
    .raw_event = razer_raw_event
};

module_hid_driver(razer_kraken_driver);
