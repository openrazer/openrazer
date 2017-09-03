"""
BlackWidow Ultimate 2013 effects
"""
from openrazer_daemon.dbus_services import endpoint

@endpoint('razer.device.lighting.bw2013', 'getEffect', out_sig='y')
def bw_get_effect(self):
    """
    Get current effect

    :return: Brightness
    :rtype: int
    """
    self.logger.debug("DBus call bw_get_effect")

    driver_path = self.get_driver_path('matrix_effect_pulsate')

    with open(driver_path, 'r') as driver_file:
        brightness = int(driver_file.read().strip())
        return brightness


@endpoint('razer.device.lighting.bw2013', 'setPulsate')
def bw_set_pulsate(self):
    """
    Set pulsate mode
    """
    self.logger.debug("DBus call bw_set_pulsate")

    driver_path = self.get_driver_path('matrix_effect_pulsate')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')

    # Notify others
    self.send_effect_event('setPulsate')


@endpoint('razer.device.lighting.bw2013', 'setStatic')
def bw_set_static(self):
    """
    Set static mode
    """
    self.logger.debug("DBus call bw_set_static")

    driver_path = self.get_driver_path('matrix_effect_static')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')

    # Notify others
    self.send_effect_event('setStatic')



