"""
DBus methods available for all devices.
"""
from razer_daemon.dbus_services import endpoint

@endpoint('razer.device.misc', 'getFirmware', out_sig='s')
def get_firmware(self):
    """
    Get the devices firmware version

    :return: Get firmware string like v1.1
    :rtype: str
    """
    driver_path = self.get_driver_path('get_firmware_version')

    with open(driver_path, 'r') as driver_file:
        return driver_file.read().strip()

@endpoint('razer.device.misc', 'getDeviceType', out_sig='s')
def get_device_type(self):
    """
    Get the device's descriptive string

    :return: Device string like 'BlackWidow Ultimate 2013'
    :rtype: str
    """
    driver_path = self.get_driver_path('get_device_type')

    with open(driver_path, 'r') as driver_file:
        return driver_file.read().strip()


