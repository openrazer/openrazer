import dbus as _dbus
from openrazer.client.devices import RazerDevice as __RazerDevice, BaseDeviceFactory as __BaseDeviceFactory
from openrazer.client.devices.mousemat import RazerMousemat as __RazerMousemat
from openrazer.client.devices.keyboard import RazerKeyboardFactory as __RazerKeyboardFactory
from openrazer.client.devices.mice import RazerMouse as __RazerMouse


DEVICE_MAP = {
    'mousemat': __RazerMousemat,
    'keyboard': __RazerKeyboardFactory,
    'mouse': __RazerMouse,
    'keypad': __RazerKeyboardFactory,
    'default': __RazerDevice
}


class RazerDeviceFactory(__BaseDeviceFactory):
    """
    Simple factory to return an object for a given device

    """
    @staticmethod
    def get_device(serial, vid_pid=None, daemon_dbus=None):
        """
        Factory for turning a serial into a class

        Device factory, will return a class fit for the device in question. The DEVICE_MAP mapping above
        can contain a device_type => DeviceClass or DeviceFactory, this allows us to specify raw device classes
        if there is only one model (like Firefly) or a factory for the keyboards (so we can differentiate between
        old blackwidows and chromas). If the device is not in the device mapping then the factory will default
        to a raw RazerDevice.

        :param serial: Device serial
        :type serial: str

        :param vid_pid: Device VID, PID
        :type vid_pid: list of int

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
        device_vid_pid = device_dbus.getVidPid()

        if device_type in DEVICE_MAP:
            # Have device mapping
            device_class = DEVICE_MAP[device_type]
            if hasattr(device_class, 'get_device'):
                # DeviceFactory
                device = device_class.get_device(serial, vid_pid=device_vid_pid, daemon_dbus=daemon_dbus)
            else:
                # DeviceClass
                device = device_class(serial, vid_pid=device_vid_pid, daemon_dbus=daemon_dbus)
        else:
            # No mapping, default to RazerDevice
            device = DEVICE_MAP['default'](serial, vid_pid=device_vid_pid, daemon_dbus=daemon_dbus)

        return device
