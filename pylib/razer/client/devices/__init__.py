import dbus as _dbus


class RazerDevice(object):
    def __init__(self, serial, daemon_dbus=None):
        # Load up the DBus
        if daemon_dbus is None:
            session_bus = _dbus.SessionBus()
            daemon_dbus = session_bus.get_object("org.razer", "/org/razer/device/{0}".format(serial))

        self._dbus = daemon_dbus
        self._dbus_interfaces = {
            'device': _dbus.Interface(self._dbus, "razer.device.misc"),
            'brightness': _dbus.Interface(self._dbus, "razer.device.lighting.brightness")
        }

        self._name = self._dbus_interfaces['device'].getDeviceName()
        self._type = self._dbus_interfaces['device'].getDeviceType()
        self._fw = self._dbus_interfaces['device'].getFirmware()
        self._serial = serial

        default_capabilities = {
            'name': True,
            'type': True,
            'firmware_version': True,
            'serial': True,
            'brightness': True,

        }
        self._update_capabilities(default_capabilities)

    def _update_capabilities(self, capabilities):
        """
        Update capabilities

        This cheap hack updates the capabilities dict with defaults if keys dont exist
        :param capabilities: Capabilities dict
        :type capabilities: dict
        """
        if hasattr(self, '_capabilities'):
            for capability, enabled in capabilities.items():
                if capability not in self._capabilities:
                    self._capabilities[capability] = enabled
        else:
            self._capabilities = capabilities


    def has(self, capability):
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
    def name(self):
        """
        Device name

        :return: Device Name
        :rtype: str
        """
        return self._name

    @property
    def type(self):
        """
        Get device type

        :return: Device Type
        :rtype: str
        """
        return self._type

    @property
    def firmware_version(self):
        """
        Device's firmware version

        :return: FW Version
        :rtype: str
        """
        return self._fw

    @property
    def serial(self):
        """
        Device's serial

        :return: Device Serial
        :rtype: str
        """
        return self._serial

    @property
    def brightness(self):
        """
        Get device brightness

        :return: Device brightness
        :rtype: float
        """
        return self._dbus_interfaces['brightness'].getBrightness()

    @brightness.setter
    def brightness(self, value):
        """
        Set device brightness

        :param value: Device brightness
        :type value: float

        :raises ValueError: When brightness is not a float or not in range 0.0->100.0
        """
        if not isinstance(value, float):
            raise ValueError("Brightness must be a float")

        if value < 0.0 or value > 100.0:
            raise ValueError("Brightness must be between 0 and 100")

        self._dbus_interfaces['brightness'].setBrightness(value)

    @property
    def capabilities(self):
        """
        Device capabilities

        :return: Device capabilities
        :rtype: dict
        """
        return self._capabilities

    def __str__(self):
        return self._name

    def __repr__(self):
        return '<{0} {1}>'.format(self.__class__.__name__, self._serial)

class BaseDeviceFactory(object):
    @staticmethod
    def get_device(serial, daemon_dbus=None):
        raise NotImplementedError()