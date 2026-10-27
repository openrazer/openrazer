# SPDX-License-Identifier: GPL-2.0-or-later

"""
Module for fan control methods
"""
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.fan', 'setFanRPM', in_sig='u')
def set_fan_rpm(self, rpm):
    """
    Set the fan speed (RPM) of a device

    Values are rounded to the nearest 50 RPM and clamped to 500-3200 by the
    kernel driver. A value of 0 releases control of the fans (i.e. turns
    them off).

    :param rpm: Fan speed in RPM
    :type rpm: int
    """
    self.logger.debug("DBus call set_fan_rpm")

    driver_path = self.get_driver_path('fan_rpm')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(rpm))


@endpoint('razer.device.fan', 'getFanRPM', out_sig='u')
def get_fan_rpm(self):
    """
    Get the last commanded fan speed (RPM) of a device

    Most devices do not provide a tachometer, so this reflects the last
    value written by the software. 0 means the fans are off (or their state
    is unknown).

    :return: Fan speed in RPM
    :rtype: int
    """
    self.logger.debug("DBus call get_fan_rpm")

    driver_path = self.get_driver_path('fan_rpm')

    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip())
