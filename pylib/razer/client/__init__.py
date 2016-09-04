import dbus as _dbus
from razer.client.device import RazerDeviceFactory as _RazerDeviceFactory
from razer.client import constants


class DaemonNotFound(Exception):
    pass


class DeviceManager(object):
    def __init__(self):
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

        for serial in self._device_serials:
            device = _RazerDeviceFactory.get_device(serial)
            self._devices.append(device)

    def stop_daemon(self):
        """
        Stop Daemon
        """
        self._dbus_daemon.stop()

    def turn_off_on_screensaver(self, enable:bool):
        """
        Set wether or not to turn off the devices when screensaver is active

        If True then when the screensaver is active the devices' brightness will be set to 0.
        When the screensaver is inactive the devices' brightness will be restored
        :param enable: True to enable screensaver disable
        :type enable: bool

        :raises ValueError: If enable isnt a bool
        """
        if not isinstance(enable, bool):
            raise ValueError("Enable must be a boolean")

        if enable:
            self._dbus_devices.enableTurnOffOnScreensaver()
        else:
            self._dbus_devices.disableTurnOffOnScreensaver()

    # TODO convert to property
    def sync_effects(self, sync:bool):
        """
        Enable or disable the syncing of effects between devices

        If sync is enabled, whenever an effect is set then it will be set on all other devices if the effect is available or a similar effect if it is not.
        :param sync: Sync effects
        :type sync: bool

        :raises ValueError: If sync isnt a bool
        """
        if not isinstance(sync, bool):
            raise ValueError("Sync must be a boolean")

        self._dbus_devices.syncEffects(sync)

    @property
    def devices(self) -> list:
        """
        Gets devices

        :return: List of devices
        :rtype: list of razer.clent.devices.RazerDevice
        """

        return self._devices


# if __name__ == '__main__':
#     from razer.client.debug import print_attrs
#     # #DodgyHackToPrintVarStructure
#     # TODO remove on debug complete
#     a = DeviceManager()
#     devices = a.devices
#
#     kbd = None
#     mouse = None
#     for device in devices:
#         if device.type == 'keyboard':
#             kbd = device
#         elif device.type == 'mouse':
#             mouse = device
#
#     if kbd is not None:
#         print_attrs(kbd, recurse_to=['fx', 'macro', 'advanced'])
#
#         # new_macro = [kbd.macro.create_url_macro_item("https://hackaday.com")]
#         # kbd.macro.add_macro('M2', new_macro)
#         #kbd.macro.del_macro('M2')
#         kbd.fx.static(255, 00, 255)
#
#         # macros = kbd.macro.get_macros()
#
#         #                      y, x       r, g, b
#         # kbd.fx.advanced.matrix[1, 1] = (255, 0, 255)
#         # kbd.fx.advanced.matrix[1, 2] = (0, 255, 255)
#         # kbd.fx.advanced.matrix[1, 3] = (255, 255, 0)
#         # kbd.fx.advanced.draw()
#
#
#
#
#         print()
#
#     # if mouse is not None:
#     #     mouse.brightness = 100.0
#
#     print()
#     print("Devices: {0}".format(devices))
