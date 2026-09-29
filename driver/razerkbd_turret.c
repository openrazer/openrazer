// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Turret receiver: mouse on interface 0, keyboard on 1, input only on 2.
 * Both roles share the wIndex 0 request slot and one exchange/frame lock.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/hid.h>
#include <linux/usb/input.h>
#include <linux/version.h>

#include "razerkbd_turret.h"
#include "razerkbd_driver.h"
#include "razercommon.h"
#include "razerchromacommon.h"

#define RAZER_TURRET_RECEIVER_MOUSE_INTERFACE 0
#define RAZER_TURRET_RECEIVER_KEYBOARD_INTERFACE 1
#define RAZER_TURRET_RECEIVER_REPORT_INDEX 0

#define RAZER_TURRET_KEYBOARD_ROW_LEN 18
#define RAZER_TURRET_KEYBOARD_ROWS_NUM 6

/* The receiver relays each request to its device before replying. BUSY
 * replies are read again, and the request is sent once more after 10 of them. */
#define RAZER_TURRET_RECEIVER_WAIT_US 30000
#define RAZER_TURRET_RECEIVER_BUSY_WAIT_US 10000
#define RAZER_TURRET_RECEIVER_BUSY_READS 10
#define RAZER_TURRET_RECEIVER_SEND_ATTEMPTS 2
#define RAZER_TURRET_RECEIVER_ROW_WAIT_US 5000

#define RAZER_TURRET_RECEIVER_SERIAL_LEN 22
#define RAZER_TURRET_RECEIVER_DPI_MIN 100
#define RAZER_TURRET_RECEIVER_DPI_MAX 16000
#define RAZER_TURRET_RECEIVER_DPI_STAGES_MAX 5
#define RAZER_TURRET_RECEIVER_DPI_STAGES_GET_SIZE 0x50
#define RAZER_TURRET_RECEIVER_ROW_SIZE (3 + RAZER_TURRET_KEYBOARD_ROW_LEN * 3)

/* Shared by all interfaces of one receiver, protected by the list lock */
struct razer_turret_receiver {
    struct list_head node;
    struct usb_device *usb_dev;
    unsigned int refcount;
    struct mutex lock;
    unsigned char keyboard_tx;
    unsigned char mouse_tx;
};

struct razer_turret_role {
    struct hid_device *hdev;
    struct razer_turret_receiver *receiver;
    bool keyboard;
};

static LIST_HEAD(razer_turret_receivers);
static DEFINE_MUTEX(razer_turret_receivers_lock);

static struct razer_turret_receiver *razer_turret_receiver_get(struct usb_device *usb_dev)
{
    struct razer_turret_receiver *receiver;

    mutex_lock(&razer_turret_receivers_lock);
    list_for_each_entry(receiver, &razer_turret_receivers, node) {
        if (receiver->usb_dev == usb_dev) {
            receiver->refcount++;
            goto out;
        }
    }

    receiver = kzalloc_obj(*receiver);
    if (receiver) {
        receiver->usb_dev = usb_dev;
        receiver->refcount = 1;
        mutex_init(&receiver->lock);
        // The first IDs sent are 0xF0 and 0x01
        receiver->keyboard_tx = 0xFF;
        receiver->mouse_tx = 0x1F;
        list_add(&receiver->node, &razer_turret_receivers);
    }
out:
    mutex_unlock(&razer_turret_receivers_lock);
    return receiver;
}

static void razer_turret_receiver_put(struct razer_turret_receiver *receiver)
{
    mutex_lock(&razer_turret_receivers_lock);
    if (--receiver->refcount == 0) {
        list_del(&receiver->node);
        mutex_destroy(&receiver->lock);
        kfree(receiver);
    }
    mutex_unlock(&razer_turret_receivers_lock);
}

/* Header only, arguments can hold the serial number */
static void razer_turret_log(struct razer_turret_role *role, struct razer_report *report, const char *message)
{
    hid_warn(role->hdev, "razerkbd: Turret %s: %s. status: %02x transaction_id.id: %02x remaining_packets: %02x protocol_type: %02x data_size: %02x, command_class: %02x, command_id.id: %02x\n",
             role->keyboard ? "keyboard" : "mouse", message, report->status, report->transaction_id.id,
             be16_to_cpu(report->remaining_packets), report->protocol_type, report->data_size,
             report->command_class, report->command_id.id);
}

/**
 * Caller holds receiver->lock. Skip mouse ID 0 to reject empty replies.
 */
static unsigned char razer_turret_next_tx(struct razer_turret_role *role)
{
    struct razer_turret_receiver *receiver = role->receiver;

    if (role->keyboard) {
        receiver->keyboard_tx = 0xF0 | ((receiver->keyboard_tx + 1) & 0x0F);
        return receiver->keyboard_tx;
    }

    if (receiver->mouse_tx >= 0x01 && receiver->mouse_tx < 0x1F)
        receiver->mouse_tx++;
    else
        receiver->mouse_tx = 0x01;
    return receiver->mouse_tx;
}

static int razer_turret_get_reply(struct razer_turret_role *role, struct razer_report *response)
{
    int err;

    err = usb_control_msg_recv(hid_to_usb_dev(role->hdev),
                               0,
                               HID_REQ_GET_REPORT,
                               USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN,
                               0x300,
                               RAZER_TURRET_RECEIVER_REPORT_INDEX,
                               response,
                               sizeof(*response),
                               USB_CTRL_SET_TIMEOUT,
                               GFP_KERNEL);
    if (err)
        hid_warn(role->hdev, "razerkbd: Turret failed to receive USB control message: %d\n", err);

    return err;
}

static bool razer_turret_reply_matches(struct razer_report *request, struct razer_report *response)
{
    return response->transaction_id.id == request->transaction_id.id &&
           response->remaining_packets == request->remaining_packets &&
           response->protocol_type == request->protocol_type &&
           response->command_class == request->command_class &&
           response->command_id.id == request->command_id.id;
}

static int razer_turret_check_reply(struct razer_turret_role *role, struct razer_report *response, unsigned char min_size)
{
    switch (response->status) {
    case RAZER_CMD_SUCCESSFUL:
        break;
    case RAZER_CMD_FAILURE:
        razer_turret_log(role, response, "Command failed");
        return -EINVAL;
    case RAZER_CMD_NOT_SUPPORTED:
        razer_turret_log(role, response, "Command not supported");
        return -ENOTSUPP;
    case RAZER_CMD_TIMEOUT:
        razer_turret_log(role, response, "Command timed out");
        return -ETIMEDOUT;
    default:
        razer_turret_log(role, response, "Unknown response status");
        return -EIO;
    }

    if (response->crc != razer_calculate_crc(response)) {
        razer_turret_log(role, response, "Invalid checksum");
        return -EIO;
    }

    if (response->data_size > ARRAY_SIZE(response->arguments) || response->data_size < min_size) {
        razer_turret_log(role, response, "Invalid data size");
        return -EIO;
    }

    return 0;
}

/**
 * Caller holds receiver->lock. Poll BUSY without SET; BUSY replies may have
 * invalid CRCs. One resend is safe for these idempotent commands.
 */
static int razer_turret_exchange_locked(struct razer_turret_role *role, struct razer_report *request, struct razer_report *response, unsigned char min_size)
{
    unsigned int attempt, busy;
    int err = -EIO;

    request->transaction_id.id = razer_turret_next_tx(role);
    request->crc = razer_calculate_crc(request);

    for (attempt = 0; attempt < RAZER_TURRET_RECEIVER_SEND_ATTEMPTS; attempt++) {
        err = razer_send_control_msg(role->hdev, request, sizeof(*request), RAZER_TURRET_RECEIVER_REPORT_INDEX, RAZER_TURRET_RECEIVER_WAIT_US);
        if (err)
            return err;

        for (busy = 1; ; busy++) {
            err = razer_turret_get_reply(role, response);
            if (err)
                return err;

            if (response->status != RAZER_CMD_BUSY || busy == RAZER_TURRET_RECEIVER_BUSY_READS)
                break;

            fsleep(RAZER_TURRET_RECEIVER_BUSY_WAIT_US);
        }

        if (response->status == RAZER_CMD_BUSY) {
            razer_turret_log(role, response, "Still busy");
            err = -EBUSY;
            continue;
        }

        if (!razer_turret_reply_matches(request, response)) {
            razer_turret_log(role, response, "Response doesn't match request");
            err = -EIO;
            continue;
        }

        return razer_turret_check_reply(role, response, min_size);
    }

    return err;
}

static int razer_turret_exchange(struct razer_turret_role *role, struct razer_report *request, struct razer_report *response, unsigned char min_size)
{
    int err;

    mutex_lock(&role->receiver->lock);
    err = razer_turret_exchange_locked(role, request, response, min_size);
    mutex_unlock(&role->receiver->lock);

    return err;
}

/**
 * SET-only rows; caller holds receiver->lock.
 */
static int razer_turret_send_row_locked(struct razer_turret_role *role, struct razer_report *request)
{
    request->transaction_id.id = razer_turret_next_tx(role);
    request->crc = razer_calculate_crc(request);

    return razer_send_control_msg(role->hdev, request, sizeof(*request), RAZER_TURRET_RECEIVER_REPORT_INDEX, RAZER_TURRET_RECEIVER_ROW_WAIT_US);
}

static ssize_t razer_turret_send_effect(struct device *dev, struct razer_report *request, size_t count)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report response = {0};
    int err;

    err = razer_turret_exchange(role, request, &response, request->data_size);
    if (err)
        return err;

    return count;
}

static struct razer_report razer_turret_custom_effect_request(void)
{
    struct razer_report request = razer_chroma_extended_matrix_effect_custom_frame();

    request.data_size = 0x06;
    return request;
}

static ssize_t razer_attr_read_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%s\n", DRIVER_VERSION);
}

static ssize_t razer_attr_read_device_type(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);

    return sysfs_emit(buf, "%s\n", role->keyboard ? "Razer Turret Keyboard for Xbox One (Wireless)" : "Razer Turret Mouse for Xbox One (Wireless)");
}

/* Each role reports the serial of its device, the same as when wired */
static ssize_t razer_attr_read_device_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request = razer_chroma_standard_get_serial();
    struct razer_report response = {0};
    char serial_string[RAZER_TURRET_RECEIVER_SERIAL_LEN + 1];
    int err;

    err = razer_turret_exchange(role, &request, &response, RAZER_TURRET_RECEIVER_SERIAL_LEN);
    if (err)
        return err;

    memcpy(serial_string, response.arguments, RAZER_TURRET_RECEIVER_SERIAL_LEN);
    serial_string[RAZER_TURRET_RECEIVER_SERIAL_LEN] = '\0';

    return sysfs_emit(buf, "%s\n", serial_string);
}

static ssize_t razer_attr_read_firmware_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request = razer_chroma_standard_get_firmware_version();
    struct razer_report response = {0};
    int err;

    err = razer_turret_exchange(role, &request, &response, 2);
    if (err)
        return err;

    return sysfs_emit(buf, "v%d.%d\n", response.arguments[0], response.arguments[1]);
}

/* Receiver keyboard replies have arguments[0] = 0x01, the value is in arguments[1] */
static ssize_t razer_attr_read_charge_level(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request = razer_chroma_misc_get_battery_level();
    struct razer_report response = {0};
    int err;

    err = razer_turret_exchange(role, &request, &response, 2);
    if (err)
        return err;

    return sysfs_emit(buf, "%d\n", response.arguments[1]);
}

static ssize_t razer_attr_read_charge_status(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request = razer_chroma_misc_get_charging_status();
    struct razer_report response = {0};
    int err;

    err = razer_turret_exchange(role, &request, &response, 2);
    if (err)
        return err;

    return sysfs_emit(buf, "%d\n", response.arguments[1]);
}

static ssize_t razer_attr_write_matrix_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request;
    struct razer_report response = {0};
    unsigned char brightness;
    int err;

    err = kstrtou8(buf, 0, &brightness);
    if (err < 0)
        return err;

    request = razer_chroma_extended_matrix_brightness(VARSTORE, ZERO_LED, brightness);
    err = razer_turret_exchange(role, &request, &response, 0);
    if (err)
        return err;

    return count;
}

/* Like the wired mouse, the mouse is set with ZERO_LED but read with LED ID 0x01 */
static ssize_t razer_attr_read_matrix_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request;
    struct razer_report response = {0};
    int err;

    request = razer_chroma_extended_matrix_get_brightness(VARSTORE, role->keyboard ? ZERO_LED : SCROLL_WHEEL_LED);
    err = razer_turret_exchange(role, &request, &response, 3);
    if (err)
        return err;

    return sysfs_emit(buf, "%d\n", response.arguments[2]);
}

static ssize_t razer_attr_write_matrix_effect_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_report request = razer_chroma_extended_matrix_effect_none(NOSTORE, ZERO_LED);

    return razer_turret_send_effect(dev, &request, count);
}

static ssize_t razer_attr_write_matrix_effect_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_report request = razer_chroma_extended_matrix_effect_spectrum(NOSTORE, ZERO_LED);

    return razer_turret_send_effect(dev, &request, count);
}

static ssize_t razer_attr_write_matrix_effect_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_report request;

    if (count != 3) {
        dev_warn(dev, "razerkbd: Static mode only accepts RGB (3byte)\n");
        return -EINVAL;
    }

    request = razer_chroma_extended_matrix_effect_static(NOSTORE, ZERO_LED, (struct razer_rgb*)&buf[0]);
    return razer_turret_send_effect(dev, &request, count);
}

static ssize_t razer_attr_write_matrix_effect_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_report request;

    switch (count) {
    case 1: // "Random" colour mode
        request = razer_chroma_extended_matrix_effect_breathing_random(NOSTORE, ZERO_LED);
        break;

    case 3: // Single colour mode
        request = razer_chroma_extended_matrix_effect_breathing_single(NOSTORE, ZERO_LED, (struct razer_rgb*)&buf[0]);
        break;

    case 6: // Dual colour mode
        request = razer_chroma_extended_matrix_effect_breathing_dual(NOSTORE, ZERO_LED, (struct razer_rgb*)&buf[0], (struct razer_rgb*)&buf[3]);
        break;

    default:
        dev_warn(dev, "razerkbd: Breathing only accepts '1' (1byte). RGB (3byte). RGB, RGB (6byte)\n");
        return -EINVAL;
    }

    return razer_turret_send_effect(dev, &request, count);
}

/* The shared encoders clamp speeds, so out of range values are rejected here */
static ssize_t razer_attr_write_matrix_effect_reactive(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_report request;
    unsigned char speed;

    if (count != 4) {
        dev_warn(dev, "razerkbd: Reactive only accepts Speed, RGB (4byte)\n");
        return -EINVAL;
    }

    speed = (unsigned char)buf[0];
    if (speed < 1 || speed > 4) {
        dev_warn(dev, "razerkbd: Reactive speed must be between 1 and 4\n");
        return -EINVAL;
    }

    request = razer_chroma_extended_matrix_effect_reactive(NOSTORE, ZERO_LED, speed, (struct razer_rgb*)&buf[1]);
    return razer_turret_send_effect(dev, &request, count);
}

static ssize_t razer_attr_write_matrix_effect_wave(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_report request;
    unsigned char direction;
    int err;

    err = kstrtou8(buf, 0, &direction);
    if (err < 0)
        return err;

    if (direction != 1 && direction != 2) {
        dev_warn(dev, "razerkbd: Wave direction must be 1 or 2\n");
        return -EINVAL;
    }

    request = razer_chroma_extended_matrix_effect_wave(NOSTORE, ZERO_LED, direction);
    return razer_turret_send_effect(dev, &request, count);
}

static ssize_t razer_attr_write_matrix_effect_starlight(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_report request;
    unsigned char speed;

    if (count != 1 && count != 4 && count != 7) {
        dev_warn(dev, "razerkbd: Starlight only accepts Speed (1byte). Speed, RGB (4byte). Speed, RGB, RGB (7byte)\n");
        return -EINVAL;
    }

    speed = (unsigned char)buf[0];
    if (speed < 1 || speed > 3) {
        dev_warn(dev, "razerkbd: Starlight speed must be between 1 and 3\n");
        return -EINVAL;
    }

    if (count == 7)
        request = razer_chroma_extended_matrix_effect_starlight_dual(NOSTORE, ZERO_LED, speed, (struct razer_rgb*)&buf[1], (struct razer_rgb*)&buf[4]);
    else if (count == 4)
        request = razer_chroma_extended_matrix_effect_starlight_single(NOSTORE, ZERO_LED, speed, (struct razer_rgb*)&buf[1]);
    else
        request = razer_chroma_extended_matrix_effect_starlight_random(NOSTORE, ZERO_LED, speed);

    return razer_turret_send_effect(dev, &request, count);
}

static ssize_t razer_attr_write_matrix_effect_custom(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_report request = razer_turret_custom_effect_request();

    return razer_turret_send_effect(dev, &request, count);
}

/**
 * Write device file "matrix_custom_frame"
 *
 * Only whole rows (ROW_ID, 0, 17, 18 * RGB) are accepted. The custom effect
 * is activated before the rows of every frame, all under the receiver lock.
 */
static ssize_t razer_attr_write_matrix_custom_frame(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request;
    struct razer_report response = {0};
    size_t offset;
    int err;

    if (count == 0 || count % RAZER_TURRET_RECEIVER_ROW_SIZE != 0) {
        dev_warn(dev, "razerkbd: Custom frame only accepts whole rows: ROW_ID, 0, 17, 18 * RGB\n");
        return -EINVAL;
    }

    for (offset = 0; offset < count; offset += RAZER_TURRET_RECEIVER_ROW_SIZE) {
        if ((unsigned char)buf[offset] >= RAZER_TURRET_KEYBOARD_ROWS_NUM ||
            buf[offset + 1] != 0 ||
            (unsigned char)buf[offset + 2] != RAZER_TURRET_KEYBOARD_ROW_LEN - 1) {
            dev_warn(dev, "razerkbd: Custom frame only accepts whole rows: ROW_ID, 0, 17, 18 * RGB\n");
            return -EINVAL;
        }
    }

    mutex_lock(&role->receiver->lock);

    request = razer_turret_custom_effect_request();
    err = razer_turret_exchange_locked(role, &request, &response, request.data_size);

    for (offset = 0; !err && offset < count; offset += RAZER_TURRET_RECEIVER_ROW_SIZE) {
        request = razer_chroma_extended_matrix_set_custom_frame2((unsigned char)buf[offset], 0, RAZER_TURRET_KEYBOARD_ROW_LEN - 1, (unsigned char*)&buf[offset + 3], 0);
        err = razer_turret_send_row_locked(role, &request);
    }

    mutex_unlock(&role->receiver->lock);

    if (err)
        return err;

    return count;
}

static ssize_t razer_attr_write_poll_rate(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request = get_razer_report(0x00, 0x0e, 0x02);
    struct razer_report response = {0};
    unsigned short polling_rate;
    int err;

    err = kstrtou16(buf, 0, &polling_rate);
    if (err < 0)
        return err;

    request.arguments[0] = 0x01;
    switch (polling_rate) {
    case 1000:
        request.arguments[1] = 0x01;
        break;
    case 500:
        request.arguments[1] = 0x02;
        break;
    default:
        dev_warn(dev, "razerkbd: Turret only supports 500 and 1000 Hz poll rates\n");
        return -EINVAL;
    }

    err = razer_turret_exchange(role, &request, &response, 0);
    if (err)
        return err;

    return count;
}

static ssize_t razer_attr_read_poll_rate(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request = get_razer_report(0x00, 0x8e, 0x02);
    struct razer_report response = {0};
    int err;

    request.arguments[0] = 0x01;
    err = razer_turret_exchange(role, &request, &response, 2);
    if (err)
        return err;

    switch (response.arguments[1]) {
    case 0x01:
        return sysfs_emit(buf, "%d\n", 1000);
    case 0x02:
        return sysfs_emit(buf, "%d\n", 500);
    default:
        razer_turret_log(role, &response, "Invalid poll rate");
        return -EIO;
    }
}

static bool razer_turret_dpi_valid(unsigned short dpi)
{
    return dpi >= RAZER_TURRET_RECEIVER_DPI_MIN && dpi <= RAZER_TURRET_RECEIVER_DPI_MAX;
}

/* 2 bytes (X = Y) or 4 bytes (X, Y), big endian */
static ssize_t razer_attr_write_dpi(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request;
    struct razer_report response = {0};
    unsigned short dpi_x, dpi_y;
    int err;

    if (count != 2 && count != 4) {
        dev_warn(dev, "razerkbd: DPI requires 2 or 4 bytes\n");
        return -EINVAL;
    }

    dpi_x = ((unsigned char)buf[0] << 8) | (unsigned char)buf[1];
    dpi_y = ((unsigned char)buf[count - 2] << 8) | (unsigned char)buf[count - 1];
    if (!razer_turret_dpi_valid(dpi_x) || !razer_turret_dpi_valid(dpi_y)) {
        dev_warn(dev, "razerkbd: DPI must be between %d and %d\n", RAZER_TURRET_RECEIVER_DPI_MIN, RAZER_TURRET_RECEIVER_DPI_MAX);
        return -EINVAL;
    }

    request = razer_chroma_misc_set_dpi_xy(VARSTORE, dpi_x, dpi_y);
    err = razer_turret_exchange(role, &request, &response, 0);
    if (err)
        return err;

    return count;
}

static ssize_t razer_attr_read_dpi(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request = razer_chroma_misc_get_dpi_xy(VARSTORE);
    struct razer_report response = {0};
    unsigned short dpi_x, dpi_y;
    int err;

    err = razer_turret_exchange(role, &request, &response, 7);
    if (err)
        return err;

    dpi_x = (response.arguments[1] << 8) | response.arguments[2];
    dpi_y = (response.arguments[3] << 8) | response.arguments[4];
    if (!razer_turret_dpi_valid(dpi_x) || !razer_turret_dpi_valid(dpi_y)) {
        razer_turret_log(role, &response, "Invalid DPI");
        return -EIO;
    }

    return sysfs_emit(buf, "%u:%u\n", dpi_x, dpi_y);
}

/* Active stage (1 byte, 1-based), then 1 to 5 stages of X, Y (big endian) */
static ssize_t razer_attr_write_dpi_stages(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request;
    struct razer_report response = {0};
    unsigned short dpi[2 * RAZER_TURRET_RECEIVER_DPI_STAGES_MAX];
    unsigned char stages_count, active_stage;
    unsigned int i;
    int err;

    if (count < 5 || (count - 1) % 4 != 0 || (count - 1) / 4 > RAZER_TURRET_RECEIVER_DPI_STAGES_MAX) {
        dev_warn(dev, "razerkbd: Invalid DPI stages\n");
        return -EINVAL;
    }

    stages_count = (count - 1) / 4;
    active_stage = buf[0];
    if (active_stage < 1 || active_stage > stages_count) {
        dev_warn(dev, "razerkbd: Invalid DPI stages\n");
        return -EINVAL;
    }

    for (i = 0; i < 2 * stages_count; i++) {
        dpi[i] = ((unsigned char)buf[1 + 2 * i] << 8) | (unsigned char)buf[2 + 2 * i];
        if (!razer_turret_dpi_valid(dpi[i])) {
            dev_warn(dev, "razerkbd: DPI must be between %d and %d\n", RAZER_TURRET_RECEIVER_DPI_MIN, RAZER_TURRET_RECEIVER_DPI_MAX);
            return -EINVAL;
        }
    }

    request = razer_chroma_misc_set_dpi_stages(VARSTORE, stages_count, active_stage, dpi);
    request.data_size = 3 + 7 * stages_count;
    err = razer_turret_exchange(role, &request, &response, 0);
    if (err)
        return err;

    return count;
}

/* The reply numbers stages from 1, while the SET numbers them from 0 */
static ssize_t razer_attr_read_dpi_stages(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_turret_role *role = dev_get_drvdata(dev);
    struct razer_report request = razer_chroma_misc_get_dpi_stages(VARSTORE);
    struct razer_report response = {0};
    unsigned char stages_count, *stage;
    unsigned int i;
    int err;

    request.data_size = RAZER_TURRET_RECEIVER_DPI_STAGES_GET_SIZE;
    err = razer_turret_exchange(role, &request, &response, 3);
    if (err)
        return err;

    stages_count = response.arguments[2];
    if (response.arguments[0] != VARSTORE ||
        stages_count < 1 || stages_count > RAZER_TURRET_RECEIVER_DPI_STAGES_MAX ||
        response.arguments[1] < 1 || response.arguments[1] > stages_count ||
        response.data_size < 3 + 7 * stages_count) {
        razer_turret_log(role, &response, "Invalid DPI stages");
        return -EIO;
    }

    for (i = 0; i < stages_count; i++) {
        stage = &response.arguments[3 + 7 * i];
        if (stage[0] != i + 1 ||
            !razer_turret_dpi_valid((stage[1] << 8) | stage[2]) ||
            !razer_turret_dpi_valid((stage[3] << 8) | stage[4])) {
            razer_turret_log(role, &response, "Invalid DPI stages");
            return -EIO;
        }
    }

    buf[0] = response.arguments[1];
    for (i = 0; i < stages_count; i++)
        memcpy(&buf[1 + 4 * i], &response.arguments[4 + 7 * i], 4);

    return 1 + 4 * stages_count;
}

static DEVICE_ATTR(version,                 0440, razer_attr_read_version,                    NULL);
static DEVICE_ATTR(device_type,             0440, razer_attr_read_device_type,                NULL);
static DEVICE_ATTR(device_serial,           0440, razer_attr_read_device_serial,              NULL);
static DEVICE_ATTR(firmware_version,        0440, razer_attr_read_firmware_version,           NULL);
static DEVICE_ATTR(charge_level,            0440, razer_attr_read_charge_level,               NULL);
static DEVICE_ATTR(charge_status,           0440, razer_attr_read_charge_status,              NULL);
static DEVICE_ATTR(matrix_brightness,       0660, razer_attr_read_matrix_brightness,          razer_attr_write_matrix_brightness);
static DEVICE_ATTR(matrix_effect_none,      0220, NULL,                                       razer_attr_write_matrix_effect_none);
static DEVICE_ATTR(matrix_effect_static,    0220, NULL,                                       razer_attr_write_matrix_effect_static);
static DEVICE_ATTR(matrix_effect_spectrum,  0220, NULL,                                       razer_attr_write_matrix_effect_spectrum);
static DEVICE_ATTR(matrix_effect_breath,    0220, NULL,                                       razer_attr_write_matrix_effect_breath);
static DEVICE_ATTR(matrix_effect_reactive,  0220, NULL,                                       razer_attr_write_matrix_effect_reactive);
static DEVICE_ATTR(matrix_effect_wave,      0220, NULL,                                       razer_attr_write_matrix_effect_wave);
static DEVICE_ATTR(matrix_effect_starlight, 0220, NULL,                                       razer_attr_write_matrix_effect_starlight);
static DEVICE_ATTR(matrix_effect_custom,    0220, NULL,                                       razer_attr_write_matrix_effect_custom);
static DEVICE_ATTR(matrix_custom_frame,     0220, NULL,                                       razer_attr_write_matrix_custom_frame);
static DEVICE_ATTR(poll_rate,               0660, razer_attr_read_poll_rate,                  razer_attr_write_poll_rate);
static DEVICE_ATTR(dpi,                     0660, razer_attr_read_dpi,                        razer_attr_write_dpi);
static DEVICE_ATTR(dpi_stages,              0660, razer_attr_read_dpi_stages,                 razer_attr_write_dpi_stages);

static struct attribute *turret_keyboard_attrs[] = {
    &dev_attr_version.attr,
    &dev_attr_device_type.attr,
    &dev_attr_device_serial.attr,
    &dev_attr_firmware_version.attr,
    &dev_attr_charge_level.attr,
    &dev_attr_charge_status.attr,
    &dev_attr_matrix_brightness.attr,
    &dev_attr_matrix_effect_none.attr,
    &dev_attr_matrix_effect_static.attr,
    &dev_attr_matrix_effect_spectrum.attr,
    &dev_attr_matrix_effect_breath.attr,
    &dev_attr_matrix_effect_reactive.attr,
    &dev_attr_matrix_effect_wave.attr,
    &dev_attr_matrix_effect_starlight.attr,
    &dev_attr_matrix_effect_custom.attr,
    &dev_attr_matrix_custom_frame.attr,
    NULL
};

static struct attribute *turret_mouse_attrs[] = {
    &dev_attr_version.attr,
    &dev_attr_device_type.attr,
    &dev_attr_device_serial.attr,
    &dev_attr_firmware_version.attr,
    &dev_attr_charge_level.attr,
    &dev_attr_charge_status.attr,
    &dev_attr_matrix_brightness.attr,
    &dev_attr_matrix_effect_none.attr,
    &dev_attr_matrix_effect_static.attr,
    &dev_attr_matrix_effect_spectrum.attr,
    &dev_attr_matrix_effect_breath.attr,
    &dev_attr_matrix_effect_reactive.attr,
    &dev_attr_poll_rate.attr,
    &dev_attr_dpi.attr,
    &dev_attr_dpi_stages.attr,
    NULL
};

static const struct attribute_group turret_keyboard_group = {
    .attrs = turret_keyboard_attrs,
};

static const struct attribute_group turret_mouse_group = {
    .attrs = turret_mouse_attrs,
};

static const struct attribute_group *razer_turret_role_group(struct razer_turret_role *role)
{
    return role->keyboard ? &turret_keyboard_group : &turret_mouse_group;
}

/**
 * Passive probe; interface 2 has no control files.
 */
int razer_turret_receiver_probe(struct hid_device *hdev)
{
    struct razer_turret_role *role = NULL;
    struct usb_interface *intf;
    unsigned char interface_number;
    int err;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
    if (!hid_is_usb(hdev))
        return -EINVAL;
#endif

    intf = to_usb_interface(hdev->dev.parent);
    interface_number = intf->cur_altsetting->desc.bInterfaceNumber;

    if (interface_number == RAZER_TURRET_RECEIVER_MOUSE_INTERFACE ||
        interface_number == RAZER_TURRET_RECEIVER_KEYBOARD_INTERFACE) {
        role = kzalloc_obj(*role);
        if (!role)
            return -ENOMEM;

        role->hdev = hdev;
        role->keyboard = interface_number == RAZER_TURRET_RECEIVER_KEYBOARD_INTERFACE;
        role->receiver = razer_turret_receiver_get(hid_to_usb_dev(hdev));
        if (!role->receiver) {
            kfree(role);
            return -ENOMEM;
        }
    }

    hid_set_drvdata(hdev, role);

    err = hid_parse(hdev);
    if (err) {
        hid_err(hdev, "parse failed\n");
        goto exit_free;
    }

    err = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (err) {
        hid_err(hdev, "hw start failed\n");
        goto exit_free;
    }

    if (role) {
        err = device_add_group(&hdev->dev, razer_turret_role_group(role));
        if (err) {
            hid_hw_stop(hdev);
            goto exit_free;
        }
    }

    return 0;

exit_free:
    hid_set_drvdata(hdev, NULL);
    if (role) {
        razer_turret_receiver_put(role->receiver);
        kfree(role);
    }
    return err;
}

/* Removing the files waits for running callbacks before the role is freed */
void razer_turret_receiver_remove(struct hid_device *hdev)
{
    struct razer_turret_role *role = hid_get_drvdata(hdev);

    if (role)
        device_remove_group(&hdev->dev, razer_turret_role_group(role));

    hid_hw_stop(hdev);
    hid_set_drvdata(hdev, NULL);

    if (role) {
        razer_turret_receiver_put(role->receiver);
        kfree(role);
    }
}
