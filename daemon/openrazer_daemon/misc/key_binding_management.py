"""
Recives key events from the key event manager and does stuff
"""

import datetime
import json
import logging
import os
import random
import struct
import time
import sys
import subprocess
from collections import OrderedDict
from openrazer_daemon.keyboard import KeyboardColour

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))  # TODO: figure out a better way to handle this
from evdev import UInput, ecodes


class KeybindingManager(object):
    """
    Key binding manager
    """

    def __init__(self, device_id, parent, testing=False):

        self._device_id = device_id
        self._logger = logging.getLogger('razer.device{0}.bindingmanager'.format(device_id))
        self._parent = parent
        # self._parent.register_observer(self)
        self._testing = testing
        self._fake_device = UInput(name="{0} (mapped)".format(self._parent.getDeviceName()))

        self._profiles = {"0": DEFAULT_PROFILE}
        self._current_profile_id = None

        self._current_keys = []
        self._old_mapping_name = None
        self._current_mapping_name = None
        self._shift_modifier = None

        self._rows, self._cols = self._parent.MATRIX_DIMS
        self._keyboard_grid = KeyboardColour(self._rows, self._cols)

        self._macro_mode = False
        self._macro_key = None

        self._serial_number = self._parent.getSerial()
        self._config_file = self.get_config_file_name()
        if os.path.exists(self._config_file):
            self.read_config_file(self._config_file)
        self.current_profile = "0"

    #pylint: disable=no-member
    def __key_up(self, key_code):
        for _ in range(0, 5):
            try:
                key_code = int(key_code)
                self._current_keys.remove(key_code)
                self._fake_device.write(ecodes.EV_KEY, key_code, 0)
                self._fake_device.syn()
                return
            except ValueError:
                time.sleep(.005)

    #pylint: disable=no-member
    def __key_down(self, key_code):
        key_code = int(key_code)
        self._current_keys.append(key_code)
        self._fake_device.write(ecodes.EV_KEY, key_code, 1)
        self._fake_device.syn()

    def key_press(self, key_code, key_press):
        """
        Check for a binding

        :param key_code: The key code of the pressed key.
        :type key_code: int
        """
        # self._logger.debug("Key action: {0}, {1}".format(key_code, key_press))

        key_code = str(key_code)

        current_binding = self.current_mapping["binding"]
        if key_press == 'release' and key_code not in current_binding:  # Key released, but not bound
            if key_code == self._shift_modifier:  # Key is map shifted
                self.current_mapping = self._old_mapping_name
                self._shift_modifier = None
            else:
                self.__key_up(key_code)

        elif key_code not in current_binding:  # Ordinary key pressed
            self.__key_down(key_code)

        else:  # Key bound
            for action in current_binding[key_code]:
                if key_press != 'release':  # Key pressed (or autorepeat)
                    if action["type"] == "key":
                        self.__key_down(action["value"])

                    elif action["type"] == "map":
                        self.current_mapping = action["value"]

                    elif action["type"] == "shift":
                        self.current_mapping = action["value"]
                        self._shift_modifier = key_code  # Set map shift key

                    elif action["type"] == "execute":
                        subprocess.run(["/bin/sh", "-c", str(action["value"])])

                    elif action["type"] == "sleep":
                        time.sleep(int(action["value"]))

                    elif action["type"] == "release":
                        self.__key_up(action["value"])

                elif key_press == 'release':
                    if key_code not in (183, 184, 185, 186, 187):  # Macro key released, skip it
                        if action["type"] == "key":  # Key released
                            self.__key_up(action["value"])
                        elif action["type"] == "sleep":
                            time.sleep(int(action["value"]))

    @property
    def current_mapping(self):
        """

        Returns the current mapping

        :return: The current mapping
        :rtype: dict
        """
        return self._current_mapping

    @current_mapping.setter
    def current_mapping(self, value):
        """

        Sets the current mapping and changes the led matrix

        :param value: The new mapping
        :type value: str
        """
        self._logger.debug("Change mapping to {0}".format(value))
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

        capabilities = self._parent.METHODS
        if 'keypad_set_profile_led_red' in capabilities:
            self._parent.setRedLED(self.current_mapping["red_led"])
        if 'keypad_set_profile_led_green' in capabilities:
            self._parent.setGreenLED(self.current_mapping["green_led"])
        if 'keypad_set_profile_led_blue' in capabilities:
            self._parent.setBlueLED(self.current_mapping["blue_led"])

    @property
    def current_profile(self):
        """
        Returns the current profile

        :return: the current profile name
        :rtype: str
        """

        return self._current_profile["name"]

    @current_profile.setter
    def current_profile(self, value):
        """
        Sets the current profile

        :param value: The profile number
        :type: int
        """

        self._current_profile = self._profiles[value]
        self._current_profile_id = value
        self.current_mapping = self._current_profile["default_map"]

    @property
    def macro_mode(self):
        """
        Returns the state of macro mode

        :return: the macro mode state
        :rtype: bool
        """

        return self._macro_mode

    @macro_mode.setter
    def macro_mode(self, value):
        """
        Sets the state of macro mode

        :param value: The state of macro mode
        :type: bool
        """

        if value == True:
            self._macro_mode = True
            self._parent.setMacroEffect(0x01)
            self._parent.setMacroMode(True)

        elif value == False:
            self._macro_mode = False
            self.macro_key = None
            self._parent.setMacroMode(False)

    @property
    def macro_key(self):
        """
        Return the macro key being recorded to

        :return: the macro key
        :rtype: int
        """

        return self._macro_key

    @macro_key.setter
    def macro_key(self, value):
        """
        Set the macro key being recorded to

        :param value: The macro key
        :type: int
        """
        if value is not None:
            self._macro_key = value
            self._parent.setMacroEffect(0x00)
            self._parent.clearActions(self._current_profile_id, self._current_mapping_name, str(value))

        else:
            self._macro_key = None

    def read_config_file(self, config_file):
        """
        Reads the configuration file and sets the variables accordingly

        :param config_file: path to the configuration file
        :type config_file: str

        :param profile: The profile name to use
        :type profile: str
        """
        self._logger.info("Reading config file from %s", config_file)

        with open(config_file, 'r') as f:
            self._profiles = json.load(f)

    def write_config_file(self, config_file):
        """
        Writes the _profiles dict to the config file

        :param config_file: The path to the config file
        :type config_file: str
        """
        self._logger.debug("writing config file to %s", config_file)

        with open(config_file, 'w') as f:
            json.dump(self._profiles, f, indent=4)

    def get_config_file_name(self):
        """
        Gets the name of the config file
        (this currently uses a hardcoded value but I want to use
        the path to the config file currently used by the daemon)

        :return: path to config file
        :rtype: str
        """

        home = os.path.expanduser("~")
        config_path = os.path.join(home, ".config/openrazer/")
        return config_path + "keybinding_" + self._serial_number + ".json"

    def close(self):
        try:
            self._fake_device.close()
        except RuntimeError as err:
            self._logger.exception("Error closing fake device", exc_info=err)

    def __del__(self):
        self.close()

    # DBus Stuff
    def dbus_get_profiles(self):
        """
        Get list of profiles in JSON format

        Returns a JSON blob containing profiles by name
        ["Profile"]

        :return: JSON of profiles
        :rtype: str
        """
        return_list = []
        for profile in self._profiles:
            profile = self._profiles[profile]
            return_list.append(profile["name"])

        return json.dumps(return_list)

    def dbus_get_maps(self, profile):
        """
        Get a list of maps in JSON format.

        Returns a JSON blob containing the maps of the given profile by name
        ['map'] 

        :param profile: The profile number
        :type: int

        :return: JSON of maps
        :rtype: str
        """
        return_list = []
        for key, value in self._profiles[profile].items():
            return_list.append(key)

        return json.dumps(return_list)

    def dbus_add_action(self, profile, map, key_code, action_type, value, action_id=None):
        """
        Add a new action to the given key

        :param profile: The profile number
        :type: int

        :param map: The map name
        :type: str

        :param key_code: The key code
        :type: str

        :param action_type: The action type (i.e. "key")
        :type: str

        :param value: The value of the action (i.e. 2)
        :type: str

        :param action_id: The ID of the action to edit (if unset adds a new action)
        :type: str
        """
        key = self._profiles[profile][map]["binding"].setdefault(key_code, [])

        if action_id != None:
            key[action_id] = {"type": action_type, "value": value}

        else:
            key.append({"type": action_type, "value": value})

        self.write_config_file(self.get_config_file_name())

    def dbus_set_matrix(self, profile, map, frame):
        """
        Set the LED matrix for the given map

        :param profile: The profile number
        :type: int

        :param map: The map name
        :type: str

        :param frame: The frame as a dict
        :type: dict
        """
        self._profiles[profile][map].update({"matrix": frame})
        self._profiles[profile][map]["is_using_matrix"] = True

        self.write_config_file(self.get_config_file_name())


DEFAULT_PROFILE = {
    "name": "Default",
    "default_map": "Default",
    "Default": {
        "is_using_matrix": False,
        "binding": {
        }
    }
}
