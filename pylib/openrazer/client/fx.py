import numpy as _np
import dbus as _dbus
#from openrazer.client.constants import WAVE_LEFT, WAVE_RIGHT, REACTIVE_500MS, REACTIVE_1000MS, REACTIVE_1500MS, REACTIVE_2000MS
from openrazer.client import constants as c

# TODO logging.debug if value out of range v1.1


def clamp_ubyte(value):
    """
    Clamp a value to 0->255

    Aka -3453
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
class BaseRazerFX(object):
    def __init__(self, serial: str, capabilities: dict, daemon_dbus=None):
        self._capabilities = capabilities

        if daemon_dbus is None:
            session_bus = _dbus.SessionBus()
            daemon_dbus = session_bus.get_object("org.razer", "/org/razer/device/{0}".format(serial))
        self._dbus = daemon_dbus

    def has(self, capability: str) -> bool:
        """
        Convenience function to check capability

        Uses the main device capability list and automatically prefixes 'lighting_'
        :param capability: Device capability
        :type capability: str

        :return: True or False
        :rtype: bool
        """
        return self._capabilities.get('lighting_' + capability, False)


class RazerFX(BaseRazerFX):
    def __init__(self, serial: str, capabilities: dict, daemon_dbus=None, matrix_dims=(-1, -1)):
        super().__init__(serial, capabilities, daemon_dbus)

        self._lighting_dbus = _dbus.Interface(self._dbus, "razer.device.lighting.chroma")

        # all() part basically checks that all dimensions are present (-1 is bad)
        if self.has('led_matrix') and all([dim >= 1 for dim in matrix_dims]):
            self.advanced = RazerAdvancedFX(serial, capabilities, daemon_dbus=self._dbus, matrix_dims=matrix_dims)
        else:
            self.advanced = None

        # Only keyboards will have ripple set
        if self.has('led_matrix') and self.has('ripple'):
            self._custom_lighting_dbus = _dbus.Interface(self._dbus, "razer.device.lighting.custom")
        else:
            self._custom_lighting_dbus = None

        self.misc = MiscLighting(serial, capabilities, self._dbus)

    @property
    def effect(self) -> str:
        """
        Get current effect

        :return: Effect name ("static", "spectrum", etc.)
        :rtype: str
        """
        return self._lighting_dbus.getEffect()

    @property
    def colors(self) -> bytearray:
        """
        Get current effect colors

        :return: Effect colors (an array of 9 bytes, for 3 colors in RGB format)
        :rtype: bytearray
        """
        return bytes(self._lighting_dbus.getEffectColors())

    @property
    def speed(self) -> int:
        """
        Get current effect speed

        :return: Effect speed (a value between 0 and 3)
        :rtype: int
        """
        return self._lighting_dbus.getEffectSpeed()

    @property
    def wave_dir(self) -> int:
        """
        Get current wave direction

        :return: Wave direction (WAVE_LEFT or WAVE_RIGHT)
        :rtype: int
        """
        return self._lighting_dbus.getWaveDir()

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

    def wave(self, direction: int) -> bool:
        """
        Wave effect

        :param direction: Wave direction either WAVE_RIGHT (0x01) or WAVE_LEFT (0x02)
        :type direction: int

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If direction is invalid
        """
        if direction not in (c.WAVE_LEFT, c.WAVE_RIGHT):
            raise ValueError("Direction must be WAVE_RIGHT (0x01) or WAVE_LEFT (0x02)")

        if self.has('wave'):
            self._lighting_dbus.setWave(direction)

            return True
        return False

    def static(self, red: int, green: int, blue: int) -> bool:
        """
        Static effect

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

    def reactive(self, red: int, green: int, blue: int, time: int) -> bool:
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
        if time not in (c.REACTIVE_500MS, c.REACTIVE_1000MS, c.REACTIVE_1500MS, c.REACTIVE_2000MS):
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

            self._lighting_dbus.setReactive(red, green, blue, time)

            return True
        return False

    def breath_single(self, red: int, green: int, blue: int) -> bool:
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
    def breath_dual(self, red: int, green: int, blue: int, red2: int, green2: int, blue2: int) -> bool:
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

    def breath_triple(self, red: int, green: int, blue: int, red2: int, green2: int, blue2: int, red3: int, green3: int, blue3: int) -> bool:
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

        :param red3: Second red component. Must be 0->255
        :type red3: int

        :param green3: Second green component. Must be 0->255
        :type green3: int

        :param blue3: Second blue component. Must be 0->255
        :type blue3: int

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
        if not isinstance(red3, int):
            raise ValueError("Tertiary red is not an integer")
        if not isinstance(green3, int):
            raise ValueError("Tertiary green is not an integer")
        if not isinstance(blue3, int):
            raise ValueError("Tertiary blue is not an integer")

        if self.has('breath_triple'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)
            red2 = clamp_ubyte(red2)
            green2 = clamp_ubyte(green2)
            blue2 = clamp_ubyte(blue2)
            red3 = clamp_ubyte(red3)
            green3 = clamp_ubyte(green3)
            blue3 = clamp_ubyte(blue3)

            self._lighting_dbus.setBreathTriple(red, green, blue, red2, green2, blue2, red3, green3, blue3)

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

    def ripple(self, red: int, green: int, blue: int, refreshrate: float = c.RIPPLE_REFRESH_RATE) -> bool:
        """
        Set the Ripple Effect.

        The refresh rate should be set to about 0.05 for a decent effect
        :param red: Red RGB component
        :rtype red: int

        :param green: Green RGB component
        :type green: int

        :param blue: Blue RGB component
        :type blue: int

        :param refreshrate: Effect refresh rate
        :type refreshrate: float

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If arguments are invalid
        """
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

    def ripple_random(self, refreshrate: float = c.RIPPLE_REFRESH_RATE):
        """
        Set the Ripple Effect with random colours

        The refresh rate should be set to about 0.05 for a decent effect
        :param refreshrate: Effect refresh rate
        :type refreshrate: float

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If arguments are invalid
        """
        if not isinstance(refreshrate, float):
            raise ValueError("Refresh rate is not a float")

        if self.has('ripple_random'):
            self._custom_lighting_dbus.setRippleRandomColour(refreshrate)

            return True
        return False

    def starlight_single(self, red: int, green: int, blue: int, time: int) -> bool:
        """
        Starlight effect

        :param red: Red component. Must be 0->255
        :type red: int

        :param green: Green component. Must be 0->255
        :type green: int

        :param blue: Blue component. Must be 0->255
        :type blue: int

        :param time: Starlight speed
        :type time: int

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If parameters are invalid
        """
        if time not in (c.STARLIGHT_FAST, c.STARLIGHT_NORMAL, c.STARLIGHT_SLOW):
            raise ValueError("Time not one of STARLIGHT_FAST, STARLIGHT_NORMAL or STARLIGHT_SLOW")
        if not isinstance(red, int):
            raise ValueError("Red is not an integer")
        if not isinstance(green, int):
            raise ValueError("Green is not an integer")
        if not isinstance(blue, int):
            raise ValueError("Blue is not an integer")

        if self.has('starlight_single'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)

            self._lighting_dbus.setStarlightSingle(red, green, blue, time)

            return True
        return False

    def starlight_dual(self, red: int, green: int, blue: int, red2: int, green2: int, blue2: int, time: int) -> bool:
        """
        Starlight effect

        :param red: Red component. Must be 0->255
        :type red: int

        :param green: Green component. Must be 0->255
        :type green: int

        :param blue: Blue component. Must be 0->255
        :type blue: int

        :param red2: Red component. Must be 0->255
        :type red2: int

        :param green2: Green component. Must be 0->255
        :type green2: int

        :param blue2: Blue component. Must be 0->255
        :type blue2: int

        :param time: Starlight speed
        :type time: int

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If parameters are invalid
        """
        if time not in (c.STARLIGHT_FAST, c.STARLIGHT_NORMAL, c.STARLIGHT_SLOW):
            raise ValueError("Time not one of STARLIGHT_FAST, STARLIGHT_NORMAL or STARLIGHT_SLOW")
        if not isinstance(red, int):
            raise ValueError("Red is not an integer")
        if not isinstance(green, int):
            raise ValueError("Green is not an integer")
        if not isinstance(blue, int):
            raise ValueError("Blue is not an integer")
        if not isinstance(red2, int):
            raise ValueError("Red 2 is not an integer")
        if not isinstance(green2, int):
            raise ValueError("Green 2 is not an integer")
        if not isinstance(blue2, int):
            raise ValueError("Blue 2 is not an integer")

        if self.has('starlight_dual'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)
            red2 = clamp_ubyte(red2)
            green2 = clamp_ubyte(green2)
            blue2 = clamp_ubyte(blue2)

            self._lighting_dbus.setStarlightDual(red, green, blue, red2, green2, blue2, time)

            return True
        return False

    def starlight_random(self, time: int) -> bool:
        """
        Starlight effect

        :param time: Starlight speed
        :type time: int

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If parameters are invalid
        """
        if time not in (c.STARLIGHT_FAST, c.STARLIGHT_NORMAL, c.STARLIGHT_SLOW):
            raise ValueError("Time not one of STARLIGHT_FAST, STARLIGHT_NORMAL or STARLIGHT_SLOW")

        if self.has('starlight_random'):
            self._lighting_dbus.setStarlightRandom(time)

            return True
        return False


class RazerAdvancedFX(BaseRazerFX):
    def __init__(self, serial: str, capabilities: dict, daemon_dbus=None, matrix_dims=(-1, -1)):
        super().__init__(serial, capabilities, daemon_dbus)

        # Only init'd when there's a matrix
        self._capabilities = capabilities

        if not all([dim >= 1 for dim in matrix_dims]):
            raise ValueError("Matrix dimensions cannot contain -1")

        if daemon_dbus is None:
            session_bus = _dbus.SessionBus()
            daemon_dbus = session_bus.get_object("org.razer", "/org/razer/device/{0}".format(serial))

        self._matrix_dims = matrix_dims
        self._lighting_dbus = _dbus.Interface(daemon_dbus, "razer.device.lighting.chroma")

        self.matrix = Frame(matrix_dims)

    @property
    def cols(self):
        """
        Number of columns in matrix

        :return: Columns
        :rtype: int
        """
        return self._matrix_dims[1]

    @property
    def rows(self):
        """
        Number of rows in matrix

        :return: Rows
        :rtype: int
        """
        return self._matrix_dims[0]

    def _draw(self, ba):
        self._lighting_dbus.setKeyRow(ba)

        self._lighting_dbus.setCustom()

    def draw(self):
        """
        Draw what's in the current frame buffer
        """
        self._draw(bytes(self.matrix))

    def draw_fb_or(self):
        self._draw(bytes(self.matrix.draw_with_fb_or()))

    def set_key(self, column_id, rgb, row_id=0):  # Not needed on mice
        if self.has('led_single'):
            if isinstance(rgb, (tuple, list)) and len(rgb) == 3 and all([isinstance(component, int) for component in rgb]):
                if row_id < self._matrix_dims[0] and column_id < self._matrix_dims[1]:
                    self._lighting_dbus.setKey(row_id, column_id, [clamp_ubyte(component) for component in rgb])
                else:
                    raise ValueError("Row or column out of bounds. Max dimensions are: {0},{1}".format(*self._matrix_dims))
            else:
                raise ValueError("RGB must be an RGB tuple")

    def restore(self):
        """
        Restore the device to the last effect
        """
        self._lighting_dbus.restoreLastEffect()


class SingleLed(BaseRazerFX):
    def __init__(self, serial: str, capabilities: dict, daemon_dbus=None, led_name='logo'):
        super().__init__(serial, capabilities, daemon_dbus)

        self._led_name = led_name
        self._lighting_dbus = _dbus.Interface(self._dbus, "razer.device.lighting.{0}".format(led_name))

    def _shas(self, item):
        return self.has('{0}_{1}'.format(self._led_name, item))

    def _getattr(self, name):
        attr = name.replace('#', self._led_name.title())
        return getattr(self._lighting_dbus, attr, None)

    @property
    def active(self) -> bool:
        func = self._getattr('get#Active')
        if func is not None:
            return func()
        else:
            return False

    @active.setter
    def active(self, value: bool):
        func = self._getattr('set#Active')
        if func is not None:
            if value:
                func(True)
            else:
                func(False)

    @property
    def effect(self) -> str:
        """
        Get current effect

        :return: Effect name ("static", "spectrum", etc.)
        :rtype: str
        """
        return str(self._getattr('get#Effect')())

    @property
    def colors(self) -> bytearray:
        """
        Get current effect colors

        :return: Effect colors (an array of 9 bytes, for 3 colors in RGB format)
        :rtype: bytearray
        """
        return bytes(self._getattr('get#EffectColors')())

    @property
    def speed(self) -> int:
        """
        Get current effect speed

        :return: Effect speed (a value between 0 and 3)
        :rtype: int
        """
        return int(self._getattr('get#EffectSpeed')())

    @property
    def wave_dir(self) -> int:
        """
        Get current wave direction

        :return: Wave direction (WAVE_LEFT or WAVE_RIGHT)
        :rtype: int
        """
        return int(self._getattr('get#WaveDir')())

    @property
    def brightness(self):
        if self._shas('brightness'):
            return float(self._getattr('get#Brightness')())
        return 0.0

    @brightness.setter
    def brightness(self, brightness: float):
        if self._shas('brightness'):
            if not isinstance(brightness, (float, int)):
                raise ValueError("Brightness is not a float")

            if brightness > 100:
                brightness = 100.0
            elif brightness < 0:
                brightness = 0.0

            self._getattr('set#Brightness')(brightness)

    def blinking(self, red: int, green: int, blue: int) -> bool:
        if not isinstance(red, int):
            raise ValueError("Red is not an integer")
        if not isinstance(green, int):
            raise ValueError("Green is not an integer")
        if not isinstance(blue, int):
            raise ValueError("Blue is not an integer")

        if self._shas('blinking'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)

            self._getattr('set#Blinking')(red, green, blue)

            return True
        return False

    def pulsate(self, red: int, green: int, blue: int) -> bool:
        if not isinstance(red, int):
            raise ValueError("Red is not an integer")
        if not isinstance(green, int):
            raise ValueError("Green is not an integer")
        if not isinstance(blue, int):
            raise ValueError("Blue is not an integer")

        if self._shas('pulsate'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)

            self._getattr('set#Pulsate')(red, green, blue)

            return True
        return False

    def static(self, red: int, green: int, blue: int) -> bool:
        if not isinstance(red, int):
            raise ValueError("Red is not an integer")
        if not isinstance(green, int):
            raise ValueError("Green is not an integer")
        if not isinstance(blue, int):
            raise ValueError("Blue is not an integer")

        if self._shas('static'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)

            self._getattr('set#Static')(red, green, blue)

            return True
        return False

    def wave(self, direction: int) -> bool:
        if direction not in (c.WAVE_LEFT, c.WAVE_RIGHT):
            raise ValueError("Direction must be WAVE_RIGHT (0x01) or WAVE_LEFT (0x02)")

        if self._shas('wave'):
            self._getattr('set#Wave')(direction)

            return True
        return False

    def none(self) -> bool:
        if self._shas('none'):
            self._getattr('set#None')()

            return True
        return False

    def spectrum(self) -> bool:
        if self._shas('spectrum'):
            self._getattr('set#Spectrum')()

            return True
        return False

    def reactive(self, red: int, green: int, blue: int, time: int) -> bool:
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
        if time not in (c.REACTIVE_500MS, c.REACTIVE_1000MS, c.REACTIVE_1500MS, c.REACTIVE_2000MS):
            raise ValueError("Time not one of REACTIVE_500MS, REACTIVE_1000MS, REACTIVE_1500MS or REACTIVE_2000MS")
        if not isinstance(red, int):
            raise ValueError("Red is not an integer")
        if not isinstance(green, int):
            raise ValueError("Green is not an integer")
        if not isinstance(blue, int):
            raise ValueError("Blue is not an integer")

        if self._shas('reactive'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)

            self._getattr('set#Reactive')(red, green, blue, time)

            return True
        return False

    def breath_single(self, red: int, green: int, blue: int) -> bool:
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

        if self._shas('breath_single'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)

            self._getattr('set#BreathSingle')(red, green, blue)

            return True
        return False

    # TODO Change to tuple of rgb
    def breath_dual(self, red: int, green: int, blue: int, red2: int, green2: int, blue2: int) -> bool:
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

        if self._shas('breath_dual'):
            red = clamp_ubyte(red)
            green = clamp_ubyte(green)
            blue = clamp_ubyte(blue)
            red2 = clamp_ubyte(red2)
            green2 = clamp_ubyte(green2)
            blue2 = clamp_ubyte(blue2)

            self._getattr('set#BreathDual')(red, green, blue, red2, green2, blue2)

            return True
        return False

    def breath_random(self) -> bool:
        """
        Breath effect - random colours

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If parameters are invalid
        """

        if self._shas('breath_random'):
            self._getattr('set#BreathRandom')()

            return True
        return False


class MiscLighting(BaseRazerFX):
    def __init__(self, serial: str, capabilities: dict, daemon_dbus=None):
        super().__init__(serial, capabilities, daemon_dbus)

        self._lighting_dbus = _dbus.Interface(self._dbus, "razer.device.lighting.logo")

        if self.has('logo'):
            self._logo = SingleLed(serial, capabilities, daemon_dbus, 'logo')
        else:
            self._logo = None

        if self.has('scroll'):
            self._scroll = SingleLed(serial, capabilities, daemon_dbus, 'scroll')
        else:
            self._scroll = None

        if self.has('left'):
            self._left = SingleLed(serial, capabilities, daemon_dbus, 'left')
        else:
            self._left = None

        if self.has('right'):
            self._right = SingleLed(serial, capabilities, daemon_dbus, 'right')
        else:
            self._right = None

        if self.has('charging'):
            self._charging = SingleLed(serial, capabilities, daemon_dbus, 'charging')
        else:
            self._charging = None

        if self.has('fast_charging'):
            self._fast_charging = SingleLed(serial, capabilities, daemon_dbus, 'fast_charging')
        else:
            self._fast_charging = None

        if self.has('fully_charged'):
            self._fully_charged = SingleLed(serial, capabilities, daemon_dbus, 'fully_charged')
        else:
            self._fully_charged = None

        if self.has('backlight'):
            self._backlight = SingleLed(serial, capabilities, daemon_dbus, 'backlight')
        else:
            self._backlight = None

    @property
    def logo(self):
        return self._logo

    @property
    def scroll_wheel(self):
        return self._scroll

    @property
    def left(self):
        return self._left

    @property
    def right(self):
        return self._right

    @property
    def charging(self):
        return self._charging

    @property
    def fast_charging(self):
        return self._fast_charging

    @property
    def fully_charged(self):
        return self._fully_charged

    @property
    def backlight(self):
        return self._backlight


class Frame(object):
    """
    Class to represent the RGB matrix of the keyboard. So to animate you'd use multiple frames
    """

    def __init__(self, dimensions):
        self._rows, self._cols = dimensions
        self._components = 3

        self._matrix = None
        self._fb1 = None
        self.reset()

    # Index with row, col OR y, x
    def __getitem__(self, key: tuple) -> tuple:
        """
        Method to allow a slice to get an RGB tuple

        :param key: Key, must be y,x tuple
        :type key: tuple

        :return: RGB tuple
        :rtype: tuple

        :raises AssertionError: If key is invalid
        """
        assert isinstance(key, tuple), "Key is not a tuple"
        assert 0 <= key[0] < self._rows, "Row out of bounds"
        assert 0 <= key[1] < self._cols, "Column out of bounds"

        return tuple(self._matrix[:, key[0], key[1]])

    # Index with row, col OR y, x
    def __setitem__(self, key: tuple, rgb: tuple):
        """
        Method to allow a slice to set an RGB tuple

        :param key: Key, must be y,x tuple
        :type key: tuple

        :param rgb: RGB tuple
        :type rgb: tuple

        :raises AssertionError: If key is invalid
        """
        assert isinstance(key, tuple), "Key is not a tuple"
        assert 0 <= key[0] < self._rows, "Row out of bounds"
        assert 0 <= key[1] < self._cols, "Column out of bounds"
        assert isinstance(rgb, (list, tuple)) and len(rgb) == 3, "Value must be a tuple,list of 3 RGB components"

        self._matrix[:, key[0], key[1]] = rgb

    def __bytes__(self) -> bytes:
        """
        When bytes() is ran on the class will return a binary capable of being sent to the driver

        :return: Driver binary payload
        :rtype: bytes
        """
        return b''.join([self.row_binary(row_id) for row_id in range(0, self._rows)])

    def reset(self):
        """
        Init/Clear the matrix
        """
        if self._matrix is None:
            self._matrix = _np.zeros((self._components, self._rows, self._cols), 'uint8')
            self._fb1 = _np.copy(self._matrix)
        else:
            self._matrix.fill(0)

    def set(self, y: int, x: int, rgb: tuple):
        """
        Method to allow a slice to set an RGB tuple

        :param y: Row
        :type y: int

        :param x: Column
        :type x: int

        :param rgb: RGB tuple
        :type rgb: tuple

        :raises AssertionError: If key is invalid
        """
        self.__setitem__((y, x), rgb)

    def get(self, y: int, x: int) -> list:
        """
        Method to allow a slice to get an RGB tuple

        :param y: Row
        :type y: int

        :param x: Column
        :type x: int

        :return rgb: RGB tuple
        :return rgb: tuple

        :raises AssertionError: If key is invalid
        """
        return self.__getitem__((y, x))

    def row_binary(self, row_id: int) -> bytes:
        """
        Get binary payload for 1 row which is compatible with the driver

        :param row_id: Row ID
        :type row_id: int

        :return: Binary payload
        :rtype: bytes
        """
        assert 0 <= row_id < self._rows, "Row out of bounds"

        start = 0
        end = self._cols - 1

        return row_id.to_bytes(1, byteorder='big') + start.to_bytes(1, byteorder='big') + end.to_bytes(1, byteorder='big') + self._matrix[:, row_id].tobytes(order='F')

    def to_binary(self):
        """
        Get the whole binary for the keyboard to be sent to the driver.

        :return: Driver binary payload
        :rtype: bytes
        """
        return bytes(self)

    # Simple FB
    def to_framebuffer(self):
        self._fb1 = _np.copy(self._matrix)

    def to_framebuffer_or(self):
        self._fb1 = _np.bitwise_or(self._fb1, self._matrix)  # pylint: disable=no-member

    def draw_with_fb_or(self):
        self._matrix = _np.bitwise_or(self._fb1, self._matrix)  # pylint: disable=no-member
        return bytes(self)
