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

from openrazer_daemon.dbus_services.dbus_methods import set_custom_effect, set_key_row, keypad_set_profile_led_blue, keypad_set_profile_led_green, keypad_set_profile_led_red

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
        self._current_mapping = self._current_profile[0]
        self._is_using_matrix = self._current_mapping["is_using_matrix"]

        self._current_keys = []

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
                if action["type"] == "key":
                    self._current_keys.append(action["code"])
                    self._fake_device.write(ecodes.EV_KEY, action["code"], 1)
                    self._fake_device.syn()
                
                elif action["type"] == "map":
                    self.current_mapping(action["value"])

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
        Warning: this requires a full matrix

        :param value: The new mapping
        :type value: int
        """

        self._current_mapping = self._current_profile[value]
        current_matrix  = self._current_mapping["matrix"]
        matrix_length = self._device.MATRIX_DIMS = [1]

        if self._current_mapping["is_using_matrix"]:
            for row in current_matrix:
                if row.len() == matrix_length:
                    array = [current_matrix.keys()[row]]
                
                    for key in current_matrix[row]:
                        array.append(key[0], key[1], key[2])

                    set_key_row(self._device, array)
                else:
                    self._logger.error("Invalid row length, expected {0} but got {1}. Ignoring row.".format(matrix_length, row.len()))

            set_custom_effect(self._device)
        
        capabilities = self._device.METHODS
        if capabilities['keypad_set_profile_led_red']:
            keypad_set_profile_led_red(self._device, self.current_mapping["red_led"])
        if capabilities['keypad_set_profile_led_green']:
            keypad_set_profile_led_red(self._device, self.current_mapping["blue_led"])
        if capabilities['keypad_set_profile_led_blue']:
            keypad_set_profile_led_red(self._device, self.current_mapping["blue_led"])

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
        "is_using_matrix": False,
        "red_led": True,
        "green_led": False,
        "blue_led": False,
        "matrix": {
             0: {
                 0: (255, 0, 255)
             }
        },
        "binding": {
            1: {
                0: {
                    "type": "key",
                    "code": 1
                }
            }
        }
    }
}