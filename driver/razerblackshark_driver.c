// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/delay.h>
#include <linux/hid.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include "razerblackshark_driver.h"
#include "razercommon.h"

#define DRIVER_DESC "Razer BlackShark V3 X USB Driver"

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

static void razer_blackshark_build_query(struct razer_blackshark_device *device,
                                         u8 command_id,
                                         u8 *report)
{
    memset(report, 0, RAZER_BLACKSHARK_REPORT_LEN);

    report[0] = RAZER_BLACKSHARK_REPORT_ID;
    report[2] = device->transaction_id++;
    report[6] = 0x02;
    report[7] = 0x07;
    report[8] = command_id;
    report[9] = 0x00;
    report[62] = razer_blackshark_crc(report);
}

static int razer_blackshark_read_value(struct razer_blackshark_device *device,
                                       u8 command_id,
                                       u8 *value)
{
    u8 *request;
    u8 *response;
    int ret;

    mutex_lock(&device->lock);

    request = kzalloc(RAZER_BLACKSHARK_REPORT_LEN, GFP_KERNEL);
    response = kzalloc(RAZER_BLACKSHARK_REPORT_LEN, GFP_KERNEL);
    if (!request || !response) {
        ret = -ENOMEM;
        goto out_free;
    }

    razer_blackshark_build_query(device, command_id, request);

    ret = hid_hw_raw_request(device->hdev,
                             RAZER_BLACKSHARK_REPORT_ID,
                             request,
                             RAZER_BLACKSHARK_REPORT_LEN,
                             HID_FEATURE_REPORT,
                             HID_REQ_SET_REPORT);
    if (ret < 0)
        goto out_unlock;

    fsleep(50000);

    response[0] = RAZER_BLACKSHARK_REPORT_ID;
    ret = hid_hw_raw_request(device->hdev,
                             RAZER_BLACKSHARK_REPORT_ID,
                             response,
                             RAZER_BLACKSHARK_REPORT_LEN,
                             HID_FEATURE_REPORT,
                             HID_REQ_GET_REPORT);
    if (ret < 0)
        goto out_unlock;

    if (ret < RAZER_BLACKSHARK_REPORT_LEN ||
        response[0] != RAZER_BLACKSHARK_REPORT_ID ||
        response[2] != request[2] || response[6] != 0x02 ||
        response[7] != 0x07 || response[8] != command_id ||
        response[62] != razer_blackshark_crc(response)) {
        hid_err(device->hdev,
                "invalid response: ret=%d id=%02x transaction=%02x/%02x size=%02x class=%02x command=%02x value=%02x crc=%02x/%02x\n",
                ret, response[0], response[2], request[2], response[6],
                response[7], response[8], response[10], response[62],
                razer_blackshark_crc(response));
        ret = -EPROTO;
        goto out_unlock;
    }

    *value = response[10];
    ret = 0;

out_unlock:
    kfree(response);
    kfree(request);
out_free:
    mutex_unlock(&device->lock);
    return ret;
}

static ssize_t razer_blackshark_read_version(struct device *dev,
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
    default:
        device_type = "Unknown Device";
    }

    return sysfs_emit(buf, "%s\n", device_type);
}

static ssize_t razer_blackshark_read_device_serial(struct device *dev,
                                                   struct device_attribute *attr,
                                                   char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    const char *serial = device->hdev->uniq;

    return sysfs_emit(buf, "%s%04X\n", serial ? serial : "BSV3X", device->usb_pid);
}

static ssize_t razer_blackshark_read_firmware_version(struct device *dev,
                                                      struct device_attribute *attr,
                                                      char *buf)
{
    return sysfs_emit(buf, "unknown\n");
}

static ssize_t razer_blackshark_read_charge_level(struct device *dev,
                                                   struct device_attribute *attr,
                                                   char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 level;
    int ret;

    ret = razer_blackshark_read_value(device, 0x80, &level);
    if (ret)
        return ret;
    if (level > 100)
        return -EPROTO;

    /* OpenRazer's existing get_battery DBus method expects 0..255. */
    return sysfs_emit(buf, "%u\n", DIV_ROUND_CLOSEST((unsigned int)level * 255, 100));
}

static ssize_t razer_blackshark_read_charge_status(struct device *dev,
                                                    struct device_attribute *attr,
                                                    char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 status;
    int ret;

    ret = razer_blackshark_read_value(device, 0x84, &status);
    if (ret)
        return ret;
    if (status > 1)
        return -EPROTO;

    return sysfs_emit(buf, "%u\n", status);
}

static DEVICE_ATTR(version, 0440, razer_blackshark_read_version, NULL);
static DEVICE_ATTR(device_type, 0440, razer_attr_read_device_type, NULL);
static DEVICE_ATTR(device_serial, 0440, razer_blackshark_read_device_serial, NULL);
static DEVICE_ATTR(firmware_version, 0440, razer_blackshark_read_firmware_version, NULL);
static DEVICE_ATTR(charge_level, 0440, razer_blackshark_read_charge_level, NULL);
static DEVICE_ATTR(charge_status, 0440, razer_blackshark_read_charge_status, NULL);

static void razer_blackshark_remove_files(struct hid_device *hdev)
{
    device_remove_file(&hdev->dev, &dev_attr_charge_level);
    device_remove_file(&hdev->dev, &dev_attr_charge_status);
    device_remove_file(&hdev->dev, &dev_attr_firmware_version);
    device_remove_file(&hdev->dev, &dev_attr_device_serial);
    device_remove_file(&hdev->dev, &dev_attr_device_type);
    device_remove_file(&hdev->dev, &dev_attr_version);
}

static int razer_blackshark_probe(struct hid_device *hdev,
                                  const struct hid_device_id *id)
{
    struct razer_blackshark_device *device;
    int ret;

    device = kzalloc(sizeof(*device), GFP_KERNEL);
    if (!device)
        return -ENOMEM;

    device->hdev = hdev;
    device->usb_pid = hdev->product;
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

    ret = -ENOMEM;
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);
    ret = -ENOMEM;
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);
    ret = -ENOMEM;
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);
    ret = -ENOMEM;
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_firmware_version);

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLACKSHARK_V3_X_USB:
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_level);
        ret = -ENOMEM;
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_status);
        break;
    }

    return 0;

exit_free:
    razer_blackshark_remove_files(hdev);
    hid_hw_close(hdev);
stop_hardware:
    hid_hw_stop(hdev);
free_device:
    hid_set_drvdata(hdev, NULL);
    kfree(device);
    return ret;
}

static void razer_blackshark_disconnect(struct hid_device *hdev)
{
    struct razer_blackshark_device *device = hid_get_drvdata(hdev);

    razer_blackshark_remove_files(hdev);
    hid_hw_close(hdev);
    hid_hw_stop(hdev);
    hid_set_drvdata(hdev, NULL);
    kfree(device);
}

static const struct hid_device_id razer_blackshark_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_BLACKSHARK_V3_X_USB) },
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
