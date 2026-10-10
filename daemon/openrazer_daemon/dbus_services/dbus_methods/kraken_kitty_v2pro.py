# SPDX-License-Identifier: GPL-2.0-or-later

"""
Per-zone DBus methods for the Razer Kraken Kitty V2 Pro

The headset lights its two cat ears separately from its two ear cups. The
cups use the shared left/right zones; the ears have no equivalent among the
existing zone names, so they get their own here.
"""
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.lighting.left_ear', 'getLeftEarBrightness', out_sig='d')
def get_left_ear_brightness(self):
    """
    Get the left cat ear brightness

    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_left_ear_brightness")

    return self.zone["left_ear"]["brightness"]


@endpoint('razer.device.lighting.left_ear', 'setLeftEarBrightness', in_sig='d')
def set_left_ear_brightness(self, brightness):
    """
    Set the left cat ear brightness

    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_left_ear_brightness")

    driver_path = self.get_driver_path('left_ear_led_brightness')

    self.method_args['brightness'] = brightness

    if brightness > 100:
        brightness = 100
    elif brightness < 0:
        brightness = 0

    self.set_persistence("left_ear", "brightness", int(brightness))

    brightness = int(round(brightness * (255.0 / 100.0)))

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(brightness))

    # Notify others
    self.send_effect_event('setBrightness', brightness)


@endpoint('razer.device.lighting.left_ear', 'setLeftEarNone')
def set_left_ear_none(self):
    """
    Set the left cat ear to effect none
    """
    self.logger.debug("DBus call set_left_ear_none")

    # Notify others
    self.send_effect_event('setNone')

    # remember effect
    self.set_persistence("left_ear", "effect", 'none')

    driver_path = self.get_driver_path('left_ear_matrix_effect_none')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')


@endpoint('razer.device.lighting.left_ear', 'setLeftEarStatic', in_sig='yyy')
def set_left_ear_static(self, red, green, blue):
    """
    Set the left cat ear to static colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_left_ear_static")

    # Notify others
    self.send_effect_event('setStatic', red, green, blue)

    # remember effect
    self.set_persistence("left_ear", "effect", 'static')
    self.zone["left_ear"]["colors"][0:3] = int(red), int(green), int(blue)

    rgb_driver_path = self.get_driver_path('left_ear_matrix_effect_static')

    payload = bytes([red, green, blue])

    with open(rgb_driver_path, 'wb') as rgb_driver_file:
        rgb_driver_file.write(payload)


@endpoint('razer.device.lighting.left_ear', 'setLeftEarSpectrum')
def set_left_ear_spectrum(self):
    """
    Set the left cat ear to spectrum mode
    """
    self.logger.debug("DBus call set_left_ear_spectrum")

    # Notify others
    self.send_effect_event('setSpectrum')

    # remember effect
    self.set_persistence("left_ear", "effect", 'spectrum')

    effect_driver_path = self.get_driver_path('left_ear_matrix_effect_spectrum')

    with open(effect_driver_path, 'w') as effect_driver_file:
        effect_driver_file.write('1')


@endpoint('razer.device.lighting.left_ear', 'setLeftEarBreathRandom')
def set_left_ear_breath_random(self):
    """
    Set the left cat ear to random colour breathing effect
    """
    self.logger.debug("DBus call set_left_ear_breath_random")

    # Notify others
    self.send_effect_event('setBreathRandom')

    # remember effect
    self.set_persistence("left_ear", "effect", 'breathRandom')

    driver_path = self.get_driver_path('left_ear_matrix_effect_breath')

    payload = b'1'

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.left_ear', 'setLeftEarBreathSingle', in_sig='yyy')
def set_left_ear_breath_single(self, red, green, blue):
    """
    Set the left cat ear to single colour breathing effect

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_left_ear_breath_single")

    # Notify others
    self.send_effect_event('setBreathSingle', red, green, blue)

    # remember effect
    self.set_persistence("left_ear", "effect", 'breathSingle')
    self.zone["left_ear"]["colors"][0:3] = int(red), int(green), int(blue)

    driver_path = self.get_driver_path('left_ear_matrix_effect_breath')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.left_ear', 'setLeftEarBreathDual', in_sig='yyyyyy')
def set_left_ear_breath_dual(self, red1, green1, blue1, red2, green2, blue2):
    """
    Set the left cat ear to dual colour breathing effect

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
    self.logger.debug("DBus call set_left_ear_breath_dual")

    # Notify others
    self.send_effect_event('setBreathDual', red1, green1, blue1, red2, green2, blue2)

    # remember effect
    self.set_persistence("left_ear", "effect", 'breathDual')
    self.zone["left_ear"]["colors"][0:6] = int(red1), int(green1), int(blue1), int(red2), int(green2), int(blue2)

    driver_path = self.get_driver_path('left_ear_matrix_effect_breath')

    payload = bytes([red1, green1, blue1, red2, green2, blue2])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.right_ear', 'getRightEarBrightness', out_sig='d')
def get_right_ear_brightness(self):
    """
    Get the right cat ear brightness

    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_right_ear_brightness")

    return self.zone["right_ear"]["brightness"]


@endpoint('razer.device.lighting.right_ear', 'setRightEarBrightness', in_sig='d')
def set_right_ear_brightness(self, brightness):
    """
    Set the right cat ear brightness

    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_right_ear_brightness")

    driver_path = self.get_driver_path('right_ear_led_brightness')

    self.method_args['brightness'] = brightness

    if brightness > 100:
        brightness = 100
    elif brightness < 0:
        brightness = 0

    self.set_persistence("right_ear", "brightness", int(brightness))

    brightness = int(round(brightness * (255.0 / 100.0)))

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(brightness))

    # Notify others
    self.send_effect_event('setBrightness', brightness)


@endpoint('razer.device.lighting.right_ear', 'setRightEarNone')
def set_right_ear_none(self):
    """
    Set the right cat ear to effect none
    """
    self.logger.debug("DBus call set_right_ear_none")

    # Notify others
    self.send_effect_event('setNone')

    # remember effect
    self.set_persistence("right_ear", "effect", 'none')

    driver_path = self.get_driver_path('right_ear_matrix_effect_none')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')


@endpoint('razer.device.lighting.right_ear', 'setRightEarStatic', in_sig='yyy')
def set_right_ear_static(self, red, green, blue):
    """
    Set the right cat ear to static colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_right_ear_static")

    # Notify others
    self.send_effect_event('setStatic', red, green, blue)

    # remember effect
    self.set_persistence("right_ear", "effect", 'static')
    self.zone["right_ear"]["colors"][0:3] = int(red), int(green), int(blue)

    rgb_driver_path = self.get_driver_path('right_ear_matrix_effect_static')

    payload = bytes([red, green, blue])

    with open(rgb_driver_path, 'wb') as rgb_driver_file:
        rgb_driver_file.write(payload)


@endpoint('razer.device.lighting.right_ear', 'setRightEarSpectrum')
def set_right_ear_spectrum(self):
    """
    Set the right cat ear to spectrum mode
    """
    self.logger.debug("DBus call set_right_ear_spectrum")

    # Notify others
    self.send_effect_event('setSpectrum')

    # remember effect
    self.set_persistence("right_ear", "effect", 'spectrum')

    effect_driver_path = self.get_driver_path('right_ear_matrix_effect_spectrum')

    with open(effect_driver_path, 'w') as effect_driver_file:
        effect_driver_file.write('1')


@endpoint('razer.device.lighting.right_ear', 'setRightEarBreathRandom')
def set_right_ear_breath_random(self):
    """
    Set the right cat ear to random colour breathing effect
    """
    self.logger.debug("DBus call set_right_ear_breath_random")

    # Notify others
    self.send_effect_event('setBreathRandom')

    # remember effect
    self.set_persistence("right_ear", "effect", 'breathRandom')

    driver_path = self.get_driver_path('right_ear_matrix_effect_breath')

    payload = b'1'

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.right_ear', 'setRightEarBreathSingle', in_sig='yyy')
def set_right_ear_breath_single(self, red, green, blue):
    """
    Set the right cat ear to single colour breathing effect

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_right_ear_breath_single")

    # Notify others
    self.send_effect_event('setBreathSingle', red, green, blue)

    # remember effect
    self.set_persistence("right_ear", "effect", 'breathSingle')
    self.zone["right_ear"]["colors"][0:3] = int(red), int(green), int(blue)

    driver_path = self.get_driver_path('right_ear_matrix_effect_breath')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.right_ear', 'setRightEarBreathDual', in_sig='yyyyyy')
def set_right_ear_breath_dual(self, red1, green1, blue1, red2, green2, blue2):
    """
    Set the right cat ear to dual colour breathing effect

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
    self.logger.debug("DBus call set_right_ear_breath_dual")

    # Notify others
    self.send_effect_event('setBreathDual', red1, green1, blue1, red2, green2, blue2)

    # remember effect
    self.set_persistence("right_ear", "effect", 'breathDual')
    self.zone["right_ear"]["colors"][0:6] = int(red1), int(green1), int(blue1), int(red2), int(green2), int(blue2)

    driver_path = self.get_driver_path('right_ear_matrix_effect_breath')

    payload = bytes([red1, green1, blue1, red2, green2, blue2])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)
