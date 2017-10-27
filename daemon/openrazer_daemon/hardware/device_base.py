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
import hidraw

from openrazer_daemon.dbus_services.service import DBusService
import openrazer_daemon.dbus_services.dbus_methods
from openrazer_daemon.misc import effect_sync

class RazerReport(object):
    RAZER_CMD_BUSY          = 0x01
    RAZER_CMD_SUCCESSFUL    = 0x02
    RAZER_CMD_FAILURE       = 0x03
    RAZER_CMD_TIMEOUT       = 0x04
    RAZER_CMD_NOT_SUPPORTED = 0x05

    # LED STORAGE Options
    NOSTORE                 = 0x00
    VARSTORE                = 0x01

    # LED definitions
    SCROLL_WHEEL_LED        = 0x01
    BATTERY_LED             = 0x03
    LOGO_LED                = 0x04
    BACKLIGHT_LED           = 0x05
    MACRO_LED               = 0x07
    GAME_LED                = 0x08
    RED_PROFILE_LED         = 0x0C
    GREEN_PROFILE_LED       = 0x0D
    BLUE_PROFILE_LED        = 0x0E

    # LED Effect definitions
    LED_STATIC              = 0x00
    LED_BLINKING            = 0x01
    LED_PULSATING           = 0x02
    LED_SPECTRUM_CYCLING    = 0x04

    def __init__(self, bytes = None):
        if bytes is not None:
            self.bytes = bytes
        else:
            self.bytes = bytearray(90)

    @property
    def status(self):
        return self.bytes[0]

    @status.setter
    def status(self, value):
        self.bytes[0] = value

    @property
    def transaction_id(self):
        return self.bytes[1]

    @transaction_id.setter
    def transaction_id(self, value):
        self.bytes[1] = value

    @property
    def remaining_packets(self):
        return (self.bytes[2] << 8) | self.bytes[3]

    @remaining_packets.setter
    def remaining_packets(self, value):
        self.bytes[2] = (value >> 8) & 0xff
        self.bytes[3] = value & 0xff

    @property
    def protocol_type(self):
        return self.bytes[4]

    @protocol_type.setter
    def protocol_type(self, value):
        self.bytes[4] = value

    @property
    def data_size(self):
        return self.bytes[5]

    @data_size.setter
    def data_size(self, value):
        self.bytes[5] = value

    @property
    def command_class(self):
        return self.bytes[6]

    @command_class.setter
    def command_class(self, value):
        self.bytes[6] = value

    @property
    def command_id(self):
        return self.bytes[7]

    @command_id.setter
    def command_id(self, value):
        self.bytes[7] = value

    @property
    def arguments(self):
        return self.bytes[8:88]

    @arguments.setter
    def arguments(self, value):
        self.bytes[8:8 + len(value)] = value

    @property
    def crc(self):
        return self.bytes[88]

    @crc.setter
    def crc(self, value):
        self.bytes[88] = value

    def razer_calculate_crc(self):
        # second to last byte of report is a simple checksum
        # just xor all bytes up with overflow and you are done
        crc = 0
        for i in range(2, 88):
            crc ^= self.bytes[i]
        return crc

    def set_crc(self):
        crc = self.razer_calculate_crc()
        self.crc = crc


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
        "store": None,
        "top_img": None,
        "side_img": None,
        "perspective_img": None
    }

    USE_HIDRAW = False

    def __init__(self, device_path, device_number, config, testing=False, additional_interfaces=None):

        self.logger = logging.getLogger('razer.device{0}'.format(device_number))
        self.logger.info("Initialising device.%d %s", device_number, self.__class__.__name__)

        self.hidraw_path = os.path.join('/dev', os.listdir(os.path.join(device_path, 'hidraw'))[0])

        self.hid_node = hidraw.HIDRaw(open(self.hidraw_path, 'rb'))

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


    @property
    def hid_response_index(self):
        return 0x00

    @property
    def hid_request_index(self):
        return 0x00

    @property
    def hid_transaction_id(self):
        return 0xff

    def razer_get_report(self, command_class, command_id, data_size, args = []):
        report = RazerReport()
        report.transaction_id = self.hid_transaction_id
        report.data_size = data_size
        report.command_class = command_class
        report.command_id = command_id
        report.args = args
        return report

    def razer_get_usb_response(self, data):
        self.hid_node.sendFeatureReport(data.bytes, report_num = self.hid_request_index)
        time.sleep(0.001)
        answer = self.hid_node.getFeatureReport(report_num = self.hid_response_index, length = 90)
        return RazerReport(answer[1:])

    def print_erroneous_report(self, report, message):
        self.logger.warning("{}. Start Marker: {:02x} id: {:02x} Num Params: {:02x} Reserved: {:02x} Command: {:02x} Params: {:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x} .".format( \
           message,
           report.status,
           report.transaction_id,
           report.data_size,
           report.command_class,
           report.command_id,
           report.arguments[0],  report.arguments[1],  report.arguments[2],  report.arguments[3],
           report.arguments[4],  report.arguments[5],  report.arguments[6],  report.arguments[7],
           report.arguments[8],  report.arguments[9],  report.arguments[10], report.arguments[11],
           report.arguments[12], report.arguments[13], report.arguments[14], report.arguments[15]))

    def razer_send_payload(self, request_report):
        request_report.set_crc()

        response_report = self.razer_get_usb_response(request_report)

        # Check the packet number, class and command are the same
        if(response_report.remaining_packets != request_report.remaining_packets or
           response_report.command_class != request_report.command_class or
           response_report.command_id != request_report.command_id):
            self.print_erroneous_report(response_report, "Response doesnt match request");
        elif response_report.status == RazerReport.RAZER_CMD_FAILURE:
            self.print_erroneous_report(response_report, "Command failed");
        elif response_report.status == RazerReport.RAZER_CMD_NOT_SUPPORTED:
            self.print_erroneous_report(response_report, "Command not supported");
        elif response_report.status == RazerReport.RAZER_CMD_TIMEOUT:
            self.print_erroneous_report(response_report, "Command timed out");

        return response_report

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

    def get_hidraw_serial(self):
        request = self.razer_get_report(0x00, 0x82, 0x16)
        response = self.razer_send_payload(request)
        return response.arguments.decode('utf-8').split('\0', 1)[0]

    def get_serial(self):
        """
        Get serial number for device

        :return: String of the serial number
        :rtype: str
        """
        # TODO raise exception if serial cant be got and handle during device add
        if self._serial is None:
            if self.USE_HIDRAW:
                serial = self.get_hidraw_serial()
            else:
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

    def get_hidraw_device_mode(self):
        request = self.razer_get_report(0x00, 0x84, 0x02)
        response = self.razer_send_payload(request)
        return "{}:{}".format(response.arguments[0], response.arguments[1])

    def get_device_mode(self):
        """
        Get device mode

        :return: String of device mode and arg seperated by colon, e.g. 0:0 or 3:0
        :rtype: str
        """
        if self.USE_HIDRAW:
            return self.get_hidraw_device_mode()
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

    def set_hidraw_device_mode(self, mode_id, param):
        request = self.razer_get_report(0x00, 0x04, 0x02)

        if mode_id not in (0, 3):
            mode_id = 0
        if param != 0:
            param = 0

        request.arguments[0] = mode_id;
        request.arguments[1] = param;

        response = self.razer_send_payload(request)

    def set_device_mode(self, mode_id, param):
        """
        Set device mode

        :param mode_id: Device mode ID
        :type mode_id: int

        :param param: Device mode parameter
        :type param: int
        """
        if self.USE_HIDRAW:
            return self.set_hidraw_device_mode(mode_id, param)
        device_mode_path = os.path.join(self._device_path, 'device_mode')
        with open(device_mode_path, 'wb') as mode_file:

            # Do some validation (even though its in the driver)
            if mode_id not in (0, 3):
                mode_id = 0
            if param != 0:
                param = 0

            mode_file.write(bytes([mode_id, param]))

    def get_firmware_version(self):
        request = self.razer_get_report(0x00, 0x81, 0x02)
        response = self.razer_send_payload(request)
        return "v{}.{}".format(response.arguments[0], response.arguments[1])

    def get_led_active(self, led):
        request = self.razer_get_report(0x03, 0x80, 0x03)
        request.arguments = [0x01, led]
        request.transaction_id = 0x3F
        response = self.razer_send_payload(request)
        return response.arguments[2]

    def set_led_active(self, led, enabled):
        request = self.razer_get_report(0x03, 0x00, 0x03)
        led_state = 0x01 if enabled else 0x00
        request.arguments = [0x01, led, led_state]
        request.transaction_id = 0x3F
        self.razer_send_payload(request)

    def get_led_brightness(self, led):
        request = self.razer_get_report(0x03, 0x83, 0x03)
        request.arguments = [RazerReport.VARSTORE, led]
        response = self.razer_send_payload(request)
        return response.arguments[2]

    def set_led_brightness(self, led, brightness):
        # Python has a double but we need an integer
        brightness = int(brightness)
        request = self.razer_get_report(0x03, 0x03, 0x03)
        request.arguments = [RazerReport.VARSTORE, led, brightness]
        self.razer_send_payload(request)

    def get_logo_active(self):
        return self.get_led_active(RazerReport.LOGO_LED) == 1

    def set_logo_active(self, enabled):
        return self.set_led_active(RazerReport.LOGO_LED, enabled)

    def get_logo_brightness(self):
        return self.get_led_brightness(RazerReport.LOGO_LED)

    def set_logo_brightness(self, brightness):
        return self.set_led_brightness(RazerReport.LOGO_LED, brightness)

    def get_scroll_active(self):
        return self.get_led_active(RazerReport.SCROLL_WHEEL_LED) == 1

    def set_scroll_active(self, enabled):
        self.set_led_active(RazerReport.SCROLL_WHEEL_LED, enabled)

    def get_scroll_brightness(self):
        return self.get_led_brightness(RazerReport.SCROLL_WHEEL_LED)

    def set_scroll_brightness(self, brightness):
        return self.set_led_brightness(RazerReport.SCROLL_WHEEL_LED, brightness)

    def set_dpi_xy(self, dpi_x, dpi_y):
        return self.set_misc_dpi_xy(RazerReport.NOSTORE, dpi_x, dpi_y)

    def set_misc_dpi_xy(self, var_store, dpi_x, dpi_y):
        request = self.razer_get_report(0x04, 0x05, 0x07)

        # Keep the DPI within bounds
        dpi_x = max(min(dpi_x, 16000), 128);
        dpi_y = max(min(dpi_y, 16000), 128);

        request.arguments = [var_store,
                             (dpi_x >> 8) & 0x00FF,
                              dpi_x & 0x00FF,
                             (dpi_y >> 8) & 0x00FF,
                              dpi_y & 0x00FF]
        self.razer_send_payload(request)

    def get_dpi_xy(self):
        return self.get_misc_dpi_xy(RazerReport.NOSTORE)

    def get_misc_dpi_xy(self, var_store):
        request = self.razer_get_report(0x04, 0x85, 0x07)
        request.arguments = [var_store]
        response = self.razer_send_payload(request)

        dpi_x = (response.arguments[1] << 8) | (response.arguments[2] & 0xFF)
        dpi_y = (response.arguments[3] << 8) | (response.arguments[4] & 0xFF)

        return [dpi_x, dpi_y]

    def set_poll_rate(self, rate):
        request = self.razer_get_report(0x00, 0x05, 0x01)
        request.arguments = [int(1000 / rate)]
        self.razer_send_payload(request)

    def get_poll_rate(self):
        request = self.razer_get_report(0x00, 0x85, 0x01)
        response = self.razer_send_payload(request)
        rate = response.arguments[0]
        return 1000 / rate if rate > 0 else 0

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
    def razer_feature_report_desc(cls):
        return ''.join(chr(x) for x in (0x06, 0x00, 0xff, 0x09, 0x02, 0x15, 0x00, 0x25, 0x01, 0x75, 0x08, 0x95, 0x5a, 0xb1, 0x01))

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
            if cls.USE_HIDRAW:
                hidraw_path = os.path.join('/dev', os.listdir(os.path.join(dev_path, 'hidraw'))[0])

                with open(hidraw_path, 'rb') as hid:
                    hid_node = hidraw.HIDRaw(hid)

                    return cls.razer_feature_report_desc() in hid_node.getRawReportDescriptor()

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
