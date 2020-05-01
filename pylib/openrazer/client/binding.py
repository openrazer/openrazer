"""
Client binding controls
"""

import dbus as _dbus


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
        """

        self._binding_dbus.addAction(int(profile), mapping, str(key_code), "key", str(value))
