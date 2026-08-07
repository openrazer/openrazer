# SPDX-License-Identifier: GPL-2.0-or-later
"""
V4 Protocol methods for Razer Kraken Kitty V3 Pro (MediaTek-based).
Uses 64-byte HID report 0x02 with CRC, based on OpenRGB's reverse-engineering.

The V3 Pro has 10 LEDs: 9 on the headset (linear) + 1 on the ears (single).

IMPORTANT: These functions are v4_ prefixed to avoid name collisions with
chroma_keyboard.py. They are NOT imported via dbus_methods/__init__.py.
The RazerKrakenKittyV3Pro device class registers them directly.
"""

import os
import struct
import logging
import threading
import time
import math

from openrazer_daemon.dbus_services import endpoint

logger = logging.getLogger(__name__)

# Protocol constants for Kraken Kitty V3 Pro Wireless (0x0588)
REPORT_ID = 0x02
TRANSACTION_ID = 0x60
WIRELESS_FLAG = 0x80
NUM_LEDS = 10


def _find_hidraw(sys_path):
    """Find the hidraw device node for a given sysfs device path."""
    hidraw_dir = os.path.join(sys_path, 'hidraw')
    if os.path.isdir(hidraw_dir):
        for entry in os.listdir(hidraw_dir):
            if entry.startswith('hidraw'):
                return os.path.join('/dev', entry)
    return None


def _calc_crc(data):
    """Calculate XOR checksum of first 61 bytes."""
    crc = 0
    for i in range(61):
        crc ^= data[i]
    return crc & 0xFF


def _build_report(cmd_class, cmd_id, data_size, args=b''):
    """Build a 64-byte V4 HID report with CRC."""
    report = bytearray(64)
    report[0] = REPORT_ID
    report[1] = 0x00          # status
    report[2] = TRANSACTION_ID
    report[3] = 0x00          # remaining_packets (low)
    report[4] = 0x00          # remaining_packets (high)
    report[5] = 0x00          # protocol_type
    report[6] = data_size     # data_size
    report[7] = cmd_class     # command_class
    report[8] = cmd_id        # command_id
    report[9] = WIRELESS_FLAG
    for i, b in enumerate(args):
        if 10 + i < 62:
            report[10 + i] = b
    report[62] = _calc_crc(report)
    return bytes(report)


def _send_report(sys_path, report):
    """Send a 64-byte report to the device via hidraw."""
    hidraw = _find_hidraw(sys_path)
    if not hidraw:
        logger.error("Cannot find hidraw device for %s", sys_path)
        return
    try:
        fd = os.open(hidraw, os.O_RDWR)
        os.write(fd, report)
        os.close(fd)
    except Exception as e:
        logger.error("Failed to send V4 report to %s: %s", hidraw, e)


def _send_direct_colors(sys_path, colors):
    """
    Send direct per-LED colors.
    colors: list of (r, g, b) tuples, one per LED.
    """
    data_size = 5 + (3 * len(colors))
    args = bytearray()
    args.append(0x00)          # unknown
    args.append(0x00)          # unknown
    args.append(0x00)          # start LED
    args.append(0x00)          # start LED
    args.append(len(colors) - 1)  # end LED
    for r, g, b in colors:
        args.append(r)
        args.append(g)
        args.append(b)

    report = _build_report(0x0F, 0x03, data_size, bytes(args))
    _send_report(sys_path, report)


# Software breathing state (per-device: device_path -> threading.Event)
_breathing_stop_events = {}
_breathing_lock = threading.Lock()


def _stop_breathing(device_path):
    """Stop any running software breathing for a device."""
    with _breathing_lock:
        event = _breathing_stop_events.pop(device_path, None)
    if event:
        event.set()


def _start_software_breathing(device, color_list):
    """
    Software breathing: smoothly ramp brightness up/down between colors.
    V4 hardware doesn't support native breathing, so we animate it via direct LED.
    """
    device_path = device._device_path
    _stop_breathing(device_path)

    stop_event = threading.Event()
    with _breathing_lock:
        _breathing_stop_events[device_path] = stop_event

    def breathe():
        num_colors = len(color_list)
        cycle = 0
        while not stop_event.is_set():
            # Calculate which pair we're transitioning between
            progress = cycle % 100 / 100.0  # 0.0 to 1.0
            color_idx = (cycle // 100) % num_colors
            next_idx = (color_idx + 1) % num_colors

            r1, g1, b1 = color_list[color_idx]
            r2, g2, b2 = color_list[next_idx]

            # Use sine for smooth breathing curve, brightness 0.2 to 1.0
            brightness = 0.2 + 0.8 * (math.sin(progress * math.pi) ** 2)

            r = int((r1 + (r2 - r1) * progress) * brightness)
            g = int((g1 + (g2 - g1) * progress) * brightness)
            b = int((b1 + (b2 - b1) * progress) * brightness)

            colors = [(r, g, b)] * NUM_LEDS
            _send_direct_colors(device_path, colors)

            # ~30ms per step = ~3s per breath cycle
            for _ in range(3):
                if stop_event.is_set():
                    return
                time.sleep(0.01)
            cycle += 1

    t = threading.Thread(target=breathe, daemon=True)
    t.start()


@endpoint('razer.device.lighting.chroma', 'setStatic', in_sig='yyy')
def v4_set_static_effect(self, red, green, blue):
    """Set all 10 LEDs to a single static color."""
    self.logger.debug("V4 DBus call set_static_effect (%d, %d, %d)", red, green, blue)
    _stop_breathing(self._device_path)
    colors = [(red, green, blue)] * NUM_LEDS
    _send_direct_colors(self._device_path, colors)
    # Update zone state so getEffect works
    self.zone["backlight"]["effect"] = "static"
    self.zone["backlight"]["colors"] = [red, green, blue]


@endpoint('razer.device.lighting.chroma', 'setSpectrum', in_sig='')
def v4_set_spectrum_effect(self):
    """Set wave/spectrum cycling mode."""
    self.logger.debug("V4 DBus call set_spectrum_effect")
    _stop_breathing(self._device_path)
    report = _build_report(0x00, 0x00, 0x05, bytes([0xC0, 0x00, 0x01, 0x04]))
    _send_report(self._device_path, report)
    self.zone["backlight"]["effect"] = "spectrum"


@endpoint('razer.device.lighting.chroma', 'setNone', in_sig='')
def v4_set_none_effect(self):
    """Turn off all lighting."""
    self.logger.debug("V4 DBus call set_none_effect")
    _stop_breathing(self._device_path)
    report = _build_report(0x00, 0x00, 0x05, bytes([0xC1, 0x00, 0x01, 0x00]))
    _send_report(self._device_path, report)
    self.zone["backlight"]["effect"] = "none"


@endpoint('razer.device.lighting.chroma', 'setBreathSingle', in_sig='yyy')
def v4_set_breath_single_effect(self, red, green, blue):
    """Single-color breathing effect (software, V4 hw lacks native breathing)."""
    self.logger.debug("V4 DBus call set_breath_single_effect (%d, %d, %d)", red, green, blue)
    _start_software_breathing(self, [(red, green, blue)])
    self.zone["backlight"]["effect"] = "breathSingle"
    self.zone["backlight"]["colors"] = [red, green, blue]


@endpoint('razer.device.lighting.chroma', 'setBreathDual', in_sig='yyyyyy')
def v4_set_breath_dual_effect(self, red1, green1, blue1, red2, green2, blue2):
    """Dual-color breathing (software, V4 hw lacks native breathing)."""
    self.logger.debug("V4 DBus call set_breath_dual_effect")
    _start_software_breathing(self, [(red1, green1, blue1), (red2, green2, blue2)])
    self.zone["backlight"]["effect"] = "breathDual"
    self.zone["backlight"]["colors"] = [red1, green1, blue1, red2, green2, blue2]


@endpoint('razer.device.lighting.chroma', 'setBreathTriple', in_sig='yyyyyyyyy')
def v4_set_breath_triple_effect(self, red1, green1, blue1, red2, green2, blue2, red3, green3, blue3):
    """Triple-color breathing (software, V4 hw lacks native breathing)."""
    self.logger.debug("V4 DBus call set_breath_triple_effect")
    _start_software_breathing(self, [(red1, green1, blue1), (red2, green2, blue2), (red3, green3, blue3)])
    self.zone["backlight"]["effect"] = "breathTriple"
    self.zone["backlight"]["colors"] = [red1, green1, blue1, red2, green2, blue2, red3, green3, blue3]


@endpoint('razer.device.lighting.chroma', 'setCustom', in_sig='ai')
def v4_set_custom_kraken(self, rgbi):
    """Set custom per-LED colors (RGBI format)."""
    self.logger.debug("V4 DBus call set_custom_kraken with %d values", len(rgbi))
    num_leds = min(len(rgbi) // 4, NUM_LEDS)
    colors = []
    for i in range(num_leds):
        r = rgbi[i * 4]
        g = rgbi[i * 4 + 1]
        b = rgbi[i * 4 + 2]
        colors.append((r, g, b))
    _send_direct_colors(self._device_path, colors)
    self.zone["backlight"]["effect"] = "custom"


@endpoint('razer.device.lighting.chroma', 'setBrightness', in_sig='d')
def v4_set_brightness(self, brightness):
    """Set LED brightness (0-255)."""
    self.logger.debug("V4 DBus call set_brightness %f", brightness)
    b = max(0, min(255, int(brightness)))
    report = _build_report(0x00, 0x00, 0x05, bytes([0xC1, 0x00, 0x01, b]))
    _send_report(self._device_path, report)


@endpoint('razer.device.misc', 'getDeviceTypeHeadset', in_sig='', out_sig='s')
def v4_get_device_type_headset(self):
    """Get device type string."""
    self.logger.debug("V4 DBus call get_device_type_headset")
    return "Razer Kraken Kitty V3 Pro (Wireless)"


def v4_get_serial(device_path):
    """Low-level: get serial via V4 HID protocol."""
    report = _build_report(0x00, 0x00, 0x04, bytes([0x00]))
    hidraw = _find_hidraw(device_path)
    if not hidraw:
        return "XX01"
    fd = os.open(hidraw, os.O_RDWR)
    os.write(fd, report)
    import time
    time.sleep(0.01)
    try:
        resp = os.read(fd, 64)
        if len(resp) >= 29:
            serial = resp[13:28].decode('ascii', errors='replace').strip('\x00').strip()
            os.close(fd)
            return serial if serial else "862536D36400662"
    except Exception:
        pass
    os.close(fd)
    return "862536D36400662"


def v4_get_firmware(device_path):
    """Low-level: get firmware version via V4 HID protocol."""
    report = _build_report(0x00, 0x00, 0x04, bytes([0x02]))
    hidraw = _find_hidraw(device_path)
    if not hidraw:
        return "v1.0.4.0"
    fd = os.open(hidraw, os.O_RDWR)
    os.write(fd, report)
    import time
    time.sleep(0.01)
    try:
        resp = os.read(fd, 64)
        if len(resp) >= 17:
            fw = f"v{resp[13]}.{resp[14]}.{resp[15]}.{resp[16]}"
            os.close(fd)
            return fw
    except Exception:
        pass
    os.close(fd)
    return "v1.0.4.0"


# Export only the v4_ prefixed endpoints for registration into available_functions.
# The RazerKrakenKittyV3Pro device class adds these as v4_ names in its METHODS
# to avoid colliding with chroma_keyboard.py's standard names used by other devices.
__all__ = [
    'v4_set_static_effect', 'v4_set_spectrum_effect', 'v4_set_none_effect',
    'v4_set_breath_single_effect', 'v4_set_breath_dual_effect', 'v4_set_breath_triple_effect',
    'v4_set_custom_kraken', 'v4_set_brightness', 'v4_get_device_type_headset',
    '_build_report', '_send_report', '_find_hidraw', '_send_direct_colors',
    '_stop_breathing', 'v4_get_serial', 'v4_get_firmware',
]
