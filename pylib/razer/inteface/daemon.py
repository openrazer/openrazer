import dbus

from razer.inteface.device import Device

DAEMON_BUS_NAME = 'org.razer'
DAEMON_OBJECT_PATH = '/org/razer'


class Daemon(object):
    def __init__(self):
        self._session_bus = dbus.SessionBus()
        self._daemon_dbus_object = self._session_bus.get_object(DAEMON_BUS_NAME, DAEMON_OBJECT_PATH)

        self._control_interface = dbus.Interface(self._daemon_dbus_object, 'razer.daemon')
        self._devices_interface = dbus.Interface(self._daemon_dbus_object, 'razer.devices')

    def stop_daemon(self):
        """
        Stops the daemon
        """
        self._control_interface.stop()

    def turn_off_on_screensaver(self, turn_off):
        """
        Turn off the razer devices whilst the screensaver is active

        :param turn_off: True to turn off when screensaver, false otherwise
        :type turn_off: bool
        """
        if turn_off:
            self._devices_interface.enableTurnOffOnScreensaver()
        else:
            self._devices_interface.disableTurnOffOnScreensaver()

    def sync_effects(self, sync):
        """
        Synchronize effects across razer devices

        :param sync: Sync Effects
        :type sync: bool
        """
        if isinstance(sync, bool):
            self._devices_interface.syncEffects(sync)
        else:
            raise ValueError("sync should be a boolean")

    @property
    def device_ids(self):
        return self._devices_interface.getDevices()

    def get_device(self, device_id):
        return Device(device_id)


if __name__ == '__main__':
    daemon = Daemon()

    devices = daemon.device_ids

    device1 = devices[0]
    device_obj = daemon.get_device(device1)


    print()