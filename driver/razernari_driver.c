// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Razer Nari (including Nari Ultimate) kernel driver for OpenRazer.
 *
 * Protocol decoded from USB pcap captures in felixZmn/razer-nari-driver
 * via pattern analysis. Initial driver skeleton by Garfonso.
 *
 * Supported features:
 *   - Logo LED on/off (Nari Ultimate and Nari)
 *   - Haptic motor intensity 0..100 (Nari Ultimate only; the non-Ultimate
 *     Nari lacks the motors and will ignore the haptic command harmlessly)
 *   - Device type / serial introspection
 *
 * USB microphone mute and volume controls are USB Audio Class Feature Unit
 * commands rather than Razer-specific reports, so they are handled by the
 * kernel's standard snd-usb-audio driver without any driver work here.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/random.h>

#include "razernari_driver.h"
#include "razercommon.h"

#define DRIVER_DESC "Razer Nari Headset Device Driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE(DRIVER_LICENSE);

#define NARI_DEBUG_REPORTS 0

/* ---------------------------------------------------------------------
 * USB transport
 * ------------------------------------------------------------------ */

/**
 * Send a Feature Report to the Nari over the control endpoint.
 *
 * The Nari uses HID Feature Report 0xff on interface 5:
 *   SET_REPORT, wValue=0x03ff, wIndex=5, wLength=64.
 * Byte 0 of the payload is the report ID (0xff).
 *
 * @skip: when non-zero, skip the post-send delay. Razer devices normally
 * want a len*15ms idle after a SET_REPORT, but for the short Nari
 * commands the delay is unnecessary in practice.
 */
static int razer_nari_send_control_msg(struct usb_device *usb_dev,
                                       struct razer_nari_request_report *report,
                                       unsigned char skip)
{
    uint request = HID_REQ_SET_REPORT;
    uint request_type = USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT;
    uint value = 0x03ff;
    uint index = 5;
    uint size = RAZER_NARI_REPORT_LEN;
    char *buf;
    int len;

    buf = kmemdup(report, size, GFP_KERNEL);
    if (buf == NULL)
        return -ENOMEM;

    len = usb_control_msg(usb_dev, usb_sndctrlpipe(usb_dev, 0),
                          request, request_type,
                          value, index,
                          buf, size,
                          USB_CTRL_SET_TIMEOUT);

    if (skip != 1)
        msleep(report->length * 15);

    kfree(buf);
    if (len != size)
        printk(KERN_WARNING "razernari: Device data transfer failed (len=%d expected=%u).\n",
               len, size);

    return ((len < 0) ? len : ((len != size) ? -EIO : 0));
}

#if NARI_DEBUG_REPORTS
/**
 * Pretty-print a 64-byte report to the kernel log.
 */
static void print_nari_report(unsigned char *report)
{
    int i;

    pr_info("razernari GET_REPORT response:");
    for (i = 0; i < RAZER_NARI_REPORT_LEN; i++) {
        if (i % 16 == 0) {
            pr_cont("\n");
            pr_info("\t");
        }
        pr_cont("%02x ", report[i]);
    }
    pr_cont("\n");
}

/**
 * Issue a GET_REPORT on Feature Report 0xff.
 * The response layout is not yet decoded; this is a debugging aid only.
 */
static int razer_nari_send_request_report_msg(struct hid_device *hdev)
{
    int ret;
    unsigned char *report;
    struct razer_nari_device *device;

    report = kzalloc(RAZER_NARI_REPORT_LEN, GFP_KERNEL);
    if (!report)
        return -ENOMEM;

    ret = hid_hw_raw_request(hdev, 0xff, report, RAZER_NARI_REPORT_LEN,
                             HID_FEATURE_REPORT, HID_REQ_GET_REPORT);
    if (ret != RAZER_NARI_REPORT_LEN) {
        pr_err("razernari: Failed to send GET_REPORT: %d\n", ret);
        kfree(report);
        return ret;
    }

    device = dev_get_drvdata(&hdev->dev);
    memcpy(&device->data[0], report, RAZER_NARI_REPORT_LEN);
    print_nari_report(&device->data[0]);
    kfree(report);
    return 0;
}
#endif /* NARI_DEBUG_REPORTS */

/* ---------------------------------------------------------------------
 * Report builders
 *
 * Each builder returns a 64-byte Feature Report with the common header
 * (bytes 0..4 = ff 0a 00 ff 04) pre-filled. Command-specific bytes are
 * layered on top.
 * ------------------------------------------------------------------ */

/**
 * Base report with only the common 5-byte header filled.
 */
static struct razer_nari_request_report get_nari_base_request_report(void)
{
    struct razer_nari_request_report report;

    memset(&report, 0, sizeof(struct razer_nari_request_report));

    report.length = RAZER_NARI_REPORT_LEN;
    report.arguments[0] = 0xff; /* Report ID, always fixed */
    report.arguments[1] = 0x0a; /* Magic, 0x0a for most commands */
    report.arguments[2] = 0x00;
    report.arguments[3] = 0xff; /* 0xfd for some rare requests */
    report.arguments[4] = 0x04; /* Common wrapper byte */

    return report;
}

/**
 * LED logo on/off command. state = 0x00 (off) or 0xff (on).
 */
static struct razer_nari_request_report get_nari_led_request_report(unsigned char state)
{
    struct razer_nari_request_report report = get_nari_base_request_report();

    report.arguments[5] = 0x12;
    report.arguments[6] = 0xf1;
    report.arguments[7] = 0x03;
    report.arguments[8] = 0x71;
    report.arguments[9] = state;

    return report;
}

/**
 * Haptic motor intensity command (Nari Ultimate only).
 *
 * @enable:    0 = disable, 1 = enable
 * @intensity: 0..100, passed directly as byte 10 of the report (linear)
 */
static struct razer_nari_request_report get_nari_haptic_request_report(unsigned char enable,
                                                                       unsigned char intensity)
{
    struct razer_nari_request_report report = get_nari_base_request_report();

    report.arguments[5] = 0x02;
    report.arguments[6] = 0xf1;
    report.arguments[7] = 0x06;
    report.arguments[8] = 0x20;
    report.arguments[9] = enable ? 0x01 : 0x00;
    report.arguments[10] = intensity;

    return report;
}

#if NARI_DEBUG_REPORTS
/**
 * Speculative RGB colour command from Garfonso's original skeleton.
 * This path has not been verified: the Nari Ultimate's logo is not
 * known to accept colour values, and Synapse itself cannot read the
 * current colour back. Kept for future reverse-engineering work.
 */
static struct razer_nari_request_report get_nari_color_request_report(unsigned char red,
                                                                      unsigned char green,
                                                                      unsigned char blue)
{
    struct razer_nari_request_report report = get_nari_base_request_report();

    report.arguments[5] = 0x12;
    report.arguments[6] = 0xf1;
    report.arguments[7] = 0x05;
    report.arguments[8] = 0x72;
    report.arguments[9]  = red;
    report.arguments[10] = green;
    report.arguments[11] = blue;

    return report;
}
#endif

/* ---------------------------------------------------------------------
 * State setters
 * ------------------------------------------------------------------ */

static void set_led_state(struct razer_nari_device *device, unsigned char on)
{
    struct razer_nari_request_report report =
        get_nari_led_request_report(on ? 0xff : 0x00);

    device->led_brightness = on ? 255 : 0;

    mutex_lock(&device->lock);
    razer_nari_send_control_msg(device->usb_dev, &report, 1);
    mutex_unlock(&device->lock);
}

static void set_haptic(struct razer_nari_device *device,
                       unsigned char enable,
                       unsigned char intensity)
{
    struct razer_nari_request_report report;

    if (intensity > 100)
        intensity = 100;

    device->haptic_enabled = enable ? 1 : 0;
    device->haptic_intensity = intensity;

    report = get_nari_haptic_request_report(device->haptic_enabled,
                                            device->haptic_intensity);

    mutex_lock(&device->lock);
    razer_nari_send_control_msg(device->usb_dev, &report, 1);
    mutex_unlock(&device->lock);
}

/* ---------------------------------------------------------------------
 * Sysfs: introspection
 * ------------------------------------------------------------------ */

static ssize_t razer_attr_read_version(struct device *dev,
                                       struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", DRIVER_VERSION);
}

static ssize_t razer_attr_read_device_type(struct device *dev,
                                           struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    const char *device_type;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS:
    case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_USB:
        device_type = "Razer Nari Ultimate\n";
        break;
    case USB_DEVICE_ID_RAZER_NARI_WIRELESS:
    case USB_DEVICE_ID_RAZER_NARI_USB:
        device_type = "Razer Nari\n";
        break;
    default:
        device_type = "Unknown Device\n";
    }

    return sprintf(buf, "%s", device_type);
}

static ssize_t razer_attr_read_device_serial(struct device *dev,
                                             struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%s\n", device->name);
}

static ssize_t razer_attr_write_test(struct device *dev,
                                     struct device_attribute *attr,
                                     const char *buf, size_t count)
{
    return count;
}

static ssize_t razer_attr_read_test(struct device *dev,
                                    struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "\n");
}

/* ---------------------------------------------------------------------
 * Sysfs: main zone -- haptic motor
 *
 * OpenRazer's daemon treats `matrix_brightness` as a zone-wide slider.
 * For the Nari Ultimate we map this to haptic intensity (0..100), the
 * only meaningful analogue scale the device exposes on this zone.
 * Writing 0 disables the motor; any other value enables it at the
 * given intensity.
 * ------------------------------------------------------------------ */

static ssize_t razer_attr_read_matrix_brightness(struct device *dev,
                                                 struct device_attribute *attr,
                                                 char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n",
                   device->haptic_enabled ? device->haptic_intensity : 0);
}

static ssize_t razer_attr_write_matrix_brightness(struct device *dev,
                                                  struct device_attribute *attr,
                                                  const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    unsigned long v;

    if (kstrtoul(buf, 10, &v) != 0)
        return -EINVAL;
    if (v > 100)
        v = 100;

    set_haptic(device, v > 0 ? 1 : 0, (unsigned char)v);
    return count;
}

static ssize_t razer_attr_write_matrix_effect_none(struct device *dev,
                                                   struct device_attribute *attr,
                                                   const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    set_haptic(device, 0, device->haptic_intensity);
    return count;
}

/* ---------------------------------------------------------------------
 * Sysfs: logo zone -- status LED
 *
 * The logo LED is binary (on/off) on the Nari. We expose it as a second
 * zone so Polychromatic shows it distinctly.
 *
 *   logo_matrix_brightness:    0 = off, any other value = on
 *   logo_matrix_effect_none:   turn the logo off
 *   logo_matrix_effect_static: turn the logo on (RGB bytes are recorded
 *       but only the on/off dimension is known to reach hardware;
 *       colour decoding is future work).
 * ------------------------------------------------------------------ */

static ssize_t razer_attr_read_logo_matrix_brightness(struct device *dev,
                                                      struct device_attribute *attr,
                                                      char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->led_brightness ? 100 : 0);
}

static ssize_t razer_attr_write_logo_matrix_brightness(struct device *dev,
                                                       struct device_attribute *attr,
                                                       const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    unsigned long v;

    if (kstrtoul(buf, 10, &v) != 0)
        return -EINVAL;

    set_led_state(device, v > 0 ? 1 : 0);
    return count;
}

static ssize_t razer_attr_write_logo_matrix_effect_none(struct device *dev,
                                                        struct device_attribute *attr,
                                                        const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    set_led_state(device, 0);
    return count;
}

static ssize_t razer_attr_write_logo_matrix_effect_static(struct device *dev,
                                                          struct device_attribute *attr,
                                                          const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    if (count != 3 && count != 4) {
        printk(KERN_WARNING "razernari: static effect expects 3 bytes (RGB) or 4 bytes (RGBA)\n");
        return -EINVAL;
    }

    /*
     * Store the requested colour for read-back and turn the LED on.
     * Real RGB colour control is not decoded yet; we only drive the
     * verified on/off command here. Colour bytes are kept so
     * user-space sees a consistent value when reading back.
     */
    device->red = buf[0];
    device->green = buf[1];
    device->blue = buf[2];
    set_led_state(device, 1);

#if NARI_DEBUG_REPORTS
    {
        struct razer_nari_request_report rgb =
            get_nari_color_request_report(buf[0], buf[1], buf[2]);
        mutex_lock(&device->lock);
        razer_nari_send_control_msg(device->usb_dev, &rgb, 1);
        mutex_unlock(&device->lock);
    }
#endif

    return count;
}

static ssize_t razer_attr_read_logo_matrix_effect_static(struct device *dev,
                                                         struct device_attribute *attr,
                                                         char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    buf[0] = device->red;
    buf[1] = device->green;
    buf[2] = device->blue;
    return 3;
}

/* ---------------------------------------------------------------------
 * Device attribute declarations
 * ------------------------------------------------------------------ */

static DEVICE_ATTR(test,                      0660, razer_attr_read_test,                       razer_attr_write_test);
static DEVICE_ATTR(version,                   0440, razer_attr_read_version,                    NULL);
static DEVICE_ATTR(device_type,               0440, razer_attr_read_device_type,                NULL);
static DEVICE_ATTR(device_serial,             0440, razer_attr_read_device_serial,              NULL);

static DEVICE_ATTR(matrix_brightness,         0660, razer_attr_read_matrix_brightness,          razer_attr_write_matrix_brightness);
static DEVICE_ATTR(matrix_effect_none,        0220, NULL,                                       razer_attr_write_matrix_effect_none);

static DEVICE_ATTR(logo_matrix_brightness,    0660, razer_attr_read_logo_matrix_brightness,     razer_attr_write_logo_matrix_brightness);
static DEVICE_ATTR(logo_matrix_effect_none,   0220, NULL,                                       razer_attr_write_logo_matrix_effect_none);
static DEVICE_ATTR(logo_matrix_effect_static, 0660, razer_attr_read_logo_matrix_effect_static,  razer_attr_write_logo_matrix_effect_static);

/* ---------------------------------------------------------------------
 * Init / probe / disconnect
 * ------------------------------------------------------------------ */

static void razer_nari_init(struct razer_nari_device *dev,
                            struct usb_interface *intf,
                            struct hid_device *hdev)
{
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    mutex_init(&dev->lock);
    dev->usb_dev = usb_dev;
    dev->usb_interface_protocol = intf->cur_altsetting->desc.bInterfaceProtocol;
    dev->usb_vid = usb_dev->descriptor.idVendor;
    dev->usb_pid = usb_dev->descriptor.idProduct;
    dev->hid_dev = hdev;
    dev->haptic_enabled = 0;
    dev->haptic_intensity = 0;
    dev->led_brightness = 0;

    switch (dev->usb_pid) {
    case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS:
    case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_USB:
        dev->name = "Razer Nari Ultimate";
        break;
    case USB_DEVICE_ID_RAZER_NARI_USB:
    case USB_DEVICE_ID_RAZER_NARI_WIRELESS:
        dev->name = "Razer Nari";
        break;
    default:
        dev->name = "Unknown Device";
        break;
    }
}

static int razer_nari_probe(struct hid_device *hdev,
                            const struct hid_device_id *id)
{
    int retval = 0;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_nari_device *dev = NULL;

    dev = kzalloc(sizeof(struct razer_nari_device), GFP_KERNEL);
    if (dev == NULL) {
        dev_err(&intf->dev, "out of memory\n");
        return -ENOMEM;
    }

    razer_nari_init(dev, intf, hdev);

    if (dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE) {
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_test);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);

        /* Main zone: haptic motor */
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_brightness);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);

        /* Logo zone: status LED */
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_brightness);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_none);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_static);
    }

    dev_set_drvdata(&hdev->dev, dev);

    if (hid_parse(hdev)) {
        hid_err(hdev, "parse failed\n");
        goto exit_free;
    }

    if (hid_hw_start(hdev, HID_CONNECT_DEFAULT)) {
        hid_err(hdev, "hw start failed\n");
        goto exit_free;
    }

    usb_disable_autosuspend(usb_dev);
    return 0;

exit_free:
    kfree(dev);
    return retval;
}

static void razer_nari_disconnect(struct hid_device *hdev)
{
    struct razer_nari_device *dev;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);

    dev = hid_get_drvdata(hdev);

    if (dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE) {
        device_remove_file(&hdev->dev, &dev_attr_version);
        device_remove_file(&hdev->dev, &dev_attr_test);
        device_remove_file(&hdev->dev, &dev_attr_device_type);
        device_remove_file(&hdev->dev, &dev_attr_device_serial);

        device_remove_file(&hdev->dev, &dev_attr_matrix_brightness);
        device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);

        device_remove_file(&hdev->dev, &dev_attr_logo_matrix_brightness);
        device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_none);
        device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_static);
    }

    hid_hw_stop(hdev);
    kfree(dev);
    dev_info(&intf->dev, "Razer Nari disconnected\n");
}

/* ---------------------------------------------------------------------
 * HID device table and module hookup
 * ------------------------------------------------------------------ */

static const struct hid_device_id razer_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_NARI_ULTIMATE_USB) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_NARI_WIRELESS) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_NARI_USB) },
    { 0 }
};

MODULE_DEVICE_TABLE(hid, razer_devices);

static struct hid_driver razer_nari_driver = {
    .name = "razernari",
    .id_table = razer_devices,
    .probe = razer_nari_probe,
    .remove = razer_nari_disconnect
};

module_hid_driver(razer_nari_driver);
