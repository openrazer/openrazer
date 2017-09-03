from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.lighting.logo', 'getLogoActive', out_sig='b')
def blade_get_logo_active(self):
    """
    Get if the logo is light up

    :return: Active
    :rtype: bool
    """
    self.logger.debug("DBus call get_logo_active")

    driver_path = self.get_driver_path('logo_led_state')

    with open(driver_path, 'r') as driver_file:
        active = int(driver_file.read().strip())
        return active == 0


@endpoint('razer.device.lighting.logo', 'setLogoActive', in_sig='b')
def blade_set_logo_active(self, active):
    """
    Get if the logo is light up

    :param active: Is active
    :type active: bool
    """
    self.logger.debug("DBus call set_logo_active")

    driver_path = self.get_driver_path('logo_led_state')

    with open(driver_path, 'w') as driver_file:
        if active:
            driver_file.write('0')
        else:
            driver_file.write('1')
