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
from evdev import UInput, ecodes

from openrazer_daemon.keyboard import KeyboardColour

class KeybindingManager(object):
    """
    Key binding manager

    """

    def __init__(self, device_id, parent, testing=False):

        self._device_id = device_id
        self._logger = logging.getLogger('razer.device{0}.bindingmanager'.format(device_id))
        self._parent = parent
#        self._parent.register_observer(self)
        self._testing = testing
        self._fake_device = UInput(name="{0} (mapped)".format(self._parent.getDeviceName))

        self._profiles = {0:DEFAULT_PROFILE}
        self._current_profile = self._profiles[0]
        self._current_mapping = []
        self._is_using_matrix = False

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
        self._logger.debug("Key action: {0}, {1}".format(key_code, key_press))

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
                        self.__key_down(action["code"])
                
                    elif action["type"] == "map":
                        self.current_mapping = action["value"]

                elif action["type"] == "key": # Key released
                        self.__key_up(action["code"])

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
        :type value: int
        """
        self._logger.debug("Change mapping to {0}".format(value))
        self._current_mapping = self._current_profile[value]

        if self._current_mapping["is_using_matrix"]:
            current_matrix  = self._current_mapping["matrix"]
            for row in current_matrix:
                for key in current_matrix[row]:
                    self._keyboard_grid.set_key_colour(row, key, current_matrix[row][key])

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

    def __key_up(self, key_code):
        self._current_keys.remove(key_code)
        self._fake_device.write(ecodes.EV_KEY, key_code, 0)    
        self._fake_device.syn()

    def __key_down(self, key_code):
        self._current_keys.append(key_code)
        self._fake_device.write(ecodes.EV_KEY, key_code, 1)    
        self._fake_device.syn()

DEFAULT_PROFILE = {
    "name": "Default",
    "default_map": 0,
    0: {
        "name": "Default",
        "is_using_matrix": True,
        "red_led": True,
        "green_led": False,
        "blue_led": False,
        "matrix": {
             1: {
                 1: (255, 0, 255)
             },
             2: {
                 1: (255, 0, 255)
             }
        },
        "binding": {
            2: {
                0: {
                    "type": "key",
                    "code": 2
                },
                1: {
                    "type": "key",
                    "code": 3
                }
            }
        }
    },
    1: {
        "name": "Default",
        "is_using_matrix": True,
        "red_led": True,
        "green_led": False,
        "blue_led": True,
        "matrix": {
             1: {
                 1: (255, 255, 0)
             },
             2: {
                 1: (255, 255, 0)
             }
        },
        "binding": {
            2: {
                0: {
                    "type": "key",
                    "code": 3
                },
                1: {
                    "type": "key",
                    "code": 2
                },
                2: {
                    "type": "map",
                    "value": 0
                }
            }
        }
    }
}