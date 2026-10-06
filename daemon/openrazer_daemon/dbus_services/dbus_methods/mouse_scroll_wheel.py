# SPDX-License-Identifier: GPL-2.0-or-later

from openrazer_daemon.dbus_services import endpoint

# Scroll modes supported per SCROLL_MODE_VERSION, indexed by the integer sent to setScrollMode
SCROLL_MODES_BY_VERSION = {
    1: ["tactile", "free_spin"],
    2: ["tactile", "free_spin", "precision_tactile"],
}


@endpoint('razer.device.scroll', 'setScrollMode', in_sig='y')
def set_scroll_mode(self, mode):
    """
    Set the device's scroll mode

    :param mode: The mode to set (0 = tactile, 1 = free spin, 2 = precision tactile)
    :type mode: int
    """
    self.logger.debug("DBus call set_scroll_mode")

    mode_max = len(SCROLL_MODES_BY_VERSION[self.SCROLL_MODE_VERSION]) - 1

    if mode < 0 or mode > mode_max:
        raise ValueError("mode has to be in the range of 0 and {0}".format(mode_max))

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


@endpoint('razer.device.scroll', 'getScrollModeOptions', out_sig='as')
def get_scroll_mode_options(self):
    """
    Get the scroll modes supported by the device

    :return: List of supported scroll modes, in the order accepted by setScrollMode
    :rtype: list of str
    """
    self.logger.debug("DBus call get_scroll_mode_options")

    return SCROLL_MODES_BY_VERSION[self.SCROLL_MODE_VERSION]


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
