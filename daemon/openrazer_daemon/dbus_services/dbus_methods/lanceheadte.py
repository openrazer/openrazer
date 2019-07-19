from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.lighting.logo', 'setLogoWave', in_sig='i')
def set_logo_wave(self, direction):
    """
    Set the wave effect on the device
    :param direction: (0|1) - down to up, (1|2) up to down
    :type direction: int
    """
    self.logger.debug("DBus call set_logo_wave")

    # Notify others
    self.send_effect_event('setWave', direction)

    driver_path = self.get_driver_path('logo_matrix_effect_wave')

    if direction not in self.WAVE_DIRS:
        direction = self.WAVE_DIRS[0]

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(direction))


@endpoint('razer.device.lighting.scroll', 'setScrollWave', in_sig='i')
def set_scroll_wave(self, direction):
    """
    Set the wave effect on the device
    :param direction: (0|1) - down to up, (1|2) up to down
    :type direction: int
    """
    self.logger.debug("DBus call set_scroll_wave")

    # Notify others
    self.send_effect_event('setWave', direction)

    driver_path = self.get_driver_path('scroll_matrix_effect_wave')

    if direction not in self.WAVE_DIRS:
        direction = self.WAVE_DIRS[0]

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(direction))


@endpoint('razer.device.lighting.left', 'getLeftBrightness', out_sig='d')
def get_left_brightness(self):
    """
    Get the device's brightness
    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_left_brightness")

    driver_path = self.get_driver_path('left_led_brightness')

    with open(driver_path, 'r') as driver_file:
        brightness = round(float(driver_file.read()) * (100.0 / 255.0), 2)

        return brightness


@endpoint('razer.device.lighting.left', 'setLeftBrightness', in_sig='d')
def set_left_brightness(self, brightness):
    """
    Set the device's brightness
    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_left_brightness")

    driver_path = self.get_driver_path('left_led_brightness')

    self.method_args['brightness'] = brightness

    brightness = int(round(brightness * (255.0 / 100.0)))
    if brightness > 255:
        brightness = 255
    elif brightness < 0:
        brightness = 0

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(brightness))

    # Notify others
    self.send_effect_event('setBrightness', brightness)


@endpoint('razer.device.lighting.left', 'setLeftWave', in_sig='i')
def set_left_wave(self, direction):
    """
    Set the wave effect on the device
    :param direction: (0|1) - down to up, (1|2) up to down
    :type direction: int
    """
    self.logger.debug("DBus call set_left_wave")

    # Notify others
    self.send_effect_event('setWave', direction)

    driver_path = self.get_driver_path('left_matrix_effect_wave')

    if direction not in self.WAVE_DIRS:
        direction = self.WAVE_DIRS[0]

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(direction))


@endpoint('razer.device.lighting.left', 'setLeftStatic', in_sig='yyy')
def set_left_static(self, red, green, blue):
    """
    Set the device to static colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_left_static")

    # Notify others
    self.send_effect_event('setStatic', red, green, blue)

    rgb_driver_path = self.get_driver_path('left_matrix_effect_static')

    payload = bytes([red, green, blue])

    with open(rgb_driver_path, 'wb') as rgb_driver_file:
        rgb_driver_file.write(payload)


@endpoint('razer.device.lighting.left', 'setLeftSpectrum')
def set_left_spectrum(self):
    """
    Set the device to spectrum mode
    """
    self.logger.debug("DBus call set_left_spectrum")

    # Notify others
    self.send_effect_event('setSpectrum')

    effect_driver_path = self.get_driver_path('left_matrix_effect_spectrum')

    with open(effect_driver_path, 'w') as effect_driver_file:
        effect_driver_file.write('1')


@endpoint('razer.device.lighting.left', 'setLeftNone')
def set_left_none(self):
    """
    Set the device to effect none
    """
    self.logger.debug("DBus call set_left_none")

    # Notify others
    self.send_effect_event('setNone')

    driver_path = self.get_driver_path('left_matrix_effect_none')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')


@endpoint('razer.device.lighting.left', 'setLeftReactive', in_sig='yyyy')
def set_left_reactive(self, red, green, blue, speed):
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
    self.logger.debug("DBus call set_left_reactive")

    driver_path = self.get_driver_path('left_matrix_effect_reactive')

    # Notify others
    self.send_effect_event('setReactive', red, green, blue, speed)

    if speed not in (1, 2, 3, 4):
        speed = 4

    payload = bytes([speed, red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.left', 'setLeftBreathRandom')
def set_left_breath_random(self):
    """
    Set the device to random colour breathing effect
    """
    self.logger.debug("DBus call set_left_breath_random")

    # Notify others
    self.send_effect_event('setBreathRandom')

    driver_path = self.get_driver_path('left_matrix_effect_breath')

    payload = b'1'

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.left', 'setLeftBreathSingle', in_sig='yyy')
def set_left_breath_single(self, red, green, blue):
    """
    Set the device to single colour breathing effect

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_left_breath_single")

    # Notify others
    self.send_effect_event('setBreathSingle', red, green, blue)

    driver_path = self.get_driver_path('left_matrix_effect_breath')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.left', 'setLeftBreathDual', in_sig='yyyyyy')
def set_left_breath_dual(self, red1, green1, blue1, red2, green2, blue2):
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
    self.logger.debug("DBus call set_left_breath_dual")

    # Notify others
    self.send_effect_event('setBreathDual', red1, green1, blue1, red2, green2, blue2)

    driver_path = self.get_driver_path('left_matrix_effect_breath')

    payload = bytes([red1, green1, blue1, red2, green2, blue2])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.right', 'getRightBrightness', out_sig='d')
def get_right_brightness(self):
    """
    Get the device's brightness
    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_right_brightness")

    driver_path = self.get_driver_path('right_led_brightness')

    with open(driver_path, 'r') as driver_file:
        brightness = round(float(driver_file.read()) * (100.0 / 255.0), 2)

        return brightness


@endpoint('razer.device.lighting.right', 'setRightBrightness', in_sig='d')
def set_right_brightness(self, brightness):
    """
    Set the device's brightness
    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_right_brightness")

    driver_path = self.get_driver_path('right_led_brightness')

    self.method_args['brightness'] = brightness

    brightness = int(round(brightness * (255.0 / 100.0)))
    if brightness > 255:
        brightness = 255
    elif brightness < 0:
        brightness = 0

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(brightness))

    # Notify others
    self.send_effect_event('setBrightness', brightness)


@endpoint('razer.device.lighting.right', 'setRightWave', in_sig='i')
def set_right_wave(self, direction):
    """
    Set the wave effect on the device
    :param direction: (0|1) - down to up, (1|2) up to down
    :type direction: int
    """
    self.logger.debug("DBus call set_right_wave")

    # Notify others
    self.send_effect_event('setWave', direction)

    driver_path = self.get_driver_path('right_matrix_effect_wave')

    if direction not in self.WAVE_DIRS:
        direction = self.WAVE_DIRS[0]

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(direction))


@endpoint('razer.device.lighting.right', 'setRightStatic', in_sig='yyy')
def set_right_static(self, red, green, blue):
    """
    Set the device to static colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_right_static")

    # Notify others
    self.send_effect_event('setStatic', red, green, blue)

    rgb_driver_path = self.get_driver_path('right_matrix_effect_static')

    payload = bytes([red, green, blue])

    with open(rgb_driver_path, 'wb') as rgb_driver_file:
        rgb_driver_file.write(payload)


@endpoint('razer.device.lighting.right', 'setRightSpectrum')
def set_right_spectrum(self):
    """
    Set the device to spectrum mode
    """
    self.logger.debug("DBus call set_right_spectrum")

    # Notify others
    self.send_effect_event('setSpectrum')

    effect_driver_path = self.get_driver_path('right_matrix_effect_spectrum')

    with open(effect_driver_path, 'w') as effect_driver_file:
        effect_driver_file.write('1')


@endpoint('razer.device.lighting.right', 'setRightNone')
def set_right_none(self):
    """
    Set the device to effect none
    """
    self.logger.debug("DBus call set_right_none")

    # Notify others
    self.send_effect_event('setNone')

    driver_path = self.get_driver_path('right_matrix_effect_none')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')


@endpoint('razer.device.lighting.right', 'setRightReactive', in_sig='yyyy')
def set_right_reactive(self, red, green, blue, speed):
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
    self.logger.debug("DBus call set_right_reactive")

    driver_path = self.get_driver_path('right_matrix_effect_reactive')

    # Notify others
    self.send_effect_event('setReactive', red, green, blue, speed)

    if speed not in (1, 2, 3, 4):
        speed = 4

    payload = bytes([speed, red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.right', 'setRightBreathRandom')
def set_right_breath_random(self):
    """
    Set the device to random colour breathing effect
    """
    self.logger.debug("DBus call set_right_breath_random")

    # Notify others
    self.send_effect_event('setBreathRandom')

    driver_path = self.get_driver_path('right_matrix_effect_breath')

    payload = b'1'

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.right', 'setRightBreathSingle', in_sig='yyy')
def set_right_breath_single(self, red, green, blue):
    """
    Set the device to single colour breathing effect

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_right_breath_single")

    # Notify others
    self.send_effect_event('setBreathSingle', red, green, blue)

    driver_path = self.get_driver_path('right_matrix_effect_breath')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.right', 'setRightBreathDual', in_sig='yyyyyy')
def set_right_breath_dual(self, red1, green1, blue1, red2, green2, blue2):
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
    self.logger.debug("DBus call set_right_breath_dual")

    # Notify others
    self.send_effect_event('setBreathDual', red1, green1, blue1, red2, green2, blue2)

    driver_path = self.get_driver_path('right_matrix_effect_breath')

    payload = bytes([red1, green1, blue1, red2, green2, blue2])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)
