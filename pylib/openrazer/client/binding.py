"""
Client binding controls
"""

import dbus as _dbus
import json

action_types = ["key", "map", "shift"]


class Binding(object):
    def __init__(self, serial: str, capabilities: dict, daemon_dbus=None):
        self._capabilities = capabilities

        if daemon_dbus is None:
            session_bus = _dbus.SessionBus()
            daemon_dbus = session_bus.get_object("org.razer", "/org/razer/device/{0}".format(serial))
        self._dbus = daemon_dbus

        self._binding_dbus = _dbus.Interface(self._dbus, "razer.device.binding")

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

    ### Map Methods ###

    def add_map(self, profile: str, map_name: str):
        """
        Add a map to the given profile

        :param profile: The profile number
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

    ### Action Methods ###

    def get_actions(self, profile: str, mapping: str, key_code: int) -> dict:
        """
        Returns a dict of the actions for the given key

        :param profile: The profile number
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

        return json.loads(self._binding_dbus.getActions(profile, mapping, str(key_code)))

    def add_action(self, profile: str, mapping: str, key_code: int, action_type: str, value: str):
        """
        Add an action to the given key

        :param profile: The profile number
        :type: str

        :param mapping: The map name
        :type: str

        :param key_code: The key code
        :type: int

        :param action_type: The type of action
        :type: str, must be one of the following: "key", "map", "shift"

        :param value: The action value
        :type: str

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if not isinstance(key_code, int):
            raise ValueError("key_code must be an integer")
        if action_type not in action_types:
            raise ValueError("action_type must be on of the following values: {0}".format(action_types))
        if not isinstance(value, str):
            raise ValueError("value must be an string")

        self._binding_dbus.addAction(profile, mapping, str(key_code), action_type, value)

    def remove_action(self, profile: str, mapping: str, key_code: int, action_id: int):
        """
        Remove the specified action

        :param profile: The profile number
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

        self._binding_dbus.removeAction(profile, mapping, str(key_code), str(action_id))

    def clear_actions(self, profile: str, mapping: str, key_code: int):
        """
        Clear all actions for the given key

        :param profile: The profile number
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

        self._binding_dbus.clearActions(profile, mapping, str(key_code))
