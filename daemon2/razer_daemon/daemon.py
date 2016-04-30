import logging
import os

import razer_daemon.hardware
from razer_daemon.dbus_services.service import DBusService


class Daemon(DBusService):
    BUS_PATH = 'org.razer'

    def __init__(self, logging_level=logging.WARNING):
        DBusService.__init__(self, self.BUS_PATH, '/org/razer')

        # TODO set logging format to include an argument of where it is, then log everything

        # Set logging level
        logging.basicConfig(level=logging_level)

        self.razer_devices = {}
        self.load_devices()

        self.add_dbus_method('razer.devices', 'getDevices', self.get_serial_list, out_signature='as')

    def get_serial_list(self):
        """
        Get list of devices serials
        """
        devices = list(self.razer_devices.keys())
        return devices

    def load_devices(self):
        """
        Go through supported devices and load them

        Loops through the available hardware classes, loops through
        each device in the system and adds it if needs be.
        """
        devices = os.listdir('/sys/bus/hid/devices')
        classes = razer_daemon.hardware.get_device_classes()

        for DeviceClass in classes:
            for device_id in devices:
                if DeviceClass.match(device_id):
                    print("Found device: {0}".format(device_id))
                    device_path = os.path.join('/sys/bus/hid/devices', device_id)
                    razer_device = DeviceClass(device_path)
                    device_serial = razer_device.get_serial()
                    self.razer_devices[device_serial] = razer_device


if __name__ == '__main__':
    from gi.repository import GObject
    from dbus.mainloop.glib import DBusGMainLoop

    DBusGMainLoop(set_as_default=True)

    daemon = Daemon(logging_level=logging.DEBUG)




    print("Running main loop")
    GObject.MainLoop().run()