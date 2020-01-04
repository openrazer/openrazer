from openrazer_daemon.dbus_services import endpoint
from openrazer_daemon.dbus_services.dbus_methods.lanceheadte import get_left_brightness as _get_left_brightness, \
    get_right_brightness as _get_right_brightness, \
    set_left_wave as _set_left_wave, set_right_wave as _set_right_wave, \
    set_left_static as _set_left_static, set_right_static as _set_right_static, \
    set_left_spectrum as _set_left_spectrum, set_right_spectrum as _set_right_spectrum, \
    set_left_none as _set_left_none, set_right_none as _set_right_none, \
    set_left_reactive as _set_left_reactive, set_right_reactive as _set_right_reactive, \
    set_left_breath_random as _set_left_breath_random, set_right_breath_random as _set_right_breath_random, \
    set_left_breath_single as _set_left_breath_single, set_right_breath_single as _set_right_breath_single, \
    set_left_breath_dual as _set_left_breath_dual, set_right_breath_dual as _set_right_breath_dual


@endpoint('razer.device.lighting.brightness', 'getBrightness', out_sig='d')
def get_brightness_mamba_elite(self):
    """
    Get the device's brightness not supported and splited into set of brightnesses(scroll, logo left and right)
    Current brightness == a left+right

    :return: Brightness
    :rtype: float
    """
    self.logger.debug("DBus call get_brightness")
    return (_get_left_brightness(self) + _get_right_brightness(self)) / 2


@endpoint('razer.device.lighting.chroma', 'setWave', in_sig='i')
def set_wave_effect_mamba_elite(self, direction):
    """
    Set the wave effect on the device

    :param direction: 1 - left to right, 2 right to left
    :type direction: int
    """
    self.logger.debug("DBus call set_wave_effect_mamba_elite")

    # direction
    _set_left_wave(self, direction)
    _set_right_wave(self, direction)


@endpoint('razer.device.lighting.chroma', 'setStatic', in_sig='yyy')
def set_static_effect_mamba_elite(self, red, green, blue):
    """
    Set the device to static colour

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_static_effect_mamba_elite")

    # Notify others
    self.send_effect_event('setStatic', red, green, blue)

    _set_left_static(self, red, green, blue)
    _set_right_static(self, red, green, blue)


@endpoint('razer.device.lighting.chroma', 'setSpectrum')
def set_spectrum_effect_mamba_elite(self):
    """
    Set the device to spectrum mode
    """
    self.logger.debug("DBus call set_spectrum_effect_mamba_elite")

    # Notify others
    self.send_effect_event('setSpectrum')

    _set_left_spectrum(self)
    _set_right_spectrum(self)


@endpoint('razer.device.lighting.chroma', 'setNone')
def set_none_effect_mamba_elite(self):
    """
    Set the device to spectrum mode
    """
    self.logger.debug("DBus call set_none_effect_mamba_elite")

    # Notify others
    self.send_effect_event('setNone')
    _set_left_none(self)
    _set_right_none(self)


@endpoint('razer.device.lighting.chroma', 'setReactive', in_sig='yyyy')
def set_reactive_effect_mamba_elite(self, red, green, blue, speed):
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

    # Notify others
    self.send_effect_event('setReactive', red, green, blue, speed)

    if speed not in (1, 2, 3, 4):
        speed = 4

    _set_left_reactive(self, red, green, blue, speed)
    _set_right_reactive(self, red, green, blue, speed)


@endpoint('razer.device.lighting.chroma', 'setBreathRandom')
def set_breath_random_effect_mamba_elite(self):
    """
    Set the device to random colour breathing effect
    """
    self.logger.debug("DBus call set_breath_random_effect_mamba_elite")

    # Notify others
    self.send_effect_event('setBreathRandom')
    _set_left_breath_random(self)
    _set_right_breath_random(self)


@endpoint('razer.device.lighting.chroma', 'setBreathSingle', in_sig='yyy')
def set_breath_single_effect_mamba_elite(self, red, green, blue):
    """
    Set the device to single colour breathing effect

    :param red: Red component
    :type red: int

    :param green: Green component
    :type green: int

    :param blue: Blue component
    :type blue: int
    """
    self.logger.debug("DBus call set_breath_single_effect_mamba_elite")

    # Notify others
    self.send_effect_event('setBreathSingle', red, green, blue)

    _set_left_breath_single(self, red, green, blue)
    _set_right_breath_single(self, red, green, blue)


@endpoint('razer.device.lighting.chroma', 'setBreathDual', in_sig='yyyyyy')
def set_breath_dual_effect_mamba_elite(self, red1, green1, blue1, red2, green2, blue2):
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
    self.logger.debug("DBus call set_breath_dual_effect_mamba_elite")

    # Notify others
    self.send_effect_event('setBreathDual', red1, green1, blue1, red2, green2, blue2)

    driver_path = self.get_driver_path('matrix_effect_breath')

    _set_left_breath_dual(self, red1, green1, blue1, red2, green2, blue2)
    _set_right_breath_dual(self, red1, green1, blue1, red2, green2, blue2)
