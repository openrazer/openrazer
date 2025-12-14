# SPDX-License-Identifier: GPL-2.0-or-later

import json
import dbus as _dbus
from openrazer.client.device import RazerDeviceFactory as _RazerDeviceFactory
from openrazer.client.devices import RazerDevice as _RazerDevice

__version__ = '3.11.0'


class DaemonNotFound(Exception):
    pass


class DeviceManager(object):
    """
    DeviceManager Class
    """

    def __init__(self) -> None:
        # Load up the DBus
        session_bus = _dbus.SessionBus()
        try:
            self._dbus = session_bus.get_object("org.razer", "/org/razer")
        except _dbus.DBusException:
            raise DaemonNotFound("Could not connect to daemon")

        # Get interface for daemon methods
        self._dbus_daemon = _dbus.Interface(self._dbus, "razer.daemon")

        # Get interface for devices methods
        self._dbus_devices = _dbus.Interface(self._dbus, "razer.devices")

        self._device_serials = self._dbus_devices.getDevices()
        self._devices = []

        self._daemon_version = self._dbus_daemon.version()

        for serial in self._device_serials:
            device = _RazerDeviceFactory.get_device(serial)
            self._devices.append(device)

    def stop_daemon(self) -> None:
        """
        Stops the Daemon via a DBus call
        """
        self._dbus_daemon.stop()

    @property
    def turn_off_on_screensaver(self) -> bool:
        return bool(self._dbus_devices.getOffOnScreensaver())

    @turn_off_on_screensaver.setter
    def turn_off_on_screensaver(self, enable: bool) -> None:
        """
        Enable or Disable the logic to turn off the devices whilst the screensaver is active

        If True, when the screensaver is active the devices' brightness will be set to 0.
        When the screensaver is inactive the devices' brightness will be restored
        :param enable: True to enable screensaver disable
        :type enable: bool

        :raises ValueError: If enable isn't a bool
        """
        if not isinstance(enable, bool):
            raise ValueError("Enable must be a boolean")

        self._dbus_devices.enableTurnOffOnScreensaver(enable)

    @property
    def sync_effects(self) -> bool:
        return bool(self._dbus_devices.getSyncEffects())

    @sync_effects.setter
    def sync_effects(self, sync: bool) -> None:
        """
        Enable or disable the syncing of effects between devices

        If sync is enabled, whenever an effect is set then it will be set on all other devices if the effect is available or a similar effect if it is not.

        :param sync: Sync effects
        :type sync: bool

        :raises ValueError: If sync isn't a bool
        """
        if not isinstance(sync, bool):
            raise ValueError("Sync must be a boolean")

        self._dbus_devices.syncEffects(sync)

    @property
    def supported_devices(self) -> dict[str, tuple[int, int]]:
        json_data = self._dbus_daemon.supportedDevices()

        return json.loads(json_data)

    @property
    def devices(self) -> list[_RazerDevice]:
        """
        A list of Razer devices

        :return: List of devices
        :rtype: list[razer.client.devices.RazerDevice]
        """

        return self._devices

    @property
    def version(self) -> str:
        """
        Python library version

        :return: Version
        :rtype: str
        """
        return __version__

    @property
    def daemon_version(self) -> str:
        """
        Daemon version

        :return: Daemon version
        :rtype: str
        """
        return str(self._daemon_version)
