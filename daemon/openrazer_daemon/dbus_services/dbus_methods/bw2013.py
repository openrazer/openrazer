# SPDX-License-Identifier: GPL-2.0-or-later

"""
BlackWidow Ultimate 2013 effects
"""
from openrazer_daemon.dbus_services import endpoint

@endpoint('razer.device.lighting.bw2013', 'getPulsate', out_sig='b')
def bw_get_pulsate(self):
    """
    Get pulsate mode

    :return: Get pulsate mode
    :rtype: bool
    """
    self.logger.debug("DBus call bw_get_pulsate")

    driver_path = self.get_driver_path('matrix_effect_pulsate')

    with open(driver_path, 'r') as driver_file:
        return driver_file.read().strip() == '1'


@endpoint('razer.device.lighting.bw2013', 'setPulsate', in_sig='b')
def bw_set_pulsate(self, enable):
    """
    Set pulsate mode
    """
    self.logger.debug("DBus call bw_set_pulsate")

    driver_path = self.get_driver_path('matrix_effect_pulsate')

    # remember effect
    self.set_persistence("backlight", "effect", 'pulsate')

    with open(driver_path, 'w') as driver_file:
        if enable:
            driver_file.write('1')
        else:
            driver_file.write('0')

    # Notify others
    self.send_effect_event('setPulsate')


@endpoint('razer.device.lighting.bw2013', 'setStatic', in_sig='b')
def bw_set_static(self, enable):
    """
    Set static mode
    """
    self.logger.debug("DBus call bw_set_static")

    driver_path = self.get_driver_path('matrix_effect_static')

    # remember effect
    self.set_persistence("backlight", "effect", 'static')

    with open(driver_path, 'w') as driver_file:
        if enable:
            driver_file.write('1')
        else:
            driver_file.write('0')

    # Notify others
    self.send_effect_event('setStatic')
