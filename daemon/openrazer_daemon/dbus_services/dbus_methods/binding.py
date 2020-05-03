"""
Keyboard Binding methods
"""
import json
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.binding', 'addProfile', in_sig='s')
def add_profile(self, profile):
    """
    Add a new profile

    :param profile: The profile name
    :type: str
    """
    self.logger.debug("DBus call add_profile")

    self.binding_manager._profiles.update({str(len(self.binding_manager._profiles)): {"name": profile, "default_map": "Default", "Default": {"is_using_matrix": False, "binding": {}}}})


@endpoint('razer.device.binding', 'removeProfile', in_sig='s')
def remove_profile(self, profile):
    """
    Delete the specified profile

    :param profile: The profile number
    :type: str
    """
    self.logger.debug("DBus call remove_profile")

    self.binding_manager._profiles.pop(str(profile))


@endpoint('razer.device.binding', 'getProfiles', out_sig='s')
def get_profiles(self):
    """
    Get profiles

    :return: JSON of profiles
    :rtype: str
    """
    self.logger.debug("DBus call get_profiles")

    return self.binding_manager.dbus_get_profiles()


@endpoint('razer.device.binding', 'getActiveProfile', out_sig='s')
def get_active_profile(self):
    """
    Get the active profile

    :return: Name of active profile
    :rtype: str
    """

    self.logger.debug("DBus call get_active_profile")

    return self.binding_manager.current_profile


@endpoint('razer.device.binding', 'setActiveProfile', in_sig='s')
def set_active_profile(self, profile):
    """
    Set the active profile

    :param profile: The profile number
    :type: str
    """

    self.logger.debug("DBus call set_active_profile")

    self.binding_manager.current_profile = profile


@endpoint('razer.device.binding', 'setDefaultMap', in_sig='ss')
def set_default_map(self, profile, mapping):
    """
    Set the default map

    :param profile: The profile number
    :type: str

    :param map: The map
    :type: str
    """
    self.logger.debug("DBus call set_default_map")

    self.binding_manager._profiles[profile]["default_map"] = mapping

    self.write_config_file(self.get_config_file_name())


@endpoint('razer.device.binding', 'getDefaultMap', in_sig='str', out_sig='s')
def get_default_map(self, profile):
    """
    Get the default map for a specific profile

    :param profile: The profile number
    :type: str

    :return: The default map
    :rtype: str
    """
    self.logger.debug("DBus call get_default_profile")

    return self.binding_manager._profiles[profile]["default_map"]


@endpoint('razer.device.binding', 'getMaps', in_sig='str', out_sig='s')
def get_maps(self, profile):
    """
    Get maps

    :param profile: The profile number
    :type: str

    :return: JSON of maps
    :rtype: str
    """

    self.logger.debug("DBus call get_maps")

    return self.binding_manager.dbus_get_maps(profile)


@endpoint('razer.device.binding', 'getMap', in_sig='ss', out_sig='s')
def get_map(self, profile, map):
    """
    Get map

    :param profile: The profile number
    :type: str

    :param map: The map name
    :type: str

    :return: JSON of maps
    :rtype: str
    """

    self.logger.debug("DBus call get_map")

    return json.dumps(self.binding_manager._profiles[profile][map])


@endpoint('razer.device.binding', 'getActiveMap', out_sig='s')
def get_active_map(self):
    """
    Get the active map

    :return: The active map
    :rtype: str
    """

    self.logger.debug("DBus call get_active_map")

    return self.binding_manager.current_mapping


@endpoint('razer.device.binding', 'setActiveMap', in_sig='s')
def set_active_map(self, map):
    """
    Set the active map

    :param map: The map name
    :type: str
    """

    self.logger.debug("DBus call set_active_map")

    self.binding_manager.current_mapping = map


@endpoint('razer.device.binding', 'getActions', in_sig='sss', out_sig='s')
def get_actions(self, profile, map, key_code):
    """
    Get profiles

    :param profile: The profile number
    :type: str

    :param map: The map number
    :type: str

    :param key_code: The key code
    :type: str

    :return: JSON of actions
    :rtype: str
    """
    self.logger.debug("DBus call get_actions")

    return json.dumps(self.binding_manager._profiles[profile][map]["binding"][key_code])


@endpoint('razer.device.binding', 'addMap', in_sig='ss')
def add_map(self, profile, map):
    """
    Add a map

    :param profile: The profile number
    :type: str

    :param map: The map name
    :type: str
    """

    self.logger.debug("DBus call add_map")

    self.binding_manager._profiles[profile].update({map: {"is_using_matrix": False, "binding": {}}})

    self.write_config_file(self.get_config_file_name())


@endpoint('razer.device.binding', 'addAction', in_sig='sssss')
def add_action(self, profile, map, key_code, action_type, value):
    """
    Add an action to the given key

    :param profile: The profile number
    :type: str

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


@endpoint('razer.device.binding', 'updateAction', in_sig='sssss')
def update_action(self, profile, map, key_code, action_type, value, action_id):
    """
    Add an action to the given key

    :param profile: The profile number
    :type: str

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

    self.binding_manager.dbus_add_action(
        profile, map, key_code, action_type, value, action_id)


@endpoint('razer.device.binding', 'removeAction', in_sig='ssss')
def remove_action(self, profile, map, key_code, action_id):
    """
    Remove the specified action

    :param profile: The profile number
    :type: str

    :param map: The map number
    :type: str

    :param key_code: The key code
    :type: str

    :param action_id: The action id
    :type: str
    """
    self.logger.debug("DBus call remove_action")

    self.binding_manager.dbus_remove_action(profile, map, key_code, action_id)


@endpoint('razer.device.binding', 'clearActions', in_sig='sss')
def clear_actions(self, profile, map, key_code):
    """
    Clear all actions for a given key

    :param profile: The profile number
    :type: str

    :param map: The map name
    :type: str

    :param key_code: The key code
    :type: str
    """

    self.logger.debug("DBus call clear_actions")

    self.binding_manager._profiles[profile][map]["binding"].pop(key_code)

    self.write_config_file(self.get_config_file_name())


@endpoint('razer.device.binding.lighting', 'getProfileLEDs', in_sig='ss', out_sig='bbb')
def get_profile_leds(self, profile, map):
    """
    Get the state of the profile LEDs

    :param profile: The profile number
    :type: str

    :param map: The map name
    :type: str

    :return: The state of the Red profile LED
    :type: bool

    :return: The state of the Green profile LED
    :type: bool

    :return: The state of the Blue profile LED
    :type: bool
    """
    self.logger.debug("DBus call get_profile_leds")

    map = self.binding_manager._profiles[profile][map]

    return map["red_led"], map["green_led"], map["blue_led"]


@endpoint('razer.device.binding.lighting', 'setProfileLEDs', in_sig='ssbbb')
def set_profile_leds(self, profile, map, red, green, blue):
    """
    Set the profile LED state

    :param profile: The profile number
    :type: str

    :param map: The map name
    :type: str

    :param red: The red LED state
    :type: bool

    :param green: The green LED state
    :type: bool

    :param blue: The blue LED state
    :type: bool
    """

    self.logger.debug("DBus call set_profile_leds")

    self.binding_manager._profiles[profile][map].update(
        {'red_led': red, 'green_led': green, 'blue_led': blue})

    self.write_config_file(self.get_config_file_name())


@endpoint('razer.device.binding.lighting', 'getMatrix', in_sig='ss', out_sig='s')
def get_matrix(self, profile, map):
    """
    Get the led matrix of the specified map

    :param profile: The profile number
    :type: str

    :param map: The map name
    :type: str

    :return: JSON of matrix
    :rtype: str
    """

    self.logger.debug("DBus call get_matrix")

    return json.dumps(self.key_manager._profiles[profile][map]["matrix"])


@endpoint('razer.device.binding.lighting', 'setMatrix', in_sig='sss')
def set_matrix(self, profile, map, matrix):
    """
    Set the led matrix

    :param profile: The profile number
    :type: str

    :param map: The map name
    :type: str

    :param matrix: The led matrix
    :type: str
    """

    self.logger.debug("DBus call set_matrix")

    self.binding_manager.dbus_set_matrix(profile, map, json.loads(matrix))
