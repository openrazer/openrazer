# SPDX-License-Identifier: GPL-2.0-or-later

import math
import struct

from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.scroll', 'setScrollMode', in_sig='y')
def set_scroll_mode(self, mode):
    """
    Set the device's scroll mode

    :param mode: The mode to set (0 = tactile, 1 = free spin)
    :type mode: int
    """
    self.logger.debug("DBus call set_scroll_mode")

    if mode not in (0, 1):
        raise ValueError("mode has to be 0 or 1")

    driver_path = self.get_driver_path('scroll_mode')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(int(mode)))


@endpoint('razer.device.scroll', 'getScrollMode', out_sig='y')
def get_scroll_mode(self):
    """
    Get the device's current scroll mode

    :return: The device's current scroll mode (0 = tactile, 1 = free spin)
    :rtype: int
    """
    self.logger.debug("DBus call get_scroll_mode")

    driver_path = self.get_driver_path('scroll_mode')

    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip())


@endpoint('razer.device.scroll', 'setScrollAcceleration', in_sig='b')
def set_scroll_acceleration(self, enabled):
    """
    Set the device's scroll acceleration state

    :param enabled: true to enable acceleration, false to disable it
    :type enabled: bool
    """
    self.logger.debug("DBus call set_scroll_acceleration")

    driver_path = self.get_driver_path('scroll_acceleration')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(int(enabled)))


@endpoint('razer.device.scroll', 'getScrollAcceleration', out_sig='b')
def get_scroll_acceleration(self):
    """
    Get the device's scroll acceleration state

    :return: true if acceleration enabled, false otherwise
    :rtype: bool
    """
    self.logger.debug("DBus call get_scroll_acceleration")

    driver_path = self.get_driver_path('scroll_acceleration')

    with open(driver_path, 'r') as driver_file:
        return bool(int(driver_file.read().strip()))


@endpoint('razer.device.scroll', 'setScrollSmartReel', in_sig='b')
def set_scroll_smart_reel(self, enabled):
    """
    Set the device's "smart reel" state

    :param enabled: true to enable smart reel, false to disable it
    :type enabled: bool
    """
    self.logger.debug("DBus call set_scroll_smart_reel")

    driver_path = self.get_driver_path('scroll_smart_reel')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(int(enabled)))


@endpoint('razer.device.scroll', 'getScrollSmartReel', out_sig='b')
def get_scroll_smart_reel(self):
    """
    Get the device's "smart reel" state

    :return: true if smart reel enabled, false otherwise
    :rtype: bool
    """
    self.logger.debug("DBus call get_scroll_smart_reel")

    driver_path = self.get_driver_path('scroll_smart_reel')

    with open(driver_path, 'r') as driver_file:
        return bool(int(driver_file.read().strip()))


@endpoint('razer.device.scroll', 'setScrollProfile', in_sig='y')
def set_scroll_profile(self, profile):
    """
    Set the device's HyperScroll profile

    The device rejects a profile that is not enabled, see setScrollProfilesEnabled.

    :param profile: The profile to set (1 = standard, 2 = distinct, 3 = precise, 4 = adaptive, 5 = smooth, 6 = custom)
    :type profile: int
    """
    self.logger.debug("DBus call set_scroll_profile")

    if profile not in range(1, 7):
        raise ValueError("profile has to be between 1 and 6")

    driver_path = self.get_driver_path('scroll_profile')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(int(profile)))


@endpoint('razer.device.scroll', 'getScrollProfile', out_sig='y')
def get_scroll_profile(self):
    """
    Get the device's current HyperScroll profile

    :return: The current profile (1 = standard, 2 = distinct, 3 = precise, 4 = adaptive, 5 = smooth, 6 = custom)
    :rtype: int
    """
    self.logger.debug("DBus call get_scroll_profile")

    driver_path = self.get_driver_path('scroll_profile')

    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip())


@endpoint('razer.device.scroll', 'setScrollProfilesEnabled', in_sig='y')
def set_scroll_profiles_enabled(self, mask):
    """
    Set which HyperScroll profiles are enabled

    :param mask: Bitmask where bit n-1 enables profile n, at least one bit set
    :type mask: int
    """
    self.logger.debug("DBus call set_scroll_profiles_enabled")

    if mask not in range(1, 0x40):
        raise ValueError("mask has to be between 1 and 63")

    driver_path = self.get_driver_path('scroll_profiles_enabled')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(int(mask)))


@endpoint('razer.device.scroll', 'getScrollProfilesEnabled', out_sig='y')
def get_scroll_profiles_enabled(self):
    """
    Get which HyperScroll profiles are enabled

    :return: Bitmask where bit n-1 is set if profile n is enabled
    :rtype: int
    """
    self.logger.debug("DBus call get_scroll_profiles_enabled")

    driver_path = self.get_driver_path('scroll_profiles_enabled')

    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip())


SCROLL_NOTCH_STRENGTH_MAX = 0.2  # Synapse's "scroll tension" 100 with the peak at the top of the graph
SCROLL_NOTCH_WIDTH_MIN = 360 / 96  # Synapse's "scroll steps" per turn, 8 to 96
SCROLL_NOTCH_WIDTH_MAX = 360 / 8
SCROLL_CURVE_MAX_POINTS = 80


def _check_scroll_curve(name, points, y_max, notch):
    if len(points) not in range(2, SCROLL_CURVE_MAX_POINTS + 1):
        raise ValueError("{0} curve needs between 2 and {1} points".format(name, SCROLL_CURVE_MAX_POINTS))
    if not all(math.isfinite(x) for x, _ in points):
        raise ValueError("{0} curve x values have to be finite".format(name))
    if any(b[0] <= a[0] for a, b in zip(points, points[1:])):
        raise ValueError("{0} curve x values have to be strictly ascending".format(name))
    if any(not 0 <= y <= y_max for _, y in points):
        raise ValueError("{0} curve y values have to be between 0 and {1}".format(name, y_max))
    if notch and points[0][0] != 0:
        raise ValueError("{0} curve has to start at x = 0".format(name))
    if notch and not SCROLL_NOTCH_WIDTH_MIN <= points[-1][0] <= SCROLL_NOTCH_WIDTH_MAX:
        raise ValueError("{0} curve has to end between {1} and {2} degrees".format(name, SCROLL_NOTCH_WIDTH_MIN, SCROLL_NOTCH_WIDTH_MAX))


@endpoint('razer.device.scroll', 'setScrollCustomProfile', in_sig='a(dd)a(dd)a(dd)')
def set_scroll_custom_profile(self, up, down, speed):
    """
    Set the detent curves of the custom HyperScroll profile (6)

    A notch spans from x = 0 to the last point's x, in degrees of wheel rotation: 360 / 96 to
    360 / 8, matching Synapse's 8 to 96 steps per turn. Up and down should end at the same x.

    :param up: (degrees, strength) points for scrolling up; strength 0 to 0.2
    :type up: list of (float, float)

    :param down: (degrees, strength) points for scrolling down
    :type down: list of (float, float)

    :param speed: (signed wheel speed, multiplier) points; a multiplier of 1 leaves the strength unchanged, 0 removes the notch
    :type speed: list of (float, float)
    """
    self.logger.debug("DBus call set_scroll_custom_profile")

    up = [(float(x), float(y)) for x, y in up]
    down = [(float(x), float(y)) for x, y in down]
    speed = [(float(x), float(y)) for x, y in speed]

    _check_scroll_curve('up', up, SCROLL_NOTCH_STRENGTH_MAX, True)
    _check_scroll_curve('down', down, SCROLL_NOTCH_STRENGTH_MAX, True)
    _check_scroll_curve('speed', speed, 1.0, False)
    if up[-1][0] != down[-1][0]:
        raise ValueError("up and down curves have to end at the same x")

    payload = struct.pack('<BBB', len(up), len(down), len(speed))
    for x, y in up + down + speed:
        payload += struct.pack('<ff', x, y)

    driver_path = self.get_driver_path('scroll_custom_profile')

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)
