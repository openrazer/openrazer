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


@endpoint('razer.device.lighting.logo', 'setLogoStatic', in_sig='yyy')
def set_logo_static_lancehead_te(self, red, green, blue):
    """
    Set the device to static colour
    :param red: Red component
    :type red: int
    :param green: Green component
    :type green: int
    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_static_effect")

    # Notify others
    self.send_effect_event('setStatic', red, green, blue)

    rgb_driver_path = self.get_driver_path('logo_matrix_effect_static')

    payload = bytes([red, green, blue])

    with open(rgb_driver_path, 'wb') as rgb_driver_file:
        rgb_driver_file.write(payload)


@endpoint('razer.device.lighting.logo', 'setLogoSpectrum')
def set_logo_spectrum_lancehead_te(self):
    """
    Set the device to pulsate
    :param red: Red component
    :type red: int
    :param green: Green component
    :type green: int
    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_logo_spectrum")

    # Notify others
    self.send_effect_event('setSpectrum')

    effect_driver_path = self.get_driver_path('logo_matrix_effect_spectrum')

    with open(effect_driver_path, 'w') as effect_driver_file:
        effect_driver_file.write('1')


@endpoint('razer.device.lighting.logo', 'setLogoNone')
def set_logo_none_lancehead_te(self):
    """
    Set the device to spectrum mode
    """
    self.logger.debug("DBus call set_none_effect")

    # Notify others
    self.send_effect_event('setNone')

    driver_path = self.get_driver_path('logo_matrix_effect_none')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')


@endpoint('razer.device.lighting.logo', 'setLogoReactive', in_sig='yyyy')
def set_logo_reactive_lancehead_te(self, red, green, blue, speed):
    """
    Set the device to reactive effect
    :param red: Red component
    :type red: int
    :param green: Green component
    :type green: int
    :param blue: Blue component
    :type blue: int
    :param speed: Speed
    :type speed: int
    """
    self.logger.debug("DBus call set_reactive_effect")

    driver_path = self.get_driver_path('logo_matrix_effect_reactive')

    # Notify others
    self.send_effect_event('setReactive', red, green, blue, speed)

    if speed not in (1, 2, 3, 4):
        speed = 4

    payload = bytes([speed, red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.logo', 'setLogoBreathRandom')
def set_logo_breath_random_lancehead_te(self):
    """
    Set the device to random colour breathing effect
    """
    self.logger.debug("DBus call set_breath_random_effect")

    # Notify others
    self.send_effect_event('setBreathRandom')

    driver_path = self.get_driver_path('logo_matrix_effect_breath')

    payload = b'1'

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.logo', 'setLogoBreathSingle', in_sig='yyy')
def set_logo_breath_single_lancehead_te(self, red, green, blue):
    """
    Set the device to single colour breathing effect
    :param red: Red component
    :type red: int
    :param green: Green component
    :type green: int
    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_breath_single_effect")

    # Notify others
    self.send_effect_event('setBreathSingle', red, green, blue)

    driver_path = self.get_driver_path('logo_matrix_effect_breath')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.logo', 'setLogoBreathDual', in_sig='yyyyyy')
def set_logo_breath_dual_lancehead_te(self, red1, green1, blue1, red2, green2, blue2):
    """
    Set the device to dual colour breathing effect
    :param red1: Red component
    :type red1: int
    :param green1: Green component
    :type green1: int
    :param blue1: Blue component
    :type blue1: int
    :param red2: Red component
    :type red2: int
    :param green2: Green component
    :type green2: int
    :param blue2: Blue component
    :type blue2: int
    """
    self.logger.debug("DBus call set_breath_dual_effect")

    # Notify others
    self.send_effect_event('setBreathDual', red1, green1, blue1, red2, green2, blue2)

    driver_path = self.get_driver_path('logo_matrix_effect_breath')

    payload = bytes([red1, green1, blue1, red2, green2, blue2])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


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


@endpoint('razer.device.lighting.scroll', 'setScrollStatic', in_sig='yyy')
def set_scroll_static_lancehead_te(self, red, green, blue):
    """
    Set the device to static colour
    :param red: Red component
    :type red: int
    :param green: Green component
    :type green: int
    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_static_effect")

    # Notify others
    self.send_effect_event('setStatic', red, green, blue)

    rgb_driver_path = self.get_driver_path('scroll_matrix_effect_static')

    payload = bytes([red, green, blue])

    with open(rgb_driver_path, 'wb') as rgb_driver_file:
        rgb_driver_file.write(payload)


@endpoint('razer.device.lighting.scroll', 'setScrollSpectrum')
def set_scroll_spectrum_lancehead_te(self):
    """
    Set the device to pulsate
    :param red: Red component
    :type red: int
    :param green: Green component
    :type green: int
    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_scroll_spectrum")

    # Notify others
    self.send_effect_event('setSpectrum')

    effect_driver_path = self.get_driver_path('scroll_matrix_effect_spectrum')

    with open(effect_driver_path, 'w') as effect_driver_file:
        effect_driver_file.write('1')


@endpoint('razer.device.lighting.scroll', 'setScrollNone')
def set_scroll_none_lancehead_te(self):
    """
    Set the device to spectrum mode
    """
    self.logger.debug("DBus call set_none_effect")

    # Notify others
    self.send_effect_event('setNone')

    driver_path = self.get_driver_path('scroll_matrix_effect_none')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')


@endpoint('razer.device.lighting.scroll', 'setScrollReactive', in_sig='yyyy')
def set_scroll_reactive_lancehead_te(self, red, green, blue, speed):
    """
    Set the device to reactive effect
    :param red: Red component
    :type red: int
    :param green: Green component
    :type green: int
    :param blue: Blue component
    :type blue: int
    :param speed: Speed
    :type speed: int
    """
    self.logger.debug("DBus call set_reactive_effect")

    driver_path = self.get_driver_path('scroll_matrix_effect_reactive')

    # Notify others
    self.send_effect_event('setReactive', red, green, blue, speed)

    if speed not in (1, 2, 3, 4):
        speed = 4

    payload = bytes([speed, red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.scroll', 'setScrollBreathRandom')
def set_scroll_breath_random_lancehead_te(self):
    """
    Set the device to random colour breathing effect
    """
    self.logger.debug("DBus call set_breath_random_effect")

    # Notify others
    self.send_effect_event('setBreathRandom')

    driver_path = self.get_driver_path('scroll_matrix_effect_breath')

    payload = b'1'

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.scroll', 'setScrollBreathSingle', in_sig='yyy')
def set_scroll_breath_single_lancehead_te(self, red, green, blue):
    """
    Set the device to single colour breathing effect
    :param red: Red component
    :type red: int
    :param green: Green component
    :type green: int
    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_breath_single_effect")

    # Notify others
    self.send_effect_event('setBreathSingle', red, green, blue)

    driver_path = self.get_driver_path('scroll_matrix_effect_breath')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.scroll', 'setScrollBreathDual', in_sig='yyyyyy')
def set_scroll_breath_dual_lancehead_te(self, red1, green1, blue1, red2, green2, blue2):
    """
    Set the device to dual colour breathing effect
    :param red1: Red component
    :type red1: int
    :param green1: Green component
    :type green1: int
    :param blue1: Blue component
    :type blue1: int
    :param red2: Red component
    :type red2: int
    :param green2: Green component
    :type green2: int
    :param blue2: Blue component
    :type blue2: int
    """
    self.logger.debug("DBus call set_breath_dual_effect")

    # Notify others
    self.send_effect_event('setBreathDual', red1, green1, blue1, red2, green2, blue2)

    driver_path = self.get_driver_path('scroll_matrix_effect_breath')

    payload = bytes([red1, green1, blue1, red2, green2, blue2])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)
