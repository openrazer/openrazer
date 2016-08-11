import dbus as _dbus
from razer.client.devices import RazerDevice as __RazerDevice, BaseDeviceFactory as __BaseDeviceFactory
from razer.client.devices.firefly import RazerFirefly as __RazerFirefly
from razer.client.devices.keyboard import RazerKeyboard as __RazerKeyboard


DEVICE_MAP ={
    'firefly': __RazerFirefly,
    'keyboard': __RazerKeyboard,
    'default': __RazerDevice
}


class RazerDeviceFactory(__BaseDeviceFactory):
    @staticmethod
    def get_device(serial, daemon_dbus=None):
        """
        Factory for turning a serial into a class

        Device factory, will return a class fit for the device in question. The DEVICE_MAP mapping above
        can contain a device_type => DeviceClass or DeviceFactory, this allows us to specify raw device classes
        if there is only one model (like Firefly) or a factory for the keyboards (so we can differentiate between
        old blackwidows and chromas). If the device is not in the device mapping then the factory will default
        to a raw RazerDevice.
        :param serial: Device serial
        :type serial: str

        :param daemon_dbus: Daemon DBus object
        :type daemon_dbus: object or None

        :return: RazerDevice object (or subclass)
        :rtype: RazerDevice
        """
        if daemon_dbus is None:
            session_bus = _dbus.SessionBus()
            daemon_dbus = session_bus.get_object("org.razer", "/org/razer/device/{0}".format(serial))

        device_dbus = _dbus.Interface(daemon_dbus, "razer.device.misc")

        device_type = device_dbus.getDeviceType()

        if device_type in DEVICE_MAP:
            # Have device mapping
            device_class = DEVICE_MAP[device_type]
            if hasattr(device_class, 'get_device'):
                # DeviceFactory
                device = device_class.get_device(serial, daemon_dbus=daemon_dbus)
            else:
                # DeviceClass
                device = device_class(serial, daemon_dbus=daemon_dbus)
        else:
            # No mapping, default to RazerDevice
            device = DEVICE_MAP['default'](serial, daemon_dbus=daemon_dbus)

        return device

