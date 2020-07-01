"""
Recives key events from the key event manager and does stuff.
"""

import json
import logging
import os
import time
import sys
import subprocess
from openrazer_daemon.keyboard import KeyboardColour
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from evdev import UInput, ecodes

DEFAULT_PROFILE = {
    "default_map": "Default",
    "Default": {
        "is_using_matrix": False,
        "binding": {
        }
    }
}

# pylint: disable=no-member
CAPABILITIES = {
    ecodes.EV_KEY: ecodes.keys.keys(),
    ecodes.EV_MSC: [ecodes.MSC_SCAN]
}


class KeybindingManager():
    """
    Key binding manager
    """
    # pylint: disable=too-many-instance-attributes

    def __init__(self, device_id, parent, testing=False):
        self._parent = parent
        self._device_id = device_id
        self._serial_number = self._parent.getSerial()
        self._config_file = self.get_config_file_name()

        self._logger = logging.getLogger('razer.device{0}.bindingmanager'.format(device_id))
        # self._parent.register_observer(self)
        self._testing = testing
        self._fake_device = UInput(CAPABILITIES, name="{0} (mapped)".format(self._parent.getDeviceName()))

        self._profiles = {"Default": DEFAULT_PROFILE}
        self._current_profile_name = None

        self._current_keys = []
        self._old_mapping_name = None
        self._current_mapping_name = None
        self._shift_modifier = None

        self._rows, self._cols = self._parent.MATRIX_DIMS
        self._keyboard_grid = KeyboardColour(self._rows, self._cols)

        if os.path.exists(self._config_file):
            self.read_config_file(self._config_file)
        self.current_profile = "Default"

    # pylint: disable=no-member
    def __key(self, key_code, key_action, scan_code=None):
        key_code = int(key_code)
        _fake_device = self._fake_device
        _write = _fake_device.write
        _ecodes = ecodes

        if scan_code is not None:
            _write(_ecodes.EV_MSC, _ecodes.MSC_SCAN, scan_code)

        _write(_ecodes.EV_KEY, key_code, key_action)
        _fake_device.syn()

        if key_action == 0:
            for _ in range(0, 5):
                try:
                    self._current_keys.remove(key_code)
                    break
                except ValueError:
                    time.sleep(.005)

        elif key_action == 1:
            self._current_keys.append(key_code)

    def key_action(self, key_code, key_action, scan_code=None):
        """
        Check for a binding and act on it.

        :param key_code: The key code of the pressed key.
        :type key_code: int
        """
        # self._logger.debug("Key action: {0}, {1}".format(key_code, key_action))

        key_code = str(key_code)
        current_binding = self.current_mapping["binding"]
        _key = self.__key

        if key_code in current_binding:  # Key bound
            for action in current_binding[key_code]:
                _type = action["type"]
                if key_action != 0:  # Key pressed (or autorepeat)
                    if _type == "key":
                        _key(action["value"], key_action, scan_code)

                    elif _type == "execute":
                        subprocess.run(["/bin/sh", "-c", action["value"]], check=False)

                    elif _type == "map":
                        self.current_mapping = action["value"]
                        self._shift_modifier = None  # No happy accidents

                    elif _type == "profile":
                        self.current_profile = action["value"]
                        self._shift_modifier = None  # No happy accidents

                    elif _type == "release":
                        _key(action["value"], 0, scan_code)

                    elif _type == "shift":
                        self.current_mapping = action["value"]
                        self._shift_modifier = key_code  # Set map shift key

                    elif _type == "sleep":
                        time.sleep(int(action["value"]))

                else:
                    if key_code not in (183, 184, 185, 186, 187):  # Macro key released, skip it
                        if _type == "key":  # Key released
                            _key(action["value"], 0, scan_code)

                        elif _type == "sleep":  # Wait for key to be added before removing it
                            time.sleep(int(action["value"]))

        else:  # Ordinary key action
            if key_code == self._shift_modifier:  # Key is the shift modifier
                if key_action == 0:
                    self.current_mapping = self._old_mapping_name
                    self._shift_modifier = None
                    for key in self._current_keys:  # If you forget to release a key before the releasing shift modifier, release it now.
                        _key(key, 0, scan_code)
            else:
                _key(key_code, key_action, scan_code)

    @property
    def current_mapping(self):
        """
        Return the current mapping

        :return: The current mapping
        :rtype: dict
        """
        return self._current_mapping

    @current_mapping.setter
    def current_mapping(self, value):
        """
        Set the current mapping and changes the led matrix

        :param value: The new mapping
        :type value: str
        """
        # self._logger.debug("Change mapping to %s", str(value))

        self._current_mapping = self._current_profile[value]
        self._old_mapping_name = self._current_mapping_name
        self._current_mapping_name = value

        if self._current_mapping["is_using_matrix"]:
            current_matrix = self._current_mapping["matrix"]
            for row in current_matrix:
                for key in current_matrix[row]:
                    self._keyboard_grid.set_key_colour(int(row), int(key), tuple(current_matrix[row][key]))

            payload = self._keyboard_grid.get_total_binary()
            self._parent.setKeyRow(payload)
            self._parent.setCustom()

        try:
            capabilities = self._parent.METHODS
            if 'keypad_set_profile_led_red' in capabilities:
                self._parent.setRedLED(self.current_mapping["red_led"])
            if 'keypad_set_profile_led_green' in capabilities:
                self._parent.setGreenLED(self.current_mapping["green_led"])
            if 'keypad_set_profile_led_blue' in capabilities:
                self._parent.setBlueLED(self.current_mapping["blue_led"])
        except KeyError:
            pass

    @property
    def current_profile(self):
        """
        Return the current profile

        :return: the current profile name
        :rtype: str
        """
        return self._current_profile_name

    @current_profile.setter
    def current_profile(self, value):
        """
        Set the current profile

        :param value: The profile name
        :type: int
        """
        self._current_profile = self._profiles[value]
        self._current_profile_name = value
        self.current_mapping = self._current_profile["default_map"]

    def read_config_file(self, config_file):
        """
        Read the configuration file and sets the variables accordingly

        :param config_file: path to the configuration file
        :type config_file: str

        :param profile: The profile name to use
        :type profile: str
        """
        self._logger.info("Reading config file from %s", config_file)

        with open(config_file, 'r') as file:
            self._profiles = json.load(file)

    def write_config_file(self, config_file):
        """
        Write the _profiles dict to the config file

        :param config_file: The path to the config file
        :type config_file: str
        """
        self._logger.debug("writing config file to %s", config_file)

        with open(config_file, 'w') as file:
            json.dump(self._profiles, file, indent=4)

    def get_config_file_name(self):
        """
        Get the name of the config file

        :return: path to config file
        :rtype: str
        """
        # pylint: disable=protected-access
        config_path = os.path.dirname(self._parent._config_file)
        return config_path + "/keybinding_" + self._serial_number + ".json"

    def close(self):
        try:
            self._fake_device.close()
        except RuntimeError as err:
            self._logger.exception("Error closing fake device", exc_info=err)

    def __del__(self):
        self.close()

    # DBus Stuff
    def dbus_get_profiles(self):
        return_list = []
        for profile in self._profiles:
            return_list.append(profile)

        return json.dumps(return_list)

    def dbus_get_maps(self, profile):
        return_list = []
        for key in self._profiles[profile]:
            if key not in ("name", "default_map"):
                return_list.append(key)

        return json.dumps(return_list)

    # pylint: disable=too-many-arguments
    def dbus_add_action(self, profile, mapping, key_code, action_type, value, action_id=None):
        key = self._profiles[profile][mapping]["binding"].setdefault(str(key_code), [])

        if action_id is not None:
            key[action_id] = {"type": action_type, "value": value}
        else:
            key.append({"type": action_type, "value": value})

        self.write_config_file(self._config_file)

    def dbus_set_matrix(self, profile, mapping, frame):
        self._profiles[profile][mapping].update({"matrix": frame})
        self._profiles[profile][mapping]["is_using_matrix"] = True

        self.write_config_file(self._config_file)
