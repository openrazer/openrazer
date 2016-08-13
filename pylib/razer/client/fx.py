import dbus as _dbus
from razer.client.constants import WAVE_LEFT, WAVE_RIGHT, REACTIVE_500MS, REACTIVE_1000MS, REACTIVE_1500MS, REACTIVE_2000MS


def clamp_ubyte(value):
    """
    Clamp a value to 0->255

    :param value: Integer
    :type value: int

    :return: Integer 0->255
    :rtype: int
    """
    if value > 255:
        value = 255
    elif value < 0:
        value = 0
    return value


# Default Chroma lighting
class RazerFX(object):
    # TODO custom effect activation, set_key_row, rippleSingle, rippleRandom

    def __init__(self, serial:str, capabilities:dict, daemon_dbus=None):
        self._capabilities = capabilities

        if daemon_dbus is None:
            session_bus = _dbus.SessionBus()
            daemon_dbus = session_bus.get_object("org.razer", "/org/razer/device/{0}".format(serial))

        self._lighting_dbus = _dbus.Interface(daemon_dbus, "razer.device.lighting.chroma")

        if self.has('ripple'):
            self._custom_lighting_dbus = _dbus.Interface(daemon_dbus, "razer.device.lighting.custom")
        else:
            self._custom_lighting_dbus = None

    def has(self, capability:str) -> bool:
        """
        Convenience function to check capability

        Uses the main device capability list and automatically prefixes 'lighting_'
        :param capability: Device capability
        :type capability: str

        :return: True or False
        :rtype: bool
        """
        return self._capabilities.get('lighting_' + capability, False)

    def none(self) -> bool:
        """
        No effect

        :return: True if success, False otherwise
        :rtype: bool
        """
        if self.has('none'):
            self._lighting_dbus.setNone()

            return True
        return False

    def spectrum(self) -> bool:
        """
        Spectrum effect

        :return: True if success, False otherwise
        :rtype: bool
        """
        if self.has('spectrum'):
            self._lighting_dbus.setSpectrum()

            return True
        return False

    def wave(self, direction:int) -> bool:
        """
        Wave effect

        :param direction: Wave direction either WAVE_RIGHT (0x01) or WAVE_LEFT (0x02)
        :type direction: int

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If direction is invalid
        """
        if direction not in (WAVE_LEFT, WAVE_RIGHT):
            raise ValueError("Direction must be WAVE_RIGHT (0x01) or WAVE_LEFT (0x02)")

        if self.has('wave'):
            self._lighting_dbus.setWave(direction)

            return True
        return False

    def static(self, red:int, green:int, blue:int) -> bool:
        """
        Wave effect

        :param red: Red component. Must be 0->255
        :type red: int

        :param green: Green component. Must be 0->255
        :type green: int

        :param blue: Blue component. Must be 0->255
        :type blue: int

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(red, int):
            raise ValueError("Red is not an integer")
        if not isinstance(green, int):
            raise ValueError("Green is not an integer")
        if not isinstance(blue, int):
            raise ValueError("Blue is not an integer")

        if self.has('static'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)

            self._lighting_dbus.setStatic(red, green, blue)

            return True
        return False

    def reactive(self, time:int, red:int, green:int, blue:int) -> bool:
        """
        Reactive effect

        :param time: Reactive speed. One of REACTIVE_500MS, REACTIVE_1000MS, REACTIVE_1500MS or REACTIVE_2000MS
        :param time: int

        :param red: Red component. Must be 0->255
        :type red: int

        :param green: Green component. Must be 0->255
        :type green: int

        :param blue: Blue component. Must be 0->255
        :type blue: int

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If parameters are invalid
        """
        if time not in (REACTIVE_500MS, REACTIVE_1000MS, REACTIVE_1500MS, REACTIVE_2000MS):
            raise ValueError("Time not one of REACTIVE_500MS, REACTIVE_1000MS, REACTIVE_1500MS or REACTIVE_2000MS")
        if not isinstance(red, int):
            raise ValueError("Red is not an integer")
        if not isinstance(green, int):
            raise ValueError("Green is not an integer")
        if not isinstance(blue, int):
            raise ValueError("Blue is not an integer")

        if self.has('reactive'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)

            self._lighting_dbus.setReactive(time, red, green, blue)

            return True
        return False

    def breath_single(self, red:int, green:int, blue:int) -> bool:
        """
        Breath effect - single colour

        :param red: Red component. Must be 0->255
        :type red: int

        :param green: Green component. Must be 0->255
        :type green: int

        :param blue: Blue component. Must be 0->255
        :type blue: int

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(red, int):
            raise ValueError("Red is not an integer")
        if not isinstance(green, int):
            raise ValueError("Green is not an integer")
        if not isinstance(blue, int):
            raise ValueError("Blue is not an integer")

        if self.has('breath_single'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)

            self._lighting_dbus.setBreathSingle(red, green, blue)

            return True
        return False

    # TODO Change to tuple of rgb
    def breath_dual(self, red:int, green:int, blue:int, red2:int, green2:int, blue2:int) -> bool:
        """
        Breath effect - single colour

        :param red: First red component. Must be 0->255
        :type red: int

        :param green: First green component. Must be 0->255
        :type green: int

        :param blue: First blue component. Must be 0->255
        :type blue: int

        :param red2: Second red component. Must be 0->255
        :type red2: int

        :param green2: Second green component. Must be 0->255
        :type green2: int

        :param blue2: Second blue component. Must be 0->255
        :type blue2: int

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(red, int):
            raise ValueError("Primary red is not an integer")
        if not isinstance(green, int):
            raise ValueError("Primary green is not an integer")
        if not isinstance(blue, int):
            raise ValueError("Primary blue is not an integer")
        if not isinstance(red2, int):
            raise ValueError("Secondary red is not an integer")
        if not isinstance(green2, int):
            raise ValueError("Secondary green is not an integer")
        if not isinstance(blue2, int):
            raise ValueError("Secondary blue is not an integer")

        if self.has('breath_dual'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)
            red2 = clamp_ubyte(red2)
            green2 = clamp_ubyte(green2)
            blue2 = clamp_ubyte(blue2)

            self._lighting_dbus.setBreathDual(red, green, blue, red2, green2, blue2)

            return True
        return False

    def breath_random(self) -> bool:
        """
        Breath effect - random colours

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If parameters are invalid
        """

        if self.has('breath_random'):
            self._lighting_dbus.setBreathRandom()

            return True
        return False

    def ripple(self, red:int, green:int, blue:int, refreshrate:float):
        if not isinstance(refreshrate, float):
            raise ValueError("Refresh rate is not a float")
        if not isinstance(red, int):
            raise ValueError("Red is not an integer")
        if not isinstance(green, int):
            raise ValueError("Green is not an integer")
        if not isinstance(blue, int):
            raise ValueError("Blue is not an integer")

        if self.has('ripple'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)

            self._custom_lighting_dbus.setRipple(red, green, blue, refreshrate)

            return True
        return False

    def ripple_random(self, refreshrate:float):
        if not isinstance(refreshrate, float):
            raise ValueError("Refresh rate is not a float")

        if self.has('ripple'):
            self._custom_lighting_dbus.setRippleRandomColour(refreshrate)

            return True
        return False


