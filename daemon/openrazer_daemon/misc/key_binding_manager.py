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



    def key_press(self, key_code):
        """
        Check for a binding

        :param key_code: The key code of the pressed key.
        :type key_code: int
        """
        