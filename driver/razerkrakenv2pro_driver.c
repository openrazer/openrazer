// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Memoraike
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/random.h>

#include "razerkrakenv2pro_driver.h"
#include "razercommon.h"

/*
 * Version Information
 */
#define DRIVER_DESC "Razer Kraken Kitty V2 Pro Device Driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE(DRIVER_LICENSE);

/**
 * Get a report with the header filled in and everything else zeroed
 */
static struct razer_krakenv2pro_report get_kraken_v2pro_report(unsigned char command, unsigned char subcommand)
{
    struct razer_krakenv2pro_report report = {0};

    report.report_id = KRAKEN_V2_PRO_REPORT_ID;
    report.command = command;
    report.subcommand = subcommand;

    return report;
}

static int razer_krakenv2pro_send(struct hid_device *hdev, struct razer_krakenv2pro_report *report)
{
    struct usb_device *usb_dev = hid_to_usb_dev(hdev);
    int ret;

    ret = usb_control_msg_send(usb_dev,
                               0, // endpoint to send the message to
                               HID_REQ_SET_REPORT, // USB message request value (0x09)
                               USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT, // USB message request type value (0x21)
                               KRAKEN_V2_PRO_USB_VALUE, // USB message value
                               KRAKEN_V2_PRO_USB_INDEX, // USB message index value
                               report, // pointer to the data to send
                               sizeof(*report), // length in bytes of the data to send
                               USB_CTRL_SET_TIMEOUT, // time in msecs to wait for the message to complete before timing out
                               GFP_KERNEL);

    if (ret)
        hid_warn(hdev, "Failed to send USB control message: %d\n", ret);

    return ret;
}

/**
 * Read device file "version"
 */
static ssize_t razer_attr_read_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%s\n", DRIVER_VERSION);
}

/**
 * Read device file "device_type"
 */
static ssize_t razer_attr_read_device_type(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_krakenv2pro_device *device = dev_get_drvdata(dev);

    char *device_type;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2_PRO:
        device_type = "Razer Kraken Kitty V2 Pro";
        break;

    default:
        device_type = "Unknown Device";
    }

    return sysfs_emit(buf, "%s\n", device_type);
}

/**
 * Read device file "device_serial"
 *
 * The device carries a real serial in its USB descriptor, so there is no
 * need to ask the protocol for one - which would not work anyway.
 */
static ssize_t razer_attr_read_device_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_krakenv2pro_device *device = dev_get_drvdata(dev);

    return sysfs_emit(buf, "%s\n", device->serial);
}

/**
 * Read device file "firmware_version"
 *
 * Taken from bcdDevice: the protocol has no way to report a firmware version.
 */
static ssize_t razer_attr_read_firmware_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_krakenv2pro_device *device = dev_get_drvdata(dev);
    unsigned short bcd = le16_to_cpu(hid_to_usb_dev(device->hdev)->descriptor.bcdDevice);

    return sysfs_emit(buf, "v%x.%x\n", bcd >> 8, bcd & 0xff);
}

/**
 * Read device file "device_mode"
 *
 * The protocol has no device mode command, so report the normal mode.
 */
static ssize_t razer_attr_read_device_mode(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "0:0\n");
}

/**
 * Send a hardware effect to the zones named by mask
 *
 * colour_count is 0, 1 or 2 and colours points at colour_count * 3 bytes.
 * The device rejects a count of 3 outright, leaving its previous state in
 * place, so callers must refuse anything larger.
 */
static int razer_krakenv2pro_send_effect(struct razer_krakenv2pro_device *device,
        u8 mask, u8 effect, u8 colour_count, const u8 *colours)
{
    struct razer_krakenv2pro_report report =
        get_kraken_v2pro_report(KRAKEN_V2_PRO_CMD_MODE, KRAKEN_V2_PRO_SUB_EFFECT);

    report.data[0] = mask;
    report.data[1] = effect;
    report.data[2] = colour_count;

    if(colour_count)
        memcpy(&report.data[3], colours, colour_count * 3);

    return razer_krakenv2pro_send(device->hdev, &report);
}

/**
 * Apply an effect to one or more zones and remember it
 *
 * device->effect tracks the whole-device effect only, so a write aimed at a
 * single zone leaves it alone.
 */
static ssize_t razer_krakenv2pro_zone_effect(struct device *dev, u8 mask,
        u8 effect, u8 colour_count, const u8 *colours, size_t count)
{
    struct razer_krakenv2pro_device *device = dev_get_drvdata(dev);

    mutex_lock(&device->lock);
    razer_krakenv2pro_send_effect(device, mask, effect, colour_count, colours);
    if(mask == KRAKEN_V2_PRO_ZONE_MASK_ALL) {
        device->effect = (effect == KRAKEN_V2_PRO_EFFECT_STATIC && colour_count == 0)
                         ? KRAKEN_V2_PRO_EFFECT_NONE : effect;
        device->direct = false;
    }
    mutex_unlock(&device->lock);

    return count;
}

static ssize_t razer_krakenv2pro_zone_brightness(struct device *dev, u8 mask,
        const char *buf, size_t count)
{
    struct razer_krakenv2pro_device *device = dev_get_drvdata(dev);
    struct razer_krakenv2pro_report report =
        get_kraken_v2pro_report(KRAKEN_V2_PRO_CMD_BRIGHTNESS, KRAKEN_V2_PRO_SUB_BRIGHTNESS);
    unsigned char brightness;
    int ret;

    ret = kstrtou8(buf, 10, &brightness);
    if(ret)
        return ret;

    report.data[0] = mask;
    report.data[1] = brightness;

    mutex_lock(&device->lock);
    if(mask == KRAKEN_V2_PRO_ZONE_MASK_ALL)
        device->brightness = brightness;
    razer_krakenv2pro_send(device->hdev, &report);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Apply the breathing effect to one or more zones
 *
 * The daemon writes 1 byte for a random colour, 3 for one colour and 6 for
 * two. Nine bytes are refused: the device rejects three colours outright and
 * keeps whatever it was already showing.
 */
static ssize_t razer_krakenv2pro_zone_breath(struct device *dev, u8 mask,
        const char *buf, size_t count)
{
    u8 colour_count;

    switch(count) {
    case 3: // Single colour mode
        colour_count = 1;
        break;

    case 6: // Dual colour mode
        colour_count = 2;
        break;

    case 1: // "Random" colour mode
        colour_count = 0;
        break;

    default:
        dev_warn(dev, "razerkrakenv2pro: breathing mode accepts 1, 3 or 6 bytes\n");
        return -EINVAL;
    }

    return razer_krakenv2pro_zone_effect(dev, mask, KRAKEN_V2_PRO_EFFECT_BREATHING,
                                         colour_count, colour_count ? buf : NULL,
                                         count);
}

/**
 * Switch the device into direct mode, where it takes colour frames
 *
 * Caller holds device->lock.
 */
static int razer_krakenv2pro_enter_direct(struct razer_krakenv2pro_device *device)
{
    struct razer_krakenv2pro_report report =
        get_kraken_v2pro_report(KRAKEN_V2_PRO_CMD_MODE, KRAKEN_V2_PRO_SUB_MODE);
    int ret;

    report.data[0] = KRAKEN_V2_PRO_ZONE_MASK_ALL;
    report.data[1] = KRAKEN_V2_PRO_MODE_DIRECT;

    ret = razer_krakenv2pro_send(device->hdev, &report);
    if(!ret) {
        device->effect = KRAKEN_V2_PRO_MODE_DIRECT;
        device->direct = true;
    }

    return ret;
}

/**
 * Write device file "matrix_effect_custom"
 *
 * Only switches if the device is not in direct mode already. The mode command
 * discards a frame that arrived just before it, and pylib draws by writing
 * matrix_custom_frame and then matrix_effect_custom, so sending it every time
 * would swallow every frame: the colours would only appear one draw late.
 */
static ssize_t razer_attr_write_matrix_effect_custom(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_krakenv2pro_device *device = dev_get_drvdata(dev);

    mutex_lock(&device->lock);
    if(!device->direct)
        razer_krakenv2pro_enter_direct(device);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Write device file "matrix_custom_frame"
 *
 * Takes row, start column, end column and then RGB per column, as everywhere
 * else in openrazer. The device is a single row of four zones.
 */
static ssize_t razer_attr_write_matrix_custom_frame(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_krakenv2pro_device *device = dev_get_drvdata(dev);
    struct razer_krakenv2pro_report report =
        get_kraken_v2pro_report(KRAKEN_V2_PRO_CMD_FRAME, 0x00);
    unsigned char row, start_col, end_col, num_cols;

    if(count < 4)
        return -EINVAL;

    row = buf[0];
    start_col = buf[1];
    end_col = buf[2];

    // The device has a single row
    if(row != 0)
        return -EINVAL;

    if(start_col > end_col || end_col >= KRAKEN_V2_PRO_ZONES)
        return -EINVAL;

    num_cols = end_col - start_col + 1;

    if(count != (size_t)(3 + num_cols * 3))
        return -EINVAL;

    mutex_lock(&device->lock);

    /*
     * The device discards frames unless direct mode is in force, and pylib
     * writes the frame before matrix_effect_custom, so the first frame of a
     * session would be lost. Switch on the transition only, not per frame.
     */
    if(!device->direct)
        razer_krakenv2pro_enter_direct(device);

    /*
     * The wire format has no partial frame, so merge into the shadow copy and
     * send all four zones. Without this a write naming only some columns
     * would blank the rest.
     */
    memcpy(&device->frame[start_col * 3], &buf[3], num_cols * 3);
    memcpy(report.data, device->frame, KRAKEN_V2_PRO_FRAME_LEN);

    razer_krakenv2pro_send(device->hdev, &report);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Write device file "matrix_brightness"
 */
static ssize_t razer_attr_write_matrix_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return razer_krakenv2pro_zone_brightness(dev, KRAKEN_V2_PRO_ZONE_MASK_ALL, buf, count);
}

/**
 * Read device file "matrix_brightness"
 *
 * Returns the cached value: the device answers no reads.
 */
static ssize_t razer_attr_read_matrix_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_krakenv2pro_device *device = dev_get_drvdata(dev);
    unsigned char brightness;

    mutex_lock(&device->lock);
    brightness = device->brightness;
    mutex_unlock(&device->lock);

    return sysfs_emit(buf, "%d\n", brightness);
}

/**
 * Read device file "matrix_current_effect"
 *
 * Reports the wire value of the last effect applied to the whole device.
 * A write aimed at a single zone does not change it, and the device cannot
 * be asked what it is actually showing.
 */
static ssize_t razer_attr_read_matrix_current_effect(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_krakenv2pro_device *device = dev_get_drvdata(dev);
    unsigned char effect;

    mutex_lock(&device->lock);
    effect = device->effect;
    mutex_unlock(&device->lock);

    return sysfs_emit(buf, "%02x\n", effect);
}

/**
 * Write device file "matrix_effect_none"
 *
 * Static with no colour switches the LEDs off in hardware, and that survives
 * brightness changes, so there is no need to gate brightness behind a flag.
 */
static ssize_t razer_attr_write_matrix_effect_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return razer_krakenv2pro_zone_effect(dev, KRAKEN_V2_PRO_ZONE_MASK_ALL,
                                         KRAKEN_V2_PRO_EFFECT_STATIC, 0, NULL, count);
}

/**
 * Write device file "matrix_effect_static"
 */
static ssize_t razer_attr_write_matrix_effect_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    if(count != 3) {
        dev_warn(dev, "razerkrakenv2pro: static mode only accepts RGB (3byte)\n");
        return -EINVAL;
    }

    return razer_krakenv2pro_zone_effect(dev, KRAKEN_V2_PRO_ZONE_MASK_ALL,
                                         KRAKEN_V2_PRO_EFFECT_STATIC, 1, buf, count);
}

/**
 * Write device file "matrix_effect_breath"
 */
static ssize_t razer_attr_write_matrix_effect_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return razer_krakenv2pro_zone_breath(dev, KRAKEN_V2_PRO_ZONE_MASK_ALL, buf, count);
}

/**
 * Write device file "matrix_effect_spectrum"
 */
static ssize_t razer_attr_write_matrix_effect_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return razer_krakenv2pro_zone_effect(dev, KRAKEN_V2_PRO_ZONE_MASK_ALL,
                                         KRAKEN_V2_PRO_EFFECT_SPECTRUM, 0, NULL, count);
}

/*
 * Set up the device driver files
 *
 * Read only is 0440
 * Write only is 0220
 * Read and write is 0660
 */

static DEVICE_ATTR(version,           0440, razer_attr_read_version,          NULL);
static DEVICE_ATTR(device_type,       0440, razer_attr_read_device_type,      NULL);
static DEVICE_ATTR(device_serial,     0440, razer_attr_read_device_serial,    NULL);
static DEVICE_ATTR(firmware_version,  0440, razer_attr_read_firmware_version, NULL);
static DEVICE_ATTR(device_mode,       0440, razer_attr_read_device_mode,      NULL);

static DEVICE_ATTR(matrix_current_effect,  0440, razer_attr_read_matrix_current_effect, NULL);
static DEVICE_ATTR(matrix_brightness,      0660, razer_attr_read_matrix_brightness, razer_attr_write_matrix_brightness);
static DEVICE_ATTR(matrix_effect_none,     0220, NULL, razer_attr_write_matrix_effect_none);
static DEVICE_ATTR(matrix_effect_static,   0220, NULL, razer_attr_write_matrix_effect_static);
static DEVICE_ATTR(matrix_effect_breath,   0220, NULL, razer_attr_write_matrix_effect_breath);
static DEVICE_ATTR(matrix_effect_spectrum, 0220, NULL, razer_attr_write_matrix_effect_spectrum);
static DEVICE_ATTR(matrix_effect_custom,   0220, NULL, razer_attr_write_matrix_effect_custom);
static DEVICE_ATTR(matrix_custom_frame,    0220, NULL, razer_attr_write_matrix_custom_frame);

/*
 * Each zone takes effects and brightness on its own, through the zone mask.
 * Writing the twenty attributes out by hand would be several hundred lines of
 * copy-paste, so they are generated from the mask-aware helpers above.
 *
 * Brightness is write-only per zone: only the whole-device value is cached,
 * and the device cannot be asked, so a per-zone read would be a guess.
 */
#define KRAKEN_V2_PRO_ZONE_ATTRS(zone, mask)                                                        \
static ssize_t razer_attr_write_##zone##_matrix_effect_none(struct device *dev,                     \
        struct device_attribute *attr, const char *buf, size_t count)                               \
{                                                                                                   \
    return razer_krakenv2pro_zone_effect(dev, mask,                                                \
            KRAKEN_V2_PRO_EFFECT_STATIC, 0, NULL, count);                                           \
}                                                                                                   \
static ssize_t razer_attr_write_##zone##_matrix_effect_static(struct device *dev,                   \
        struct device_attribute *attr, const char *buf, size_t count)                               \
{                                                                                                   \
    if(count != 3) {                                                                                \
        dev_warn(dev, "razerkrakenv2pro: static mode only accepts RGB (3byte)\n");                  \
        return -EINVAL;                                                                             \
    }                                                                                               \
    return razer_krakenv2pro_zone_effect(dev, mask,                                                \
            KRAKEN_V2_PRO_EFFECT_STATIC, 1, buf, count);                                            \
}                                                                                                   \
static ssize_t razer_attr_write_##zone##_matrix_effect_spectrum(struct device *dev,                 \
        struct device_attribute *attr, const char *buf, size_t count)                               \
{                                                                                                   \
    return razer_krakenv2pro_zone_effect(dev, mask,                                                \
            KRAKEN_V2_PRO_EFFECT_SPECTRUM, 0, NULL, count);                                         \
}                                                                                                   \
static ssize_t razer_attr_write_##zone##_matrix_effect_breath(struct device *dev,                   \
        struct device_attribute *attr, const char *buf, size_t count)                               \
{                                                                                                   \
    return razer_krakenv2pro_zone_breath(dev, mask, buf, count);                                   \
}                                                                                                   \
static ssize_t razer_attr_write_##zone##_led_brightness(struct device *dev,                         \
        struct device_attribute *attr, const char *buf, size_t count)                               \
{                                                                                                   \
    return razer_krakenv2pro_zone_brightness(dev, mask, buf, count);                               \
}                                                                                                   \
static DEVICE_ATTR(zone##_matrix_effect_none,     0220, NULL, razer_attr_write_##zone##_matrix_effect_none);     \
static DEVICE_ATTR(zone##_matrix_effect_static,   0220, NULL, razer_attr_write_##zone##_matrix_effect_static);   \
static DEVICE_ATTR(zone##_matrix_effect_spectrum, 0220, NULL, razer_attr_write_##zone##_matrix_effect_spectrum); \
static DEVICE_ATTR(zone##_matrix_effect_breath,   0220, NULL, razer_attr_write_##zone##_matrix_effect_breath);   \
static DEVICE_ATTR(zone##_led_brightness,         0220, NULL, razer_attr_write_##zone##_led_brightness)

/*
 * The ear cups use openrazer's existing left/right zones, so the daemon
 * already has D-Bus methods writing to these exact attribute names. The cat
 * ears have no equivalent in the zone vocabulary and get their own.
 */
KRAKEN_V2_PRO_ZONE_ATTRS(left_ear,  KRAKEN_V2_PRO_ZONE_LEFT_EAR);
KRAKEN_V2_PRO_ZONE_ATTRS(right_ear, KRAKEN_V2_PRO_ZONE_RIGHT_EAR);
KRAKEN_V2_PRO_ZONE_ATTRS(left,      KRAKEN_V2_PRO_ZONE_LEFT_CUP);
KRAKEN_V2_PRO_ZONE_ATTRS(right,     KRAKEN_V2_PRO_ZONE_RIGHT_CUP);

static void razer_krakenv2pro_init(struct razer_krakenv2pro_device *dev, struct hid_device *hdev)
{
    struct usb_device *usb_dev = hid_to_usb_dev(hdev);
    unsigned int rand_serial = 0;

    // Initialise mutex
    mutex_init(&dev->lock);
    // Setup values
    dev->hdev = hdev;
    dev->usb_vid = usb_dev->descriptor.idVendor;
    dev->usb_pid = usb_dev->descriptor.idProduct;

    // usbhid puts the USB serial string in hdev->uniq
    if (hdev->uniq[0] != '\0') {
        strscpy(dev->serial, hdev->uniq, sizeof(dev->serial));
    } else {
        // Get a "random" integer
        get_random_bytes(&rand_serial, sizeof(unsigned int));
        scnprintf(dev->serial, sizeof(dev->serial), "HN%015u", rand_serial);
    }

    /*
     * The device keeps its effect across a replug and cannot be asked what it
     * is showing, so these are a nominal default rather than the truth. No
     * command is sent here, which leaves whatever the user set in place.
     */
    dev->brightness = 0xff;
    dev->effect = KRAKEN_V2_PRO_EFFECT_SPECTRUM;
    dev->direct = false;
}

static int razer_krakenv2pro_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int retval = 0;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct razer_krakenv2pro_device *dev = NULL;

    dev = kzalloc_obj(*dev);
    if(dev == NULL) {
        hid_err(hdev, "out of memory\n");
        return -ENOMEM;
    }

    razer_krakenv2pro_init(dev, hdev);

    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_NONE) {
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);                               // Get driver version
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);                           // Get string of device type
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);                         // Get string of device serial
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_firmware_version);                      // Get string of device fw version
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_mode);                           // Get device mode

        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_brightness);                     // Brightness
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);                    // No effect
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);                  // Static effect
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);                  // Breathing effect
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);                // Spectrum effect
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);                  // Custom effect
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);                   // Custom frame
        switch(dev->usb_pid) {
        case USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2_PRO:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_current_effect);                 // Get current effect

            // left cat ear
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_ear_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_ear_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_ear_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_ear_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_ear_led_brightness);

            // right cat ear
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_ear_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_ear_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_ear_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_ear_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_ear_led_brightness);

            // left ear cup
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_left_led_brightness);

            // right ear cup
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_right_led_brightness);
            break;
        }
    }

    dev_set_drvdata(&hdev->dev, dev);

    if(hid_parse(hdev)) {
        hid_err(hdev, "parse failed\n");
        goto exit_free;
    }

    if(hid_hw_start(hdev, HID_CONNECT_DEFAULT)) {
        hid_err(hdev, "hw start failed\n");
        goto exit_free;
    }

    return 0;

exit_free:
    kfree(dev);
    return retval;
}

static void razer_krakenv2pro_disconnect(struct hid_device *hdev)
{
    struct razer_krakenv2pro_device *dev = dev_get_drvdata(&hdev->dev);
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);

    if(intf->cur_altsetting->desc.bInterfaceProtocol == USB_INTERFACE_PROTOCOL_NONE) {
        device_remove_file(&hdev->dev, &dev_attr_version);                               // Get driver version
        device_remove_file(&hdev->dev, &dev_attr_device_type);                           // Get string of device type
        device_remove_file(&hdev->dev, &dev_attr_device_serial);                         // Get string of device serial
        device_remove_file(&hdev->dev, &dev_attr_firmware_version);                      // Get string of device fw version
        device_remove_file(&hdev->dev, &dev_attr_device_mode);                           // Get device mode

        device_remove_file(&hdev->dev, &dev_attr_matrix_brightness);                     // Brightness
        device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);                    // No effect
        device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);                  // Static effect
        device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);                  // Breathing effect
        device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);                // Spectrum effect
        device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);                  // Custom effect
        device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);                   // Custom frame
        device_remove_file(&hdev->dev, &dev_attr_matrix_current_effect);                 // Get current effect

        // left cat ear
        device_remove_file(&hdev->dev, &dev_attr_left_ear_matrix_effect_none);
        device_remove_file(&hdev->dev, &dev_attr_left_ear_matrix_effect_static);
        device_remove_file(&hdev->dev, &dev_attr_left_ear_matrix_effect_spectrum);
        device_remove_file(&hdev->dev, &dev_attr_left_ear_matrix_effect_breath);
        device_remove_file(&hdev->dev, &dev_attr_left_ear_led_brightness);

        // right cat ear
        device_remove_file(&hdev->dev, &dev_attr_right_ear_matrix_effect_none);
        device_remove_file(&hdev->dev, &dev_attr_right_ear_matrix_effect_static);
        device_remove_file(&hdev->dev, &dev_attr_right_ear_matrix_effect_spectrum);
        device_remove_file(&hdev->dev, &dev_attr_right_ear_matrix_effect_breath);
        device_remove_file(&hdev->dev, &dev_attr_right_ear_led_brightness);

        // left ear cup
        device_remove_file(&hdev->dev, &dev_attr_left_matrix_effect_none);
        device_remove_file(&hdev->dev, &dev_attr_left_matrix_effect_static);
        device_remove_file(&hdev->dev, &dev_attr_left_matrix_effect_spectrum);
        device_remove_file(&hdev->dev, &dev_attr_left_matrix_effect_breath);
        device_remove_file(&hdev->dev, &dev_attr_left_led_brightness);

        // right ear cup
        device_remove_file(&hdev->dev, &dev_attr_right_matrix_effect_none);
        device_remove_file(&hdev->dev, &dev_attr_right_matrix_effect_static);
        device_remove_file(&hdev->dev, &dev_attr_right_matrix_effect_spectrum);
        device_remove_file(&hdev->dev, &dev_attr_right_matrix_effect_breath);
        device_remove_file(&hdev->dev, &dev_attr_right_led_brightness);
    }

    hid_hw_stop(hdev);
    kfree(dev);
    dev_info(&intf->dev, "Razer Device disconnected\n");
}

static const struct hid_device_id razer_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_KRAKEN_KITTY_V2_PRO) },
    { }
};

MODULE_DEVICE_TABLE(hid, razer_devices);

static struct hid_driver razer_krakenv2pro_driver = {
    .name = "razerkrakenv2pro",
    .id_table = razer_devices,
    .probe = razer_krakenv2pro_probe,
    .remove = razer_krakenv2pro_disconnect,
};

module_hid_driver(razer_krakenv2pro_driver);
