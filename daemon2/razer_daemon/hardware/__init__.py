import re
import os
import types

from razer_daemon.dbus.service import DBusService
import razer_daemon.dbus.dbus_methods

class RazerDevice(DBusService):
    BUS_PATH = 'org.razer'
    OBJECT_PATH = '/org/razer/device/'
    METHODS = ['get_firmware']

    USB_VID = None
    USB_PID = None

    def __init__(self, device_path):
        self._device_path = device_path
        self._serial = self.get_serial()

        object_path = os.path.join(self.OBJECT_PATH, self._serial)
        DBusService.__init__(self, self.BUS_PATH, object_path)

        self.add_dbus_method('device.misc', 'getSerial', self.get_serial, out_signature='s')

        self.load_methods()

    def get_driver_path(self, pathname):
        return os.path.join(self._device_path, pathname)

    def get_serial(self):
        serial_path = os.path.join(self._device_path, 'get_serial')
        with open(serial_path, 'r') as serial_file:
            return serial_file.read().strip()

    def load_methods(self):
        available_functions = {}
        methods = dir(razer_daemon.dbus.dbus_methods)
        for method in methods:
            potential_function = getattr(razer_daemon.dbus.dbus_methods, method)
            if isinstance(potential_function, types.FunctionType) and hasattr(potential_function, 'endpoint') and potential_function.endpoint:
                available_functions[potential_function.__name__] = potential_function

        for method_name in self.METHODS:
            try:
                new_function = available_functions[method_name]
                self.add_dbus_method(new_function.interface, method_name, new_function, new_function.in_sig, new_function.out_sig, new_function.byte_arrays)
            except KeyError:
                pass



        # for method_name in self.METHODS:






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

    # def __init__(self, device_path):
    #     RazerDevice.__init__(self, device_path)






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

