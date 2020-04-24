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


class KeyBindingManager(object):
    """
    Key binding manager

    """

    def __init__(self, device_id, parent, fake_device, testing=False):

        self._device_id = device_id
        self._logger = logging.getLogger('razer.device{0}.bindingmanager'.format(device_id))
        self._parent = parent
        self._parent._parent.register_observer(self)
        self._testing = testing
        self._fake_device = fake_device
        self._device = self._parent._parent

        self._profiles = {0:DEFAULT_PROFILE}
        self._current_profile = self._profiles[0]
        self._current_mapping = []
        self._is_using_matrix = False

        self._current_keys = []

        self._rows, self._cols = self._device.MATRIX_DIMS
        self._keyboard_grid = KeyboardColour(self._rows, self._cols)

        self.current_mapping = self._current_profile["default_map"]

    def key_press(self, key_code):
        """
        Check for a binding

        :param key_code: The key code of the pressed key.
        :type key_code: int
        """
        self._logger.debug("Key press: %s", key_code)

        current_binding = self.current_mapping["binding"]
        if key_code not in current_binding:
            self._current_keys.append(key_code)
            self._fake_device.write(ecodes.EV_KEY, key_code, 1)
            self._fake_device.syn()

        else:
            for action in current_binding[key_code]:
                action = current_binding[key_code][action]
                if action["type"] == "key":
                    self._current_keys.append(action["code"])
                    self._fake_device.write(ecodes.EV_KEY, action["code"], 1)
                    self._fake_device.syn()
                
                elif action["type"] == "map":
                    self.current_mapping = action["value"]

        for key in self._current_keys:
            self._fake_device.write(ecodes.EV_KEY, key, 0)
            
        self._fake_device.syn()
        self._current_keys = []

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
            self._device.setKeyRow(payload)
            self._device.setCustom()


        capabilities = self._device.METHODS
        if 'keypad_set_profile_led_red' in capabilities:
            self._device.setRedLED(self.current_mapping["red_led"])
        if 'keypad_set_profile_led_green' in capabilities:
            self._device.setGreenLED(self.current_mapping["green_led"])
        if 'keypad_set_profile_led_blue' in capabilities:
            self._device.setBlueLED(self.current_mapping["blue_led"])

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
                },
                2: {
                    "type": "map",
                    "value": 1
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