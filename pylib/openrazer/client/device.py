# SPDX-License-Identifier: GPL-2.0-or-later

import dbus
from openrazer.client.devices import RazerDevice, BaseDeviceFactory
from openrazer.client.devices.mousemat import RazerMousemat
from openrazer.client.devices.keyboard import RazerKeyboard
from openrazer.client.devices.mice import RazerMouse


DEVICE_MAP = {
    'mousemat': RazerMousemat,
    'keyboard': RazerKeyboard,
    'mouse': RazerMouse,
    'keypad': RazerKeyboard,
}


class RazerDeviceFactory(BaseDeviceFactory):
    """
    Simple factory to return an object for a given device

    """
    @staticmethod
    def get_device(serial: str) -> RazerDevice:
        """
        Factory for turning a serial into a class

        Device factory, will return a class fit for the device in question. The DEVICE_MAP mapping above
        can contain a device_type => DeviceClass or DeviceFactory, this allows us to specify raw device classes
        if there is only one model (like Firefly) or a factory for the keyboards (so we can differentiate between
        old blackwidows and chromas). If the device is not in the device mapping then the factory will default
        to a raw RazerDevice.

        :param serial: Device serial
        :type serial: str

        :return: RazerDevice object (or subclass)
        :rtype: RazerDevice
        """
        session_bus = dbus.SessionBus()
        daemon_dbus = session_bus.get_object("org.razer", "/org/razer/device/{0}".format(serial))
        device_dbus = dbus.Interface(daemon_dbus, "razer.device.misc")

        device_type = device_dbus.getDeviceType()

        device_class = DEVICE_MAP.get(device_type, RazerDevice)

        return device_class(serial)
