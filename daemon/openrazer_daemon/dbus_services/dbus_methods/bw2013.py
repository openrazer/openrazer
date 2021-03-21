"""
BlackWidow Ultimate 2013 effects
"""
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.lighting.bw2013', 'setPulsate')
def bw_set_pulsate(self):
    """
    Set pulsate mode
    """
    self.logger.debug("DBus call bw_set_pulsate")

    driver_path = self.get_driver_path('matrix_effect_pulsate')

    # remember effect
    self.set_persistence("backlight", "effect", 'pulsate')

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

    # remember effect
    self.set_persistence("backlight", "effect", 'static')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')

    # Notify others
    self.send_effect_event('setStatic')
