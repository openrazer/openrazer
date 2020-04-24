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

from openrazer_daemon.keyboard import EVENT_MAPPING

class KeyBindingManager(object):
    """
    Key binding manager

    """

    def __init__(self, device_id, parent, fake_device, testing=False):

        self._device_id = device_id
        self._logger = logging.getLogger('razer.device{0}.keymanager'.format(device_id))
        self._parent = parent
        self._parent._parent.register_observer(self)
        self._testing = testing
        self._fake_device = fake_device

        self._profiles = {0:DEFAULT_PROFILE}
        self._current_profile = self._profiles[0]
        self._current_map = self._current_profile[0]
        self._current_binding = self._current_map["binding"]

    def key_press(self, key_code):
        """
        Check for a binding

        :param key_code: The key code of the pressed key.
        :type key_code: int
        """

        binding = self._current_binding
        current_key = binding[key_code]
        for sequence in current_key:
            operation_type = sequence["type"]

            if operation_type == "key":
                self.press_key(sequence["code"])
            if operation_type == "sleep":
                time.sleep(operation_type["time"])

    def press_key(self, key_code):
        """
        do some evdev stuff to press key

        :param key_code: The key code of the key we want to send to the system
        :type key_code: int
        """



DEFAULT_PROFILE = {
    "name": "Default",
    0: {
        "name": "Default",
        "matrix": {
             0: {
                 0: (255, 0, 255)
             }
        },
        "binding" : {
            1: {
                0: {
                    "type": "key",
                    "code": 1
                }
            }
        }
    }
}