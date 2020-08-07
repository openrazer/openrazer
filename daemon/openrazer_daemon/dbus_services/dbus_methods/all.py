"""
DBus methods available for all devices.
"""
import os
from openrazer_daemon.dbus_services import endpoint

# Keyboard layout IDs
# There are more than listed here, but others are not known yet.
layoutids = {"01": "en_US",
             "02": "el_GR",
             "03": "de_DE",
             "04": "fr_FR",
             "05": "ru_RU",
             "06": "en_GB",
             "07": "Nordic",
             "0A": "tr_TR",
             "0C": "ja_JP",
             "0F": "de_CH",
             "10": "es_ES",
             "11": "it_IT",
             "12": "pt_PT",
             "81": "en_US_mac"}


@endpoint('razer.device.misc', 'getDriverVersion', out_sig='s')
def version(self):
    """
    Get the devices driver version

    :return: Get driver version string like 1.0.7
    :rtype: str
    """
    self.logger.debug("DBus call version")

    # Caching
    if 'driver_version' in self.method_args:
        return self.method_args['driver_version']

    driver_path = self.get_driver_path('version')

    driver_version = '0.0.0'

    if os.path.exists(driver_path):
        # Check it exists, as people might not have reloaded driver
        with open(driver_path, 'r') as driver_file:
            driver_version = driver_file.read().strip()

    self.method_args['driver_version'] = driver_version
    return driver_version


@endpoint('razer.device.misc', 'getFirmware', out_sig='s')
def get_firmware(self):
    """
    Get the devices firmware version

    :return: Get firmware string like v1.1
    :rtype: str
    """
    self.logger.debug("DBus call get_firmware")

    driver_path = self.get_driver_path('firmware_version')

    with open(driver_path, 'r') as driver_file:
        return driver_file.read().strip()


@endpoint('razer.device.misc', 'getDeviceName', out_sig='s')
def get_device_name(self):
    """
    Get the device's descriptive string

    :return: Device string like 'BlackWidow Ultimate 2013'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_name")

    driver_path = self.get_driver_path('device_type')

    with open(driver_path, 'r') as driver_file:
        return driver_file.read().strip()


@endpoint('razer.device.misc', 'getKeyboardLayout', out_sig='s')
def get_keyboard_layout(self):
    """
    Get the device's keyboard layout

    :return: String like 'en_US', 'de_DE', 'en_GB' or 'unknown'
    :rtype: str
    """
    self.logger.debug("DBus call get_keyboard_layout")

    driver_path = self.get_driver_path('kbd_layout')

    with open(driver_path, 'r') as driver_file:
        try:
            return layoutids[driver_file.read().strip()]
        except KeyError:
            return "unknown"


# Functions to define a hardware class
@endpoint('razer.device.misc', 'getDeviceType', out_sig='s')
def get_device_type_keyboard(self):
    """
    Get the device's type

    :return: 'keyboard'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'keyboard'


@endpoint('razer.device.misc', 'getDeviceType', out_sig='s')
def get_device_type_mouse(self):
    """
    Get the device's type

    :return:'mouse'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'mouse'


@endpoint('razer.device.misc', 'getDeviceType', out_sig='s')
def get_device_type_mousemat(self):
    """
    Get the device's type

    :return:'mousemat'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'mousemat'


@endpoint('razer.device.misc', 'getDeviceType', out_sig='s')
def get_device_type_core(self):
    """
    Get the device's type

    :return:'core'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'core'


@endpoint('razer.device.misc', 'getDeviceType', out_sig='s')
def get_device_type_keypad(self):
    """
    Get the device's type

    :return:'keypad'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'keypad'


@endpoint('razer.device.misc', 'getDeviceType', out_sig='s')
def get_device_type_headset(self):
    """
    Get the device's type

    :return:'headset'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'headset'


@endpoint('razer.device.misc', 'getDeviceType', out_sig='s')
def get_device_type_accessory(self):
    """
    Get the device's type

    :return:'accessory'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'accessory'


@endpoint('razer.device.misc', 'hasMatrix', out_sig='b')
def has_matrix(self):
    """
    If the device has an LED matrix
    """
    self.logger.debug("DBus call has_matrix")

    return self.HAS_MATRIX


@endpoint('razer.device.misc', 'getMatrixDimensions', out_sig='ai')
def get_matrix_dims(self):
    """
    If the device has an LED matrix
    """
    self.logger.debug("DBus call has_matrix")

    return list(self.MATRIX_DIMS)


@endpoint('razer.device.misc', 'setFanSpeed', in_sig='yy')
def set_fan_speed(self, fan_id, fan_speed):
    """
     set fan speed
     :param fan_id: might change fans separately if more than one available (ususally 0x00)
     :param fan_speed: speed in RPM/100
     Values appear to be RPM/100, observed were 0x2b - 0x36, i.e. 4300RPM - 5400RPM
    """

    self.logger.debug("DBus call set_fan_speed")

    driver_path = self.get_driver_path('fan_{0:d}_speed'.format(int(fan_id)))

    payload = bytes([fan_speed])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.misc', 'getFanSpeed', in_sig='y', out_sig='y')
def get_fan_speed(self, fan_id):
    """
     get fan speed
     :return fan_speed: speed in RPM/100
     Values appear to be RPM/100, observed were 0x2b - 0x36, i.e. 4300RPM - 5400RPM
    """

    self.logger.debug("DBus call get_fan_speed")

    driver_path = self.get_driver_path('fan_{0:d}_speed'.format(int(fan_id)))

    with open(driver_path, 'rb') as driver_file:
        result_string = driver_file.read()
        return int.from_bytes(result_string, 'little')


@endpoint('razer.device.misc', 'setFanMode', in_sig='yyy')
def set_fan_mode(self, fan_id, fan_mode, game_mode):
    """
     set fan mode
     :param fan_id: might change fans separately if more than one available (ususally 0x00)
     :param fan_mode: 0x00: automatic 0x01: manual
     :param game_mode: This argument is for power mode of the CPU,
        0 - balanced
        1 - gaming
        2 - creator
        4 - custom
        with 4 goes CPU boost and GPU boost power
    """

    self.logger.debug("DBus call set_fan_mode")

    driver_path = self.get_driver_path('fan_{0:d}_mode'.format(int(fan_id)))

    payload = bytes([fan_mode, game_mode])

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.misc', 'getFanMode', in_sig='y', out_sig='yy')
def get_fan_mode(self, fan_id):
    """
     get fan mode
     :return fan_id: might change fans separately if more than one available (ususally 0x00)
     :return fan_mode: 0x00: automatic 0x01: manual
     :return game_mode: This argument is for power mode of the CPU,
        0 - balanced
        1 - gaming
        2 - creator
        4 - custom
        with 4 goes CPU boost and GPU boost power
    """

    self.logger.debug("DBus call get_fan_mode")

    driver_path = self.get_driver_path('fan_{0:d}_mode'.format(int(fan_id)))

    with open(driver_path, 'rb') as driver_file:
        result = driver_file.read()
        return (int.from_bytes(result[:1], 'little'), int.from_bytes(result[1:], 'little'),)
