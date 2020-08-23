"""
Keyboard Binding methods
"""
import json
from openrazer_daemon.dbus_services import endpoint
# pylint: disable=protected-access,too-many-arguments


@endpoint('razer.device.binding', 'addProfile', in_sig='s')
def add_profile(self, profile):
    """
    Add a new profile

    :param profile: The profile name
    :type: str
    """
    self.logger.debug("DBus call add_profile")

    self.binding_manager._profiles.update({profile: {"default_map": "Default", "Default": {"is_using_matrix": False, "binding": {}}}})

    self.binding_manager.write_config_file(self.binding_manager._config_file)


@endpoint('razer.device.binding', 'removeProfile', in_sig='s')
def remove_profile(self, profile):
    """
    Delete the specified profile

    :param profile: The profile name
    :type: str
    """
    self.logger.debug("DBus call remove_profile")

    self.binding_manager._profiles.pop(profile)

    self.binding_manager.write_config_file(self.binding_manager._config_file)


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

    return self.binding_manager._current_profile_name


@endpoint('razer.device.binding', 'setActiveProfile', in_sig='s')
def set_active_profile(self, profile):
    """
    Set the active profile

    :param profile: The profile name
    :type: str
    """
    self.logger.debug("DBus call set_active_profile")

    self.binding_manager.current_profile = profile


@endpoint('razer.device.binding', 'setDefaultMap', in_sig='ss')
def set_default_map(self, profile, mapping):
    """
    Set the default map

    :param profile: The profile name
    :type: str

    :param mapping: The map
    :type: str
    """
    self.logger.debug("DBus call set_default_map")

    self.binding_manager._profiles[profile]["default_map"] = mapping

    self.binding_manager.write_config_file(self.binding_manager._config_file)


@endpoint('razer.device.binding', 'getDefaultMap', in_sig='s', out_sig='s')
def get_default_map(self, profile):
    """
    Get the default map for a specific profile

    :param profile: The profile name
    :type: str

    :return: The default map
    :rtype: str
    """
    self.logger.debug("DBus call get_default_profile")

    return self.binding_manager._profiles[profile]["default_map"]


@endpoint('razer.device.binding', 'getMaps', in_sig='s', out_sig='s')
def get_maps(self, profile):
    """
    Get maps

    :param profile: The profile name
    :type: str

    :return: JSON of maps
    :rtype: str
    """
    self.logger.debug("DBus call get_maps")

    return self.binding_manager.dbus_get_maps(profile)


@endpoint('razer.device.binding', 'copyMap', in_sig='ssss')
def copy_map(self, profile, mapping, dest_profile, map_name):
    """
    Copy a mapping to the specified destination profile with the specified name

    :param profile: The profile to copy the mapping from
    :type: str

    :param mapping: The map to copy
    :type: str

    :param dest_profile: The destination profile
    :type: str

    :param map_name: The name to give the new mapping
    :type: str
    """
    self.logger.debug("DBus call copy_map")

    self.binding_manager._profiles[dest_profile].update({map_name: dict(self.binding_manager._profiles[profile][mapping])})


@endpoint('razer.device.binding', 'getActiveMap', out_sig='s')
def get_active_map(self):
    """
    Get the active map

    :return: The active map
    :rtype: str
    """
    self.logger.debug("DBus call get_active_map")

    return self.binding_manager._current_mapping_name


@endpoint('razer.device.binding', 'setActiveMap', in_sig='s')
def set_active_map(self, mapping):
    """
    Set the active map within the active profile

    :param mapping: The map name
    :type: str
    """
    self.logger.debug("DBus call set_active_map")

    self.binding_manager.current_mapping = mapping


@endpoint('razer.device.binding', 'getActions', in_sig='sss', out_sig='s')
def get_actions(self, profile, mapping, key_code):
    """
    Get profiles

    :param profile: The profile name
    :type: str

    :param mapping: The map number
    :type: str

    :param key_code: The key code
    :type: str

    :return: JSON of actions
    :rtype: str
    """
    self.logger.debug("DBus call get_actions")

    return json.dumps(self.binding_manager._profiles[profile][mapping]["binding"][key_code])


@endpoint('razer.device.binding', 'addMap', in_sig='ss')
def add_map(self, profile, mapping):
    """
    Add a map

    :param profile: The profile name
    :type: str

    :param mapping: The map name
    :type: str
    """
    self.logger.debug("DBus call add_map")

    self.binding_manager._profiles[profile].update({mapping: {"is_using_matrix": False, "binding": {}}})

    self.binding_manager.write_config_file(self.binding_manager._config_file)


@endpoint('razer.device.binding', 'removeMap', in_sig='ss')
def remove_map(self, profile, mapping):
    """
    Remove a map

    :param profile: The profile name
    :type: str

    :param mapping: The map name
    :type: str
    """
    self.logger.debug("DBus call remove_map")

    self.binding_manager._profiles[profile].pop(mapping)

    self.binding_manager.write_config_file(self.binding_manager._config_file)


@endpoint('razer.device.binding', 'addAction', in_sig='ssyss')
def add_action(self, profile, mapping, key_code, action_type, value):
    """
    Add an action to the given key

    :param profile: The profile name
    :type: str

    :param mapping: The map name
    :type: str

    :param key_code: The key code
    :type: int

    :param action_type: The action type
    :type: str

    :param value: The value
    :type: str
    """
    self.logger.debug("DBus call add_action")

    self.binding_manager.dbus_add_action(profile, mapping, key_code, action_type, value)


@endpoint('razer.device.binding', 'updateAction', in_sig='ssyssy')
def update_action(self, profile, mapping, key_code, action_type, value, action_id):
    """
    Add an action to the given key

    :param profile: The profile name
    :type: str

    :param mapping: The map name
    :type: str

    :param key_code: The key code
    :type: int

    :param action_type: The action type
    :type: str

    :param value: The value
    :type: str

    :param action_id: The action to update
    :type: int
    """
    self.logger.debug("DBus call update_action")

    self.binding_manager.dbus_add_action(
        profile, mapping, key_code, action_type, value, action_id)


@endpoint('razer.device.binding', 'removeAction', in_sig='ssyy')
def remove_action(self, profile, mapping, key_code, action_id):
    """
    Remove the specified action

    :param profile: The profile name
    :type: str

    :param mapping: The map name
    :type: str

    :param key_code: The key code
    :type: int

    :param action_id: The action id
    :type: int
    """
    self.logger.debug("DBus call remove_action")

    if len(self.binding_manager._profiles[profile][mapping]["binding"][str(key_code)]) == 1:
        self.clearActions(profile, mapping, key_code)
    else:
        del self.binding_manager._profiles[profile][mapping]["binding"][str(key_code)][action_id]

    self.binding_manager.write_config_file(self.binding_manager._config_file)


@endpoint('razer.device.binding', 'clearActions', in_sig='ssy')
def clear_actions(self, profile, mapping, key_code):
    """
    Clear all actions for a given key

    :param profile: The profile name
    :type: str

    :param mapping: The map name
    :type: str

    :param key_code: The key code
    :type: int
    """
    self.logger.debug("DBus call clear_actions")
    if self.binding_manager._profiles[profile]:
        if self.binding_manager._profiles[profile][mapping]:
            if str(key_code) in self.binding_manager._profiles[profile][mapping]["binding"]:
                self.binding_manager._profiles[profile][mapping]["binding"].pop(str(key_code))

    self.binding_manager.write_config_file(self.binding_manager._config_file)


@endpoint('razer.device.binding.lighting', 'getProfileLEDs', in_sig='ss', out_sig='bbb')
def get_profile_leds(self, profile, mapping):
    """
    Get the state of the profile LEDs

    :param profile: The profile name
    :type: str

    :param mapping: The map name
    :type: str

    :return: The state of the Red profile LED
    :type: bool

    :return: The state of the Green profile LED
    :type: bool

    :return: The state of the Blue profile LED
    :type: bool
    """
    self.logger.debug("DBus call get_profile_leds")

    mapping = self.binding_manager._profiles[profile][mapping]

    return mapping["red_led"], mapping["green_led"], mapping["blue_led"]


@endpoint('razer.device.binding.lighting', 'setProfileLEDs', in_sig='ssbbb')
def set_profile_leds(self, profile, mapping, red, green, blue):
    """
    Set the profile LED state

    :param profile: The profile name
    :type: str

    :param mapping: The map name
    :type: str

    :param red: The red LED state
    :type: bool

    :param green: The green LED state
    :type: bool

    :param blue: The blue LED state
    :type: bool
    """
    self.logger.debug("DBus call set_profile_leds")

    self.binding_manager._profiles[profile][mapping].update(
        {'red_led': red, 'green_led': green, 'blue_led': blue})

    self.binding_manager.write_config_file(self.binding_manager._config_file)


@endpoint('razer.device.binding.lighting', 'getMatrix', in_sig='ss', out_sig='s')
def get_matrix(self, profile, mapping):
    """
    Get the led matrix of the specified map

    :param profile: The profile name
    :type: str

    :param mapping: The map name
    :type: str

    :return: JSON of matrix
    :rtype: str
    """
    self.logger.debug("DBus call get_matrix")

    return json.dumps(self.binding_manager._profiles[profile][mapping]["matrix"])


@endpoint('razer.device.binding.lighting', 'setMatrix', in_sig='sss')
def set_matrix(self, profile, mapping, matrix):
    """
    Set the led matrix

    :param profile: The profile name
    :type: str

    :param mapping: The map name
    :type: str

    :param matrix: The led matrix
    :type: str
    """
    self.logger.debug("DBus call set_matrix")

    self.binding_manager.dbus_set_matrix(profile, mapping, json.loads(matrix))
