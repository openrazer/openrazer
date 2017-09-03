"""
DBus methods available for all devices.
"""
import os
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.misc', 'getDriverVersion', out_sig='s')
def version(self):
    """
    Get the devices driver version

    :return: Get driver version string like 1.0.7
    :rtype: str
    """
    self.logger.debug("DBus call version")

    #Caching
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
def get_device_type_firefly(self):
    """
    Get the device's type

    :return:'firefly'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'firefly'


@endpoint('razer.device.misc', 'getDeviceType', out_sig='s')
def get_device_type_tartarus(self):
    """
    Get the device's type

    :return:'tartarus'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'tartarus'


@endpoint('razer.device.misc', 'getDeviceType', out_sig='s')
def get_device_type_orbweaver(self):
    """
    Get the device's type

    :return:'tartarus'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'orbweaver'


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

    :return:'core'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'keypad'


@endpoint('razer.device.misc', 'getDeviceType', out_sig='s')
def get_device_type_headset(self):
    """
    Get the device's type

    :return:'tartarus'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'headset'


@endpoint('razer.device.misc', 'getDeviceType', out_sig='s')
def get_device_type_mug(self):
    """
    Get the device's type

    :return:'tartarus'
    :rtype: str
    """
    self.logger.debug("DBus call get_device_type")
    return 'mug'


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

