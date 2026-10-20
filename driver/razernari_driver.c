// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Razer Nari & Nari Ultimate kernel driver for OpenRazer.
 *
 * Copyright (c) 2015 Terry Cain <terrys-home.co.uk>
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/delay.h>
#include <linux/random.h>
#include <linux/jiffies.h>

#include "razernari_driver.h"
#include "razercommon.h"

#define DRIVER_DESC "Razer Nari Headset Device Driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE(DRIVER_LICENSE);

#define NARI_ANIM_INTERVAL_MS 40

static unsigned int spectrum_period_ms = 24000;
module_param(spectrum_period_ms, uint, 0644);
MODULE_PARM_DESC(spectrum_period_ms, "Spectrum cycle period in milliseconds (default: 24000)");

static unsigned int spectrum_phase_ms = 0;
module_param(spectrum_phase_ms, uint, 0644);
MODULE_PARM_DESC(spectrum_phase_ms, "Spectrum phase offset in milliseconds (default: 0)");

static unsigned int breath_period_ms = 7000;
module_param(breath_period_ms, uint, 0644);
MODULE_PARM_DESC(breath_period_ms, "Breathing cycle period in milliseconds (default: 7000)");

static unsigned int wave_period_ms = 3800;
module_param(wave_period_ms, uint, 0644);
MODULE_PARM_DESC(wave_period_ms, "Wave cycle period in milliseconds (default: 3800)");

/**
 * Send a 64-byte Feature Report 0xFF to interface 5 (SET_REPORT).
 */
static int razer_nari_send_report(struct usb_device *usb_dev, const struct razer_nari_report *report)
{
    uint request = HID_REQ_SET_REPORT;
    uint request_type = USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT;
    char *buf;
    int len;

    buf = kmemdup(report->data, RAZER_NARI_REPORT_LEN, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    len = usb_control_msg(usb_dev, usb_sndctrlpipe(usb_dev, 0),
                          request, request_type,
                          RAZER_NARI_VALUE, RAZER_NARI_INTERFACE_INDEX,
                          buf, RAZER_NARI_REPORT_LEN,
                          USB_CTRL_SET_TIMEOUT);

    usleep_range(3000, 5000);
    kfree(buf);

    if (len != RAZER_NARI_REPORT_LEN) {
        dev_warn(&usb_dev->dev, "razernari: SET_REPORT failed (len=%d)\n", len);
        return (len < 0) ? len : -EIO;
    }

    return 0;
}

/**
 * Read a 64-byte Feature Report 0xFF from interface 5 (GET_REPORT).
 */
static int razer_nari_get_report(struct usb_device *usb_dev, struct razer_nari_report *report)
{
    uint request = HID_REQ_GET_REPORT;
    uint request_type = USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN;
    char *buf;
    int len;

    buf = kzalloc(RAZER_NARI_REPORT_LEN, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    buf[0] = RAZER_NARI_REPORT_ID;
    len = usb_control_msg(usb_dev, usb_rcvctrlpipe(usb_dev, 0),
                          request, request_type,
                          RAZER_NARI_VALUE, RAZER_NARI_INTERFACE_INDEX,
                          buf, RAZER_NARI_REPORT_LEN,
                          USB_CTRL_GET_TIMEOUT);

    if (len == RAZER_NARI_REPORT_LEN)
        memcpy(report->data, buf, RAZER_NARI_REPORT_LEN);

    kfree(buf);

    if (len != RAZER_NARI_REPORT_LEN) {
        dev_warn(&usb_dev->dev, "razernari: GET_REPORT failed (len=%d)\n", len);
        return (len < 0) ? len : -EIO;
    }

    return 0;
}

/**
 * Send a query report and poll GET_REPORT until the response is ready
 * (response->data[1] > 0x01 indicates response payload length).
 */
static int razer_nari_query(struct razer_nari_device *device,
                            const struct razer_nari_report *request,
                            struct razer_nari_report *response)
{
    int ret;
    int attempt;

    ret = razer_nari_send_report(device->usb_dev, request);
    if (ret)
        return ret;

    for (attempt = 0; attempt < 6; attempt++) {
        msleep(20);
        ret = razer_nari_get_report(device->usb_dev, response);
        if (ret)
            return ret;

        if (response->data[0] == RAZER_NARI_REPORT_ID && response->data[1] > 0x01)
            return 0;
    }

    return -ETIMEDOUT;
}

static struct razer_nari_report get_nari_base_report(unsigned char dir_flag, unsigned char cmd_class)
{
    struct razer_nari_report report = {0};

    report.data[0] = RAZER_NARI_REPORT_ID;
    report.data[1] = 0x0A;
    report.data[2] = 0x00;
    report.data[3] = dir_flag; /* 0xFF = set, 0xFD = get */
    report.data[4] = 0x04;
    report.data[5] = cmd_class;
    report.data[6] = 0xF1;

    return report;
}

static struct razer_nari_report get_nari_led_init_report(void)
{
    struct razer_nari_report report = get_nari_base_report(0xFF, 0x12);

    report.data[7] = 0x06;
    report.data[8] = 0x70;
    report.data[9] = 0x08;

    return report;
}

static struct razer_nari_report get_nari_led_brightness_report(unsigned char brightness)
{
    struct razer_nari_report report = get_nari_base_report(0xFF, 0x12);

    report.data[7] = 0x03;
    report.data[8] = 0x71;
    report.data[9] = brightness;

    return report;
}

static struct razer_nari_report get_nari_led_rgb_report(unsigned char red, unsigned char green, unsigned char blue)
{
    struct razer_nari_report report = get_nari_base_report(0xFF, 0x12);

    report.data[7] = 0x05;
    report.data[8] = 0x72;
    report.data[9] = red;
    report.data[10] = green;
    report.data[11] = blue;

    return report;
}

static struct razer_nari_report get_nari_haptic_report(unsigned char enable, unsigned char intensity)
{
    struct razer_nari_report report = get_nari_base_report(0xFF, 0x02);

    report.data[7] = 0x06;
    report.data[8] = 0x20;
    report.data[9] = enable ? 0x01 : 0x00;
    report.data[10] = intensity;

    return report;
}

/**
 * Compute the 6-phase Razer Chroma Spectrum Cycling color at elapsed_ms.
 * Starts at pure Violet (255, 0, 255) at t = 0 to match Razer Chroma hardware:
 * Violet -> Red -> Yellow -> Green -> Cyan -> Blue -> Violet.
 */
static void nari_spectrum_rgb(u64 elapsed_ms, unsigned int period_ms,
                             unsigned char *r, unsigned char *g, unsigned char *b)
{
    u64 phase_ms;
    u64 seg_ms;
    unsigned int seg;
    unsigned int prog;

    if (period_ms == 0)
        period_ms = 24000;

    phase_ms = (elapsed_ms + (u64)spectrum_phase_ms) % (u64)period_ms;
    seg_ms = (u64)period_ms / 6ULL;
    if (seg_ms == 0)
        seg_ms = 1;

    seg = (unsigned int)(phase_ms / seg_ms);
    if (seg > 5)
        seg = 5;

    prog = (unsigned int)(((phase_ms % seg_ms) * 255ULL) / seg_ms);
    if (prog > 255)
        prog = 255;

    switch (seg) {
    case 0: /* Violet (255, 0, 255) -> Red (255, 0, 0) */
        *r = 255;
        *g = 0;
        *b = (unsigned char)(255 - prog);
        break;
    case 1: /* Red (255, 0, 0) -> Yellow (255, 255, 0) via Orange */
        *r = 255;
        *g = (unsigned char)prog;
        *b = 0;
        break;
    case 2: /* Yellow (255, 255, 0) -> Green (0, 255, 0) */
        *r = (unsigned char)(255 - prog);
        *g = 255;
        *b = 0;
        break;
    case 3: /* Green (0, 255, 0) -> Cyan (0, 255, 255) */
        *r = 0;
        *g = 255;
        *b = (unsigned char)prog;
        break;
    case 4: /* Cyan (0, 255, 255) -> Blue (0, 0, 255) */
        *r = 0;
        *g = (unsigned char)(255 - prog);
        *b = 255;
        break;
    case 5: /* Blue (0, 0, 255) -> Violet (255, 0, 255) */
    default:
        *r = (unsigned char)prog;
        *g = 0;
        *b = 255;
        break;
    }
}

/**
 * Standard Razer Breathing brightness envelope (0..255).
 * Standard period is 7000 ms:
 * - 0..2200ms: fade in
 * - 2200..2600ms: full hold
 * - 2600..4800ms: fade out
 * - 4800..7000ms: dark pause
 */
static unsigned int nari_breath_level(u64 cycle_ms, unsigned int period_ms)
{
    u64 t_in = (period_ms * 31ULL) / 100ULL;
    u64 t_hold = t_in + (period_ms * 6ULL) / 100ULL;
    u64 t_out = t_hold + (period_ms * 31ULL) / 100ULL;

    if (cycle_ms < t_in) {
        return (unsigned int)((cycle_ms * 255ULL) / (t_in ? t_in : 1));
    } else if (cycle_ms < t_hold) {
        return 255;
    } else if (cycle_ms < t_out) {
        u64 rem = t_out - cycle_ms;
        u64 span = t_out - t_hold;
        return (unsigned int)((rem * 255ULL) / (span ? span : 1));
    } else {
        return 0;
    }
}

static void nari_pick_random_color(unsigned char *rgb)
{
    unsigned int hue = get_random_u32() % 360;

    nari_spectrum_rgb((u64)hue * 100ULL, 36000, &rgb[0], &rgb[1], &rgb[2]);
}

/**
 * Periodic workqueue handler that drives animated lighting effects
 * (Spectrum, Wave, Breathing, Reactive, Starlight) on the Razer Nari.
 * Timing is calculated from monotonic kernel time (ktime_get()) to guarantee
 * exact lockstep synchronization immune to USB control latency.
 */
static void razer_nari_effect_work_fn(struct work_struct *work)
{
    struct razer_nari_device *device = container_of(to_delayed_work(work),
                                       struct razer_nari_device,
                                       effect_work);
    unsigned char r = 0, g = 0, b = 0;
    bool send_color = false;
    bool reschedule = false;
    u64 elapsed_ms;

    mutex_lock(&device->lock);

    if (!device->led_on || device->led_brightness == 0) {
        mutex_unlock(&device->lock);
        return;
    }

    elapsed_ms = (u64)ktime_to_ms(ktime_sub(ktime_get(), device->effect_start_ktime));

    switch (device->effect_mode) {
    case NARI_EFFECT_SPECTRUM:
        nari_spectrum_rgb(elapsed_ms, spectrum_period_ms, &r, &g, &b);
        send_color = true;
        reschedule = true;
        break;

    case NARI_EFFECT_WAVE: {
        unsigned int w_period = wave_period_ms ? wave_period_ms : 3800;
        u64 wave_ms = elapsed_ms % (u64)w_period;

        if (device->wave_dir == 2)
            wave_ms = (u64)w_period - wave_ms;
        nari_spectrum_rgb(wave_ms, w_period, &r, &g, &b);
        send_color = true;
        reschedule = true;
        break;
    }

    case NARI_EFFECT_BREATH_SINGLE:
    case NARI_EFFECT_BREATH_DUAL:
    case NARI_EFFECT_BREATH_TRIPLE:
    case NARI_EFFECT_BREATH_RANDOM: {
        unsigned int b_period = breath_period_ms ? breath_period_ms : 7000;
        u64 cycle_idx = elapsed_ms / (u64)b_period;
        u64 cycle_ms = elapsed_ms % (u64)b_period;
        unsigned int level = nari_breath_level(cycle_ms, b_period);
        unsigned char base_r, base_g, base_b;

        if (device->anim_step != (unsigned int)cycle_idx) {
            device->anim_step = (unsigned int)cycle_idx;
            if (device->effect_mode == NARI_EFFECT_BREATH_RANDOM) {
                nari_pick_random_color(device->random_rgb);
            } else if (device->effect_mode == NARI_EFFECT_BREATH_DUAL) {
                device->breath_color_idx = (device->breath_color_idx + 1) % 2;
            } else if (device->effect_mode == NARI_EFFECT_BREATH_TRIPLE) {
                device->breath_color_idx = (device->breath_color_idx + 1) % 3;
            }
        }

        if (device->effect_mode == NARI_EFFECT_BREATH_RANDOM) {
            base_r = device->random_rgb[0];
            base_g = device->random_rgb[1];
            base_b = device->random_rgb[2];
        } else {
            unsigned int num_colors = (device->effect_mode == NARI_EFFECT_BREATH_TRIPLE) ? 3 :
                                      ((device->effect_mode == NARI_EFFECT_BREATH_DUAL) ? 2 : 1);
            unsigned int offset = (device->breath_color_idx % num_colors) * 3;

            base_r = device->colors[offset];
            base_g = device->colors[offset + 1];
            base_b = device->colors[offset + 2];
        }

        r = (unsigned char)((base_r * level) / 255);
        g = (unsigned char)((base_g * level) / 255);
        b = (unsigned char)((base_b * level) / 255);
        send_color = true;
        reschedule = true;
        break;
    }

    case NARI_EFFECT_REACTIVE: {
        u64 react_ms = (u64)ktime_to_ms(ktime_sub(ktime_get(), device->reactive_trigger_ktime));
        u64 duration = (u64)(device->speed ? device->speed : 2) * 500ULL;

        if (react_ms < duration) {
            unsigned int level = (unsigned int)(((duration - react_ms) * 255ULL) / duration);

            r = (unsigned char)((device->colors[0] * level) / 255);
            g = (unsigned char)((device->colors[1] * level) / 255);
            b = (unsigned char)((device->colors[2] * level) / 255);
            send_color = true;
            reschedule = true;
        } else if (device->reactive_active) {
            device->reactive_active = false;
            r = 0;
            g = 0;
            b = 0;
            send_color = true;
            reschedule = false;
        }
        break;
    }

    case NARI_EFFECT_STARLIGHT_SINGLE:
    case NARI_EFFECT_STARLIGHT_DUAL:
    case NARI_EFFECT_STARLIGHT_RANDOM: {
        unsigned int s_period = (device->speed == 1) ? 1200 : ((device->speed == 2) ? 1800 : 2500);
        u64 cycle_idx = elapsed_ms / (u64)s_period;
        u64 phase_ms = elapsed_ms % (u64)s_period;
        u64 half_ms = (u64)s_period / 2ULL;
        unsigned int level;
        unsigned char base_r, base_g, base_b;

        if (device->anim_step != (unsigned int)cycle_idx) {
            device->anim_step = (unsigned int)cycle_idx;
            if (device->effect_mode == NARI_EFFECT_STARLIGHT_RANDOM)
                nari_pick_random_color(device->random_rgb);
            else if (device->effect_mode == NARI_EFFECT_STARLIGHT_DUAL)
                device->breath_color_idx = (device->breath_color_idx + 1) % 2;
        }

        if (phase_ms < half_ms)
            level = (unsigned int)((phase_ms * 255ULL) / (half_ms ? half_ms : 1));
        else
            level = (unsigned int)(((s_period - phase_ms) * 255ULL) / (half_ms ? half_ms : 1));

        if (device->effect_mode == NARI_EFFECT_STARLIGHT_RANDOM) {
            base_r = device->random_rgb[0];
            base_g = device->random_rgb[1];
            base_b = device->random_rgb[2];
        } else {
            unsigned int offset = (device->breath_color_idx % 2) * 3;

            base_r = device->colors[offset];
            base_g = device->colors[offset + 1];
            base_b = device->colors[offset + 2];
        }

        r = (unsigned char)((base_r * level) / 255);
        g = (unsigned char)((base_g * level) / 255);
        b = (unsigned char)((base_b * level) / 255);
        send_color = true;
        reschedule = true;
        break;
    }

    default:
        break;
    }

    if (send_color) {
        struct razer_nari_report rgb_rep = get_nari_led_rgb_report(r, g, b);

        razer_nari_send_report(device->usb_dev, &rgb_rep);
    }

    mutex_unlock(&device->lock);

    if (reschedule)
        schedule_delayed_work(&device->effect_work, msecs_to_jiffies(NARI_ANIM_INTERVAL_MS));
}

static void nari_ensure_led_enabled_locked(struct razer_nari_device *device)
{
    struct razer_nari_report init_rep = get_nari_led_init_report();
    unsigned char target_br = device->led_brightness ? device->led_brightness : 0xFF;
    struct razer_nari_report br_rep = get_nari_led_brightness_report(target_br);

    device->led_brightness = target_br;
    device->led_on = 1;

    razer_nari_send_report(device->usb_dev, &init_rep);
    razer_nari_send_report(device->usb_dev, &br_rep);
}

static void nari_start_animated_effect(struct razer_nari_device *device, enum razer_nari_effect_mode mode)
{
    cancel_delayed_work_sync(&device->effect_work);

    mutex_lock(&device->lock);
    device->effect_mode = mode;
    device->anim_step = 0;
    device->breath_color_idx = 0;
    device->effect_start_ktime = ktime_get();
    if (mode == NARI_EFFECT_REACTIVE) {
        device->reactive_trigger_ktime = device->effect_start_ktime;
        device->reactive_active = true;
    }
    if (mode == NARI_EFFECT_BREATH_RANDOM || mode == NARI_EFFECT_STARLIGHT_RANDOM)
        nari_pick_random_color(device->random_rgb);
    nari_ensure_led_enabled_locked(device);
    mutex_unlock(&device->lock);

    schedule_delayed_work(&device->effect_work, 0);
}

static void nari_apply_led_brightness(struct razer_nari_device *device, unsigned char brightness)
{
    struct razer_nari_report init_rep = get_nari_led_init_report();
    struct razer_nari_report br_rep = get_nari_led_brightness_report(brightness);

    mutex_lock(&device->lock);
    device->led_brightness = brightness;
    device->led_on = (brightness > 0) ? 1 : 0;
    razer_nari_send_report(device->usb_dev, &init_rep);
    razer_nari_send_report(device->usb_dev, &br_rep);
    mutex_unlock(&device->lock);

    if (brightness > 0 && device->effect_mode >= NARI_EFFECT_SPECTRUM && device->effect_mode <= NARI_EFFECT_STARLIGHT_RANDOM)
        schedule_delayed_work(&device->effect_work, 0);
}

static void nari_apply_led_none(struct razer_nari_device *device)
{
    struct razer_nari_report br_rep = get_nari_led_brightness_report(0x00);

    cancel_delayed_work_sync(&device->effect_work);

    mutex_lock(&device->lock);
    device->effect_mode = NARI_EFFECT_NONE;
    device->led_on = 0;
    razer_nari_send_report(device->usb_dev, &br_rep);
    mutex_unlock(&device->lock);
}

static void nari_apply_led_static(struct razer_nari_device *device, unsigned char r, unsigned char g, unsigned char b)
{
    struct razer_nari_report rgb_rep = get_nari_led_rgb_report(r, g, b);

    cancel_delayed_work_sync(&device->effect_work);

    mutex_lock(&device->lock);
    device->effect_mode = NARI_EFFECT_STATIC;
    device->red = r;
    device->green = g;
    device->blue = b;
    nari_ensure_led_enabled_locked(device);
    razer_nari_send_report(device->usb_dev, &rgb_rep);
    mutex_unlock(&device->lock);
}

static ssize_t nari_handle_breath_write(struct razer_nari_device *device, const char *buf, size_t count)
{
    if (count == 1) {
        nari_start_animated_effect(device, NARI_EFFECT_BREATH_RANDOM);
    } else if (count == 3) {
        memcpy(&device->colors[0], buf, 3);
        nari_start_animated_effect(device, NARI_EFFECT_BREATH_SINGLE);
    } else if (count == 6) {
        memcpy(&device->colors[0], buf, 6);
        nari_start_animated_effect(device, NARI_EFFECT_BREATH_DUAL);
    } else if (count == 9) {
        memcpy(&device->colors[0], buf, 9);
        nari_start_animated_effect(device, NARI_EFFECT_BREATH_TRIPLE);
    } else {
        return -EINVAL;
    }

    return count;
}

static ssize_t nari_handle_reactive_write(struct razer_nari_device *device, const char *buf, size_t count)
{
    if (count != 4)
        return -EINVAL;

    device->speed = (unsigned char)buf[0];
    if (device->speed < 1 || device->speed > 4)
        device->speed = 2;
    device->colors[0] = (unsigned char)buf[1];
    device->colors[1] = (unsigned char)buf[2];
    device->colors[2] = (unsigned char)buf[3];

    nari_start_animated_effect(device, NARI_EFFECT_REACTIVE);
    return count;
}

static ssize_t nari_handle_starlight_write(struct razer_nari_device *device, const char *buf, size_t count)
{
    if (count == 1) {
        device->speed = (unsigned char)buf[0];
        nari_start_animated_effect(device, NARI_EFFECT_STARLIGHT_RANDOM);
    } else if (count == 4) {
        device->speed = (unsigned char)buf[0];
        memcpy(&device->colors[0], &buf[1], 3);
        nari_start_animated_effect(device, NARI_EFFECT_STARLIGHT_SINGLE);
    } else if (count == 7) {
        device->speed = (unsigned char)buf[0];
        memcpy(&device->colors[0], &buf[1], 6);
        nari_start_animated_effect(device, NARI_EFFECT_STARLIGHT_DUAL);
    } else {
        return -EINVAL;
    }

    return count;
}

static void nari_apply_haptic(struct razer_nari_device *device, unsigned char enable, unsigned char intensity)
{
    struct razer_nari_report report;

    if (intensity > 100)
        intensity = 100;

    device->haptic_enabled = enable ? 1 : 0;
    device->haptic_intensity = intensity;

    report = get_nari_haptic_report(device->haptic_enabled, device->haptic_intensity);

    mutex_lock(&device->lock);
    razer_nari_send_report(device->usb_dev, &report);
    mutex_unlock(&device->lock);
}

static void nari_update_battery(struct razer_nari_device *device)
{
    struct razer_nari_report req = get_nari_base_report(0xFD, 0x12);
    struct razer_nari_report resp = {0};

    req.data[7] = 0x02;
    req.data[8] = 0x05;

    mutex_lock(&device->lock);
    if (razer_nari_query(device, &req, &resp) == 0) {
        if (resp.data[1] == 0x0F && resp.data[4] == 0x12 && resp.data[8] == 0x05) {
            unsigned int pct = resp.data[14];

            if (pct <= 100) {
                device->charge_level = (unsigned char)((pct * 255u + 50u) / 100u);
                device->charge_status = (resp.data[11] == 0x01 || resp.data[9] == 0x01) ? 1 : 0;
            }
        }
    }
    mutex_unlock(&device->lock);
}

static bool nari_has_haptics(struct razer_nari_device *device)
{
    return (device->usb_pid == USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS ||
            device->usb_pid == USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRED);
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
    struct razer_nari_device *device = dev_get_drvdata(dev);
    char *device_type;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS:
        device_type = "Razer Nari Ultimate (Wireless)";
        break;

    case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRED:
        device_type = "Razer Nari Ultimate (Wired)";
        break;

    case USB_DEVICE_ID_RAZER_NARI_WIRELESS:
        device_type = "Razer Nari (Wireless)";
        break;

    case USB_DEVICE_ID_RAZER_NARI_WIRED:
        device_type = "Razer Nari (Wired)";
        break;

    default:
        device_type = "Unknown Device";
        break;
    }

    return sysfs_emit(buf, "%s\n", device_type);
}

/**
 * Read device file "device_serial"
 */
static ssize_t razer_attr_read_device_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    if (device->serial[0] == '\0') {
        struct razer_nari_report setup1 = {
            .data = {
                0xFF, 0x0A, 0x00, 0xFF, 0x04, 0x00, 0x00, 0x0C,
                0x00, 0x2E, 0xC4, 0x00, 0x7F, 0x00, 0x00, 0x60,
                0xC0, 0x10
            }
        };
        struct razer_nari_report setup2 = {
            .data = {
                0xFF, 0x0A, 0x00, 0xFF, 0x04, 0x00, 0x00, 0x02,
                0x84
            }
        };
        struct razer_nari_report req = {
            .data = {
                0xFF, 0x0A, 0x00, 0xFD, 0x04, 0x00, 0x00, 0x05,
                0x81, 0x60, 0xC0, 0x10
            }
        };
        struct razer_nari_report resp = {0};

        mutex_lock(&device->lock);
        razer_nari_send_report(device->usb_dev, &setup1);
        msleep(15);
        razer_nari_send_report(device->usb_dev, &setup2);
        msleep(15);

        if (razer_nari_query(device, &req, &resp) == 0 &&
            resp.data[1] == 0x1B &&
            resp.data[8] == 0x81 &&
            resp.data[9] == 0x60 &&
            resp.data[10] == 0xC0 &&
            resp.data[11] >= '0' && resp.data[11] <= 'Z') {
            memcpy(device->serial, &resp.data[11], RAZER_NARI_SERIAL_LEN);
            device->serial[RAZER_NARI_SERIAL_LEN] = '\0';
        } else {
            snprintf(device->serial, sizeof(device->serial), "NARI0000000%04X", device->usb_pid);
        }
        mutex_unlock(&device->lock);
    }

    return sysfs_emit(buf, "%s\n", device->serial);
}

/**
 * Read device file "firmware_version"
 */
static ssize_t razer_attr_read_firmware_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    if (!device->fw_loaded) {
        struct razer_nari_report req = get_nari_base_report(0xFD, 0x07);
        struct razer_nari_report resp = {0};

        req.data[7] = 0x05;
        req.data[8] = 0x02;
        req.data[9] = 0x05;

        mutex_lock(&device->lock);
        if (razer_nari_query(device, &req, &resp) == 0 &&
            resp.data[1] == 0x1B &&
            resp.data[4] == 0x07 &&
            resp.data[8] == 0x02 &&
            resp.data[9] >= '0' && resp.data[9] <= '9' &&
            resp.data[10] >= '0' && resp.data[10] <= '9' &&
            resp.data[11] >= '0' && resp.data[11] <= '9' &&
            resp.data[12] >= '0' && resp.data[12] <= '9') {
            device->fw_major = (resp.data[9] - '0') * 10 + (resp.data[10] - '0');
            device->fw_minor = (resp.data[11] - '0') * 10 + (resp.data[12] - '0');
        } else {
            device->fw_major = 1;
            device->fw_minor = 0;
        }
        device->fw_loaded = true;
        mutex_unlock(&device->lock);
    }

    return sysfs_emit(buf, "v%u.%u\n", device->fw_major, device->fw_minor);
}

/**
 * Write device file "test"
 */
static ssize_t razer_attr_write_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return count;
}

/**
 * Read device file "test"
 */
static ssize_t razer_attr_read_test(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "\n");
}

/**
 * Read device file "charge_level" (0..255)
 */
static ssize_t razer_attr_read_charge_level(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    nari_update_battery(device);
    return sysfs_emit(buf, "%u\n", device->charge_level);
}

/**
 * Read device file "charge_status" (0 or 1)
 */
static ssize_t razer_attr_read_charge_status(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    nari_update_battery(device);
    return sysfs_emit(buf, "%u\n", device->charge_status);
}

/**
 * Read device file "device_idle_time" (in seconds)
 */
static ssize_t razer_attr_read_device_idle_time(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    struct razer_nari_report req = get_nari_base_report(0xFD, 0x12);
    struct razer_nari_report resp = {0};

    req.data[7] = 0x02;
    req.data[8] = 0x41;

    mutex_lock(&device->lock);
    if (razer_nari_query(device, &req, &resp) == 0 &&
        resp.data[1] == 0x0C &&
        resp.data[4] == 0x12 &&
        resp.data[8] == 0x41) {
        unsigned short secs = resp.data[10] | ((unsigned short)resp.data[11] << 8);

        if (secs > 0 && secs <= 3600)
            device->idle_time = secs;
    }
    mutex_unlock(&device->lock);

    return sysfs_emit(buf, "%u\n", device->idle_time);
}

/**
 * Write device file "device_idle_time" (in seconds)
 */
static ssize_t razer_attr_write_device_idle_time(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    struct razer_nari_report report = get_nari_base_report(0xFF, 0x12);
    unsigned long secs;

    if (kstrtoul(buf, 10, &secs) != 0)
        return -EINVAL;
    if (secs > 3600)
        secs = 3600;

    device->idle_time = (unsigned short)secs;

    report.data[7] = 0x05;
    report.data[8] = 0x41;
    report.data[9] = (secs > 0) ? 0x01 : 0x00;
    report.data[10] = (unsigned char)(secs & 0xFF);
    report.data[11] = (unsigned char)((secs >> 8) & 0xFF);

    mutex_lock(&device->lock);
    razer_nari_send_report(device->usb_dev, &report);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Main zone brightness:
 * - On Nari Ultimate (0x051A, 0x051B): maps 0..255 to Haptic intensity (0..100).
 * - On Nari (0x051C, 0x051D): controls LED brightness (0..255).
 */
static ssize_t razer_attr_read_matrix_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    if (nari_has_haptics(device)) {
        unsigned int scaled = device->haptic_enabled
                              ? (unsigned int)device->haptic_intensity * 255u / 100u
                              : 0u;
        return sysfs_emit(buf, "%u\n", scaled);
    }

    return sysfs_emit(buf, "%u\n", device->led_brightness);
}

static ssize_t razer_attr_write_matrix_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    unsigned long v;

    if (kstrtoul(buf, 10, &v) != 0)
        return -EINVAL;
    if (v > 255)
        v = 255;

    if (nari_has_haptics(device)) {
        unsigned char intensity = (unsigned char)((v * 100u) / 255u);

        nari_apply_haptic(device, v > 0 ? 1 : 0, intensity);
    } else {
        nari_apply_led_brightness(device, (unsigned char)v);
    }

    return count;
}

static ssize_t razer_attr_write_matrix_effect_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    nari_apply_led_none(device);
    return count;
}

static ssize_t razer_attr_write_matrix_effect_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    if (count != 3 && count != 4) {
        dev_warn(dev, "razernari: static effect expects 3 bytes (RGB)\n");
        return -EINVAL;
    }

    nari_apply_led_static(device, (unsigned char)buf[0], (unsigned char)buf[1], (unsigned char)buf[2]);
    return count;
}

static ssize_t razer_attr_read_matrix_effect_static(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    buf[0] = device->red;
    buf[1] = device->green;
    buf[2] = device->blue;
    return 3;
}

static ssize_t razer_attr_write_matrix_effect_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    nari_start_animated_effect(device, NARI_EFFECT_SPECTRUM);
    return count;
}

static ssize_t razer_attr_write_matrix_effect_wave(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    if (count >= 1 && buf[0] == '2')
        device->wave_dir = 2;
    else
        device->wave_dir = 1;

    nari_start_animated_effect(device, NARI_EFFECT_WAVE);
    return count;
}

static ssize_t razer_attr_write_matrix_effect_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    return nari_handle_breath_write(device, buf, count);
}

static ssize_t razer_attr_write_matrix_effect_reactive(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    return nari_handle_reactive_write(device, buf, count);
}

static ssize_t razer_attr_write_matrix_reactive_trigger(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    if (device->effect_mode == NARI_EFFECT_REACTIVE) {
        mutex_lock(&device->lock);
        device->reactive_trigger_ktime = ktime_get();
        device->reactive_active = true;
        mutex_unlock(&device->lock);
        schedule_delayed_work(&device->effect_work, 0);
    }

    return count;
}

static ssize_t razer_attr_write_matrix_effect_starlight(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    return nari_handle_starlight_write(device, buf, count);
}

static ssize_t razer_attr_write_matrix_custom_frame(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    /* Format: [row_id, start_col, end_col, R0, G0, B0, ...] */
    if (count >= 6) {
        device->custom_rgb[0] = (unsigned char)buf[3];
        device->custom_rgb[1] = (unsigned char)buf[4];
        device->custom_rgb[2] = (unsigned char)buf[5];
    } else if (count >= 3) {
        device->custom_rgb[0] = (unsigned char)buf[0];
        device->custom_rgb[1] = (unsigned char)buf[1];
        device->custom_rgb[2] = (unsigned char)buf[2];
    }

    return count;
}

static ssize_t razer_attr_write_matrix_effect_custom(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    struct razer_nari_report rgb_rep;

    cancel_delayed_work_sync(&device->effect_work);

    mutex_lock(&device->lock);
    device->effect_mode = NARI_EFFECT_CUSTOM;
    device->red = device->custom_rgb[0];
    device->green = device->custom_rgb[1];
    device->blue = device->custom_rgb[2];
    nari_ensure_led_enabled_locked(device);
    rgb_rep = get_nari_led_rgb_report(device->red, device->green, device->blue);
    razer_nari_send_report(device->usb_dev, &rgb_rep);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Logo zone attributes (used on Nari Ultimate so Haptics and Logo LED have distinct zones)
 */
static ssize_t razer_attr_read_logo_led_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    return sysfs_emit(buf, "%u\n", device->led_brightness);
}

static ssize_t razer_attr_write_logo_led_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    unsigned long v;

    if (kstrtoul(buf, 10, &v) != 0)
        return -EINVAL;
    if (v > 255)
        v = 255;

    nari_apply_led_brightness(device, (unsigned char)v);
    return count;
}

static ssize_t razer_attr_read_logo_led_state(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    return sysfs_emit(buf, "%u\n", device->led_on ? 1u : 0u);
}

static ssize_t razer_attr_write_logo_led_state(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    if (count < 1)
        return -EINVAL;

    if (buf[0] != '0') {
        unsigned char br = device->led_brightness ? device->led_brightness : 0xFF;

        nari_apply_led_brightness(device, br);
    } else {
        nari_apply_led_none(device);
    }

    return count;
}

static ssize_t razer_attr_write_logo_matrix_effect_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    nari_apply_led_none(device);
    return count;
}

static ssize_t razer_attr_write_logo_matrix_effect_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    if (count != 3 && count != 4) {
        dev_warn(dev, "razernari: static effect expects 3 bytes (RGB)\n");
        return -EINVAL;
    }

    nari_apply_led_static(device, (unsigned char)buf[0], (unsigned char)buf[1], (unsigned char)buf[2]);
    return count;
}

static ssize_t razer_attr_read_logo_matrix_effect_static(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    buf[0] = device->red;
    buf[1] = device->green;
    buf[2] = device->blue;
    return 3;
}

static ssize_t razer_attr_write_logo_matrix_effect_spectrum(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    nari_start_animated_effect(device, NARI_EFFECT_SPECTRUM);
    return count;
}

static ssize_t razer_attr_write_logo_matrix_effect_wave(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    if (count >= 1 && buf[0] == '2')
        device->wave_dir = 2;
    else
        device->wave_dir = 1;

    nari_start_animated_effect(device, NARI_EFFECT_WAVE);
    return count;
}

static ssize_t razer_attr_write_logo_matrix_effect_breath(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    return nari_handle_breath_write(device, buf, count);
}

static ssize_t razer_attr_write_logo_matrix_effect_reactive(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    return nari_handle_reactive_write(device, buf, count);
}

static DEVICE_ATTR(test,                       0660, razer_attr_read_test,                       razer_attr_write_test);
static DEVICE_ATTR(version,                    0440, razer_attr_read_version,                    NULL);
static DEVICE_ATTR(device_type,                0440, razer_attr_read_device_type,                NULL);
static DEVICE_ATTR(device_serial,              0440, razer_attr_read_device_serial,              NULL);
static DEVICE_ATTR(firmware_version,           0440, razer_attr_read_firmware_version,           NULL);
static DEVICE_ATTR(charge_level,               0440, razer_attr_read_charge_level,               NULL);
static DEVICE_ATTR(charge_status,              0440, razer_attr_read_charge_status,              NULL);
static DEVICE_ATTR(device_idle_time,           0660, razer_attr_read_device_idle_time,           razer_attr_write_device_idle_time);

static DEVICE_ATTR(matrix_brightness,          0660, razer_attr_read_matrix_brightness,          razer_attr_write_matrix_brightness);
static DEVICE_ATTR(matrix_effect_none,         0220, NULL,                                       razer_attr_write_matrix_effect_none);
static DEVICE_ATTR(matrix_effect_static,       0660, razer_attr_read_matrix_effect_static,       razer_attr_write_matrix_effect_static);
static DEVICE_ATTR(matrix_effect_spectrum,     0220, NULL,                                       razer_attr_write_matrix_effect_spectrum);
static DEVICE_ATTR(matrix_effect_wave,         0220, NULL,                                       razer_attr_write_matrix_effect_wave);
static DEVICE_ATTR(matrix_effect_breath,       0220, NULL,                                       razer_attr_write_matrix_effect_breath);
static DEVICE_ATTR(matrix_effect_reactive,     0220, NULL,                                       razer_attr_write_matrix_effect_reactive);
static DEVICE_ATTR(matrix_reactive_trigger,    0220, NULL,                                       razer_attr_write_matrix_reactive_trigger);
static DEVICE_ATTR(matrix_effect_starlight,    0220, NULL,                                       razer_attr_write_matrix_effect_starlight);
static DEVICE_ATTR(matrix_custom_frame,        0220, NULL,                                       razer_attr_write_matrix_custom_frame);
static DEVICE_ATTR(matrix_effect_custom,       0220, NULL,                                       razer_attr_write_matrix_effect_custom);

static DEVICE_ATTR(logo_led_brightness,        0660, razer_attr_read_logo_led_brightness,        razer_attr_write_logo_led_brightness);
static DEVICE_ATTR(logo_led_state,             0660, razer_attr_read_logo_led_state,             razer_attr_write_logo_led_state);
static DEVICE_ATTR(logo_matrix_effect_none,    0220, NULL,                                       razer_attr_write_logo_matrix_effect_none);
static DEVICE_ATTR(logo_matrix_effect_static,  0660, razer_attr_read_logo_matrix_effect_static,  razer_attr_write_logo_matrix_effect_static);
static DEVICE_ATTR(logo_matrix_effect_spectrum, 0220, NULL,                                      razer_attr_write_logo_matrix_effect_spectrum);
static DEVICE_ATTR(logo_matrix_effect_wave,    0220, NULL,                                       razer_attr_write_logo_matrix_effect_wave);
static DEVICE_ATTR(logo_matrix_effect_breath,  0220, NULL,                                       razer_attr_write_logo_matrix_effect_breath);
static DEVICE_ATTR(logo_matrix_effect_reactive, 0220, NULL,                                      razer_attr_write_logo_matrix_effect_reactive);

static void razer_nari_init(struct razer_nari_device *dev, struct usb_interface *intf, struct hid_device *hdev)
{
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    mutex_init(&dev->lock);
    INIT_DELAYED_WORK(&dev->effect_work, razer_nari_effect_work_fn);

    dev->usb_dev = usb_dev;
    dev->usb_interface_protocol = intf->cur_altsetting->desc.bInterfaceProtocol;
    dev->usb_vid = usb_dev->descriptor.idVendor;
    dev->usb_pid = usb_dev->descriptor.idProduct;
    dev->hid_dev = hdev;

    dev->green = 0xFF;
    dev->colors[1] = 0xFF;
    dev->led_brightness = 0xFF;
    dev->led_on = 1;
    dev->effect_mode = NARI_EFFECT_STATIC;
    dev->wave_dir = 1;
    dev->speed = 2;
    dev->charge_level = 255;
    dev->charge_status = 0;
    dev->idle_time = RAZER_NARI_DEFAULT_IDLE_TIME;
    dev->effect_start_ktime = ktime_get();
}

static int razer_nari_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int retval = 0;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_nari_device *dev = NULL;

    dev = kzalloc_obj(*dev);
    if (dev == NULL) {
        hid_err(hdev, "out of memory\n");
        return -ENOMEM;
    }

    razer_nari_init(dev, intf, hdev);

    if (dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE) {
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_test);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_firmware_version);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_level);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_charge_status);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_idle_time);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_brightness);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_custom_frame);
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_custom);

        switch (dev->usb_pid) {
        case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS:
        case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRED:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_brightness);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_led_state);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_wave);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_logo_matrix_effect_reactive);
            break;

        case USB_DEVICE_ID_RAZER_NARI_WIRELESS:
        case USB_DEVICE_ID_RAZER_NARI_WIRED:
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_wave);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_breath);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_reactive);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_reactive_trigger);
            CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_starlight);
            break;
        }
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
    cancel_delayed_work_sync(&dev->effect_work);
    kfree(dev);
    return retval;
}

static void razer_nari_disconnect(struct hid_device *hdev)
{
    struct razer_nari_device *dev;

    dev = hid_get_drvdata(hdev);
    cancel_delayed_work_sync(&dev->effect_work);

    if (dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE) {
        device_remove_file(&hdev->dev, &dev_attr_version);
        device_remove_file(&hdev->dev, &dev_attr_test);
        device_remove_file(&hdev->dev, &dev_attr_device_type);
        device_remove_file(&hdev->dev, &dev_attr_device_serial);
        device_remove_file(&hdev->dev, &dev_attr_firmware_version);
        device_remove_file(&hdev->dev, &dev_attr_charge_level);
        device_remove_file(&hdev->dev, &dev_attr_charge_status);
        device_remove_file(&hdev->dev, &dev_attr_device_idle_time);
        device_remove_file(&hdev->dev, &dev_attr_matrix_brightness);
        device_remove_file(&hdev->dev, &dev_attr_matrix_custom_frame);
        device_remove_file(&hdev->dev, &dev_attr_matrix_effect_custom);

        switch (dev->usb_pid) {
        case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS:
        case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRED:
            device_remove_file(&hdev->dev, &dev_attr_logo_led_brightness);
            device_remove_file(&hdev->dev, &dev_attr_logo_led_state);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_logo_matrix_effect_reactive);
            break;

        case USB_DEVICE_ID_RAZER_NARI_WIRELESS:
        case USB_DEVICE_ID_RAZER_NARI_WIRED:
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_spectrum);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_wave);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_breath);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_reactive);
            device_remove_file(&hdev->dev, &dev_attr_matrix_reactive_trigger);
            device_remove_file(&hdev->dev, &dev_attr_matrix_effect_starlight);
            break;
        }
    }

    hid_hw_stop(hdev);
    kfree(dev);
    hid_info(hdev, "Razer Nari disconnected\n");
}

static const struct hid_device_id razer_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRED) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_NARI_WIRELESS) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_NARI_WIRED) },
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
