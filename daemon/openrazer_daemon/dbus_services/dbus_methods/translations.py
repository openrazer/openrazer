
"""
DBus methods available for devices that have ramapping option.
"""
import os
from openrazer_daemon.dbus_services import endpoint


def find_driver_path(_self):
    """
    Get path to driver helper if device is mouse path is in another interface

    :param Self from parent function
    :type object
    :return Path to driver
    :rtype string
    """
    driver_path = _self.get_driver_path('button_translations')

    if not os.path.exists(driver_path):
        for kb_int in _self.additional_interfaces:
            driver_path = os.path.join(kb_int, 'button_translations')
            if os.path.exists(driver_path):
                return driver_path

    return driver_path


@endpoint('razer.device.translations', 'getTranslations', out_sig='ay')
def translations_get(self):
    """
    Get current keybindings from device

    :return Array of bytes representing current translations
    :rtype bytes
    """
    self.logger.debug("DBus call translations_get")

    driver_path = find_driver_path(self)

    with open(driver_path, 'rb') as driver_file:
        return driver_file.read()


@endpoint('razer.device.translations', 'setTranslations', in_sig='ay')
def translations_set(self, translations):
    """
    Set keybindings for device

    :param translations: Array of bytes representing translations to set
    :type translations: bytes
    """
    self.logger.debug("DBus call translations_set")

    driver_path = find_driver_path(self)

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(bytes([v for v in translations]))
