import json
import dbus as _dbus
from openrazer.client.fx import RazerFX as _RazerFX
from xml.etree import ElementTree as _ET
from openrazer.client.macro import RazerMacro as _RazerMacro


class RazerDevice(object):
    """
    Raw razer base device
    """
    _FX = _RazerFX
    _MACRO_CLASS = _RazerMacro

    def __init__(self, serial, vid_pid=None, daemon_dbus=None):
        # Load up the DBus
        if daemon_dbus is None:
            session_bus = _dbus.SessionBus()
            daemon_dbus = session_bus.get_object("org.razer", "/org/razer/device/{0}".format(serial))

        self._dbus = daemon_dbus

        self._available_features = self._get_available_features()

        self._dbus_interfaces = {
            'device': _dbus.Interface(self._dbus, "razer.device.misc"),
            'brightness': _dbus.Interface(self._dbus, "razer.device.lighting.brightness")
        }

        self._name = str(self._dbus_interfaces['device'].getDeviceName())
        self._type = str(self._dbus_interfaces['device'].getDeviceType())
        self._fw = str(self._dbus_interfaces['device'].getFirmware())
        self._drv_version = str(self._dbus_interfaces['device'].getDriverVersion())
        self._has_dedicated_macro = None
        self._urls = None

        if vid_pid is None:
            self._vid, self._pid = self._dbus_interfaces['device'].getVidPid()
        else:
            self._vid, self._pid = vid_pid

        self._serial = serial

        self._capabilities = {
            'name': True,
            'type': True,
            'firmware_version': True,
            'serial': True,
            'brightness': self._has_feature('razer.device.lighting.brightness'),

            'macro_logic': self._has_feature('razer.device.macro'),

            # Default device is a chroma so lighting capabilities
            'lighting': self._has_feature('razer.device.lighting.chroma'),
            'lighting_breath_single': self._has_feature('razer.device.lighting.chroma', 'setBreathSingle'),
            'lighting_breath_dual': self._has_feature('razer.device.lighting.chroma', 'setBreathDual'),
            'lighting_breath_triple': self._has_feature('razer.device.lighting.chroma', 'setBreathTriple'),
            'lighting_breath_random': self._has_feature('razer.device.lighting.chroma', 'setBreathRandom'),
            'lighting_wave': self._has_feature('razer.device.lighting.chroma', 'setWave'),
            'lighting_reactive': self._has_feature('razer.device.lighting.chroma', 'setReactive'),
            'lighting_none': self._has_feature('razer.device.lighting.chroma', 'setNone'),
            'lighting_spectrum': self._has_feature('razer.device.lighting.chroma', 'setSpectrum'),
            'lighting_static': self._has_feature('razer.device.lighting.chroma', 'setStatic'),

            'lighting_starlight_single': self._has_feature('razer.device.lighting.chroma', 'setStarlightSingle'),
            'lighting_starlight_dual': self._has_feature('razer.device.lighting.chroma', 'setStarlightDual'),
            'lighting_starlight_random': self._has_feature('razer.device.lighting.chroma', 'setStarlightRandom'),

            'lighting_ripple': self._has_feature('razer.device.lighting.custom', 'setRipple'),  # Thinking of extending custom to do more hence the key check
            'lighting_ripple_random': self._has_feature('razer.device.lighting.custom', 'setRippleRandomColour'),

            'lighting_pulsate': self._has_feature('razer.device.lighting.bw2013', 'setPulsate'),

            # Get if the device has an LED Matrix, == True as its a DBus boolean otherwise, so for consistency sake we coerce it into a native bool
            'lighting_led_matrix': self._dbus_interfaces['device'].hasMatrix() == True,
            'lighting_led_single': self._has_feature('razer.device.lighting.chroma', 'setKey'),

            # Mouse lighting attrs
            'lighting_logo': self._has_feature('razer.device.lighting.logo'),
            'lighting_logo_active': self._has_feature('razer.device.lighting.logo', 'setLogoActive'),
            'lighting_logo_blinking': self._has_feature('razer.device.lighting.logo', 'setLogoBlinking'),
            'lighting_logo_brightness': self._has_feature('razer.device.lighting.logo', 'setLogoBrightness'),
            'lighting_logo_pulsate': self._has_feature('razer.device.lighting.logo', 'setLogoPulsate'),
            'lighting_logo_spectrum': self._has_feature('razer.device.lighting.logo', 'setLogoSpectrum'),
            'lighting_logo_static': self._has_feature('razer.device.lighting.logo', 'setLogoStatic'),
            'lighting_logo_none': self._has_feature('razer.device.lighting.logo', 'setLogoNone'),
            'lighting_logo_reactive': self._has_feature('razer.device.lighting.logo', 'setLogoReactive'),
            'lighting_logo_breath_single': self._has_feature('razer.device.lighting.logo', 'setLogoBreathSingle'),
            'lighting_logo_breath_dual': self._has_feature('razer.device.lighting.logo', 'setLogoBreathDual'),
            'lighting_logo_breath_random': self._has_feature('razer.device.lighting.logo', 'setLogoBreathRandom'),

            'lighting_scroll': self._has_feature('razer.device.lighting.scroll'),
            'lighting_scroll_active': self._has_feature('razer.device.lighting.scroll', 'setScrollActive'),
            'lighting_scroll_blinking': self._has_feature('razer.device.lighting.scroll', 'setScrollBlinking'),
            'lighting_scroll_brightness': self._has_feature('razer.device.lighting.scroll', 'setScrollBrightness'),
            'lighting_scroll_pulsate': self._has_feature('razer.device.lighting.scroll', 'setScrollPulsate'),
            'lighting_scroll_spectrum': self._has_feature('razer.device.lighting.scroll', 'setScrollSpectrum'),
            'lighting_scroll_static': self._has_feature('razer.device.lighting.scroll', 'setScrollStatic'),
            'lighting_scroll_none': self._has_feature('razer.device.lighting.scroll', 'setScrollNone'),
            'lighting_scroll_reactive': self._has_feature('razer.device.lighting.scroll', 'setScrollReactive'),
            'lighting_scroll_breath_single': self._has_feature('razer.device.lighting.scroll', 'setScrollBreathSingle'),
            'lighting_scroll_breath_dual': self._has_feature('razer.device.lighting.scroll', 'setScrollBreathDual'),
            'lighting_scroll_breath_random': self._has_feature('razer.device.lighting.scroll', 'setScrollBreathRandom'),

            'lighting_backlight': self._has_feature('razer.device.lighting.backlight'),
            'lighting_backlight_active': self._has_feature('razer.device.lighting.backlight', 'setBacklightActive'),
        }

        # Nasty hack to convert dbus.Int32 into native
        self._matrix_dimensions = tuple([int(dim) for dim in self._dbus_interfaces['device'].getMatrixDimensions()])

        # Setup FX
        if self._FX is None:
            self.fx = None
        else:
            self.fx = self._FX(serial, capabilities=self._capabilities, daemon_dbus=daemon_dbus, matrix_dims=self._matrix_dimensions)

        # Setup Macro
        if self.has('macro_logic'):
            if self._MACRO_CLASS is not None:
                self.macro = self._MACRO_CLASS(serial, daemon_dbus=daemon_dbus, capabilities=self._capabilities)
            else:
                self._capabilities['macro_logic'] = False
                self.macro = None
        else:
            self.macro = None

    def _get_available_features(self):
        introspect_interface = _dbus.Interface(self._dbus, 'org.freedesktop.DBus.Introspectable')
        xml_spec = introspect_interface.Introspect()
        root = _ET.fromstring(xml_spec)

        interfaces = {}

        for child in root:

            if child.tag != 'interface' or child.attrib.get('name') == 'org.freedesktop.DBus.Introspectable':
                continue

            current_interface = child.attrib['name']
            current_interface_methods = []

            for method in child:
                if method.tag != 'method':
                    continue

                current_interface_methods.append(method.attrib.get('name'))

            interfaces[current_interface] = current_interface_methods

        return interfaces

    def _has_feature(self, object_path:str, method_name=None) -> bool:
        """
        Checks to see if the device has said DBus method

        :param object_path: Object path
        :type object_path: str

        :param method_name: Method name, or list of methods
        :type method_name: str or list or tuple

        :return: True if method/s exist
        :rtype: bool
        """
        if method_name is None:
            return object_path in self._available_features
        elif isinstance(method_name, str):
            return object_path in self._available_features and method_name in self._available_features[object_path]
        elif isinstance(method_name, (list, tuple)):
            result = True
            for method in method_name:
                result &= object_path in self._available_features and method in self._available_features[object_path]
            return result
        else:
            return False

    def has(self, capability:str) -> bool:
        """
        Convenience function to check capability

        :param capability: Device capability
        :type capability: str

        :return: True or False
        :rtype: bool
        """
        # Could do capability in self._capabilitys but they might be explicitly disabled
        return self._capabilities.get(capability, False)

    @property
    def name(self) -> str:
        """
        Device name

        :return: Device Name
        :rtype: str
        """
        return self._name

    @property
    def type(self) -> str:
        """
        Get device type

        :return: Device Type
        :rtype: str
        """
        return self._type

    @property
    def firmware_version(self) -> str:
        """
        Device's firmware version

        :return: FW Version
        :rtype: str
        """
        return self._fw

    @property
    def driver_version(self) -> str:
        """
        Device's driver version

        :return: Driver Version
        :rtype: str
        """
        return self._drv_version

    @property
    def serial(self) -> str:
        """
        Device's serial

        :return: Device Serial
        :rtype: str
        """
        return self._serial

    @property
    def brightness(self) -> float:
        """
        Get device brightness

        :return: Device brightness
        :rtype: float
        """
        return self._dbus_interfaces['brightness'].getBrightness()

    @brightness.setter
    def brightness(self, value:float):
        """
        Set device brightness

        :param value: Device brightness
        :type value: float

        :raises ValueError: When brightness is not a float or not in range 0.0->100.0
        """
        if isinstance(value, int):
            value = float(value)

        if not isinstance(value, float):
            raise ValueError("Brightness must be a float")

        if value < 0.0 or value > 100.0:
            raise ValueError("Brightness must be between 0 and 100")

        self._dbus_interfaces['brightness'].setBrightness(value)

    @property
    def capabilities(self) -> dict:
        """
        Device capabilities

        :return: Device capabilities
        :rtype: dict
        """
        return self._capabilities

    @property
    def dedicated_macro(self) -> bool:
        """
        Device has dedicated macro keys

        :return: If the device has macro keys
        :rtype: bool
        """
        if self._has_dedicated_macro is None:
            self._has_dedicated_macro = self._dbus_interfaces['device'].hasDedicatedMacroKeys()

        return self._has_dedicated_macro

    @property
    def razer_urls(self) -> dict:
        if self._urls is None:
            self._urls = json.loads(str(self._dbus_interfaces['device'].getRazerUrls()))

        return self._urls

    def __str__(self):
        return self._name

    def __repr__(self):
        return '<{0} {1}>'.format(self.__class__.__name__, self._serial)


class BaseDeviceFactory(object):
    @staticmethod
    def get_device(serial:str, daemon_dbus=None) -> RazerDevice:
        raise NotImplementedError()
