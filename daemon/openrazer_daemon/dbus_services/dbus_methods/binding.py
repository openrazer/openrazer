"""
Keyboard Binding methods
"""
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.binding', 'getProfiles', out_sig='s')
def get_profiles(self):
    """
    Get profiles

    :return: JSON of profiles
    :rtype: str
    """
    self.logger.debug("DBus call get_profiles")

    return self.binding_manager.dbus_get_profiles()
