// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Razer BlackShark headset driver.
 *
 * These headsets do NOT use the standard Razer "Chroma" report protocol. Their
 * vendor HID interface (Usage Page 0xFF00, Report ID 0x02) uses 64-byte
 * interrupt Output/Input reports carrying a vendor command protocol whose
 * sub-frame is common to the model family:
 *
 *   [10]=cmd id  [11]=flag (0 request / 0x01 ACK in a reply)
 *   [12]=payload length  [13..]=payload
 *
 * The command ids are common to the family too; only the envelope around the
 * sub-frame, a few hardware ranges and the exposed attribute set differ per
 * model, which is why all of that lives in struct blackshark_model rather than
 * in the command paths. The V2 Pro 2.4 (1532:0555) wraps the sub-frame in a
 * 'P'/'A' marked envelope carrying a command type at [9], and needs a
 * set_remote_mode (0xE1) handshake around writes, matching Razer Synapse.
 *
 * Battery level (cmd_type 0x03, cmd_id 0x21) returns 0-100 directly; charging
 * state (cmd_id 0x2a) returns 0 on battery / nonzero on the cable.
 *
 * Protocol reverse-engineered from Ashesh3/razer-device-control and a local
 * hidraw capture of this exact device (1532:0555). Replies arrive on the
 * interrupt IN endpoint and are matched in .raw_event.
 *
 * Copyright (c) 2024 Openrazer contributors
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/delay.h>

#include "razerblackshark_driver.h"
#include "razercommon.h"

#define DRIVER_DESC "Razer BlackShark Device Driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE(DRIVER_LICENSE);

/* ------------------------------------------------------------------ */
/* Per-model framing                                                  */
/* ------------------------------------------------------------------ */

/**
 * V2 Pro 2.4 envelope: 'P'/'A' markers, a command type at [9] and a total
 * length that counts the payload.
 */
static void blackshark_v2pro_write_header(u8 *buf, u8 cmd_type, u8 data_len)
{
    buf[BS_OFF_DIR]       = BS_DIR_OUT;
    buf[BS_OFF_TOTAL_LEN] = BS_TOTAL_LEN_BASE + data_len;
    buf[BS_OFF_MARK_P]    = BS_MARK_P;
    buf[BS_OFF_MARK_A]    = BS_MARK_A;
    buf[BS_OFF_INNER_LEN] = BS_INNER_LEN_STD;
    buf[BS_OFF_CMD_TYPE]  = cmd_type;
}

/* The model table itself lives below, next to the attributes it references. */

/* ------------------------------------------------------------------ */
/* Command helpers                                                    */
/* ------------------------------------------------------------------ */

/**
 * Build a 64-byte command into dev->cmd_buf: the common sub-frame plus the
 * model's envelope.
 */
static void blackshark_build_cmd(struct razer_blackshark_device *dev,
                                 u8 cmd_type, u8 cmd_id,
                                 const u8 *data, size_t data_len)
{
    memset(dev->cmd_buf, 0, BLACKSHARK_REPORT_LEN);
    dev->cmd_buf[0] = BLACKSHARK_REPORT_ID;

    dev->model->write_header(dev->cmd_buf, cmd_type, data_len);

    dev->cmd_buf[BS_OFF_CMD_ID]   = cmd_id;
    dev->cmd_buf[BS_OFF_DATA_LEN] = data_len;
    if (data && data_len)
        memcpy(&dev->cmd_buf[BS_OFF_DATA], data, data_len);

    if (dev->model->finalize)
        dev->model->finalize(dev->cmd_buf);
}

/**
 * Send the currently-built command out via the interrupt OUT endpoint.
 */
static int blackshark_send_cmd(struct razer_blackshark_device *dev)
{
    int ret = hid_hw_output_report(dev->hdev, dev->cmd_buf, BLACKSHARK_REPORT_LEN);

    if (ret < 0)
        hid_err(dev->hdev, "blackshark: output report failed: %d\n", ret);
    return ret;
}

/**
 * set_remote_mode: hand software control to/from the host. Synapse sends this
 * before querying, so we mirror it.
 *
 * This frame does not follow the common sub-frame layout: it carries its
 * parameter in the flag byte and uses its own length pair, so it is built
 * directly rather than through blackshark_build_cmd().
 */
static void blackshark_set_remote(struct razer_blackshark_device *dev, bool on)
{
    if (!dev->model->needs_remote_mode)
        return;

    memset(dev->cmd_buf, 0, BLACKSHARK_REPORT_LEN);
    dev->cmd_buf[0]                   = BLACKSHARK_REPORT_ID;
    dev->cmd_buf[BS_OFF_DIR]          = BS_DIR_OUT;
    dev->cmd_buf[BS_OFF_TOTAL_LEN]    = 0x07;
    dev->cmd_buf[BS_OFF_MARK_P]       = BS_MARK_P;
    dev->cmd_buf[BS_OFF_MARK_A]       = BS_MARK_A;
    dev->cmd_buf[BS_OFF_INNER_LEN]    = BS_INNER_LEN_REMOTE;
    dev->cmd_buf[BS_OFF_CMD_TYPE]     = BS_TYPE_REMOTE;
    dev->cmd_buf[BS_OFF_CMD_ID]       = BS_CMD_REMOTE;
    dev->cmd_buf[BS_OFF_FLAG]         = on ? 1 : 0;

    if (dev->model->finalize)
        dev->model->finalize(dev->cmd_buf);

    blackshark_send_cmd(dev);
    msleep(35);
}

/**
 * Query via cmd_type 0x03 / cmd_id, copying up to out_len reply bytes.
 * Sequence mirrors the working userspace probe: remote on, query, remote off.
 * Serialised by req_lock.
 *
 * Returns the number of payload bytes copied, or a negative errno.
 */
static int blackshark_query_buf(struct razer_blackshark_device *dev, u8 cmd_id,
                                u8 *out, size_t out_len)
{
    int ret;
    long left;

    if (out_len > sizeof(dev->resp_buf))
        return -EINVAL;

    mutex_lock(&dev->req_lock);

    blackshark_set_remote(dev, true);

    WRITE_ONCE(dev->expected_cmd, cmd_id);
    reinit_completion(&dev->resp_done);
    WRITE_ONCE(dev->resp_pending, true);

    blackshark_build_cmd(dev, BS_TYPE_QUERY, cmd_id, NULL, 0);
    ret = blackshark_send_cmd(dev);
    if (ret < 0)
        goto out;

    left = wait_for_completion_timeout(&dev->resp_done,
                                       msecs_to_jiffies(BLACKSHARK_RESPONSE_TIMEOUT_MS));
    if (!left) {
        /*
         * Not an error worth a warning: the dongle enumerates whether or not
         * the headset is powered on, so an unlinked headset makes every
         * poll time out. At hid_warn that floods the kernel log, since the
         * daemon polls the battery on a timer.
         */
        hid_dbg(dev->hdev, "blackshark: timeout waiting for reply to cmd 0x%02x\n", cmd_id);
        ret = -ETIMEDOUT;
        goto out;
    }

    ret = min_t(size_t, out_len, dev->resp_len);
    memcpy(out, dev->resp_buf, ret);

out:
    WRITE_ONCE(dev->resp_pending, false);
    blackshark_set_remote(dev, false);
    mutex_unlock(&dev->req_lock);
    return ret;
}

/* Read a single-byte value. Returns 0 on success, or a negative errno. */
static int blackshark_query(struct razer_blackshark_device *dev, u8 cmd_id, u8 *out)
{
    int ret = blackshark_query_buf(dev, cmd_id, out, 1);

    if (ret < 0)
        return ret;
    return ret == 1 ? 0 : -EIO;
}

/**
 * raw_event: replies (and unsolicited telemetry) arrive here. When a request is
 * pending, match the echoed cmd id at the model's reply offset and capture the
 * payload.
 */
static int blackshark_raw_event(struct hid_device *hdev, struct hid_report *report,
                                u8 *data, int size)
{
    struct razer_blackshark_device *dev = hid_get_drvdata(hdev);
    u8 cmd_off, data_off;
    u8 want;

    if (!dev)
        return 0;

    cmd_off  = dev->model->reply_cmd_off;
    data_off = BS_REPLY_DATA_OFF(dev->model);
    if (size <= data_off || data[0] != BLACKSHARK_REPORT_ID)
        return 0;

    /* The headset's EQ button emits unsolicited preset-change events (same id
     * as the preset query); track them so the cached preset stays fresh. */
    if (data[cmd_off] == BS_CMD_PRESET_GET) {
        switch (data[data_off]) {
        case BS_PRESET_GAME:
        case BS_PRESET_MUSIC:
        case BS_PRESET_MOVIE:
        case BS_PRESET_CUSTOM:
            WRITE_ONCE(dev->eq_preset, data[data_off]);
            break;
        default:
            if (BS_PRESET_IS_GAME_EQ(data[data_off]))
                WRITE_ONCE(dev->eq_preset, data[data_off]);
            break;
        }
    }

    if (!READ_ONCE(dev->resp_pending))
        return 0;

    want = READ_ONCE(dev->expected_cmd);

    /* Match on the echoed id *and* the ACK byte: the device also emits
     * unsolicited 'PI' telemetry frames, which would otherwise satisfy an
     * id-only match by coincidence. */
    if (data[cmd_off] == want && data[BS_REPLY_ACK_OFF(dev->model)] == 0x01) {
        /* The device reports the payload length at [cmd + 2]; trust it only
         * as far as the report actually reaches. */
        size_t n = min3((size_t)data[BS_REPLY_LEN_OFF(dev->model)],
                        sizeof(dev->resp_buf),
                        (size_t)(size - data_off));

        memcpy(dev->resp_buf, &data[data_off], n);
        dev->resp_len = n;
        complete(&dev->resp_done);
    }

    return 0;
}

/**
 * Send the preset apply sequence, SELECTOR ONLY - no band frame. Captured
 * order (Synapse): remote(on) x2 -> prep(0x1e) -> preset(0x93) ->
 * flag(0x9d: 2 for the game-EQ presets, 1 otherwise) -> remote(on).
 *
 * No band data is ever sent here. Every preset's curve lives on the device
 * (game/music/movie in ROM; custom and each of the five game EQs in its own
 * writable slot) and the selector alone activates it - verified by ear and
 * by the 0x15 getter, which follows the active preset's slot. Not bundling
 * bands with a selector change also matters for correctness: a 0x95 frame
 * sent during a cross-family transition is stored into the slot the device
 * momentarily lands on, corrupting another preset's curve (observed on
 * hardware). Serialised by req_lock.
 */
static void blackshark_apply_preset(struct razer_blackshark_device *dev)
{
    u8 preset = dev->eq_preset;
    u8 flag = BS_PRESET_IS_GAME_EQ(dev->eq_preset) ? 0x02 : 0x01;

    mutex_lock(&dev->req_lock);

    blackshark_set_remote(dev, true);
    blackshark_set_remote(dev, true);

    blackshark_build_cmd(dev, BS_TYPE_QUERY, BS_CMD_PREP, NULL, 0);
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);
    blackshark_build_cmd(dev, BS_TYPE_AUDIO, BS_CMD_PRESET, &preset, 1);
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);
    blackshark_build_cmd(dev, BS_TYPE_AUDIO, BS_CMD_ENHANCE, &flag, 1);
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);

    mutex_unlock(&dev->req_lock);
}

/**
 * apply_preset with read-back verification. The firmware remembers a
 * "classic" (7/8/9/0xff) and a "game EQ" (0xfa-0xfe) preset separately; a
 * 0x93 write whose id crosses families only switches the family - the
 * device lands on that family's remembered preset, ignoring the requested
 * id (back-to-back selector frames in one sequence don't help; the switch
 * commits only once the sequence has settled). A repeated apply is then
 * same-family and sticks, so retry until the read-back matches (2 attempts
 * suffice on hardware; 4 for margin). Synapse never hits this because it
 * never reads device state.
 *
 * Returns 0 once the device confirms the target preset is active.
 */
static int blackshark_apply_preset_verified(struct razer_blackshark_device *dev)
{
    const u8 target = dev->eq_preset;
    u8 got;
    int attempt;

    for (attempt = 0; attempt < 4; attempt++) {
        /* our own apply makes the device emit preset events that overwrite
         * the cache with the slot it actually landed on; re-arm the target */
        WRITE_ONCE(dev->eq_preset, target);
        blackshark_apply_preset(dev);
        msleep(100);
        if (blackshark_query(dev, BS_CMD_PRESET_GET, &got))
            return -EIO;
        if (got == target)
            return 0;
    }
    return -EIO;
}

/**
 * Write the cached custom bands into the device's custom slot and latch
 * them into the live audio path. Only called once the custom preset is
 * verifiably active (see apply_preset_verified) - the device stores a 0x95
 * frame into whatever preset is active at that moment. The bands ride in
 * the full captured apply sequence (prep -> preset -> flag -> bands): a
 * bare 0x95 frame only updates the slot while the audio path keeps playing
 * the previously latched curve (heard as "the EQ lags one write behind");
 * bundled with a same-preset sequence it latches immediately. Serialised
 * by req_lock.
 */
static void blackshark_write_bands(struct razer_blackshark_device *dev)
{
    u8 preset = BS_PRESET_CUSTOM;
    u8 flag = 0x01;
    u8 eq[BS_EQ_BANDS];
    int i;

    /*
     * Some models store (written - eq_write_offset), so the gain has to be
     * pre-biased on the way out; reads come back as the plain gain either
     * way. The V2 Pro round-trips exactly and sets the offset to 0.
     */
    for (i = 0; i < BS_EQ_BANDS; i++)
        eq[i] = (u8)(s8)(dev->eq_bands[i] + dev->model->eq_write_offset);

    mutex_lock(&dev->req_lock);

    blackshark_set_remote(dev, true);
    blackshark_set_remote(dev, true);

    blackshark_build_cmd(dev, BS_TYPE_QUERY, BS_CMD_PREP, NULL, 0);
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);
    blackshark_build_cmd(dev, BS_TYPE_AUDIO, BS_CMD_PRESET, &preset, 1);
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);
    blackshark_build_cmd(dev, BS_TYPE_AUDIO, BS_CMD_ENHANCE, &flag, 1);
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);
    blackshark_build_cmd(dev, BS_TYPE_EQ, BS_CMD_EQ, eq, sizeof(eq));
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);

    mutex_unlock(&dev->req_lock);
}

/**
 * Write a single-byte 0x04-family setting (power-off timeout, sidetone).
 * These need no prep query, just remote mode. Serialised by req_lock.
 */
static void blackshark_write_value(struct razer_blackshark_device *dev, u8 cmd_id, u8 value)
{
    mutex_lock(&dev->req_lock);
    blackshark_set_remote(dev, true);
    blackshark_build_cmd(dev, BS_TYPE_AUDIO, cmd_id, &value, 1);
    blackshark_send_cmd(dev);
    blackshark_set_remote(dev, false);
    mutex_unlock(&dev->req_lock);
}

/* ------------------------------------------------------------------ */
/* sysfs attributes                                                   */
/* ------------------------------------------------------------------ */

static ssize_t razer_attr_read_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "%s\n", DRIVER_VERSION);
}

static ssize_t razer_attr_read_device_type(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    char *device_type;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_BLACKSHARK_V2_PRO_2_4:
        device_type = "Razer BlackShark V2 Pro 2.4";
        break;

    default:
        device_type = "Unknown Device";
        break;
    }

    return sysfs_emit(buf, "%s\n", device_type);
}

static ssize_t razer_attr_write_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return count;
}

static ssize_t razer_attr_read_test(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "\n");
}

/**
 * Fetch the serial number (cmd 0x00) and firmware version (cmd 0x02) from the
 * device, replacing the probe-time fallbacks. Best-effort and idempotent: a
 * query right after enumeration can time out before the link has settled, so
 * the read handlers call this until it succeeds once, after which @ids_read
 * latches and further calls return immediately.
 */
static void blackshark_read_ids(struct razer_blackshark_device *device)
{
    u8 tmp[BS_RESP_BUF_LEN];
    bool got_serial = false;
    bool got_fw = false;
    int n;

    if (READ_ONCE(device->ids_read))
        return;

    mutex_lock(&device->ids_lock);

    /* another reader may have completed the fetch while we waited */
    if (device->ids_read)
        goto out;

    n = blackshark_query_buf(device, BS_CMD_SERIAL, tmp, sizeof(tmp));
    if (n > 0) {
        n = min_t(int, n, (int)sizeof(device->serial) - 1);
        memcpy(device->serial, tmp, n);
        device->serial[n] = '\0';
        got_serial = true;
    }

    n = blackshark_query_buf(device, BS_CMD_FW_VER, tmp, sizeof(tmp));
    if (n >= 2) {
        device->fw_major = tmp[0];
        device->fw_minor = tmp[1];
        got_fw = true;
    }

    if (got_serial && got_fw)
        WRITE_ONCE(device->ids_read, true);

out:
    mutex_unlock(&device->ids_lock);
}

/*
 * Background half of the above: runs once just after probe so the identity is
 * in place before anything reads it, without blocking the hotplug path. If it
 * fails (headset asleep) the read handlers keep retrying lazily.
 */
static void blackshark_ids_work(struct work_struct *work)
{
    struct razer_blackshark_device *device =
        container_of(to_delayed_work(work), struct razer_blackshark_device, ids_work);

    blackshark_read_ids(device);
}

static ssize_t razer_attr_read_firmware_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    ssize_t ret;

    blackshark_read_ids(device);

    /* under ids_lock: a concurrent first fetch may still be publishing */
    mutex_lock(&device->ids_lock);
    ret = sysfs_emit(buf, "v%u.%u\n", device->fw_major, device->fw_minor);
    mutex_unlock(&device->ids_lock);

    return ret;
}

static ssize_t razer_attr_read_device_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    ssize_t ret;

    blackshark_read_ids(device);

    mutex_lock(&device->ids_lock);
    ret = sysfs_emit(buf, "%s\n", device->serial);
    mutex_unlock(&device->ids_lock);

    return ret;
}

/*
 * hw_model: the product id the headset reports for itself (cmd 0x03), high
 * byte first. Over the 2.4GHz dongle this identifies the paired headset, so it
 * differs from the USB product id the driver is bound to (0x0555 -> 0x0556).
 */
static ssize_t razer_attr_read_hw_model(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 id[2];
    int n = blackshark_query_buf(device, BS_CMD_HW_MODEL, id, sizeof(id));

    if (n < 0)
        return n;
    if (n != sizeof(id))
        return -EIO;

    return sysfs_emit(buf, "%04x\n", (id[0] << 8) | id[1]);
}

/*
 * mic_mute: state of the headset's hardware mic-mute button (cmd 0x55).
 * Read-only - the button is the only thing that changes it.
 */
static ssize_t razer_attr_read_mic_mute(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 muted = 0;
    int ret = blackshark_query(device, BS_CMD_MIC_MUTE, &muted);

    if (ret)
        return ret;

    return sysfs_emit(buf, "%d\n", muted ? 1 : 0);
}

/*
 * charge_level: daemon expects a 0-255 value (it scales /255*100). The device
 * reports 0-100, so scale up.
 */
static ssize_t razer_attr_read_charge_level(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 capacity = 0;
    int ret = blackshark_query(device, BS_CMD_BATTERY, &capacity);

    if (ret)
        return ret;
    if (capacity > 100)
        capacity = 100;

    return sysfs_emit(buf, "%d\n", DIV_ROUND_CLOSEST(capacity * 255, 100));
}

/*
 * charge_status: query cmd 0x2a. Observed values: 0 = on battery, 2 = on the
 * charging cable. Report 1 (charging) for any nonzero state, 0 otherwise.
 */
static ssize_t razer_attr_read_charge_status(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 state = 0;
    int ret = blackshark_query(device, BS_CMD_CHARGING, &state);

    if (ret)
        return ret;

    return sysfs_emit(buf, "%d\n", state ? 1 : 0);
}

/*
 * equalizer: 10 signed bytes, one gain (dB) per band (31Hz..16kHz). Writing
 * stores a custom curve: the driver switches to the custom preset first and
 * only sends the bands once the device confirms it is active, so they can
 * never land in another preset's slot. Reading returns the ACTIVE preset's
 * stored curve (0x15 follows the selected preset).
 */
static ssize_t razer_attr_write_equalizer(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    bool preset_changed = true;
    u8 cur = 0;
    int i;

    if (count < BS_EQ_BANDS)
        return -EINVAL;

    for (i = 0; i < BS_EQ_BANDS; i++)
        device->eq_bands[i] = (s8)buf[i];

    if (!blackshark_query(device, BS_CMD_PRESET_GET, &cur))
        preset_changed = (cur != BS_PRESET_CUSTOM);

    device->eq_preset = BS_PRESET_CUSTOM;

    /*
     * A bands sequence only latches into the live audio path when the DSP
     * is not still settling from a preceding apply sequence (otherwise it
     * stores the curve but the sound keeps playing the previously latched
     * one). So: when already on custom, send a single bands sequence with
     * nothing before it. When the preset has to change first, switch it
     * (verified), then send the bands twice with a settle pause - the
     * first write stores, the second latches. Both paths verified by ear.
     */
    if (preset_changed &&
        blackshark_apply_preset_verified(device))
        return -EIO; /* custom not confirmed active: don't risk the write */

    /*
     * Within one apply sequence the selector frame latches the slot's
     * CURRENT content into the audio path first, and the band frame only
     * updates the slot afterwards - so a single sequence always sounds one
     * write behind. Store the bands, then send a selector-only re-apply to
     * latch the freshly updated slot.
     */
    blackshark_write_bands(device);
    msleep(100);
    blackshark_apply_preset(device);
    return count;
}

static ssize_t razer_attr_read_equalizer(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 bands[BS_EQ_BANDS];
    int ret = blackshark_query_buf(device, BS_CMD_EQ_GET, bands, sizeof(bands));

    if (ret != BS_EQ_BANDS) /* device unreachable: fall back to the last curve */
        memcpy(buf, device->eq_bands, BS_EQ_BANDS);
    else {
        memcpy(device->eq_bands, bands, BS_EQ_BANDS);
        memcpy(buf, bands, BS_EQ_BANDS);
    }

    return BS_EQ_BANDS;
}

/*
 * device_idle_time: auto power-off timeout. The daemon's power interface speaks
 * seconds (like other Razer devices); the headset works in minutes, so convert.
 * A value of 0 disables auto power-off. Reading queries the device (cmd 0x2c).
 */
static ssize_t razer_attr_write_idle_time(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    unsigned long seconds;
    unsigned long minutes;

    if (kstrtoul(buf, 10, &seconds))
        return -EINVAL;

    /*
     * The device's selectable auto power-off range is 15-60 minutes (matching
     * Razer Synapse); 0 disables it. Clamp non-zero values into that range so a
     * generic seconds-based caller can't set an out-of-range timeout.
     */
    minutes = seconds / 60;
    if (seconds && minutes < BS_IDLE_MIN_MINUTES)
        minutes = BS_IDLE_MIN_MINUTES;
    if (minutes > BS_IDLE_MAX_MINUTES)
        minutes = BS_IDLE_MAX_MINUTES;

    device->idle_minutes = minutes;
    blackshark_write_value(device, BS_CMD_IDLE_SET, minutes);
    return count;
}

static ssize_t razer_attr_read_idle_time(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 minutes = 0;
    int ret = blackshark_query(device, BS_CMD_IDLE_GET, &minutes);

    if (ret)
        return ret;

    device->idle_minutes = minutes;
    return sysfs_emit(buf, "%u\n", minutes * 60);
}

/*
 * equalizer_preset: selects the device EQ preset (cmd 0x93). Accepts the raw
 * selector value: 7=game, 8=music, 9=movie, 250-254=the five game-specific
 * EQs, 255=custom. Game/Music/Movie use curves stored on the device; the
 * game EQs apply their fixed curve and custom applies the host bands (see
 * equalizer).
 */
static ssize_t razer_attr_write_equalizer_preset(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    unsigned long val;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;

    switch (val) {
    case BS_PRESET_GAME:
    case BS_PRESET_MUSIC:
    case BS_PRESET_MOVIE:
    case BS_PRESET_CUSTOM:
        break;
    default:
        if (!BS_PRESET_IS_GAME_EQ(val))
            return -EINVAL;
        break;
    }

    device->eq_preset = val;
    if (blackshark_apply_preset_verified(device))
        return -EIO;
    return count;
}

static ssize_t razer_attr_read_equalizer_preset(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 preset = 0;
    int ret = blackshark_query(device, BS_CMD_PRESET_GET, &preset);

    if (ret) /* device unreachable: fall back to the cached value */
        preset = device->eq_preset;
    else
        device->eq_preset = preset;

    return sysfs_emit(buf, "%d\n", preset);
}

/*
 * sidetone: microphone monitoring - hear your own mic in the headset. One
 * value in device units, 0 = off, 1..model->sidetone_max = active at that
 * level.
 *
 * The device keeps the enable flag (0x98) and the level (0x99) as two
 * registers; folding them into one attribute keeps the sysfs surface the same
 * across BlackShark models, whose levels differ only in range.
 */
static ssize_t razer_attr_write_sidetone(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    unsigned long val;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    if (val > device->model->sidetone_max)
        val = device->model->sidetone_max;

    device->sidetone = val;
    blackshark_write_value(device, BS_CMD_SIDETONE, val ? 1 : 0);
    if (val)
        blackshark_write_value(device, BS_CMD_SIDETONE_LVL, val);
    return count;
}

static ssize_t razer_attr_read_sidetone(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 enabled = 0;
    u8 level = 0;

    if (blackshark_query(device, BS_CMD_SIDETONE_GET, &enabled))
        return sysfs_emit(buf, "%d\n", device->sidetone); /* unreachable: cached */

    if (enabled && blackshark_query(device, BS_CMD_SIDETONE_LVL_GET, &level))
        level = device->sidetone;

    device->sidetone = enabled ? level : 0;
    return sysfs_emit(buf, "%d\n", device->sidetone);
}

/*
 * dnd: the headset's "Do Not Disturb" toggle (cmd 0xa7), stored on-device and
 * read back via 0x27. On this dual-mode (2.4GHz + Bluetooth) model Synapse
 * presents it alongside the Bluetooth settings.
 *
 * It has no effect on the audio path, and - tested by power-cycling the
 * headset with it both on and off - it does not silence the spoken power/
 * battery prompts either, so it is purely a connectivity flag here.
 */
static ssize_t razer_attr_write_dnd(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    unsigned long val;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;

    device->dnd = val ? 1 : 0;
    blackshark_write_value(device, BS_CMD_DND, device->dnd);
    return count;
}

static ssize_t razer_attr_read_dnd(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 state = 0;

    if (blackshark_query(device, BS_CMD_DND_GET, &state))
        state = device->dnd; /* device unreachable: cached value */
    else
        device->dnd = state ? 1 : 0;

    return sysfs_emit(buf, "%d\n", state ? 1 : 0);
}

static DEVICE_ATTR(version,          0440, razer_attr_read_version,          NULL);
static DEVICE_ATTR(test,             0660, razer_attr_read_test,             razer_attr_write_test);
static DEVICE_ATTR(firmware_version, 0440, razer_attr_read_firmware_version, NULL);
static DEVICE_ATTR(device_type,      0440, razer_attr_read_device_type,      NULL);
static DEVICE_ATTR(device_serial,    0440, razer_attr_read_device_serial,    NULL);
static DEVICE_ATTR(hw_model,         0440, razer_attr_read_hw_model,         NULL);
static DEVICE_ATTR(mic_mute,         0440, razer_attr_read_mic_mute,         NULL);
static DEVICE_ATTR(charge_level,     0440, razer_attr_read_charge_level,     NULL);
static DEVICE_ATTR(charge_status,    0440, razer_attr_read_charge_status,    NULL);
static DEVICE_ATTR(equalizer,        0660, razer_attr_read_equalizer,        razer_attr_write_equalizer);
static DEVICE_ATTR(equalizer_preset, 0660, razer_attr_read_equalizer_preset, razer_attr_write_equalizer_preset);
static DEVICE_ATTR(device_idle_time, 0660, razer_attr_read_idle_time,        razer_attr_write_idle_time);
static DEVICE_ATTR(sidetone,         0660, razer_attr_read_sidetone,         razer_attr_write_sidetone);
static DEVICE_ATTR(dnd,              0660, razer_attr_read_dnd,              razer_attr_write_dnd);

/* ------------------------------------------------------------------ */
/* Model table                                                        */
/* ------------------------------------------------------------------ */

/*
 * Which attributes each model exposes is expressed in probe()/disconnect()
 * below rather than as an attribute_group here: the fake-driver generator
 * (scripts/generate_fake_driver.sh) derives each device's sysfs surface by
 * parsing the per-PID CREATE_DEVICE_FILE lines out of the probe function.
 *
 * Deliberately absent for this model: status_indicator (0x66/0xe6), because
 * the setter acknowledges every write and the mode never actually changes -
 * see the register notes in the header.
 */
static const struct blackshark_model blackshark_models[] = {
    {
        .usb_pid          = USB_DEVICE_ID_RAZER_BLACKSHARK_V2_PRO_2_4,
        .serial_fallback  = "BLACKSHARKV2PRO",

        .write_header     = blackshark_v2pro_write_header,
        .finalize         = NULL,
        .reply_cmd_off    = 12,
        .needs_remote_mode = true,

        .sidetone_max     = BS_V2PRO_SIDETONE_MAX,
        .sidetone_default = BS_V2PRO_SIDETONE_DEFAULT,
        .eq_write_offset  = 0, /* this model round-trips the written gain exactly */
    },
};

/*
 * No fallback entry: a PID in razer_devices[] without a descriptor here would
 * otherwise be framed like some unrelated model and appear to half-work.
 */
static const struct blackshark_model *blackshark_model_for(unsigned short pid)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(blackshark_models); i++)
        if (blackshark_models[i].usb_pid == pid)
            return &blackshark_models[i];

    return NULL;
}

/* ------------------------------------------------------------------ */
/* probe / disconnect                                                 */
/* ------------------------------------------------------------------ */

static int razer_blackshark_init(struct razer_blackshark_device *dev, struct usb_interface *intf, struct hid_device *hdev)
{
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    dev->hdev = hdev;
    dev->usb_pid = usb_dev->descriptor.idProduct;
    dev->usb_interface_protocol = intf->cur_altsetting->desc.bInterfaceProtocol;

    dev->model = blackshark_model_for(dev->usb_pid);
    if (!dev->model) {
        hid_err(hdev, "blackshark: no model descriptor for pid %04x\n", dev->usb_pid);
        return -ENODEV;
    }

    mutex_init(&dev->req_lock);
    mutex_init(&dev->ids_lock);
    init_completion(&dev->resp_done);
    INIT_DELAYED_WORK(&dev->ids_work, blackshark_ids_work);

    /* The driver never writes to the device on its own: presets are
     * selector-only (curves live in per-preset slots on the device) and
     * custom bands are only sent on an explicit equalizer write, after the
     * custom preset is confirmed active. eq_bands is just a read fallback. */
    dev->eq_preset = BS_PRESET_CUSTOM;
    dev->idle_minutes = 0;
    dev->sidetone = dev->model->sidetone_default;

    /*
     * Placeholders until blackshark_read_ids() gets the real values off the
     * device.
     *
     * Deliberately NOT hdev->uniq: over a 2.4GHz dongle that is the dongle's
     * own USB iSerial, not the headset's, so reporting it is a wrong answer
     * rather than a degraded one - and it is indistinguishable from a real
     * serial to everything downstream. The daemon reads device_serial exactly
     * once and keys the D-Bus object path and its persistence store off it, so
     * handing it a plausible-looking wrong value is worse than an obvious
     * placeholder.
     */
    strscpy(dev->serial, dev->model->serial_fallback, sizeof(dev->serial));
    dev->fw_major = 1;
    dev->fw_minor = 0;

    return 0;
}

static int razer_blackshark_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int retval = 0;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct razer_blackshark_device *dev = NULL;

    dev = kzalloc(sizeof(struct razer_blackshark_device), GFP_KERNEL);
    if (dev == NULL) {
        dev_err(&intf->dev, "out of memory\n");
        return -ENOMEM;
    }

    retval = razer_blackshark_init(dev, intf, hdev);
    if (retval)
        goto exit_kfree;

    hid_set_drvdata(hdev, dev);
    dev_set_drvdata(&hdev->dev, dev);

    retval = hid_parse(hdev);
    if (retval) {
        hid_err(hdev, "parse failed\n");
        goto exit_kfree;
    }

    retval = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (retval) {
        hid_err(hdev, "hw start failed\n");
        goto exit_kfree;
    }

    /* Open the device so the interrupt IN endpoint is polled and replies reach
     * .raw_event. */
    retval = hid_hw_open(hdev);
    if (retval) {
        hid_err(hdev, "hw open failed\n");
        goto exit_stop;
    }

    if (dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE) {
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_test);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_firmware_version);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);

        switch (dev->usb_pid) {
        case USB_DEVICE_ID_RAZER_BLACKSHARK_V2_PRO_2_4:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_hw_model);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mic_mute);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_level);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_status);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_equalizer);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_equalizer_preset);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_idle_time);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_sidetone);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_dnd);
            break;
        }
    }

    /*
     * Kick off the identity fetch, once the attributes that expose it exist.
     * It matters that this happens early: the daemon reads device_serial
     * exactly once, on discovery, and caches the result for the lifetime of
     * the device - so a placeholder read there is never corrected, it becomes
     * the device's identity (D-Bus object path and persistence key) until the
     * next reboot.
     *
     * Deferred rather than run inline because with the headset asleep both
     * queries time out: done inline that blocked probe, and so the hotplug
     * path, for ~2.3s and still yielded only the placeholder.
     *
     * Scheduled last so that the CREATE_DEVICE_FILE failure path above, which
     * jumps straight to kfree(), cannot free the device out from under it.
     */
    schedule_delayed_work(&dev->ids_work, 0);

    usb_disable_autosuspend(interface_to_usbdev(intf));

    return 0;

exit_free:
    /* attribute creation failed with the device open: unwind that too */
    hid_hw_close(hdev);
exit_stop:
    hid_hw_stop(hdev);
exit_kfree:
    kfree(dev);
    return retval;
}

static void razer_blackshark_disconnect(struct hid_device *hdev)
{
    struct razer_blackshark_device *dev = hid_get_drvdata(hdev);

    /*
     * First: the background identity fetch does HID I/O and dereferences dev,
     * so it must be finished before the device is stopped or freed below.
     */
    cancel_delayed_work_sync(&dev->ids_work);

    if (dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE) {
        device_remove_file(&hdev->dev, &dev_attr_version);
        device_remove_file(&hdev->dev, &dev_attr_test);
        device_remove_file(&hdev->dev, &dev_attr_firmware_version);
        device_remove_file(&hdev->dev, &dev_attr_device_type);
        device_remove_file(&hdev->dev, &dev_attr_device_serial);

        switch (dev->usb_pid) {
        case USB_DEVICE_ID_RAZER_BLACKSHARK_V2_PRO_2_4:
            device_remove_file(&hdev->dev, &dev_attr_hw_model);
            device_remove_file(&hdev->dev, &dev_attr_mic_mute);
            device_remove_file(&hdev->dev, &dev_attr_charge_level);
            device_remove_file(&hdev->dev, &dev_attr_charge_status);
            device_remove_file(&hdev->dev, &dev_attr_equalizer);
            device_remove_file(&hdev->dev, &dev_attr_equalizer_preset);
            device_remove_file(&hdev->dev, &dev_attr_device_idle_time);
            device_remove_file(&hdev->dev, &dev_attr_sidetone);
            device_remove_file(&hdev->dev, &dev_attr_dnd);
            break;
        }
    }

    hid_hw_close(hdev);
    hid_hw_stop(hdev);
    kfree(dev);
}

static const struct hid_device_id razer_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_BLACKSHARK_V2_PRO_2_4) },
    { 0 }
};

MODULE_DEVICE_TABLE(hid, razer_devices);

static struct hid_driver razer_blackshark_driver = {
    .name      = "razerblackshark",
    .id_table  = razer_devices,
    .probe     = razer_blackshark_probe,
    .remove    = razer_blackshark_disconnect,
    .raw_event = blackshark_raw_event,
};

module_hid_driver(razer_blackshark_driver);
