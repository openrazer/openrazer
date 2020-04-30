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
from collections import OrderedDict
from openrazer_daemon.keyboard import KeyboardColour

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__))) # TODO: figure out a better way to handle this
from evdev import UInput, ecodes # noqa: 402, isort:skip

class KeybindingManager(object):
    """
    Key binding manager

    """

    def __key_up(self, key_code):
        key_code = int(key_code)
        self._current_keys.remove(key_code)
        self._fake_device.write(ecodes.EV_KEY, key_code, 0)    
        self._fake_device.syn()

    def __key_down(self, key_code):
        key_code = int(key_code)
        self._current_keys.append(key_code)
        self._fake_device.write(ecodes.EV_KEY, key_code, 1)    
        self._fake_device.syn()

    def __init__(self, device_id, parent, testing=False):

        self._device_id = device_id
        self._logger = logging.getLogger('razer.device{0}.bindingmanager'.format(device_id))
        self._parent = parent
#        self._parent.register_observer(self)
        self._testing = testing
        self._fake_device = UInput(name="{0} (mapped)".format(self._parent.getDeviceName()))

        self._profiles = {0:DEFAULT_PROFILE}
        self._current_profile = self._profiles[0]
        self._current_mapping = {}

        self._current_keys = []

        self._rows, self._cols = self._parent.MATRIX_DIMS
        self._keyboard_grid = KeyboardColour(self._rows, self._cols)

        self.current_mapping = self._current_profile["default_map"]

    def key_press(self, key_code, key_press):
        """
        Check for a binding

        :param key_code: The key code of the pressed key.
        :type key_code: int
        """
        # self._logger.debug("Key action: {0}, {1}".format(key_code, key_press))

        key_code = str(key_code)

        current_binding = self.current_mapping["binding"]
        if key_press == 'release' and key_code not in current_binding: # Key released, but not bound
            self.__key_up(key_code)

        elif key_code not in current_binding: # Ordinary key pressed
            self.__key_down(key_code)

        else: # Key bound
            for action in current_binding[key_code]: 
                action = current_binding[key_code][action]
                if key_press != 'release': # Key pressed (or autorepeat)
                    if action["type"] == "key":
                        self.__key_down(action["value"])
                
                    elif action["type"] == "map":
                        self.current_mapping = action["value"]

                elif action["type"] == "key": # Key released
                        self.__key_up(action["value"])

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

        if self._current_mapping["is_using_matrix"]:
            current_matrix  = self._current_mapping["matrix"]
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
        self.current_mapping = self._current_profile["default_map"]

    def read_config_file(self, config_file, profile):
        """
        Reads the configuration file and sets the variables accordingly

        :param config_file: path to the configuration file
        :type config_file: str

        :param profile: The profile name to use
        :type profile: str
        """

        f = open(config_file, 'r')
        
        self._profiles = json.load(f)
        self._current_profile = self._profiles[profile]
        self.current_mapping = self._current_profile["default_map"]
        f.close()

    def close(self):
        try:
            self._fake_device.close()
        except RuntimeError as err:
            self._logger.exception("Error closing fake device", exc_info=err)

    def __del__(self):
        self.close()

    ### DBus Stuff
    def dbus_get_profiles(self):
        """
        Get list of profiles in JSON format

        Returns a JSON blob containing profiles by name
        {0: "Profile"}

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
        ('0': 'map') 

        :param profile: The profile number
        :type: int

        :return: JSON of maps
        :rtype: str
        """
        return_list = []
        for mapping in self._profiles[profile]:
            return_list.append(mapping)

        return json.dumps(return_list)

    def dbus_get_map(self, profile, map):
        """
        Get the full map  for the given map id

        :param profile: The profile number
        :type: int

        :param map: The map name
        :type: str

        :return: JSON of the map
        :rtype: str
        """
        return json.dumps(self._profiles[profile][map])

    def dbus_get_actions(self, profile, map, key_code):
        """
        Get a list of actions for a given key.

        Returns a JSON blob containing the actions for a given key
        {0:{"type": "key", "value": 2}}

        :param profile: The profile number
        :type: int

        :param map: The map number
        :type: str

        :param key_code: The key_code
        :type: str

        :return: JSON of actions
        :rtype: str
        """
        return_list = self._profiles[profile][map]["bindings"][key_code]

        return json.dumps(return_list)

    def dbus_add_map(self, profile, map_name):
        """
        Add a new map to the given profile.

        :param profile: The profile number
        :type: int

        :param map_name: The name of the new map
        :type: str
        """
        self._profiles[profile].update({map_name: {"is_using_matrix": False, "binding": {}}})

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
        if not self._profiles[profile][map]["bindings"][key_code]:
            self._profiles[profile][map]["bindings"].update({key_code: {}})

        key = self._profiles[profile][map]["bindings"][key_code]

        if action_id == None:
            action_id == len(key)

        key.update({action_id: {"type": action_type, "value": value}})

    def dbus_remove_action(self, profile, map, key_code, action_id):
        """
        Removes an action from the given key

        :param profile: The profile number
        :type: int

        :param map: The map name
        :type: str

        :param key_code: The key code
        :type: str

        :param action_id: The id of the action
        :type: str
        """
        binding = self._profiles[profile][map]["bindings"][key_code]
        binding.pop(action_id)
        
        actions = 0
        for action in binding: # Reorder the actions to avoid overwrites
            action = binding[action]
            binding.update({str(actions): action})
            actions += 1

        binding = dict(OrderedDict(sorted(binding.items(), key=lambda t: t[0]))) # Sort

    def dbus_clear_actions(self, profile, map, key_code):
        """
        Clear all actions from the given key

        :param profile: The profile number
        :type: int

        :param map: The map name
        :type: str

        :param key_code: The key code
        :type: str
        """
        self._profiles[profile][map]["bindings"].pop(key_code)

    def dbus_get_profile_leds(self, profile, map):
        """
        Gets the state of the profile LEDs for the given map

        :param profile: The profile number
        :type: int

        :param map: The map name
        :type: str

        :return: The state of the Red profile LED
        :type: bool

        :return: The state of the Green profile LED
        :type: bool

        :return: The state of the Blue profile LED
        :type: bool

        """
        map = self._profiles[profile][map]

        return map["red_led"], map["green_led"], map["blue_led"]

    def dbus_set_profile_leds(self, profile, map, red, green, blue):
        """
        Set the profile LED state for the given map

        :param profile: The profile number
        :type: int

        :param map: The map name
        :type: str

        :param red: The red LED state
        :type: bool

        :param green: The green LED state
        :type: bool

        :param blue: The blue LED state
        :type: bool
        """        
        self._profiles[profile][map].update({'red_led': red, 'green_led': green, 'blue_led': blue})

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

    def dbus_get_matrix(self, profile, map):
        """
        Get the matrix for the given map

        :param profile: The profile number
        :type: int

        :param map: The map name
        :type: str

        :return: JSON of the matrix
        :rtype: str
        """
        return json.dumps(self._profiles[profile][map]["matrix"])


DEFAULT_PROFILE = {'name': 'Default', 'default_map': 'Default', 'Default': {'is_using_matrix': True, 'red_led': True, 'green_led': False, 'blue_led': False, 'matrix': {'1': {'1': [255, 0, 0], '2': [255, 0, 0], '3': [255, 0, 0], '4': [255, 0, 0], '5': [255, 0, 0]}, '2': {'1': [0, 255, 0], '2': [0, 255, 0], '3': [0, 255, 0], '4': [0, 255, 0], '5': [0, 255, 0]}, '3': {'1': [0, 255, 0], '2': [0, 255, 0], '3': [0, 255, 0], '4': [0, 255, 0], '5': [0, 255, 0]}, '4': {'1': [0, 255, 0], '3': [0, 255, 0], '4': [0, 255, 0], '5': [0, 255, 0], '6': [0, 255, 0]}}, 'binding': {'41': {'0': {'type': 'key', 'value': 2}}, '2': {'0': {'type': 'key', 'value': 3}}, '3': {'0': {'type': 'key', 'value': 4}}, '4': {'0': {'type': 'key', 'value': 5}}, '5': {'0': {'type': 'key', 'value': 6}}, '15': {'0': {'type': 'key', 'value': 16}}, '16': {'0': {'type': 'key', 'value': 17}}, '17': {'0': {'type': 'key', 'value': 18}}, '18': {'0': {'type': 'key', 'value': 19}}, '19': {'0': {'type': 'key', 'value': 20}}, '58': {'0': {'type': 'key', 'value': 30}}, '30': {'0': {'type': 'key', 'value': 31}}, '31': {'0': {'type': 'key', 'value': 32}}, '32': {'0': {'type': 'key', 'value': 33}}, '33': {'0': {'type': 'key', 'value': 34}}, '42': {'0': {'type': 'key', 'value': 44}}, '44': {'0': {'type': 'key', 'value': 45}}, '45': {'0': {'type': 'key', 'value': 46}}, '46': {'0': {'type': 'key', 'value': 47}}, '47': {'0': {'type': 'key', 'value': 48}}, '29': {'0': {'type': 'key', 'value': 14}}}}}
