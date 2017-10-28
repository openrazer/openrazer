"""
Hardware base class
"""
import re
import os
import types
import logging
import time
import json
import random

from openrazer_daemon.dbus_services.service import DBusService
import openrazer_daemon.dbus_services.dbus_methods
from openrazer_daemon.misc import effect_sync


# pylint: disable=too-many-instance-attributes
class RazerDevice(DBusService):
    """
    Base class

    Sets up the logger, sets up DBus
    """
    BUS_PATH = 'org.razer'
    OBJECT_PATH = '/org/razer/device/'
    METHODS = []

    EVENT_FILE_REGEX = None

    USB_VID = None
    USB_PID = None
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [-1, -1]

    WAVE_DIRS = (1, 2)

    RAZER_URLS = {
        "top_img": None,
        "side_img": None,
        "perspective_img": None
    }

    def __init__(self, device_path, device_number, config, testing=False, additional_interfaces=None):

        self.logger = logging.getLogger('razer.device{0}'.format(device_number))
        self.logger.info("Initialising device.%d %s", device_number, self.__class__.__name__)

        # Serial cache
        self._serial = None

        self._observer_list = []
        self._effect_sync_propagate_up = False
        self._disable_notifications = False
        self.additional_interfaces = []
        if additional_interfaces is not None:
            self.additional_interfaces.extend(additional_interfaces)

        self.config = config
        self._testing = testing
        self._parent = None
        self._device_path = device_path
        self._device_number = device_number
        self.serial = self.get_serial()

        self._effect_sync = effect_sync.EffectSync(self, device_number)

        self._is_closed = False

        # Find event files in /dev/input/by-id/ by matching against regex
        self.event_files = []

        if self._testing:
            search_dir = os.path.join(device_path, 'input')
        else:
            search_dir = '/dev/input/by-id/'

        if os.path.exists(search_dir):
            for event_file in os.listdir(search_dir):
                if self.EVENT_FILE_REGEX is not None and self.EVENT_FILE_REGEX.match(event_file) is not None:
                    self.event_files.append(os.path.join(search_dir, event_file))

        object_path = os.path.join(self.OBJECT_PATH, self.serial)
        DBusService.__init__(self, self.BUS_PATH, object_path)

        # Set up methods to suspend and restore device operation
        self.suspend_args = {}
        self.method_args = {}

        methods = {
            # interface, method, callback, in-args, out-args
            ('razer.device.misc', 'getSerial', self.get_serial, None, 's'),
            ('razer.device.misc', 'suspendDevice', self.suspend_device, None, None),
            ('razer.device.misc', 'getDeviceMode', self.get_device_mode, None, 's'),
            ('razer.device.misc', 'getRazerUrls', self.get_image_json, None, 's'),
            ('razer.device.misc', 'setDeviceMode', self.set_device_mode, 'yy', None),
            ('razer.device.misc', 'resumeDevice', self.resume_device, None, None),
            ('razer.device.misc', 'getVidPid', self.get_vid_pid, None, 'ai'),
            ('razer.device.misc', 'getDriverVersion', openrazer_daemon.dbus_services.dbus_methods.version, None, 's'),
            ('razer.device.misc', 'hasDedicatedMacroKeys', self.dedicated_macro_keys, None, 'b'),
        }

        for m in methods:
            self.logger.debug("Adding {}.{} method to DBus".format(m[0], m[1]))
            self.add_dbus_method(m[0], m[1], m[2], in_signature=m[3], out_signature=m[4])

        # Load additional DBus methods
        self.load_methods()

    def send_effect_event(self, effect_name, *args):
        """
        Send effect event

        :param effect_name: Effect name
        :type effect_name: str

        :param args: Effect arguments
        :type args: list
        """
        payload = ['effect', self, effect_name]
        payload.extend(args)

        self.notify_observers(tuple(payload))

    def dedicated_macro_keys(self):
        """
        Returns if the device has dedicated macro keys

        :return: Macro keys
        :rtype: bool
        """
        return self.DEDICATED_MACRO_KEYS

    @property
    def effect_sync(self):
        """
        Propagate the obsever call upwards, used for syncing effects

        :return: Effects sync flag
        :rtype: bool
        """
        return self._effect_sync_propagate_up

    @effect_sync.setter
    def effect_sync(self, value):
        """
        Setting to true will propagate observer events upwards

        :param value: Effect sync
        :type value: bool
        """
        self._effect_sync_propagate_up = value

    @property
    def disable_notify(self):
        """
        Disable notifications flag

        :return: Flag
        :rtype: bool
        """
        return self._disable_notifications

    @disable_notify.setter
    def disable_notify(self, value):
        """
        Set the disable notifications flag

        :param value: Disable
        :type value: bool
        """
        self._disable_notifications = value

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
        # TODO raise exception if serial cant be got and handle during device add
        if self._serial is None:
            serial_path = os.path.join(self._device_path, 'device_serial')
            count = 0
            serial = ''
            while len(serial) == 0:
                if count >= 5:
                    break

                try:
                    serial = open(serial_path, 'r').read().strip()
                except (PermissionError, OSError) as err:
                    self.logger.warning('getting serial: {0}'.format(err))
                    serial = ''

                count += 1
                time.sleep(0.1)

                if len(serial) == 0:
                    self.logger.debug('getting serial: {0} count:{1}'.format(serial, count))

            if serial == '' or serial == 'Default string':
                serial = 'UNKWN{0:012}'.format(random.randint(0, 4096))

            self._serial = serial

        return self._serial

    def get_device_mode(self):
        """
        Get device mode

        :return: String of device mode and arg seperated by colon, e.g. 0:0 or 3:0
        :rtype: str
        """
        device_mode_path = os.path.join(self._device_path, 'device_mode')
        with open(device_mode_path, 'r') as mode_file:
            count = 0
            mode = mode_file.read().strip()
            while len(mode) == 0:
                if count >= 3:
                    break
                mode = mode_file.read().strip()

                count += 1
                time.sleep(0.1)

            return mode

    def set_device_mode(self, mode_id, param):
        """
        Set device mode

        :param mode_id: Device mode ID
        :type mode_id: int

        :param param: Device mode parameter
        :type param: int
        """
        device_mode_path = os.path.join(self._device_path, 'device_mode')
        with open(device_mode_path, 'wb') as mode_file:

            # Do some validation (even though its in the driver)
            if mode_id not in (0, 3):
                mode_id = 0
            if param != 0:
                param = 0

            mode_file.write(bytes([mode_id, param]))

    def get_vid_pid(self):
        """
        Get the usb VID PID

        :return: List of VID PID
        :rtype: list of int
        """
        result = [self.USB_VID, self.USB_PID]
        return result

    def get_image_json(self):
        return json.dumps(self.RAZER_URLS)

    def load_methods(self):
        """
        Load DBus methods

        Goes through the list in self.METHODS and loads each effect and adds it to DBus
        """
        available_functions = {}
        methods = dir(openrazer_daemon.dbus_services.dbus_methods)
        for method in methods:
            potential_function = getattr(openrazer_daemon.dbus_services.dbus_methods, method)
            if isinstance(potential_function, types.FunctionType) and hasattr(potential_function, 'endpoint') and potential_function.endpoint:
                available_functions[potential_function.__name__] = potential_function

        for method_name in self.METHODS:
            try:
                new_function = available_functions[method_name]
                self.logger.debug("Adding %s.%s method to DBus", new_function.interface, new_function.name)
                self.add_dbus_method(new_function.interface, new_function.name, new_function, new_function.in_sig, new_function.out_sig, new_function.byte_arrays)
            except KeyError:
                pass

    def suspend_device(self):
        """
        Suspend device
        """
        self.logger.info("Suspending %s", self.__class__.__name__)
        self._suspend_device()

    def resume_device(self):
        """
        Resume device
        """
        self.logger.info("Resuming %s", self.__class__.__name__)
        self._resume_device()

    def _suspend_device(self):
        """
        Suspend device
        """
        raise NotImplementedError()

    def _resume_device(self):
        """
        Resume device
        """
        raise NotImplementedError()

    def _close(self):
        """
        To be overrided by any subclasses to do cleanup
        """
        # Clear observer list
        self._observer_list.clear()

    def close(self):
        """
        Close any resources opened by subclasses
        """
        if not self._is_closed:
            self._close()

            self._is_closed = True

    def register_observer(self, observer):
        """
        Observer design pattern, register

        :param observer: Observer
        :type observer: object
        """
        if observer not in self._observer_list:
            self._observer_list.append(observer)

    def register_parent(self, parent):
        """
        Register the parent as an observer to be optionally notified (sends to other devices)

        :param parent: Observer
        :type parent: object
        """
        self._parent = parent

    def remove_observer(self, observer):
        """
        Obsever design pattern, remove

        :param observer: Observer
        :type observer: object
        """
        try:
            self._observer_list.remove(observer)
        except ValueError:
            pass

    def notify_observers(self, msg):
        """
        Notify observers with msg

        :param msg: Tuple with first element a string
        :type msg: tuple
        """
        if not self._disable_notifications:
            self.logger.debug("Sending observer message: %s", str(msg))

            if self._effect_sync_propagate_up and self._parent is not None:
                self._parent.notify_parent(msg)

            for observer in self._observer_list:
                observer.notify(msg)

    def notify(self, msg):
        """
        Recieve observer messages

        :param msg: Tuple with first element a string
        :type msg: tuple
        """
        self.logger.debug("Got observer message: %s", str(msg))

        for observer in self._observer_list:
            observer.notify(msg)

    @classmethod
    def match(cls, device_id, dev_path):
        """
        Match against the device ID

        :param device_id: Device ID like 0000:0000:0000.0000
        :type device_id: str

        :param dev_path: Device path. Normally '/sys/bus/hid/devices/0000:0000:0000.0000'
        :type dev_path: str

        :return: True if its the correct device ID
        :rtype: bool
        """
        pattern = r'^[0-9A-F]{4}:' + '{0:04X}'.format(cls.USB_VID) + ':' + '{0:04X}'.format(cls.USB_PID) + r'\.[0-9A-F]{4}$'

        if re.match(pattern, device_id) is not None:
            if 'device_type' in os.listdir(dev_path):
                return True

        return False

    def __del__(self):
        self.close()

    def __repr__(self):
        return "{0}:{1}".format(self.__class__.__name__, self.serial)


class RazerDeviceBrightnessSuspend(RazerDevice):
    """
    Class for suspend using brightness

    Suspend functions
    """
    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = openrazer_daemon.dbus_services.dbus_methods.get_brightness(self)

        # Todo make it context?
        self.disable_notify = True
        openrazer_daemon.dbus_services.dbus_methods.set_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        brightness = self.suspend_args.get('brightness', 100)

        self.disable_notify = True
        openrazer_daemon.dbus_services.dbus_methods.set_brightness(self, brightness)
        self.disable_notify = False
