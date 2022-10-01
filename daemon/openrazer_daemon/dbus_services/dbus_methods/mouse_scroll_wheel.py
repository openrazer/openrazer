# SPDX-License-Identifier: GPL-2.0-or-later

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
