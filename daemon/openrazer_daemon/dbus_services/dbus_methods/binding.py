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

@endpoint('razer.device.binding', 'getMaps', in_sig='y', out_sig='s')
def get_maps(self, profile):
    """
    Get maps

    :param profile: The profile number
    :type: int

    :return: JSON of maps
    :rtype: str
    """

    self.logger.debug("DBus call get_maps")

    return self.binding_manager.dbus_get_maps(profile)

@endpoint('razer.device.binding', 'getActions', in_sig='yyy' out_sig='s')
def get_actions(self, profile, map, key_code):
    """
    Get profiles

    :param profile: The profile number
    :type: int

    :param map: The map number
    :type: int

    :param key_code: The key code
    :type: int

    :return: JSON of actions
    :rtype: str
    """
    self.logger.debug("DBus call get_actions")
  
    return self.binding_manager.dbus_get_actions(profile, map, key_code)