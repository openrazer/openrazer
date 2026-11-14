# SPDX-License-Identifier: GPL-2.0-or-later

"""
Module for accessory methods
"""
from openrazer_daemon.dbus_services import endpoint
from openrazer_daemon.hardware.device_base import RazerDevice


@endpoint('razer.device.misc.mug', 'isMugPresent', out_sig='b')
def is_mug_present(self: RazerDevice) -> bool:
    """
    Get if the mug is present

    :return: True if there's a mug
    :rtype: bool
    """
    self.logger.debug("DBus call is_mug_present")

    driver_path = self.get_driver_path('is_mug_present')

    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip()) == 1
