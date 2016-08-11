import dbus as _dbus
from razer.client.device import RazerDeviceFactory as _RazerDeviceFactory


class DeviceManager(object):
    def __init__(self):
        # Load up the DBus
        session_bus = _dbus.SessionBus()
        self._dbus = session_bus.get_object("org.razer", "/org/razer")

        # Get interface for daemon methods
        self._dbus_daemon = _dbus.Interface(self._dbus, "razer.daemon")

        # Get interface for devices methods
        self._dbus_devices = _dbus.Interface(self._dbus, "razer.devices")

        self._device_serials = [serial for serial in self._dbus_devices.getDevices() if len(serial) > 0]
        self._devices = None

    def stop_daemon(self):
        """
        Stop Daemon
        """
        self._dbus_daemon.stop()

    def turn_off_on_screensaver(self, enable):
        """
        Set wether or not to turn off the devices when screensaver is active

        If True then when the screensaver is active the devices' brightness will be set to 0.
        When the screensaver is inactive the devices' brightness will be restored
        :param enable: True to enable screensaver disable
        :type enable: bool
        """
        # TODO typecheck enable
        if enable:
            self._dbus_devices.enableTurnOffOnScreensaver()
        else:
            self._dbus_devices.disableTurnOffOnScreensaver()

    def sync_effects(self, sync):
        """
        Enable or disable the syncing of effects between devices

        :param sync: Sync effects
        :type sync: bool
        """
        # TODO typecheck sync
        # We do if sync as then it doenst matter if some idiot doesn't pass in a bool
        if sync:
            self._dbus_devices.syncEffects(True)
        else:
            self._dbus_devices.syncEffects(False)

    def _get_devices(self):
        """
        Gets devices classes

        Goes through each device serial and gets the corrosponding device class and caches it.
        """
        self._devices = []

        for serial in self._device_serials:
            device = _RazerDeviceFactory.get_device(serial)
            self._devices.append(device)

    @property
    def devices(self):
        """
        Gets devices

        :return: List of devices
        :rtype: list of razer.clent.devices.RazerDevice
        """
        if self._devices is None:
            self._get_devices()

        return self._devices


if __name__ == '__main__':
    # TODO remove on debug complete
    a = DeviceManager()
    devices = a.devices


    print(devices)
