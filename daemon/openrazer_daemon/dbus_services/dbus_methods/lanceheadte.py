from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.lighting.logo', 'setLogoWave', in_sig='i')
def set_logo_wave_lancehead_te(self, direction):
    """
    Set the wave effect on the device
    :param direction: 1 - left to right, 2 right to left
    :type direction: int
    """
    self.logger.debug("DBus call set_wave_effect")

    # Notify others
    self.send_effect_event('setWave', direction)

    driver_path = self.get_driver_path('logo_matrix_effect_wave')

    if direction not in self.WAVE_DIRS:
        direction = self.WAVE_DIRS[0]

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(direction))


@endpoint('razer.device.lighting.scroll', 'setScrollWave', in_sig='i')
def set_scroll_wave_lancehead_te(self, direction):
    """
    Set the wave effect on the device
    :param direction: 1 - left to right, 2 right to left
    :type direction: int
    """
    self.logger.debug("DBus call set_wave_effect")

    # Notify others
    self.send_effect_event('setWave', direction)

    driver_path = self.get_driver_path('scroll_matrix_effect_wave')

    if direction not in self.WAVE_DIRS:
        direction = self.WAVE_DIRS[0]

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(direction))

