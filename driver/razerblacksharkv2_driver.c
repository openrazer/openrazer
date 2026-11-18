// SPDX-License-Identifier: GPL-2.0-or-later
/* MediaTek-based BlackShark V2 HyperSpeed. USB audio stays with snd_usb_audio. */
#include <linux/completion.h>
#include <linux/ctype.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "razercommon.h"
#include "razerblacksharkv2_driver.h"
#include "razerblacksharkv2_protocol.h"

struct razer_blacksharkv2_device {
    struct hid_device *hdev;
    struct mutex lock;
    spinlock_t reply_lock;
    struct completion received;
    unsigned char sequence;
    unsigned char command;
    unsigned char domain;
    unsigned char pending_domain;
    unsigned char reply[BLACKSHARK_REPORT_SIZE];
    unsigned int files_created;
    bool pending;
    bool stopped;
};

enum blackshark_format { VALUE, BOOLEAN, CAPACITY, SECONDS, SERIAL, FIRMWARE, VERSION, NAME, BANDS };

struct blackshark_attribute {
    struct device_attribute attribute;
    unsigned char command;
    unsigned int maximum;
    enum blackshark_format format;
    bool local;
};

/* The transaction mutex covers complete operations, including the EQ latch sequence.
 * raw_event only takes reply_lock: it must never block on the sending thread.
 */
static int blackshark_exchange(struct razer_blacksharkv2_device *device, unsigned char domain,
                               unsigned char command, const unsigned char *payload,
                               unsigned int count, unsigned char *response)
{
    unsigned char *request;
    unsigned long flags;
    int ret;

    lockdep_assert_held(&device->lock);
    if (count > BLACKSHARK_MAX_PAYLOAD)
        return -EINVAL;
    request = kmalloc(BLACKSHARK_REPORT_SIZE, GFP_KERNEL);
    if (!request)
        return -ENOMEM;

    spin_lock_irqsave(&device->reply_lock, flags);
    if (device->stopped) {
        spin_unlock_irqrestore(&device->reply_lock, flags);
        kfree(request);
        return -ENODEV;
    }
    reinit_completion(&device->received);
    device->sequence = (device->sequence + 1) & 0x1F;
    device->command = command;
    device->pending_domain = domain;
    device->pending = true;
    blackshark_prepare(request, device->sequence, domain, command, payload, count);
    spin_unlock_irqrestore(&device->reply_lock, flags);

    ret = hid_hw_output_report(device->hdev, request, BLACKSHARK_REPORT_SIZE);
    if (ret == -ENOSYS)
        ret = hid_hw_raw_request(device->hdev, 0x02, request, BLACKSHARK_REPORT_SIZE,
                                 HID_OUTPUT_REPORT, HID_REQ_SET_REPORT);
    kfree(request);
    if (ret >= 0 && ret != BLACKSHARK_REPORT_SIZE)
        ret = -EIO;
    if (ret >= 0 && !wait_for_completion_timeout(&device->received, msecs_to_jiffies(800)))
        ret = -ETIMEDOUT;

    spin_lock_irqsave(&device->reply_lock, flags);
    device->pending = false;
    if (device->stopped)
        ret = -ENODEV;
    else if (ret >= 0) {
        ret = device->reply[11] == 1 ? device->reply[12] : -EIO;
        if (device->reply[1] == BLACKSHARK_STATUS_UNSUPPORTED)
            ret = -EOPNOTSUPP;
        else if (device->reply[1] == 0x01)
            ret = -EBUSY;
        else if (device->reply[1] == 0x04)
            ret = -ETIMEDOUT;
        else if (device->reply[1] != BLACKSHARK_STATUS_OK)
            ret = -EIO;
        else if (command & 0x80)
            ret = blackshark_set_succeeded(device->reply) ? 0 : -EIO;
        if (ret > 0 && response)
            memcpy(response, device->reply + 13, ret);
    }
    spin_unlock_irqrestore(&device->reply_lock, flags);
    return ret;
}

static int blackshark_set(struct razer_blacksharkv2_device *device, unsigned char domain,
                          unsigned char command, unsigned char value)
{
    return blackshark_exchange(device, domain, command | 0x80, &value, 1, NULL);
}

static ssize_t razer_attr_read_device_type(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct hid_device *hdev = to_hid_device(dev);
    const char *device_type;

    switch (hdev->product) {
    case USB_DEVICE_ID_RAZER_BLACKSHARK_V2_HYPERSPEED_WIRELESS:
        device_type = "Razer BlackShark V2 HyperSpeed (Wireless)";
        break;
    case USB_DEVICE_ID_RAZER_BLACKSHARK_V2_HYPERSPEED_WIRED:
        device_type = "Razer BlackShark V2 HyperSpeed (Wired)";
        break;
    default:
        return -ENODEV;
    }
    return sysfs_emit(buf, "%s\n", device_type);
}

static ssize_t blackshark_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blacksharkv2_device *device = dev_get_drvdata(dev);
    struct blackshark_attribute *field = container_of(attr, struct blackshark_attribute, attribute);
    unsigned char response[BLACKSHARK_MAX_PAYLOAD] = {0};
    unsigned int i;
    int ret, used = 0;

    if (field->format == VERSION)
        return sysfs_emit(buf, "%s\n", DRIVER_VERSION);
    if (field->format == NAME)
        return razer_attr_read_device_type(dev, attr, buf);

    mutex_lock(&device->lock);
    ret = blackshark_exchange(device, field->local ? 0 : device->domain, field->command, NULL, 0, response);
    mutex_unlock(&device->lock);
    if (ret < 0)
        return ret;
    if (!ret)
        return -ENODATA;

    switch (field->format) {
    case SERIAL:
        for (i = 0; i < ret && response[i]; i++)
            if (!isprint(response[i]))
                return -EIO;
        return sysfs_emit(buf, "%.*s\n", ret, response);
    case FIRMWARE:
        if (ret < 4)
            return -EIO;
        return sysfs_emit(buf, "v%u.%u.%u.%u\n", response[0], response[1], response[2], response[3]);
    case BANDS:
        if (ret != 10)
            return -EIO;
        for (i = 0; i < 10; i++) {
            int gain = (signed char)response[i];
            if (gain < -9 || gain > 6)
                return -ERANGE;
            used += sysfs_emit_at(buf, used, "%d%s", gain, i == 9 ? "\n" : " ");
        }
        return used;
    case BOOLEAN:
        return sysfs_emit(buf, "%u\n", response[0] != 0);
    case CAPACITY:
        if (response[0] > 100)
            return -ERANGE;
        /* The existing daemon ABI is 0..255 and truncates back to percent. */
        return sysfs_emit(buf, "%u\n", DIV_ROUND_UP(response[0] * 255, 100));
    case SECONDS:
        return sysfs_emit(buf, "%u\n", response[0] * 60);
    default:
        if (response[0] > field->maximum)
            return -ERANGE;
        return sysfs_emit(buf, "%u\n", response[0]);
    }
}

static int blackshark_parse_bands(const char *buf, size_t count, unsigned char bands[10])
{
    char *copy, *cursor, *token;
    unsigned int i = 0;
    int gain, ret = 0;

    copy = kstrndup(buf, count, GFP_KERNEL);
    if (!copy)
        return -ENOMEM;
    cursor = copy;
    while ((token = strsep(&cursor, " \t\n"))) {
        if (!*token)
            continue;
        if (i == 10) {
            ret = -EINVAL;
            break;
        }
        ret = kstrtoint(token, 10, &gain);
        if (ret)
            break;
        if (gain < -9 || gain > 6) {
            ret = -ERANGE;
            break;
        }
        bands[i++] = gain + 5;
    }
    kfree(copy);
    return ret ? ret : i == 10 ? 0 : -EINVAL;
}

static ssize_t blackshark_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_blacksharkv2_device *device = dev_get_drvdata(dev);
    struct blackshark_attribute *field = container_of(attr, struct blackshark_attribute, attribute);
    unsigned char domain = field->local ? 0 : device->domain;
    unsigned char bands[10];
    unsigned int value;
    int ret;

    if (memchr(buf, '\0', count))
        return -EINVAL;
    if (field->format == BANDS) {
        ret = blackshark_parse_bands(buf, count, bands);
        if (ret)
            return ret;
    } else {
        ret = kstrtouint(buf, 10, &value);
        if (ret)
            return ret;
        if (field->format == SECONDS) {
            if (value % 60)
                return -EINVAL;
            value /= 60;
        }
        if (value > field->maximum)
            return -ERANGE;
        if (field->command == 0x13 && value != 0 && value != 7 && value != 8 && value != 9 && value != 255)
            return -EINVAL;
    }

    mutex_lock(&device->lock);
    if (field->format == BANDS) {
        ret = blackshark_set(device, domain, 0x1E, 1);
        if (ret >= 0)
            ret = blackshark_set(device, domain, 0x13, 255);
        if (ret >= 0)
            ret = blackshark_exchange(device, domain, 0x95, bands, 10, NULL);
        if (ret >= 0) {
            msleep(40);
            ret = blackshark_set(device, domain, 0x13, 255);
        }
    } else {
        ret = blackshark_set(device, domain, field->command, value);
        if (ret >= 0 && field->command == 0x13)
            ret = blackshark_set(device, domain, 0x1E, 1);
    }
    mutex_unlock(&device->lock);
    return ret < 0 ? ret : count;
}

#define BLACKSHARK_ATTR(_name, _mode, _command, _maximum, _format, _local) \
    static struct blackshark_attribute dev_attr_##_name = { \
        .attribute = __ATTR(_name, _mode, blackshark_read, (_mode & 0220) ? blackshark_write : NULL), \
        .command = _command, .maximum = _maximum, .format = _format, .local = _local, \
    }

BLACKSHARK_ATTR(version, 0440, 0, 0, VERSION, true);
BLACKSHARK_ATTR(device_type, 0440, 0, 0, NAME, true);
BLACKSHARK_ATTR(device_serial, 0440, 0x00, 0, SERIAL, true);
BLACKSHARK_ATTR(firmware_version, 0440, 0x02, 0, FIRMWARE, true);
BLACKSHARK_ATTR(headset_serial, 0440, 0x00, 0, SERIAL, false);
BLACKSHARK_ATTR(headset_firmware_version, 0440, 0x02, 0, FIRMWARE, false);
BLACKSHARK_ATTR(charge_level, 0440, 0x21, 100, CAPACITY, false);
BLACKSHARK_ATTR(charge_status, 0440, 0x2A, 1, BOOLEAN, false);
BLACKSHARK_ATTR(device_idle_time, 0660, 0x2C, 255, SECONDS, false);
BLACKSHARK_ATTR(sidetone_enabled, 0660, 0x18, 1, BOOLEAN, false);
BLACKSHARK_ATTR(sidetone_level, 0660, 0x19, 15, VALUE, false);
BLACKSHARK_ATTR(do_not_disturb, 0660, 0x27, 1, BOOLEAN, false);
BLACKSHARK_ATTR(microphone_muted, 0440, 0x55, 1, BOOLEAN, false);
BLACKSHARK_ATTR(equalizer_enabled, 0660, 0x1E, 1, BOOLEAN, false);
BLACKSHARK_ATTR(equalizer_preset, 0660, 0x13, 255, VALUE, false);
BLACKSHARK_ATTR(equalizer_bands, 0660, 0x15, 0, BANDS, false);
BLACKSHARK_ATTR(wireless_connected, 0440, 0x20, 1, BOOLEAN, true);
BLACKSHARK_ATTR(dongle_led_mode, 0660, 0x66, 3, VALUE, true);

static struct blackshark_attribute *blackshark_attributes[] = {
    &dev_attr_version, &dev_attr_device_type, &dev_attr_device_serial, &dev_attr_firmware_version,
    &dev_attr_headset_serial, &dev_attr_headset_firmware_version, &dev_attr_charge_level, &dev_attr_charge_status,
    &dev_attr_device_idle_time, &dev_attr_sidetone_enabled, &dev_attr_sidetone_level, &dev_attr_do_not_disturb,
    &dev_attr_microphone_muted, &dev_attr_equalizer_enabled, &dev_attr_equalizer_preset,
    &dev_attr_equalizer_bands, &dev_attr_wireless_connected, &dev_attr_dongle_led_mode,
};

static void blackshark_remove_files(struct razer_blacksharkv2_device *device)
{
    while (device->files_created)
        device_remove_file(&device->hdev->dev, &blackshark_attributes[--device->files_created]->attribute);
}

/* Keep the explicit file creation list consumable by generate_fake_driver.sh. */
#undef CREATE_DEVICE_FILE
#define CREATE_DEVICE_FILE(_dev, _attr) do { \
    ret = device_create_file(_dev, &(_attr)->attribute); \
    if (ret) \
        goto exit_files; \
    device->files_created++; \
} while (0)

static int razer_blacksharkv2_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    struct razer_blacksharkv2_device *device;
    unsigned int i;
    bool vendor_interface = false;
    int ret;

    ret = hid_parse(hdev);
    if (ret)
        return ret;
    for (i = 0; i < hdev->maxcollection; i++)
        if (hdev->collection[i].usage == 0xFF140001)
            vendor_interface = true;
    if (!vendor_interface)
        return -ENODEV;

    device = devm_kzalloc(&hdev->dev, sizeof(*device), GFP_KERNEL);
    if (!device)
        return -ENOMEM;
    device->hdev = hdev;
    device->domain = hdev->product == USB_DEVICE_ID_RAZER_BLACKSHARK_V2_HYPERSPEED_WIRELESS ? 0x80 : 0;
    mutex_init(&device->lock);
    spin_lock_init(&device->reply_lock);
    init_completion(&device->received);
    hid_set_drvdata(hdev, device);

    ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (ret)
        return ret;
    ret = hid_hw_open(hdev);
    if (ret)
        goto exit_stop;

    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_firmware_version);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_headset_serial);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_headset_firmware_version);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_level);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_status);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_idle_time);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_sidetone_enabled);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_sidetone_level);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_do_not_disturb);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_microphone_muted);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_equalizer_enabled);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_equalizer_preset);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_equalizer_bands);
    CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_wireless_connected);
    switch (hdev->product) {
    case USB_DEVICE_ID_RAZER_BLACKSHARK_V2_HYPERSPEED_WIRELESS:
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dongle_led_mode);
        break;
    case USB_DEVICE_ID_RAZER_BLACKSHARK_V2_HYPERSPEED_WIRED:
        break;
    }
    return 0;

exit_files:
    blackshark_remove_files(device);
    hid_hw_close(hdev);
exit_stop:
    hid_hw_stop(hdev);
    return ret;
}

static void razer_blacksharkv2_remove(struct hid_device *hdev)
{
    struct razer_blacksharkv2_device *device = hid_get_drvdata(hdev);
    unsigned long flags;

    spin_lock_irqsave(&device->reply_lock, flags);
    device->stopped = true;
    complete_all(&device->received);
    spin_unlock_irqrestore(&device->reply_lock, flags);
    blackshark_remove_files(device);
    hid_hw_close(hdev);
    hid_hw_stop(hdev);
}

static int razer_blacksharkv2_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size)
{
    struct razer_blacksharkv2_device *device = hid_get_drvdata(hdev);
    unsigned long flags;

    if (!blackshark_reply_valid(data, size))
        return 0;
    spin_lock_irqsave(&device->reply_lock, flags);
    if (device->pending && !device->stopped && blackshark_reply_matches(data, size, device->sequence,
            device->pending_domain, device->command, device->domain == 0)) {
        memcpy(device->reply, data, BLACKSHARK_REPORT_SIZE);
        device->pending = false;
        complete(&device->received);
    }
    spin_unlock_irqrestore(&device->reply_lock, flags);
    return 0;
}

static const struct hid_device_id razer_blacksharkv2_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_BLACKSHARK_V2_HYPERSPEED_WIRELESS) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_BLACKSHARK_V2_HYPERSPEED_WIRED) },
    { }
};
MODULE_DEVICE_TABLE(hid, razer_blacksharkv2_devices);

static struct hid_driver razer_blacksharkv2_driver = {
    .name = "razerblacksharkv2",
    .id_table = razer_blacksharkv2_devices,
    .probe = razer_blacksharkv2_probe,
    .remove = razer_blacksharkv2_remove,
    .raw_event = razer_blacksharkv2_raw_event,
};
module_hid_driver(razer_blacksharkv2_driver);

MODULE_DESCRIPTION("Razer BlackShark V2 HyperSpeed headset controls");
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");
