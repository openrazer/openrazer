"""
Client binding controls
"""

import dbus as _dbus
import json


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

    def add_key_action(self, profile: str, mapping: str, key_code: int, value: int) -> bool:
        """
        Add an key press action to the given key

        :param profile: The profile number
        :type: str

        :param mapping: The map name
        :type: str

        :param key_code: The key code
        :type: int

        :param value: The key to press
        :type: int

        :return: True if success, False otherwise
        :rtype: bool

        :raises ValueError: If parameters are invalid
        """
        if not isinstance(profile, str):
            raise ValueError("profile must be a string")
        if not isinstance(mapping, str):
            raise ValueError("mapping must be a string")
        if not isinstance(key_code, int):
            raise ValueError("key_code must be an integer")
        if not isinstance(value, int):
            raise ValueError("value must be an integer")

        self._binding_dbus.addAction(profile, mapping, str(key_code), "key", str(value))

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
