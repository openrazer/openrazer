// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/delay.h>
#include <linux/hid.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/usb/input.h>

#include "razerblackshark_driver.h"
#include "razercommon.h"

#define DRIVER_DESC "Razer BlackShark V3 X Driver"

/*
 * Wire protocol (validated against 1532:057C and 1532:057D):
 *
 * 64-byte feature reports on report ID 0x07, exchanged over the control
 * endpoint with SET_REPORT/GET_REPORT.
 *
 *   [0]      report id (0x07)
 *   [1]      status (0x00 on requests)
 *   [2]      transaction id (echoed by responses)
 *   [6]      argument count
 *   [7]      command class (0x07 power/battery, 0x18 audio)
 *   [8]      command id (GET ids are the SET id | 0x80)
 *   [9..]    arguments
 *   [62]     XOR checksum of bytes [3..60]
 *   [63]     reserved
 *
 * Commands are issued with SET_REPORT; the reply is collected with
 * GET_REPORT after a short delay.
 */

#define RAZER_BS_CLASS_POWER 0x07
#define RAZER_BS_CLASS_AUDIO 0x18

#define RAZER_BS_BATTERY_GET  0x80
#define RAZER_BS_CHARGING_GET 0x84
#define RAZER_BS_SLEEP_SET    0x03 /* big endian seconds */
#define RAZER_BS_SLEEP_GET    0x83
#define RAZER_BS_SAVE_SET     0x08
#define RAZER_BS_SAVE_GET     0x88

#define RAZER_BS_PRESET_SET 0x00 /* args 00 01 00 VV, VV 0..3 */
#define RAZER_BS_PRESET_GET 0x80
#define RAZER_BS_EQ_SET     0x01 /* args 00 01 B1..B10, signed magnitude dB */
#define RAZER_BS_EQ_GET     0x81
#define RAZER_BS_TONE_SET   0x03 /* args 00 03 VV, VV 0..15 */
#define RAZER_BS_TONE_GET   0x83
#define RAZER_BS_MICNC_SET  0x0F /* args 00 VV, 0/1 */
#define RAZER_BS_MICNC_GET  0x8F

#define RAZER_BS_INFO_CLASS   0x00
#define RAZER_BS_FIRMWARE_GET 0x81
#define RAZER_BS_SERIAL_GET   0x82

#define RAZER_BS_SERIAL_LEN 22

#define RAZER_BS_EQ_BANDS   10
#define RAZER_BS_EQ_DB_MAX  6
#define RAZER_BS_MAX_ARGS   52 /* response args span [9..60] */
#define RAZER_BS_WAIT_US    50000

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE(DRIVER_LICENSE);

static u8 razer_blackshark_crc(const u8 *report)
{
    u8 crc = 0;
    int i;

    for (i = 3; i <= 60; i++)
        crc ^= report[i];

    return crc;
}

static void razer_blackshark_build_command(struct razer_blackshark_device *device,
        u8 cmd_class, u8 cmd_id,
        u8 args_len, const u8 *args,
        u8 *report)
{
    memset(report, 0, RAZER_BLACKSHARK_REPORT_LEN);

    report[0] = RAZER_BLACKSHARK_REPORT_ID;
    report[2] = device->transaction_id++;
    report[6] = args_len;
    report[7] = cmd_class;
    report[8] = cmd_id;
    memcpy(&report[9], args, args_len);
    report[62] = razer_blackshark_crc(report);
}

/*
 * Send a command and collect its reply arguments.
 *
 * args must have room for RAZER_BS_MAX_ARGS bytes.
 */
static int razer_blackshark_get_args(struct razer_blackshark_device *device,
                                     u8 cmd_class, u8 cmd_id,
                                     u8 *args, u8 *args_len)
{
    static const u8 query[] = { 0x00, 0x00 };
    u8 *request;
    u8 *response;
    int ret;

    request = kzalloc(RAZER_BLACKSHARK_REPORT_LEN, GFP_KERNEL);
    response = kzalloc(RAZER_BLACKSHARK_REPORT_LEN, GFP_KERNEL);
    if (!request || !response) {
        ret = -ENOMEM;
        goto out_free;
    }

    mutex_lock(&device->lock);

    razer_blackshark_build_command(device, cmd_class, cmd_id,
                                   sizeof(query), query, request);

    ret = hid_hw_raw_request(device->hdev, RAZER_BLACKSHARK_REPORT_ID,
                             request, RAZER_BLACKSHARK_REPORT_LEN,
                             HID_FEATURE_REPORT, HID_REQ_SET_REPORT);
    if (ret < 0)
        goto out_unlock;

    fsleep(RAZER_BS_WAIT_US);

    response[0] = RAZER_BLACKSHARK_REPORT_ID;
    ret = hid_hw_raw_request(device->hdev, RAZER_BLACKSHARK_REPORT_ID,
                             response, RAZER_BLACKSHARK_REPORT_LEN,
                             HID_FEATURE_REPORT, HID_REQ_GET_REPORT);
    if (ret < 0)
        goto out_unlock;

    if (ret < RAZER_BLACKSHARK_REPORT_LEN ||
        response[0] != RAZER_BLACKSHARK_REPORT_ID ||
        response[2] != request[2] ||
        response[7] != cmd_class || response[8] != cmd_id ||
        response[62] != razer_blackshark_crc(response)) {
        hid_err(device->hdev,
                "invalid response: ret=%d id=%02x transaction=%02x/%02x class=%02x command=%02x\n",
                ret, response[0], response[2], request[2],
                response[7], response[8]);
        ret = -EPROTO;
        goto out_unlock;
    }

    /*
     * A command the firmware does not run comes back as an untouched echo
     * of the request, which satisfies every check above. Only status 0x02
     * means the reply actually carries data.
     */
    if (response[1] != RAZER_CMD_SUCCESSFUL) {
        ret = -EPROTO;
        goto out_unlock;
    }

    if (response[6] > RAZER_BS_MAX_ARGS) {
        hid_err(device->hdev, "oversized response: %u bytes\n",
                response[6]);
        ret = -EPROTO;
        goto out_unlock;
    }

    memcpy(args, &response[9], response[6]);
    *args_len = response[6];
    ret = 0;

out_unlock:
    mutex_unlock(&device->lock);
out_free:
    kfree(response);
    kfree(request);
    return ret;
}

static int razer_blackshark_set_args(struct razer_blackshark_device *device,
                                     u8 cmd_class, u8 cmd_id,
                                     u8 args_len, const u8 *args)
{
    u8 *request;
    int ret;

    request = kzalloc(RAZER_BLACKSHARK_REPORT_LEN, GFP_KERNEL);
    if (!request)
        return -ENOMEM;

    mutex_lock(&device->lock);

    razer_blackshark_build_command(device, cmd_class, cmd_id,
                                   args_len, args, request);

    ret = hid_hw_raw_request(device->hdev, RAZER_BLACKSHARK_REPORT_ID,
                             request, RAZER_BLACKSHARK_REPORT_LEN,
                             HID_FEATURE_REPORT, HID_REQ_SET_REPORT);
    if (ret >= 0)
        ret = 0;

    fsleep(RAZER_BS_WAIT_US);

    mutex_unlock(&device->lock);

    kfree(request);
    return ret;
}

static int razer_blackshark_get_byte(struct razer_blackshark_device *device,
                                     u8 cmd_class, u8 cmd_id,
                                     u8 index, u8 *value)
{
    u8 args[RAZER_BS_MAX_ARGS];
    u8 args_len;
    int ret;

    ret = razer_blackshark_get_args(device, cmd_class, cmd_id,
                                    args, &args_len);
    if (ret)
        return ret;

    if (index >= args_len)
        return -EPROTO;

    *value = args[index];
    return 0;
}

static ssize_t razer_attr_read_version(struct device *dev,
                                       struct device_attribute *attr,
                                       char *buf)
{
    return sysfs_emit(buf, "%s\n", DRIVER_VERSION);
}

static ssize_t razer_attr_read_device_type(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    char *device_type;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_X_USB:
        device_type = "Razer BlackShark V3 X (Wired)";
        break;
    case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_X:
        device_type = "Razer BlackShark V3 X (Wireless)";
        break;
    default:
        device_type = "Unknown Device";
    }

    return sysfs_emit(buf, "%s\n", device_type);
}

static ssize_t razer_attr_read_device_serial(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 args[RAZER_BS_MAX_ARGS];
    char serial[RAZER_BS_SERIAL_LEN + 1];
    u8 args_len;
    int i;
    int ret;

    /* hdev->uniq is an array, so it is empty rather than NULL when unset. */
    const char *fallback = device->hdev->uniq[0] ? device->hdev->uniq : "BSV3X";

    ret = razer_blackshark_get_args(device, RAZER_BS_INFO_CLASS,
                                    RAZER_BS_SERIAL_GET, args, &args_len);
    if (ret || args_len > RAZER_BS_SERIAL_LEN)
        goto fallback;

    /* NUL padded ASCII; reject anything that is not printable */
    for (i = 0; i < args_len && args[i]; i++) {
        if (args[i] < 0x20 || args[i] > 0x7E)
            goto fallback;
        serial[i] = args[i];
    }
    if (i == 0)
        goto fallback;
    serial[i] = '\0';

    /*
     * The wired connection and the dongle report the same headset serial,
     * so keep the PID suffix: the daemon keys its DBus object path off
     * this and both can be plugged in at once.
     */
    return sysfs_emit(buf, "%s%04X\n", serial, device->usb_pid);

fallback:
    return sysfs_emit(buf, "%s%04X\n", fallback, device->usb_pid);
}

static ssize_t razer_attr_read_firmware_version(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 args[RAZER_BS_MAX_ARGS];
    u8 args_len;
    int ret;

    /*
     * Only the dongle answers this; the wired connection reports the
     * command as not executed, which get_args() rejects on status.
     */
    ret = razer_blackshark_get_args(device, RAZER_BS_INFO_CLASS,
                                    RAZER_BS_FIRMWARE_GET, args, &args_len);
    if (ret || args_len < 2)
        return sysfs_emit(buf, "unknown\n");

    return sysfs_emit(buf, "v%u.%u\n", args[0], args[1]);
}

static ssize_t razer_attr_read_charge_level(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 level = 0;
    int ret;

    /*
     * The dongle cannot answer while the headset is unlinked. Report the
     * -1 sentinel the daemon already understands instead of an errno, so
     * that its battery polling thread keeps running.
     */
    ret = razer_blackshark_get_byte(device, RAZER_BS_CLASS_POWER,
                                    RAZER_BS_BATTERY_GET, 1, &level);
    if (ret || level > 100)
        return sysfs_emit(buf, "-1\n");

    /* OpenRazer's existing get_battery DBus method expects 0..255. */
    return sysfs_emit(buf, "%u\n",
                      DIV_ROUND_CLOSEST((unsigned int)level * 255, 100));
}

static ssize_t razer_attr_read_charge_status(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 status = 0;
    int ret;

    ret = razer_blackshark_get_byte(device, RAZER_BS_CLASS_POWER,
                                    RAZER_BS_CHARGING_GET, 1, &status);
    if (ret || status > 1)
        return sysfs_emit(buf, "0\n");

    return sysfs_emit(buf, "%u\n", status);
}

static ssize_t razer_attr_read_sidetone(struct device *dev,
                                        struct device_attribute *attr,
                                        char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 level;
    int ret;

    ret = razer_blackshark_get_byte(device, RAZER_BS_CLASS_AUDIO,
                                    RAZER_BS_TONE_GET, 2, &level);
    if (ret)
        return ret;
    if (level > 15)
        return -EPROTO;

    return sysfs_emit(buf, "%u\n", level);
}

static ssize_t razer_attr_write_sidetone(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 level;
    int ret;

    if (kstrtou8(buf, 10, &level))
        return -EINVAL;
    if (level > 15)
        return -EINVAL;

    ret = razer_blackshark_set_args(device, RAZER_BS_CLASS_AUDIO,
                                    RAZER_BS_TONE_SET, 3,
    (const u8 []) {
        0x00, 0x03, level
    });
    if (ret)
        return ret;

    return count;
}

static ssize_t razer_attr_read_idle_time(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 args[RAZER_BS_MAX_ARGS];
    u8 args_len;
    unsigned int seconds;
    int ret;

    ret = razer_blackshark_get_args(device, RAZER_BS_CLASS_POWER,
                                    RAZER_BS_SLEEP_GET, args, &args_len);
    if (ret)
        return ret;
    if (args_len < 2)
        return -EPROTO;

    seconds = (args[0] << 8) | args[1];

    /*
     * An unlinked headset makes the dongle answer with a well-formed,
     * correctly checksummed reply whose arguments are all 0xFF. Every
     * other attribute rejects that via its own range check; this one has
     * no natural upper bound, so reject the marker explicitly.
     */
    if (seconds == 0xFFFF)
        return -EPROTO;

    return sysfs_emit(buf, "%u\n", seconds);
}

static ssize_t razer_attr_write_idle_time(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    unsigned int seconds;
    int ret;

    if (kstrtouint(buf, 10, &seconds))
        return -EINVAL;
    if (seconds > 0xFFFF)
        return -EINVAL;

    ret = razer_blackshark_set_args(device, RAZER_BS_CLASS_POWER,
                                    RAZER_BS_SLEEP_SET, 2,
    (const u8 []) {
        seconds >> 8, seconds & 0xFF
    });
    if (ret)
        return ret;

    return count;
}

static ssize_t razer_attr_read_power_saving(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 enabled;
    int ret;

    ret = razer_blackshark_get_byte(device, RAZER_BS_CLASS_POWER,
                                    RAZER_BS_SAVE_GET, 1, &enabled);
    if (ret)
        return ret;
    if (enabled > 1)
        return -EPROTO;

    return sysfs_emit(buf, "%u\n", enabled);
}

static ssize_t razer_attr_write_power_saving(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 enabled;
    int ret;

    if (kstrtou8(buf, 10, &enabled))
        return -EINVAL;
    if (enabled > 1)
        return -EINVAL;

    ret = razer_blackshark_set_args(device, RAZER_BS_CLASS_POWER,
                                    RAZER_BS_SAVE_SET, 2,
    (const u8 []) {
        0x00, enabled
    });
    if (ret)
        return ret;

    return count;
}

static u8 razer_blackshark_encode_db(int db)
{
    if (db < 0)
        return 0x80 | -db;

    return db;
}

static int razer_blackshark_decode_db(u8 raw)
{
    if (raw & 0x80)
        return -(raw & 0x7F);

    return raw;
}

static ssize_t razer_attr_read_equalizer_preset(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 preset;
    int ret;

    ret = razer_blackshark_get_byte(device, RAZER_BS_CLASS_AUDIO,
                                    RAZER_BS_PRESET_GET, 3, &preset);
    if (ret)
        return ret;
    if (preset > 3)
        return -EPROTO;

    return sysfs_emit(buf, "%u\n", preset);
}

static ssize_t razer_attr_write_equalizer_preset(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 preset;
    int ret;

    if (kstrtou8(buf, 10, &preset))
        return -EINVAL;
    if (preset > 3)
        return -EINVAL;

    ret = razer_blackshark_set_args(device, RAZER_BS_CLASS_AUDIO,
                                    RAZER_BS_PRESET_SET, 4,
    (const u8 []) {
        0x00, 0x01, 0x00, preset
    });
    if (ret)
        return ret;

    return count;
}

static ssize_t razer_attr_read_equalizer(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 args[RAZER_BS_MAX_ARGS];
    u8 args_len;
    int bands[RAZER_BS_EQ_BANDS];
    int i;
    int ret;

    ret = razer_blackshark_get_args(device, RAZER_BS_CLASS_AUDIO,
                                    RAZER_BS_EQ_GET, args, &args_len);
    if (ret)
        return ret;
    if (args_len < 2 + RAZER_BS_EQ_BANDS)
        return -EPROTO;

    for (i = 0; i < RAZER_BS_EQ_BANDS; i++)
        bands[i] = razer_blackshark_decode_db(args[2 + i]);

    return sysfs_emit(buf, "%d %d %d %d %d %d %d %d %d %d\n",
                      bands[0], bands[1], bands[2], bands[3], bands[4],
                      bands[5], bands[6], bands[7], bands[8], bands[9]);
}

static ssize_t razer_attr_write_equalizer(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    int bands[RAZER_BS_EQ_BANDS];
    u8 args[2 + RAZER_BS_EQ_BANDS];
    int i;
    int ret;

    ret = sscanf(buf, "%d %d %d %d %d %d %d %d %d %d",
                 &bands[0], &bands[1], &bands[2], &bands[3], &bands[4],
                 &bands[5], &bands[6], &bands[7], &bands[8], &bands[9]);
    if (ret != RAZER_BS_EQ_BANDS)
        return -EINVAL;

    for (i = 0; i < RAZER_BS_EQ_BANDS; i++)
        if (bands[i] < -RAZER_BS_EQ_DB_MAX || bands[i] > RAZER_BS_EQ_DB_MAX)
            return -EINVAL;

    args[0] = 0x00;
    args[1] = 0x01;
    for (i = 0; i < RAZER_BS_EQ_BANDS; i++)
        args[2 + i] = razer_blackshark_encode_db(bands[i]);

    ret = razer_blackshark_set_args(device, RAZER_BS_CLASS_AUDIO,
                                    RAZER_BS_EQ_SET,
                                    2 + RAZER_BS_EQ_BANDS, args);
    if (ret)
        return ret;

    return count;
}

static ssize_t razer_attr_read_mic_noise_cancel(struct device *dev,
        struct device_attribute *attr,
        char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 enabled;
    int ret;

    ret = razer_blackshark_get_byte(device, RAZER_BS_CLASS_AUDIO,
                                    RAZER_BS_MICNC_GET, 1, &enabled);
    if (ret)
        return ret;
    if (enabled > 1)
        return -EPROTO;

    return sysfs_emit(buf, "%u\n", enabled);
}

static ssize_t razer_attr_write_mic_noise_cancel(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 enabled;
    int ret;

    if (kstrtou8(buf, 10, &enabled))
        return -EINVAL;
    if (enabled > 1)
        return -EINVAL;

    ret = razer_blackshark_set_args(device, RAZER_BS_CLASS_AUDIO,
                                    RAZER_BS_MICNC_SET, 2,
    (const u8 []) {
        0x00, enabled
    });
    if (ret)
        return ret;

    return count;
}

static DEVICE_ATTR(version, 0440, razer_attr_read_version, NULL);
static DEVICE_ATTR(device_type, 0440, razer_attr_read_device_type, NULL);
static DEVICE_ATTR(device_serial, 0440, razer_attr_read_device_serial, NULL);
static DEVICE_ATTR(firmware_version, 0440, razer_attr_read_firmware_version, NULL);
static DEVICE_ATTR(charge_level, 0440, razer_attr_read_charge_level, NULL);
static DEVICE_ATTR(charge_status, 0440, razer_attr_read_charge_status, NULL);
static DEVICE_ATTR(sidetone, 0660, razer_attr_read_sidetone, razer_attr_write_sidetone);
static DEVICE_ATTR(device_idle_time, 0660, razer_attr_read_idle_time, razer_attr_write_idle_time);
static DEVICE_ATTR(power_saving, 0660, razer_attr_read_power_saving, razer_attr_write_power_saving);
static DEVICE_ATTR(equalizer_preset, 0660, razer_attr_read_equalizer_preset, razer_attr_write_equalizer_preset);
static DEVICE_ATTR(equalizer, 0660, razer_attr_read_equalizer, razer_attr_write_equalizer);
static DEVICE_ATTR(mic_noise_cancel, 0660, razer_attr_read_mic_noise_cancel, razer_attr_write_mic_noise_cancel);

static void razer_blackshark_remove_files(struct hid_device *hdev)
{
    device_remove_file(&hdev->dev, &dev_attr_mic_noise_cancel);
    device_remove_file(&hdev->dev, &dev_attr_equalizer);
    device_remove_file(&hdev->dev, &dev_attr_equalizer_preset);
    device_remove_file(&hdev->dev, &dev_attr_power_saving);
    device_remove_file(&hdev->dev, &dev_attr_device_idle_time);
    device_remove_file(&hdev->dev, &dev_attr_sidetone);
    device_remove_file(&hdev->dev, &dev_attr_charge_status);
    device_remove_file(&hdev->dev, &dev_attr_charge_level);
    device_remove_file(&hdev->dev, &dev_attr_firmware_version);
    device_remove_file(&hdev->dev, &dev_attr_device_serial);
    device_remove_file(&hdev->dev, &dev_attr_device_type);
    device_remove_file(&hdev->dev, &dev_attr_version);
}

static int razer_blackshark_probe(struct hid_device *hdev,
                                  const struct hid_device_id *id)
{
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_blackshark_device *device;
    int ret;

    device = kzalloc(sizeof(*device), GFP_KERNEL);
    if (!device)
        return -ENOMEM;

    device->hdev = hdev;
    device->usb_pid = hdev->product;
    device->usb_interface_protocol = intf->cur_altsetting->desc.bInterfaceProtocol;
    mutex_init(&device->lock);
    hid_set_drvdata(hdev, device);

    ret = hid_parse(hdev);
    if (ret)
        goto free_device;

    ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (ret)
        goto free_device;

    ret = hid_hw_open(hdev);
    if (ret)
        goto stop_hardware;

    /*
     * Only the vendor interface speaks the command protocol; any boot
     * keyboard/mouse interface the device may expose is left alone.
     */
    if (device->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE) {
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_firmware_version);
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_level);
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_status);
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_sidetone);
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_idle_time);
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_power_saving);
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_equalizer_preset);
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_equalizer);
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mic_noise_cancel);
    }

    usb_disable_autosuspend(usb_dev);

    return 0;

exit_free:
    razer_blackshark_remove_files(hdev);
    hid_hw_close(hdev);
stop_hardware:
    hid_hw_stop(hdev);
free_device:
    hid_set_drvdata(hdev, NULL);
    mutex_destroy(&device->lock);
    kfree(device);
    return ret;
}

static void razer_blackshark_disconnect(struct hid_device *hdev)
{
    struct razer_blackshark_device *device = hid_get_drvdata(hdev);

    if (device->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE)
        razer_blackshark_remove_files(hdev);

    hid_hw_close(hdev);
    hid_hw_stop(hdev);
    hid_set_drvdata(hdev, NULL);
    mutex_destroy(&device->lock);
    kfree(device);
}

static const struct hid_device_id razer_blackshark_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_BLACKSHARK_V3_X_USB) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_BLACKSHARK_V3_X) },
    { 0 }
};

MODULE_DEVICE_TABLE(hid, razer_blackshark_devices);

static struct hid_driver razer_blackshark_driver = {
    .name = "razerblackshark",
    .id_table = razer_blackshark_devices,
    .probe = razer_blackshark_probe,
    .remove = razer_blackshark_disconnect,
};

module_hid_driver(razer_blackshark_driver);
