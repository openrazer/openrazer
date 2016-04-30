"""
BlackWidow Ultimate 2013 effects
"""
from razer_daemon.dbus_services import endpoint

@endpoint('razer.device.lighting', 'getEffect', out_sig='y')
def bw_get_effect(self):
    """
    Get current effect

    :return: Brightness
    :rtype: int
    """
    driver_path = self.get_driver_path('mode_pulsate')

    with open(driver_path, 'r') as driver_file:
        brightness = int(driver_file.read().strip())
        return brightness

@endpoint('razer.device.lighting', 'setPulsate')
def bw_set_pulsate(self):
    """
    Set pulsate mode
    """
    driver_path = self.get_driver_path('mode_pulsate')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')

@endpoint('razer.device.lighting', 'setStatic')
def bw_set_static(self):
    """
    Set static mode
    """
    driver_path = self.get_driver_path('mode_static')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')
