"""
BlackWidow Chroma Effects
"""
import os
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.lighting.brightness', 'getBrightness', out_sig='d')
def get_brightness(self):
    """
    Get the device's brightness

    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_brightness")

    return self.zone["backlight"]["brightness"]


@endpoint('razer.device.lighting.brightness', 'setBrightness', in_sig='d')
def set_brightness(self, brightness):
    """
    Set the device's brightness

    :param brightness: Brightness
    :type brightness: int
    """
    self.logger.debug("DBus call set_brightness")

    driver_path = self.get_driver_path('matrix_brightness')

    self.method_args['brightness'] = brightness

    if brightness > 100:
        brightness = 100
    elif brightness < 0:
        brightness = 0

    self.set_persistence("backlight", "brightness", int(brightness))

    brightness = int(round(brightness * (255.0 / 100.0)))

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(brightness))

    # Notify others
    self.send_effect_event('setBrightness', brightness)


@endpoint('razer.device.led.gamemode', 'getGameMode', out_sig='b')
def get_game_mode(self):
    """
    Get game mode LED state

    :return: Game mode LED state
    :rtype: bool
    """
    self.logger.debug("DBus call get_game_mode")

    driver_path = self.get_driver_path('game_led_state')

    with open(driver_path, 'r') as driver_file:
        return driver_file.read().strip() == '1'


@endpoint('razer.device.led.gamemode', 'setGameMode', in_sig='b')
def set_game_mode(self, enable):
    """
    Set game mode LED state

    :param enable: Status of game mode
    :type enable: bool
    """
    self.logger.debug("DBus call set_game_mode")

    driver_path = self.get_driver_path('game_led_state')

    for kb_int in self.additional_interfaces:
        super_file = os.path.join(kb_int, 'key_super')
        alt_tab = os.path.join(kb_int, 'key_alt_tab')
        alt_f4 = os.path.join(kb_int, 'key_alt_f4')

        if enable:
            open(super_file, 'wb').write(b'\x01')
            open(alt_tab, 'wb').write(b'\x01')
            open(alt_f4, 'wb').write(b'\x01')
        else:
            open(super_file, 'wb').write(b'\x00')
            open(alt_tab, 'wb').write(b'\x00')
            open(alt_f4, 'wb').write(b'\x00')

    with open(driver_path, 'w') as driver_file:
        if enable:
            driver_file.write('1')
        else:
            driver_file.write('0')


@endpoint('razer.device.led.macromode', 'getMacroMode', out_sig='b')
def get_macro_mode(self):
    """
    Get macro mode LED state

    :return: Status of macro mode
    :rtype: bool
    """
    self.logger.debug("DBus call get_macro_mode")

    driver_path = self.get_driver_path('macro_led_state')

    with open(driver_path, 'r') as driver_file:
        return driver_file.read().strip() == '1'


@endpoint('razer.device.led.macromode', 'setMacroMode', in_sig='b')
def set_macro_mode(self, enable):
    """
    Set macro mode LED state

    :param enable: Status of macro mode
    :type enable: bool
    """
    self.logger.debug("DBus call set_macro_mode")

    driver_path = self.get_driver_path('macro_led_state')

    with open(driver_path, 'w') as driver_file:
        if enable:
            driver_file.write('1')
        else:
            driver_file.write('0')


@endpoint('razer.device.led.macromode', 'getMacroEffect', out_sig='i')
def get_macro_effect(self):
    """
    Get the effect on the macro LED

    :return: Macro LED effect ID
    :rtype: int
    """
    self.logger.debug("DBus call get_macro_effect")

    driver_path = self.get_driver_path('macro_led_effect')

    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip())


@endpoint('razer.device.led.macromode', 'setMacroEffect', in_sig='y')
def set_macro_effect(self, effect):
    """
    Set the effect on the macro LED

    :param effect: Macro LED effect ID
    :type effect: int
    """
    self.logger.debug("DBus call set_macro_effect")

    driver_path = self.get_driver_path('macro_led_effect')

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(int(effect)))


@endpoint('razer.device.lighting.chroma', 'setWave', in_sig='i')
def set_wave_effect(self, direction):
    """
    Set the wave effect on the device

    :param direction: 1 - left to right, 2 right to left
    :type direction: int
    """
    self.logger.debug("DBus call set_wave_effect")

    # Notify others
    self.send_effect_event('setWave', direction)

    # remember effect
    self.set_persistence("backlight", "effect", 'wave')
    self.set_persistence("backlight", "wave_dir", int(direction))

    driver_path = self.get_driver_path('matrix_effect_wave')

    if direction not in self.WAVE_DIRS:
        direction = self.WAVE_DIRS[0]

    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(direction))


@endpoint('razer.device.lighting.chroma', 'setStatic', in_sig='yyy')
def set_static_effect(self, red, green, blue):
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

    # remember effect
    self.set_persistence("backlight", "effect", 'static')
    self.zone["backlight"]["colors"][0:3] = int(red), int(green), int(blue)

    driver_path = self.get_driver_path('matrix_effect_static')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.chroma', 'setBlinking', in_sig='yyy')
def set_blinking_effect(self, red, green, blue):
    """
    Set the device to static colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_blinking_effect")

    # Notify others
    self.send_effect_event('setBlinking', red, green, blue)

    # remember effect
    self.set_persistence("backlight", "effect", 'blinking')
    self.zone["backlight"]["colors"][0:3] = int(red), int(green), int(blue)

    driver_path = self.get_driver_path('matrix_effect_blinking')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.chroma', 'setSpectrum')
def set_spectrum_effect(self):
    """
    Set the device to spectrum mode
    """
    self.logger.debug("DBus call set_spectrum_effect")

    # Notify others
    self.send_effect_event('setSpectrum')

    # remember effect
    self.set_persistence("backlight", "effect", 'spectrum')

    driver_path = self.get_driver_path('matrix_effect_spectrum')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')


@endpoint('razer.device.lighting.chroma', 'setNone')
def set_none_effect(self):
    """
    Set the device to spectrum mode
    """
    self.logger.debug("DBus call set_none_effect")

    # Notify others
    self.send_effect_event('setNone')

    # remember effect
    self.set_persistence("backlight", "effect", 'none')

    driver_path = self.get_driver_path('matrix_effect_none')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')


@endpoint('razer.device.misc', 'triggerReactive')
def trigger_reactive_effect(self):
    """
    Trigger reactive on Firefly
    """
    self.logger.debug("DBus call trigger_reactive_effect")

    # Notify others
    self.send_effect_event('triggerReactive')

    driver_path = self.get_driver_path('matrix_reactive_trigger')

    with open(driver_path, 'w') as driver_file:
        driver_file.write('1')


@endpoint('razer.device.lighting.chroma', 'setReactive', in_sig='yyyy')
def set_reactive_effect(self, red, green, blue, speed):
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

    driver_path = self.get_driver_path('matrix_effect_reactive')

    # Notify others
    self.send_effect_event('setReactive', red, green, blue, speed)

    # remember effect
    self.set_persistence("backlight", "effect", 'reactive')
    self.zone["backlight"]["colors"][0:3] = int(red), int(green), int(blue)

    if speed not in (1, 2, 3, 4):
        speed = 4

    self.set_persistence("backlight", "speed", int(speed))

    payload = bytes([speed, red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.chroma', 'setBreathRandom')
def set_breath_random_effect(self):
    """
    Set the device to random colour breathing effect
    """
    self.logger.debug("DBus call set_breath_random_effect")

    # Notify others
    self.send_effect_event('setBreathRandom')

    # remember effect
    self.set_persistence("backlight", "effect", 'breathRandom')

    driver_path = self.get_driver_path('matrix_effect_breath')

    payload = b'1'

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.chroma', 'setBreathSingle', in_sig='yyy')
def set_breath_single_effect(self, red, green, blue):
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

    # remember effect
    self.set_persistence("backlight", "effect", 'breathSingle')
    self.zone["backlight"]["colors"][0:3] = int(red), int(green), int(blue)

    driver_path = self.get_driver_path('matrix_effect_breath')

    payload = bytes([red, green, blue])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.chroma', 'setBreathDual', in_sig='yyyyyy')
def set_breath_dual_effect(self, red1, green1, blue1, red2, green2, blue2):
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

    # remember effect
    self.set_persistence("backlight", "effect", 'breathDual')
    self.zone["backlight"]["colors"][0:6] = int(red1), int(green1), int(blue1), int(red2), int(green2), int(blue2)

    driver_path = self.get_driver_path('matrix_effect_breath')

    payload = bytes([red1, green1, blue1, red2, green2, blue2])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.chroma', 'setBreathTriple', in_sig='yyyyyyyyy')
def set_breath_triple_effect(self, red1, green1, blue1, red2, green2, blue2, red3, green3, blue3):
    """
    Set the device to triple colour breathing effect

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

    :param red3: Red component
    :type red3: int

    :param green3: Green component
    :type green3: int

    :param blue3: Blue component
    :type blue3: int
    """
    self.logger.debug("DBus call set_breath_triple_effect")

    # Notify others
    self.send_effect_event('setBreathTriple', red1, green1, blue1, red2, green2, blue2, red3, green3, blue3)

    # remember effect
    self.set_persistence("backlight", "effect", 'breathTriple')
    self.zone["backlight"]["colors"][0:9] = int(red1), int(green1), int(blue1), int(red2), int(green2), int(blue2), int(red3), int(green3), int(blue3)

    driver_path = self.get_driver_path('matrix_effect_breath')

    payload = bytes([red1, green1, blue1, red2, green2, blue2, red3, green3, blue3])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.chroma', 'setCustom')
def set_custom_effect(self):
    """
    Set the device to use custom LED matrix
    """
    # TODO uncomment
    # self.logger.debug("DBus call set_custom_effect")

    driver_path = self.get_driver_path('matrix_effect_custom')

    payload = b'1'

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.chroma', 'setKeyRow', in_sig='ay', byte_arrays=True)
def set_key_row(self, payload):
    """
    Set the RGB matrix on the device

    Byte array like
    [1, 255, 255, 00, 255, 255, 00, 255, 255, 00, 255, 255, 00, 255, 255, 00, 255, 255, 00, 255, 255, 00, 255, 255, 00,
        255, 255, 00, 255, 255, 00, 255, 255, 00, 255, 255, 00, 255, 255, 00, 255, 255, 00, 255, 00, 00]

    First byte is row, on firefly its always 1, on keyboard its 0-5
    Then its 3byte groups of RGB
    :param payload: Binary payload
    :type payload: bytes
    """

    # TODO uncomment
    # self.logger.debug("DBus call set_key_row")

    driver_path = self.get_driver_path('matrix_custom_frame')

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.lighting.custom', 'setRipple', in_sig='yyyd')
def set_ripple_effect(self, red, green, blue, refresh_rate):
    """
    Set the daemon to serve a ripple effect of the specified colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int

    :param refresh_rate: Refresh rate
    :type refresh_rate: int
    """
    self.logger.debug("DBus call set_ripple_effect")

    # Notify others
    self.send_effect_event('setRipple', red, green, blue, refresh_rate)

    # remember effect
    self.set_persistence("backlight", "effect", 'ripple')
    self.zone["backlight"]["colors"][0:3] = int(red), int(green), int(blue)


@endpoint('razer.device.lighting.custom', 'setRippleRandomColour', in_sig='d')
def set_ripple_effect_random_colour(self, refresh_rate):
    """
    Set the daemon to serve a ripple effect of random colours

    :param refresh_rate: Refresh rate
    :type refresh_rate: int
    """
    self.logger.debug("DBus call set_ripple_effect")

    # Notify others
    self.send_effect_event('setRipple', None, None, None, refresh_rate)

    # remember effect
    self.set_persistence("backlight", "effect", 'rippleRandomColour')


@endpoint('razer.device.lighting.chroma', 'setStarlightRandom', in_sig='y')
def set_starlight_random_effect(self, speed):
    """
    Set startlight random mode
    """
    self.logger.debug("DBus call set_starlight_random")

    driver_path = self.get_driver_path('matrix_effect_starlight')

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(bytes([speed]))

    # Notify others
    self.send_effect_event('setStarlightRandom')

    # remember effect
    self.set_persistence("backlight", "effect", 'starlightRandom')
    self.set_persistence("backlight", "speed", int(speed))


@endpoint('razer.device.lighting.chroma', 'setStarlightSingle', in_sig='yyyy')
def set_starlight_single_effect(self, red, green, blue, speed):
    """
    Set starlight mode
    """
    self.logger.debug("DBus call set_starlight_single")

    driver_path = self.get_driver_path('matrix_effect_starlight')

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(bytes([speed, red, green, blue]))

    # Notify others
    self.send_effect_event('setStarlightSingle', red, green, blue, speed)

    # remember effect
    self.set_persistence("backlight", "effect", 'starlightSingle')
    self.set_persistence("backlight", "speed", int(speed))
    self.zone["backlight"]["colors"][0:3] = int(red), int(green), int(blue)


@endpoint('razer.device.lighting.chroma', 'setStarlightDual', in_sig='yyyyyyy')
def set_starlight_dual_effect(self, red1, green1, blue1, red2, green2, blue2, speed):
    """
    Set starlight dual mode
    """
    self.logger.debug("DBus call set_starlight_dual")

    driver_path = self.get_driver_path('matrix_effect_starlight')

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(bytes([speed, red1, green1, blue1, red2, green2, blue2]))

    # Notify others
    self.send_effect_event('setStarlightDual', red1, green1, blue1, red2, green2, blue2, speed)

    # remember effect
    self.set_persistence("backlight", "effect", 'starlightDual')
    self.set_persistence("backlight", "speed", int(speed))
    self.zone["backlight"]["colors"][0:6] = int(red1), int(green1), int(blue1), int(red2), int(green2), int(blue2)
