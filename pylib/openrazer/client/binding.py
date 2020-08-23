"""
Client binding controls
"""

import json
import dbus as _dbus

from openrazer.client.constants import ACTION_TYPES


# pylint: disable=too-many-public-methods
class Binding():
    def __init__(self, serial: str, capabilities: dict, daemon_dbus=None):
        self._capabilities = capabilities

        if daemon_dbus is None:
            session_bus = _dbus.SessionBus()
            daemon_dbus = session_bus.get_object("org.razer", "/org/razer/device/{0}".format(serial))
        self._dbus = daemon_dbus

        self._binding_dbus = _dbus.Interface(self._dbus, "razer.device.binding")
        self._lighting_dbus = _dbus.Interface(self._dbus, "razer.device.binding.lighting")
        self._macro_dbus = _dbus.Interface(self._dbus, "razer.device.macro")

    def has(self, capability: str) -> bool:
        """
        Convenience function to check capability

        Uses the main device capability list and automatically prefixes 'binding_'
        :param capability: Device capability
        :type capability: str

        :return: True or False
        :rtype: bool
        """
        return self._capabilities.get('binding_' + capability, False)

    ### Profile Methods ###

    def add_profile(self, profile_name: str):
        """
        Add a profile

        :param profile_name: Name of the new profile
        :type: str

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile_name, str):
            raise ValueError("profile_name must be a string")

        self._binding_dbus.addProfile(profile_name)

    def remove_profile(self, profile: str):
        """
        Remove a profile

        :param profile: The profile name
        :type: str

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))

        self._binding_dbus.removeProfile(profile)

    def get_profiles(self) -> list:
        """
        Returns a list of the profiles

        :return: A list of profiles
        :rtype: list[str]
        """

        return json.loads(self._binding_dbus.getProfiles())

    def get_active_profile(self) -> str:
        """
        Return the active profile

        :return: The active profile name
        :rtype: str
        """

        return self._binding_dbus.getActiveProfile()

    def set_active_profile(self, profile: str):
        """
        Set the active profile

        :param profile: The profile name
        :type: str

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))

        self._binding_dbus.setActiveProfile(profile)

    def get_default_map(self) -> str:
        """
        Get the name of the default map

        :return: The name of the default map
        :rtype: str
        """

        return self._binding_dbus.getDefaultMap()

    def set_default_map(self, profile: str, mapping: str):
        """
        Set the default map of the given profile

        :param profile: The profile name
        :type: str

        :param mapping: The map name
        :type: str

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))
        if mapping not in self.get_maps(profile):
            raise ValueError("Map {0} does not exist".format(map))

        self._binding_dbus.setDefaultMap(profile, mapping)

    ### Map Methods ###

    def add_map(self, profile: str, map_name: str):
        """
        Add a map to the given profile

        :param profile: The profile name
        :type: str

        :param map_name: The name of the map
        :type: str

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(map_name, str):
            raise ValueError("map_name must be a string")

        self._binding_dbus.addMap(profile, map_name)

    def copy_map(self, profile: str, mapping: str, dest_profile: str, map_name: str):
        """
        Copy a mapping to the given profile

        :param profile: The profile name
        :type: str

        :param mapping: The map to copy
        :type: str

        :dest_profile: The destination profile number
        :type: str

        :param map_name: The name of the new map
        :type: str

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if not isinstance(dest_profile, str):
            raise ValueError("dest_profile must be a string")
        if not isinstance(map_name, str):
            raise ValueError("map_name must be a string")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))
        if mapping not in self.get_maps(profile):
            raise ValueError("Map {0} does not exist".format(map))
        if dest_profile not in self.get_profiles():
            raise ValueError("Destination profile {0} does not exist".format(dest_profile))

        self._binding_dbus.copyMap(profile, mapping, dest_profile, map_name)

    def remove_map(self, profile: str, mapping: str):
        """
        Remove the given mapping

        :param profile: The profile name
        :type: str

        :param mapping: The map to remove
        :type: str

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))
        if mapping not in self.get_maps(profile):
            raise ValueError("Map {0} does not exist".format(map))

        self._binding_dbus.removeMap(profile, mapping)

    def get_active_map(self) -> str:
        """
        Returns the active map

        :return: The active map name
        :rtype: str
        """

        return str(self._binding_dbus.getActiveMap())

    def set_active_map(self, mapping: str):
        """
        Sets the active map with in the active profile

        :param mapping: The map name
        :type: str

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if mapping not in self.get_maps(self.get_active_profile()):
            raise ValueError("Map {0} does not exist".format(map))

        self._binding_dbus.setActiveMap(mapping)

    def get_maps(self, profile: str) -> list:
        """
        Returns a list of maps foe the given profile

        :param profile: The profile name
        :type: str

        :returns: A list of maps
        :rtype: list

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))

        return json.loads(self._binding_dbus.getMaps(profile))

    ### Action Methods ###

    def get_actions(self, profile: str, mapping: str, key_code: int) -> dict:
        """
        Returns a dict of the actions for the given key

        :param profile: The profile name
        :type: str

        :param mapping: The map name
        :type: str

        :param key_code: The key code
        :type: int

        :return: A dict containing all the actions for the given key
        :rtype: dict

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if not isinstance(key_code, int):
            raise ValueError("key_code must be an integer")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))
        if mapping not in self.get_maps(profile):
            raise ValueError("Map {0} does not exist".format(map))

        return json.loads(self._binding_dbus.getActions(profile, mapping, str(key_code)))

    # pylint: disable=too-many-arguments
    def add_action(self, profile: str, mapping: str, key_code: int, action_type: str, value: str, action_id: int = None):
        """
        Add (or update) an action to the given key

        :param profile: The profile name
        :type: str

        :param mapping: The map name
        :type: str

        :param key_code: The key code
        :type: int

        :param action_type: The type of action
        :type: str, must be defined by openrazer.client.constants.ACTION_TYPES

        :param value: The action value
        :type: str

        :param action_id: The action to update
        :type: int

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if not isinstance(key_code, int):
            raise ValueError("key_code must be an integer")
        if action_type not in ACTION_TYPES:
            raise ValueError("action_type must be on of the following values: {0}".format(ACTION_TYPES))
        if not isinstance(value, str):
            raise ValueError("value must be an string")
        if not isinstance(action_id, int) and action_id is not None:
            raise ValueError("action_id must be an integer")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))
        if mapping not in self.get_maps(profile):
            raise ValueError("Map {0} does not exist".format(map))
        if action_id is not None and action_id not in self.get_actions(profile, mapping, key_code):
            raise ValueError("Action {0} does not exist".format(action_id))

        if action_id is None:
            self._binding_dbus.addAction(profile, mapping, key_code, action_type, value)
        else:
            self._binding_dbus.updateAction(profile, mapping, key_code, action_type, value, action_id)

    def remove_action(self, profile: str, mapping: str, key_code: int, action_id: int):
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

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if not isinstance(key_code, int):
            raise ValueError("key_code must be an integer")
        if not isinstance(action_id, int):
            raise ValueError("action_id must be an integer")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))
        if mapping not in self.get_maps(profile):
            raise ValueError("Map {0} does not exist".format(map))
        if action_id not in self.get_actions(profile, mapping, key_code):
            raise ValueError("Action {0} does not exist".format(action_id))

        self._binding_dbus.removeAction(profile, mapping, key_code, action_id)

    def clear_actions(self, profile: str, mapping: str, key_code: int):
        """
        Clear all actions for the given key

        :param profile: The profile name
        :type: str

        :param mapping: The map name
        :type: str

        :param key_code: The key code to clear
        :type: int

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if not isinstance(key_code, int):
            raise ValueError("key_code must be an integer")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))
        if mapping not in self.get_maps(profile):
            raise ValueError("Map {0} does not exist".format(map))

        self._binding_dbus.clearActions(profile, mapping, key_code)

    ### Lighting Methods ###

    def get_profile_leds(self, profile: str, mapping: str):
        """
        Returns the setting for the profile LEDs of a given map

        :param profile: The profile name
        :type: str

        :param mapping: The map name
        :type: str

        :return: The state for the red profile LED
        :type: bool

        :return: The state for the green profile LED
        :type: bool

        :return: The state for the blue profile LED
        :type: bool

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))
        if mapping not in self.get_maps(profile):
            raise ValueError("Map {0} does not exist".format(map))

        return self._lighting_dbus.getProfileLEDs(profile, mapping)

    def set_profile_leds(self, profile: str, mapping: str, red: bool, green: bool, blue: bool):
        """
        Set the setting for the profile LEDs of a given map

        :param profile: The profile name
        :type: str

        :param mapping: The map name
        :type: str

        :param red: The red profile LED state
        :type: bool

        :param green: The green profile LED state
        :type: bool

        :param blue: The blue profile LED state
        :type: bool

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if not isinstance(red, bool):
            raise ValueError("red must be a bool")
        if not isinstance(green, bool):
            raise ValueError("green must be a bool")
        if not isinstance(blue, bool):
            raise ValueError("blue must be a bool")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))
        if mapping not in self.get_maps(profile):
            raise ValueError("Map {0} does not exist".format(map))

        self._lighting_dbus.setProfileLEDs(profile, mapping, red, green, blue)

    def get_matrix(self, profile: str, mapping: str):
        """
        Returns a dict of the custom matrix for the given map

        :param profile: The profile name
        :type: str

        :param mapping: The map name
        :type: str

        :returns: A dict of the custom matrix
        :rtype: dict

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))
        if mapping not in self.get_maps(profile):
            raise ValueError("Map {0} does not exist".format(map))

        return json.loads(self._lighting_dbus.getMatrix(profile, mapping))

    def set_matrix(self, profile: str, mapping: str, matrix: dict):
        """
        Set the custom matrix for the given map

        :param profile: The profile name
        :type: str

        :param mapping: The map name
        :type: str

        :param matrix: The matrix, can be output by Frame.to_dict()
        :type: dict

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if not isinstance(matrix, dict):
            raise ValueError("matrix must be a dictionary")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))
        if mapping not in self.get_maps(profile):
            raise ValueError("Map {0} does not exist".format(map))

        self._lighting_dbus.setMatrix(profile, mapping, json.dumps(matrix))

    ### Macro Methods ###
    def start_macro_recording(self, profile: str, mapping: str, key_code: int):
        """
        Start recording a macro to the given key

        :param profile: The profile name
        :type: str

        :param mapping: The map name
        :type: str

        :param key_code: The key to record to
        :type: int

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if not isinstance(key_code, int):
            raise ValueError("key_code must be an integer")
        if profile not in self.get_profiles():
            raise ValueError("Profile {0} does not exist".format(profile))
        if mapping not in self.get_maps(profile):
            raise ValueError("Map {0} does not exist".format(map))

        self._macro_dbus.startMacroRecording(profile, mapping, key_code)

    def stop_macro_recording(self):
        """
        Stop recording a macro
        """
        self._macro_dbus.stopMacroRecording()

    def get_macro_recording_state(self) -> bool:
        """
        Returns True if a macro is currently being recorded, False otherwise.

        :return: True if a macro is currently being recorded, False otherwise
        :rtype: bool

        """
        return self._macro_dbus.getMacroRecordingState()

    def get_macro_key(self) -> int:
        """
        Returns the key being recorded to

        :return: The key being recorded to or None
        :rtype: int or None
        """
        return self._macro_dbus.getMacroKey()
