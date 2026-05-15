# SPDX-License-Identifier: GPL-2.0-or-later

"""
Module for accessory methods
"""
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.misc', 'setMouseDockProPair', in_sig='s')
def set_mouse_dock_pro_pair(self, pid):
    """
    Pair Mouse Dock Pro with a mouse by PID (hex string, e.g. "00ab")

    :param pid: product id
    :type pid: str
    """
    self.logger.debug("DBus call set_mouse_dock_pro_pair")

    driver_path = self.get_driver_path('pair')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(pid)


@endpoint('razer.device.misc', 'setMouseDockProUnpair', in_sig='s')
def set_mouse_dock_pro_unpair(self, pid):
    """
    Unpair Mouse Dock Pro from a mouse by PID (hex string, e.g. "00ab")

    :param pid: product id
    :type pid: str
    """
    self.logger.debug("DBus call set_mouse_dock_pro_unpair")

    driver_path = self.get_driver_path('unpair')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(pid)


@endpoint('razer.device.misc.mug', 'isMugPresent', out_sig='b')
def is_mug_present(self):
    """
    Get if the mug is present

    :return: True if there's a mug
    :rtype: bool
    """
    self.logger.debug("DBus call is_mug_present")

    driver_path = self.get_driver_path('is_mug_present')

    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip()) == 1
