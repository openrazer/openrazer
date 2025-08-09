# SPDX-License-Identifier: GPL-2.0-or-later

"""
Contains the functions and classes to perform ripple effects
"""
import datetime
import logging
import math
import threading
import time

# pylint: disable=import-error
from openrazer_daemon.misc.complex_effect import ComplexEffectThread, ComplexEffectManager, transmute_keyboard_matrix_from_polychromatic

def scale_colour(colour, scale):
    return tuple(x * scale for x in colour)

class ReactiveEffectThread(ComplexEffectThread):
    EFFECT_NAME = "reactive2"

    def load_config_file(self, json_data):
        self._matrix_map = transmute_keyboard_matrix_from_polychromatic(json_data['matrix'])

    def run(self):
        """
        Event loop
        """
        # pylint: disable=too-many-nested-blocks,too-many-branches
        expire_diff = datetime.timedelta(seconds=2)

        # self._parent: RippleManager
        while not self._shutdown:
            if self._active:
                # Clear keyboard
                self._keyboard_grid.reset_rows()

                now = datetime.datetime.now()

                radiuses = []

                for expire_time, (key_row, key_col), colour in self.key_list:
                    event_time = expire_time - expire_diff

                    now_diff = now - event_time
                    colour_scale = (expire_diff - now_diff) / expire_diff

                    if self._colour is not None:
                        colour = self.get_color_for_key(key_row, key_col)
                    self._keyboard_grid.set_key_colour(key_row, key_col, scale_colour(colour, colour_scale))

                # Set the colors on the device
                payload = self._keyboard_grid.get_total_binary()

                self._parent.set_rgb_matrix(payload)
                self._parent.refresh_keyboard()

            # Sleep until the next ripple refresh
            time.sleep(self._refresh_rate)
