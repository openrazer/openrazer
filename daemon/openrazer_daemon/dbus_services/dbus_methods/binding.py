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


@endpoint('razer.device.binding', 'getMap', in_sig='ys', out_sig='s')
def get_map(self, profile, map):
    """
    Get map

    :param profile: The profile number
    :type: int

    :param map: The map name
    :type: str

    :return: JSON of maps
    :rtype: str
    """

    self.logger.debug("DBus call get_map")

    return self.binding_manager.dbus_get_map(profile, map)


@endpoint('razer.device.binding', 'getActions', in_sig='sss', out_sig='s')
def get_actions(self, profile, map, key_code):
    """
    Get profiles

    :param profile: The profile number
    :type: int

    :param map: The map number
    :type: str

    :param key_code: The key code
    :type: str

    :return: JSON of actions
    :rtype: str
    """
    self.logger.debug("DBus call get_actions")
  
    return self.binding_manager.dbus_get_actions(profile, map, key_code)

@endpoint('razer.device.binding', 'addMap', in_sig='ys')
def add_map(self, profile, map):
    """
    Add a map

    :param profile: The profile number
    :type: int

    :param map: The map name
    :type: str
    """

    self.logger.debug("DBus call add_map")

    self.binding_manager.dbus_get_map(profile, map)

@endpoint('razer.device.binding', 'addAction', in_sig='yssss')
def add_action(self, profile, map, key_code, action_type, value):
    """
    Add an action to the given key

    :param profile: The profile number
    :type: int

    :param map: The map number
    :type: str

    :param key_code: The key code
    :type: str

    :param action_type: The action type
    :type: str

    :param value: The value
    :type: str
    """
    self.logger.debug("DBus call add_action")
  
    self.binding_manager.dbus_add_action(profile, map, key_code, action_type, value)

@endpoint('razer.device.binding', 'updateAction', in_sig='ysssss')
def update_action(self, profile, map, key_code, action_type, value, action_id):
    """
    Add an action to the given key

    :param profile: The profile number
    :type: int

    :param map: The map number
    :type: str

    :param key_code: The key code
    :type: str

    :param action_type: The action type
    :type: str

    :param value: The value
    :type: str

    :param action_id: The action to update
    :type: str
    """
    self.logger.debug("DBus call update_action")
  
    self.binding_manager.dbus_add_action(profile, map, key_code, action_type, value, action_id)

@endpoint('razer.device.binding', 'removeAction', in_sig='ysss')
def remove_action(self, profile, map, key_code, action_id):
    """
    Remove the specified action

    :param profile: The profile number
    :type: int

    :param map: The map number
    :type: str

    :param key_code: The key code
    :type: str

    :param action_id: The action id
    :type: str
    """
    self.logger.debug("DBus call remove_action")
  
    self.binding_manager.dbus_remove_action(profile, map, key_code, action_id)

