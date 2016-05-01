import re
import os
import types
import logging

from razer_daemon.dbus_services.service import DBusService
import razer_daemon.dbus_services.dbus_methods

class RazerDevice(DBusService):
    BUS_PATH = 'org.razer'
    OBJECT_PATH = '/org/razer/device/'
    METHODS = []

    USB_VID = None
    USB_PID = None

    def __init__(self, device_path, device_number):
        self._device_path = device_path
        self._device_number = device_number
        self._serial = self.get_serial()

        self.logger = logging.getLogger('razer.device{0}'.format(device_number))
        self.logger.info("Initialising device.{0} {1}".format(device_number, self.__class__.__name__))

        object_path = os.path.join(self.OBJECT_PATH, self._serial)
        DBusService.__init__(self, self.BUS_PATH, object_path)

        # Register method to get the devices serial
        self.logger.debug("Adding getSerial method to DBus")
        self.add_dbus_method('razer.device.misc', 'getSerial', self.get_serial, out_signature='s')

        # Set up methods to suspend and restore device operation
        self.suspend_args = {}
        self.logger.debug("Adding razer.device.misc.suspendDevice method to DBus")
        self.add_dbus_method('razer.device.misc', 'suspendDevice', self.suspend_device)
        self.logger.debug("Adding razer.device.misc.resumeDevice method to DBus")
        self.add_dbus_method('razer.device.misc', 'resumeDevice', self.resume_device)

        # Load additional DBus methods
        self.load_methods()

    def get_driver_path(self, driver_filename):
        """
        Get the path to a driver file

        :param driver_filename: Name of driver file
        :type driver_filename: str

        :return: Full path to driver
        :rtype: str
        """
        return os.path.join(self._device_path, driver_filename)

    def get_serial(self):
        """
        Get serial number for device

        :return: String of the serial number
        :rtype: str
        """
        serial_path = os.path.join(self._device_path, 'get_serial')
        with open(serial_path, 'r') as serial_file:
            return serial_file.read().strip()

    def load_methods(self):
        """
        Load DBus methods

        Goes through the list in self.METHODS and loads each effect and adds it to DBus
        """
        available_functions = {}
        methods = dir(razer_daemon.dbus_services.dbus_methods)
        for method in methods:
            potential_function = getattr(razer_daemon.dbus_services.dbus_methods, method)
            if isinstance(potential_function, types.FunctionType) and hasattr(potential_function, 'endpoint') and potential_function.endpoint:
                available_functions[potential_function.__name__] = potential_function

        for method_name in self.METHODS:
            try:
                new_function = available_functions[method_name]
                self.logger.debug("Adding {0}.{1} method to DBus".format(new_function.interface, new_function.name))
                self.add_dbus_method(new_function.interface, new_function.name, new_function, new_function.in_sig, new_function.out_sig, new_function.byte_arrays)
            except KeyError:
                pass

    def suspend_device(self):
        self.logger.info("Suspending {0}".format(self.__class__.__name__))
        self._suspend_device()

    def resume_device(self):
        self.logger.info("Resuming {0}".format(self.__class__.__name__))
        self._resume_device()

    def _suspend_device(self):
        raise NotImplementedError()

    def _resume_device(self):
        raise NotImplementedError()

    @classmethod
    def match(cls, device_id):
        """
        Match against the device ID

        :param device_id: Device ID like 0000:0000:0000.0000
        :type device_id: str

        :return: True if its the correct device ID
        :rtype: bool
        """
        pattern = r'^[0-9A-F]{4}:' + '{0:04X}'.format(cls.USB_VID) +':' + '{0:04X}'.format(cls.USB_PID) + r'\.[0-9A-F]{4}$'

        if re.match(pattern, device_id) is not None:
            if 'device_type' in  os.listdir(os.path.join('/sys/bus/hid/devices', device_id)):
                return True

        return False

class RazerBlackWidow2013(RazerDevice, DBusService):
    USB_VID = 0x1532
    USB_PID = 0x011A
    METHODS = ['get_firmware', 'get_brightness', 'enable_macro_keys', 'set_brightness', 'get_device_type', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static']

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = razer_daemon.dbus_services.dbus_methods.get_brightness(self)
        razer_daemon.dbus_services.dbus_methods.set_brightness(self, 0)

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        brightness = self.suspend_args.get('brightness', 100)
        razer_daemon.dbus_services.dbus_methods.set_brightness(self, brightness)


def get_device_classes():
    """
    Get a list of hardware classes

    :return: List of RazerDevice subclasses
    :rtype: list of callable
    """
    classes = []

    for class_name, class_object in globals().items():
        if class_name.startswith('__') or class_name in ('RazerDevice', 'DBusService'):
            continue

        if isinstance(class_object, type):
            classes.append(class_object)

    return classes



if __name__ == '__main__':
    from gi.repository import GObject
    from dbus.mainloop.glib import DBusGMainLoop

    DBusGMainLoop(set_as_default=True)

    razer_devices = []

    devices = os.listdir('/sys/bus/hid/devices')
    for device_id_link in devices:
        if RazerBlackWidow2013.match(device_id_link):
            print("Found device: {0}".format(device_id_link))
            device_path = os.path.join('/sys/bus/hid/devices', device_id_link)
            razer_device = RazerBlackWidow2013(device_path)
            razer_devices.append(razer_device)

    print("Running main loop")
    GObject.MainLoop().run()

