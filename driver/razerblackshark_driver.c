// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Razer BlackShark V2 Pro (2.4GHz) wireless headset driver.
 *
 * This headset does NOT use the standard Razer "Chroma" report protocol. Its
 * vendor HID interface (Usage Page 0xFF00, Report ID 0x02) uses 64-byte
 * interrupt Output/Input reports carrying the "MXIC" command protocol:
 *
 *   command (host->device):
 *     [0]=0x02 report id  [1]=0x80 dir  [2]=total_len  [5]='P' [6]='A'
 *     [7]=inner_len  [9]=cmd_type  [10]=cmd_id  [11..]=params
 *   reply (device->host):
 *     the echoed cmd_id appears at offset 12; its value sits at offset 15.
 *
 * Battery level (cmd_type 0x03, cmd_id 0x21) returns 0-100 directly; charging
 * state (cmd_id 0x2a) returns 0 on battery / nonzero on the cable. A
 * set_remote_mode (cmd_id 0xE1) is sent first, matching Razer Synapse.
 *
 * Protocol reverse-engineered from Ashesh3/razer-device-control and a local
 * hidraw capture of this exact device (1532:0555); see BLACKSHARK_NOTES.md.
 * Replies arrive on the interrupt IN endpoint and are matched in .raw_event.
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
/* MXIC protocol helpers                                              */
/* ------------------------------------------------------------------ */

/**
 * Build a 64-byte MXIC command into dev->cmd_buf.
 */
static void blackshark_build_cmd(struct razer_blackshark_device *dev,
                                 u8 total_len, u8 inner_len,
                                 u8 cmd_type, u8 cmd_id,
                                 const u8 *params, size_t params_len)
{
    memset(dev->cmd_buf, 0, BLACKSHARK_REPORT_LEN);
    dev->cmd_buf[0]                = BLACKSHARK_REPORT_ID;
    dev->cmd_buf[BS_OFF_DIR]       = BS_DIR_OUT;
    dev->cmd_buf[BS_OFF_TOTAL_LEN] = total_len;
    dev->cmd_buf[BS_OFF_MARK_P]    = BS_MARK_P;
    dev->cmd_buf[BS_OFF_MARK_A]    = BS_MARK_A;
    dev->cmd_buf[BS_OFF_INNER_LEN] = inner_len;
    dev->cmd_buf[BS_OFF_CMD_TYPE]  = cmd_type;
    dev->cmd_buf[BS_OFF_CMD_ID]    = cmd_id;
    if (params && params_len)
        memcpy(&dev->cmd_buf[BS_OFF_PARAMS], params, params_len);
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
 */
static void blackshark_set_remote(struct razer_blackshark_device *dev, bool on)
{
    u8 param = on ? 1 : 0;

    blackshark_build_cmd(dev, 0x07, 0x0E, BS_TYPE_REMOTE, BS_CMD_REMOTE, &param, 1);
    blackshark_send_cmd(dev);
    msleep(35);
}

/**
 * Query via cmd_type 0x03 / cmd_id, copying out_len reply bytes (scalar
 * getters use 1 byte; the EQ bands getter returns 10). Sequence mirrors the
 * working userspace probe: remote on, query, remote off. Serialised by
 * req_lock.
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

    blackshark_build_cmd(dev, 0x08, 0x08, BS_TYPE_QUERY, cmd_id, NULL, 0);
    ret = blackshark_send_cmd(dev);
    if (ret < 0)
        goto out;

    left = wait_for_completion_timeout(&dev->resp_done,
                                       msecs_to_jiffies(BLACKSHARK_RESPONSE_TIMEOUT_MS));
    if (!left) {
        hid_warn(dev->hdev, "blackshark: timeout waiting for reply to cmd 0x%02x\n", cmd_id);
        ret = -ETIMEDOUT;
        goto out;
    }

    memcpy(out, dev->resp_buf, out_len);
    ret = 0;

out:
    WRITE_ONCE(dev->expected_cmd, 0);
    blackshark_set_remote(dev, false);
    mutex_unlock(&dev->req_lock);
    return ret;
}

static int blackshark_query(struct razer_blackshark_device *dev, u8 cmd_id, u8 *out)
{
    return blackshark_query_buf(dev, cmd_id, out, 1);
}

/**
 * raw_event: replies (and unsolicited telemetry) arrive here. When a request is
 * pending, match the echoed cmd id at the fixed reply offset and capture the
 * value byte.
 */
static int blackshark_raw_event(struct hid_device *hdev, struct hid_report *report,
                                u8 *data, int size)
{
    struct razer_blackshark_device *dev = hid_get_drvdata(hdev);
    u8 want;

    if (!dev || size <= BS_REPLY_VAL_OFF || data[0] != BLACKSHARK_REPORT_ID)
        return 0;

    /* The headset's EQ button emits unsolicited preset-change events (same id
     * as the preset query); track them so the cached preset stays fresh. */
    if (data[BS_REPLY_CMD_OFF] == BS_CMD_PRESET_GET) {
        switch (data[BS_REPLY_VAL_OFF]) {
        case BS_PRESET_GAME:
        case BS_PRESET_MUSIC:
        case BS_PRESET_MOVIE:
        case BS_PRESET_CUSTOM:
            WRITE_ONCE(dev->eq_preset, data[BS_REPLY_VAL_OFF]);
            break;
        default:
            if (BS_PRESET_IS_GAME_EQ(data[BS_REPLY_VAL_OFF]))
                WRITE_ONCE(dev->eq_preset, data[BS_REPLY_VAL_OFF]);
            break;
        }
    }

    want = READ_ONCE(dev->expected_cmd);
    if (!want)
        return 0;

    if (data[BS_REPLY_CMD_OFF] == want) {
        size_t n = min_t(size_t, sizeof(dev->resp_buf),
                         (size_t)(size - BS_REPLY_VAL_OFF));

        memcpy(dev->resp_buf, &data[BS_REPLY_VAL_OFF], n);
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
    u8 preset[3] = { 0x00, 0x01, dev->eq_preset };
    u8 flag[3] = { 0x00, 0x01,
                   BS_PRESET_IS_GAME_EQ(dev->eq_preset) ? 0x02 : 0x01
                 };

    mutex_lock(&dev->req_lock);

    blackshark_set_remote(dev, true);
    blackshark_set_remote(dev, true);

    blackshark_build_cmd(dev, 0x08, 0x08, BS_TYPE_QUERY, BS_CMD_PREP, NULL, 0);
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);
    blackshark_build_cmd(dev, 0x09, 0x08, BS_TYPE_AUDIO, BS_CMD_PRESET, preset, sizeof(preset));
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);
    blackshark_build_cmd(dev, 0x09, 0x08, BS_TYPE_AUDIO, BS_CMD_ENHANCE, flag, sizeof(flag));
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
    u8 preset[3] = { 0x00, 0x01, BS_PRESET_CUSTOM };
    u8 flag[3] = { 0x00, 0x01, 0x01 };
    u8 eq[2 + BS_EQ_BANDS];
    int i;

    eq[0] = 0x00;
    eq[1] = BS_EQ_BANDS;
    for (i = 0; i < BS_EQ_BANDS; i++)
        eq[2 + i] = (u8)dev->eq_bands[i];

    mutex_lock(&dev->req_lock);

    blackshark_set_remote(dev, true);
    blackshark_set_remote(dev, true);

    blackshark_build_cmd(dev, 0x08, 0x08, BS_TYPE_QUERY, BS_CMD_PREP, NULL, 0);
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);
    blackshark_build_cmd(dev, 0x09, 0x08, BS_TYPE_AUDIO, BS_CMD_PRESET, preset, sizeof(preset));
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);
    blackshark_build_cmd(dev, 0x09, 0x08, BS_TYPE_AUDIO, BS_CMD_ENHANCE, flag, sizeof(flag));
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);
    blackshark_build_cmd(dev, 0x12, 0x08, BS_TYPE_EQ, BS_CMD_EQ, eq, sizeof(eq));
    blackshark_send_cmd(dev);

    blackshark_set_remote(dev, true);

    mutex_unlock(&dev->req_lock);
}

/**
 * Write a single-byte 0x04-family setting (power-off timeout, mic monitoring,
 * mic level). These need no prep query, just remote mode. Serialised by
 * req_lock.
 */
static void blackshark_write_value(struct razer_blackshark_device *dev, u8 cmd_id, u8 value)
{
    u8 p[3] = { 0x00, 0x01, value };

    mutex_lock(&dev->req_lock);
    blackshark_set_remote(dev, true);
    blackshark_build_cmd(dev, 0x09, 0x08, BS_TYPE_AUDIO, cmd_id, p, sizeof(p));
    blackshark_send_cmd(dev);
    blackshark_set_remote(dev, false);
    mutex_unlock(&dev->req_lock);
}

/* ------------------------------------------------------------------ */
/* sysfs attributes                                                   */
/* ------------------------------------------------------------------ */

static ssize_t razer_attr_read_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", DRIVER_VERSION);
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

    return sprintf(buf, "%s\n", device_type);
}

static ssize_t razer_attr_write_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return count;
}

static ssize_t razer_attr_read_test(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "\n");
}

/*
 * Firmware version is not queryable over the MXIC protocol (no known command),
 * so report a stub to keep the daemon happy.
 */
static ssize_t razer_attr_read_firmware_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "v1.0\n");
}

static ssize_t razer_attr_read_device_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);

    return sprintf(buf, "%s\n", device->serial);
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

    return sprintf(buf, "%d\n", DIV_ROUND_CLOSEST(capacity * 255, 100));
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

    return sprintf(buf, "%d\n", state ? 1 : 0);
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

    if (ret) /* device unreachable: fall back to the last known curve */
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
    return sprintf(buf, "%u\n", minutes * 60);
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

    return sprintf(buf, "%d\n", preset);
}

/*
 * mic_monitoring: sidetone on/off (cmd 0x98). Reading queries the device (0x18).
 */
static ssize_t razer_attr_write_mic_monitoring(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    unsigned long val;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;

    device->mic_monitoring = val ? 1 : 0;
    blackshark_write_value(device, BS_CMD_MIC_MON, device->mic_monitoring);
    return count;
}

static ssize_t razer_attr_read_mic_monitoring(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 state = 0;

    if (blackshark_query(device, BS_CMD_MIC_MON_GET, &state))
        state = device->mic_monitoring; /* device unreachable: cached value */
    else
        device->mic_monitoring = state ? 1 : 0;

    return sprintf(buf, "%d\n", state ? 1 : 0);
}

/*
 * bt_dnd: Bluetooth "Do Not Disturb" (cmd 0xa7) — Synapse's toggle for this
 * dual-mode (2.4GHz + Bluetooth) headset. Stored on-device; reading queries
 * the device (0x27). A pure connectivity flag: no effect on the audio path.
 */
static ssize_t razer_attr_write_bt_dnd(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    unsigned long val;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;

    device->bt_dnd = val ? 1 : 0;
    blackshark_write_value(device, BS_CMD_BT_DND, device->bt_dnd);
    return count;
}

static ssize_t razer_attr_read_bt_dnd(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 state = 0;

    if (blackshark_query(device, BS_CMD_BT_DND_GET, &state))
        state = device->bt_dnd; /* device unreachable: cached value */
    else
        device->bt_dnd = state ? 1 : 0;

    return sprintf(buf, "%d\n", state ? 1 : 0);
}

/*
 * mic_monitoring_level: sidetone loudness (cmd 0x99). Synapse uses a 0-10
 * scale on the wire. Reading queries the device (0x19).
 */
static ssize_t razer_attr_write_mic_monitoring_level(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    unsigned long val;

    if (kstrtoul(buf, 10, &val))
        return -EINVAL;
    if (val > BS_MIC_LEVEL_MAX)
        val = BS_MIC_LEVEL_MAX;

    device->mic_level = val;
    blackshark_write_value(device, BS_CMD_MIC_LVL, device->mic_level);
    return count;
}

static ssize_t razer_attr_read_mic_monitoring_level(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_blackshark_device *device = dev_get_drvdata(dev);
    u8 level = 0;

    if (blackshark_query(device, BS_CMD_MIC_LVL_GET, &level))
        level = device->mic_level; /* device unreachable: cached value */
    else
        device->mic_level = level;

    return sprintf(buf, "%d\n", level);
}

static DEVICE_ATTR(version,          0440, razer_attr_read_version,          NULL);
static DEVICE_ATTR(test,             0660, razer_attr_read_test,             razer_attr_write_test);
static DEVICE_ATTR(firmware_version, 0440, razer_attr_read_firmware_version, NULL);
static DEVICE_ATTR(device_type,      0440, razer_attr_read_device_type,      NULL);
static DEVICE_ATTR(device_serial,    0440, razer_attr_read_device_serial,    NULL);
static DEVICE_ATTR(charge_level,     0440, razer_attr_read_charge_level,     NULL);
static DEVICE_ATTR(charge_status,    0440, razer_attr_read_charge_status,    NULL);
static DEVICE_ATTR(equalizer,        0660, razer_attr_read_equalizer,        razer_attr_write_equalizer);
static DEVICE_ATTR(equalizer_preset, 0660, razer_attr_read_equalizer_preset, razer_attr_write_equalizer_preset);
static DEVICE_ATTR(device_idle_time,  0660, razer_attr_read_idle_time,        razer_attr_write_idle_time);
static DEVICE_ATTR(mic_monitoring,    0660, razer_attr_read_mic_monitoring,   razer_attr_write_mic_monitoring);
static DEVICE_ATTR(mic_monitoring_level, 0660, razer_attr_read_mic_monitoring_level, razer_attr_write_mic_monitoring_level);
static DEVICE_ATTR(bt_dnd,            0660, razer_attr_read_bt_dnd,           razer_attr_write_bt_dnd);

/* ------------------------------------------------------------------ */
/* probe / disconnect                                                 */
/* ------------------------------------------------------------------ */

static void razer_blackshark_init(struct razer_blackshark_device *dev, struct usb_interface *intf, struct hid_device *hdev)
{
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    dev->hdev = hdev;
    dev->usb_vid = usb_dev->descriptor.idVendor;
    dev->usb_pid = usb_dev->descriptor.idProduct;
    dev->usb_interface_protocol = intf->cur_altsetting->desc.bInterfaceProtocol;

    mutex_init(&dev->req_lock);
    init_completion(&dev->resp_done);

    /* The driver never writes to the device on its own: presets are
     * selector-only (curves live in per-preset slots on the device) and
     * custom bands are only sent on an explicit equalizer write, after the
     * custom preset is confirmed active. eq_bands is just a read fallback. */
    dev->eq_preset = BS_PRESET_CUSTOM;
    dev->idle_minutes = 0;
    dev->mic_level = 7; /* Synapse's observed resting level */

    /* Use the USB serial number if present, else a fixed fallback. */
    if (hdev->uniq[0])
        strscpy(dev->serial, hdev->uniq, sizeof(dev->serial));
    else
        strscpy(dev->serial, "BLACKSHARKV2PRO", sizeof(dev->serial));
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

    razer_blackshark_init(dev, intf, hdev);
    hid_set_drvdata(hdev, dev);
    dev_set_drvdata(&hdev->dev, dev);

    retval = hid_parse(hdev);
    if (retval) {
        hid_err(hdev, "parse failed\n");
        goto exit_free;
    }

    retval = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (retval) {
        hid_err(hdev, "hw start failed\n");
        goto exit_free;
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
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_level);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_status);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_equalizer);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_equalizer_preset);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_idle_time);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mic_monitoring);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_mic_monitoring_level);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_bt_dnd);
            break;
        }
    }

    usb_disable_autosuspend(interface_to_usbdev(intf));

    return 0;

exit_stop:
    hid_hw_stop(hdev);
exit_free:
    kfree(dev);
    return retval;
}

static void razer_blackshark_disconnect(struct hid_device *hdev)
{
    struct razer_blackshark_device *dev = hid_get_drvdata(hdev);

    if (dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE) {
        device_remove_file(&hdev->dev, &dev_attr_version);
        device_remove_file(&hdev->dev, &dev_attr_test);
        device_remove_file(&hdev->dev, &dev_attr_firmware_version);
        device_remove_file(&hdev->dev, &dev_attr_device_type);
        device_remove_file(&hdev->dev, &dev_attr_device_serial);

        switch (dev->usb_pid) {
        case USB_DEVICE_ID_RAZER_BLACKSHARK_V2_PRO_2_4:
            device_remove_file(&hdev->dev, &dev_attr_charge_level);
            device_remove_file(&hdev->dev, &dev_attr_charge_status);
            device_remove_file(&hdev->dev, &dev_attr_equalizer);
            device_remove_file(&hdev->dev, &dev_attr_equalizer_preset);
            device_remove_file(&hdev->dev, &dev_attr_device_idle_time);
            device_remove_file(&hdev->dev, &dev_attr_mic_monitoring);
            device_remove_file(&hdev->dev, &dev_attr_mic_monitoring_level);
            device_remove_file(&hdev->dev, &dev_attr_bt_dnd);
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
