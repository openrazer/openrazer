from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.lighting.charging', 'getChargingBrightness', out_sig='d')
def get_charging_brightness(self):
    """
    Get the device's brightness
    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_charging_brightness")

    return self.zone["charging"]["brightness"]


@endpoint('razer.device.lighting.charging', 'setChargingBrightness', in_sig='d')
def set_charging_brightness(self, brightness):
    """
    Set the device's brightness
    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_charging_brightness")

    driver_path = self.get_driver_path('charging_led_brightness')

    self.method_args['brightness'] = brightness

    if brightness > 100:
        brightness = 100
    elif brightness < 0:
        brightness = 0

    self.set_persistence("charging", "brightness", int(brightness))

    brightness = int(round(brightness * (255.0 / 100.0)))

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(brightness))

    # Notify others
    self.send_effect_event('setBrightness', brightness)


@endpoint('razer.device.lighting.charging', 'setChargingWave', in_sig='i')
def set_charging_wave(self, direction):
    """
    Set the wave effect on the device
    :param direction: (0|1) - down to up, (1|2) up to down
    :type direction: int
    """
    self.logger.debug("DBus call set_charging_wave")

    # Notify others
    self.send_effect_event('setWave', direction)

    # remember effect
    self.set_persistence("charging", "effect", 'wave')
    self.set_persistence("charging", "wave_dir", int(direction))

    driver_path = self.get_driver_path('charging_matrix_effect_wave')

    if direction not in self.WAVE_DIRS:
        direction = self.WAVE_DIRS[0]

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(direction))


@endpoint('razer.device.lighting.charging', 'setChargingStatic', in_sig='yyy')
def set_charging_static(self, red, green, blue):
    """
    Set the device to static colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_charging_static")

    # Notify others
    self.send_effect_event('setStatic', red, green, blue)

    # remember effect
    self.set_persistence("charging", "effect", 'static')
    self.zone["charging"]["colors"][0:3] = int(red), int(green), int(blue)

    rgb_driver_path = self.get_driver_path('charging_matrix_effect_static')

    payload = bytes([red, green, blue])

    with open(rgb_driver_path, 'wb') as rgb_driver_file:
        rgb_driver_file.write(payload)


@endpoint('razer.device.lighting.charging', 'setChargingSpectrum')
def set_charging_spectrum(self):
    """
    Set the device to spectrum mode
    """
    self.logger.debug("DBus call set_charging_spectrum")

    # Notify others
    self.send_effect_event('setSpectrum')

    # remember effect
    self.set_persistence("charging", "effect", 'spectrum')

    effect_driver_path = self.get_driver_path('charging_matrix_effect_spectrum')

    with open(effect_driver_path, 'w') as effect_driver_file:
        effect_driver_file.write('1')


@endpoint('razer.device.lighting.charging', 'setChargingNone')
def set_charging_none(self):
    """
    Set the device to effect none
    """
    self.logger.debug("DBus call set_charging_none")

    # Notify others
    self.send_effect_event('setNone')

    # remember effect
    self.set_persistence("charging", "effect", 'none')

    driver_path = self.get_driver_path('charging_matrix_effect_none')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')


@endpoint('razer.device.lighting.charging', 'setChargingBreathRandom')
def set_charging_breath_random(self):
    """
    Set the device to random colour breathing effect
    """
    self.logger.debug("DBus call set_charging_breath_random")

    # Notify others
    self.send_effect_event('setBreathRandom')

    # remember effect
    self.set_persistence("charging", "effect", 'breathRandom')

    driver_path = self.get_driver_path('charging_matrix_effect_breath')

    payload = b'1'

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.charging', 'setChargingBreathSingle', in_sig='yyy')
def set_charging_breath_single(self, red, green, blue):
    """
    Set the device to single colour breathing effect

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_charging_breath_single")

    # Notify others
    self.send_effect_event('setBreathSingle', red, green, blue)

    # remember effect
    self.set_persistence("charging", "effect", 'breathSingle')
    self.zone["charging"]["colors"][0:3] = int(red), int(green), int(blue)

    driver_path = self.get_driver_path('charging_matrix_effect_breath')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.charging', 'setChargingBreathDual', in_sig='yyyyyy')
def set_charging_breath_dual(self, red1, green1, blue1, red2, green2, blue2):
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
    self.logger.debug("DBus call set_charging_breath_dual")

    # Notify others
    self.send_effect_event('setBreathDual', red1, green1, blue1, red2, green2, blue2)

    # remember effect
    self.set_persistence("charging", "effect", 'breathDual')
    self.zone["charging"]["colors"][0:6] = int(red1), int(green1), int(blue1), int(red2), int(green2), int(blue2)

    driver_path = self.get_driver_path('charging_matrix_effect_breath')

    payload = bytes([red1, green1, blue1, red2, green2, blue2])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.fast_charging', 'getFastChargingBrightness', out_sig='d')
def get_fast_charging_brightness(self):
    """
    Get the device's brightness
    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_fast_charging_brightness")

    return self.zone["fast_charging"]["brightness"]


@endpoint('razer.device.lighting.fast_charging', 'setFastChargingBrightness', in_sig='d')
def set_fast_charging_brightness(self, brightness):
    """
    Set the device's brightness
    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_fast_charging_brightness")

    driver_path = self.get_driver_path('fast_charging_led_brightness')

    self.method_args['brightness'] = brightness

    if brightness > 100:
        brightness = 100
    elif brightness < 0:
        brightness = 0

    self.set_persistence("fast_charging", "brightness", int(brightness))

    brightness = int(round(brightness * (255.0 / 100.0)))

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(brightness))

    # Notify others
    self.send_effect_event('setBrightness', brightness)


@endpoint('razer.device.lighting.fast_charging', 'setFastChargingWave', in_sig='i')
def set_fast_charging_wave(self, direction):
    """
    Set the wave effect on the device
    :param direction: (0|1) - down to up, (1|2) up to down
    :type direction: int
    """
    self.logger.debug("DBus call set_fast_charging_wave")

    # Notify others
    self.send_effect_event('setWave', direction)

    # remember effect
    self.set_persistence("fast_charging", "effect", 'wave')
    self.set_persistence("fast_charging", "wave_dir", int(direction))

    driver_path = self.get_driver_path('fast_charging_matrix_effect_wave')

    if direction not in self.WAVE_DIRS:
        direction = self.WAVE_DIRS[0]

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(direction))


@endpoint('razer.device.lighting.fast_charging', 'setFastChargingStatic', in_sig='yyy')
def set_fast_charging_static(self, red, green, blue):
    """
    Set the device to static colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_fast_charging_static")

    # Notify others
    self.send_effect_event('setStatic', red, green, blue)

    # remember effect
    self.set_persistence("fast_charging", "effect", 'static')
    self.zone["fast_charging"]["colors"][0:3] = int(red), int(green), int(blue)

    rgb_driver_path = self.get_driver_path('fast_charging_matrix_effect_static')

    payload = bytes([red, green, blue])

    with open(rgb_driver_path, 'wb') as rgb_driver_file:
        rgb_driver_file.write(payload)


@endpoint('razer.device.lighting.fast_charging', 'setFastChargingSpectrum')
def set_fast_charging_spectrum(self):
    """
    Set the device to spectrum mode
    """
    self.logger.debug("DBus call set_fast_charging_spectrum")

    # Notify others
    self.send_effect_event('setSpectrum')

    # remember effect
    self.set_persistence("fast_charging", "effect", 'spectrum')

    effect_driver_path = self.get_driver_path('fast_charging_matrix_effect_spectrum')

    with open(effect_driver_path, 'w') as effect_driver_file:
        effect_driver_file.write('1')


@endpoint('razer.device.lighting.fast_charging', 'setFastChargingNone')
def set_fast_charging_none(self):
    """
    Set the device to effect none
    """
    self.logger.debug("DBus call set_fast_charging_none")

    # Notify others
    self.send_effect_event('setNone')

    # remember effect
    self.set_persistence("fast_charging", "effect", 'none')

    driver_path = self.get_driver_path('fast_charging_matrix_effect_none')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')


@endpoint('razer.device.lighting.fast_charging', 'setFastChargingBreathRandom')
def set_fast_charging_breath_random(self):
    """
    Set the device to random colour breathing effect
    """
    self.logger.debug("DBus call set_fast_charging_breath_random")

    # Notify others
    self.send_effect_event('setBreathRandom')

    # remember effect
    self.set_persistence("fast_charging", "effect", 'breathRandom')

    driver_path = self.get_driver_path('fast_charging_matrix_effect_breath')

    payload = b'1'

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.fast_charging', 'setFastChargingBreathSingle', in_sig='yyy')
def set_fast_charging_breath_single(self, red, green, blue):
    """
    Set the device to single colour breathing effect

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_fast_charging_breath_single")

    # Notify others
    self.send_effect_event('setBreathSingle', red, green, blue)

    # remember effect
    self.set_persistence("fast_charging", "effect", 'breathSingle')
    self.zone["fast_charging"]["colors"][0:3] = int(red), int(green), int(blue)

    driver_path = self.get_driver_path('fast_charging_matrix_effect_breath')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.fast_charging', 'setFastChargingBreathDual', in_sig='yyyyyy')
def set_fast_charging_breath_dual(self, red1, green1, blue1, red2, green2, blue2):
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
    self.logger.debug("DBus call set_fast_charging_breath_dual")

    # Notify others
    self.send_effect_event('setBreathDual', red1, green1, blue1, red2, green2, blue2)

    # remember effect
    self.set_persistence("fast_charging", "effect", 'breathDual')
    self.zone["fast_charging"]["colors"][0:6] = int(red1), int(green1), int(blue1), int(red2), int(green2), int(blue2)

    driver_path = self.get_driver_path('fast_charging_matrix_effect_breath')

    payload = bytes([red1, green1, blue1, red2, green2, blue2])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.fully_charged', 'getFullyChargedBrightness', out_sig='d')
def get_fully_charged_brightness(self):
    """
    Get the device's brightness
    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_fully_charged_brightness")

    return self.zone["fully_charged"]["brightness"]


@endpoint('razer.device.lighting.fully_charged', 'setFullyChargedBrightness', in_sig='d')
def set_fully_charged_brightness(self, brightness):
    """
    Set the device's brightness
    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_fully_charged_brightness")

    driver_path = self.get_driver_path('fully_charged_led_brightness')

    self.method_args['brightness'] = brightness

    if brightness > 100:
        brightness = 100
    elif brightness < 0:
        brightness = 0

    self.set_persistence("fully_charged", "brightness", int(brightness))

    brightness = int(round(brightness * (255.0 / 100.0)))

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(brightness))

    # Notify others
    self.send_effect_event('setBrightness', brightness)


@endpoint('razer.device.lighting.fully_charged', 'setFullyChargedWave', in_sig='i')
def set_fully_charged_wave(self, direction):
    """
    Set the wave effect on the device
    :param direction: (0|1) - down to up, (1|2) up to down
    :type direction: int
    """
    self.logger.debug("DBus call set_fully_charged_wave")

    # Notify others
    self.send_effect_event('setWave', direction)

    # remember effect
    self.set_persistence("fully_charged", "effect", 'wave')
    self.set_persistence("fully_charged", "wave_dir", int(direction))

    driver_path = self.get_driver_path('fully_charged_matrix_effect_wave')

    if direction not in self.WAVE_DIRS:
        direction = self.WAVE_DIRS[0]

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(direction))


@endpoint('razer.device.lighting.fully_charged', 'setFullyChargedStatic', in_sig='yyy')
def set_fully_charged_static(self, red, green, blue):
    """
    Set the device to static colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_fully_charged_static")

    # Notify others
    self.send_effect_event('setStatic', red, green, blue)

    # remember effect
    self.set_persistence("fully_charged", "effect", 'static')
    self.zone["fully_charged"]["colors"][0:3] = int(red), int(green), int(blue)

    rgb_driver_path = self.get_driver_path('fully_charged_matrix_effect_static')

    payload = bytes([red, green, blue])

    with open(rgb_driver_path, 'wb') as rgb_driver_file:
        rgb_driver_file.write(payload)


@endpoint('razer.device.lighting.fully_charged', 'setFullyChargedSpectrum')
def set_fully_charged_spectrum(self):
    """
    Set the device to spectrum mode
    """
    self.logger.debug("DBus call set_fully_charged_spectrum")

    # Notify others
    self.send_effect_event('setSpectrum')

    # remember effect
    self.set_persistence("fully_charged", "effect", 'spectrum')

    effect_driver_path = self.get_driver_path('fully_charged_matrix_effect_spectrum')

    with open(effect_driver_path, 'w') as effect_driver_file:
        effect_driver_file.write('1')


@endpoint('razer.device.lighting.fully_charged', 'setFullyChargedNone')
def set_fully_charged_none(self):
    """
    Set the device to effect none
    """
    self.logger.debug("DBus call set_fully_charged_none")

    # Notify others
    self.send_effect_event('setNone')

    # remember effect
    self.set_persistence("fully_charged", "effect", 'none')

    driver_path = self.get_driver_path('fully_charged_matrix_effect_none')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')


@endpoint('razer.device.lighting.fully_charged', 'setFullyChargedBreathRandom')
def set_fully_charged_breath_random(self):
    """
    Set the device to random colour breathing effect
    """
    self.logger.debug("DBus call set_fully_charged_breath_random")

    # Notify others
    self.send_effect_event('setBreathRandom')

    # remember effect
    self.set_persistence("fully_charged", "effect", 'breathRandom')

    driver_path = self.get_driver_path('fully_charged_matrix_effect_breath')

    payload = b'1'

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.fully_charged', 'setFullyChargedBreathSingle', in_sig='yyy')
def set_fully_charged_breath_single(self, red, green, blue):
    """
    Set the device to single colour breathing effect

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_fully_charged_breath_single")

    # Notify others
    self.send_effect_event('setBreathSingle', red, green, blue)

    # remember effect
    self.set_persistence("fully_charged", "effect", 'breathSingle')
    self.zone["fully_charged"]["colors"][0:3] = int(red), int(green), int(blue)

    driver_path = self.get_driver_path('fully_charged_matrix_effect_breath')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.fully_charged', 'setFullyChargedBreathDual', in_sig='yyyyyy')
def set_fully_charged_breath_dual(self, red1, green1, blue1, red2, green2, blue2):
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
    self.logger.debug("DBus call set_fully_charged_breath_dual")

    # Notify others
    self.send_effect_event('setBreathDual', red1, green1, blue1, red2, green2, blue2)

    # remember effect
    self.set_persistence("fully_charged", "effect", 'breathDual')
    self.zone["fully_charged"]["colors"][0:6] = int(red1), int(green1), int(blue1), int(red2), int(green2), int(blue2)

    driver_path = self.get_driver_path('fully_charged_matrix_effect_breath')

    payload = bytes([red1, green1, blue1, red2, green2, blue2])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)
