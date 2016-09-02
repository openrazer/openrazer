import json as _json

import dbus as _dbus

import razer_daemon.misc.macro as _daemon_macro
from razer_daemon import keyboard


class RazerMacro(object):
    def __init__(self, serial:str, daemon_dbus=None):
        if daemon_dbus is None:
            session_bus = _dbus.SessionBus()
            daemon_dbus = session_bus.get_object("org.razer", "/org/razer/device/{0}".format(serial))

        self._macro_dbus = _dbus.Interface(daemon_dbus, "razer.device.macro")

        self._macro_enabled = False

    def enable_macros(self):
        """
        Enable macro keys in the driver
        """
        if not self._macro_enabled:
            self._macro_dbus.enableMacroKeys()

    def get_macros(self) -> dict:
        json_payload = self._macro_dbus.getMacros()
        macro_structure = _json.loads(json_payload)

        macro_key_mapping = {}
        for bind_key, macro_list in macro_structure.items():
            macro_objects = []
            for macro_dict  in macro_list:
                macro_obj = _daemon_macro.macro_dict_to_obj(macro_dict)
                macro_objects.append(macro_obj)

            macro_key_mapping[bind_key] = macro_objects

        return macro_key_mapping

    def add_macro(self, bind_key:str, macro_object_sequence:list):
        """
        Add macro to specified bind key

        :param bind_key: Bind Key (has to be in razer.keyboard.KEY_MAPPING)
        :type bind_key: str

        :param macro_object_sequence: List of macro objects
        :type macro_object_sequence: list or tuple
        """
        if bind_key not in keyboard.KEY_MAPPING:
            raise ValueError("Key {0} is not in razer.keyboard.KEY_MAPPING".format(bind_key))
        if not isinstance(macro_object_sequence, (tuple, list)):
            raise ValueError("macro_object_sequence is not iterable")

        macro_list = []
        for macro_obj in macro_object_sequence:
            if not isinstance(macro_obj, _daemon_macro.MacroObject):
                raise ValueError("{0} is not a macro object".format(str(macro_obj)))

            macro_list.append(macro_obj.to_dict())

        json_payload = _json.dumps(macro_list)
        self._macro_dbus.addMacro(bind_key, json_payload)

    def del_macro(self, bind_key:str):
        if bind_key not in keyboard.KEY_MAPPING:
            raise ValueError("Key {0} is not in razer.keyboard.KEY_MAPPING".format(bind_key))
        else:
            self._macro_dbus.deleteMacro(bind_key)

    @staticmethod
    def create_url_macro_item(url:str) -> _daemon_macro.MacroURL:
        """
        Create a macro object that opens a URL in a browser

        :param url: URL
        :type url: str

        :return: Macro object
        :rtype: _daemon_macro.MacroURL
        """
        return _daemon_macro.MacroURL(url)