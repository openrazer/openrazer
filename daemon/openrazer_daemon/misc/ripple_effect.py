
"""
Contains the functions and classes to perform ripple effects
"""
import datetime
import logging
import math
import threading
import time

# pylint: disable=import-error
from openrazer_daemon.keyboard import KeyboardColour
from openrazer_daemon.misc.complex_effect import ComplexEffectThread, ComplexEffectManager, transmute_keyboard_matrix_from_polychromatic


class RippleEffectThread(ComplexEffectThread):
    """
    Ripple thread.

    This thread contains the run loop which performs all the circle calculations and generating of the binary payload
    """

    EFFECT_NAME = "ripple"

    def __init__(self, parent, device):
        super().__init__(parent, device)
        self._mode = 1

    def load_config_file(self, json_data):
        self._matrix_map = transmute_keyboard_matrix_from_polychromatic(json_data['matrix'])
        try:
            self._mode = json_data['mode']
        except:
            self._mode = 1


    def run(self):
        """
        Event loop
        """
        # pylint: disable=too-many-nested-blocks,too-many-branches
        expire_diff = datetime.timedelta(seconds=2)

        # self._parent: RippleManager
        # self._parent._parent: The device class (e.g. RazerBlackWidowUltimate2013)
        if self._rows == 6 and self._cols == 22:
            needslogohandling = True
            # a virtual 7th row for logo handling
            self._rows += 1
        else:
            needslogohandling = False

        # TODO time execution and then sleep for _refresh_rate - time_taken
        while not self._shutdown:
            if self._active:
                # Clear keyboard
                self._keyboard_grid.reset_rows()

                now = datetime.datetime.now()

                radiuses = []

                for expire_time, (key_row, key_col), colour in self.key_list:
                    event_time = expire_time - expire_diff

                    now_diff = now - event_time

                    # Current radius is based off a time metric
                    if self._colour is not None:
                        colour = self.get_color_for_key(key_row, key_col)
                    radiuses.append((key_row, key_col, now_diff.total_seconds() * 24, colour))

                # Iterate through the rows
                for row in range(0, self._rows):
                    # Iterate through the columns
                    for col in range(0, self._cols):
                        # The logo location is physically at (6, 11), logically at (0, 20)
                        # Skip when we come across the logo location, as the ripple would look wrong
                        if needslogohandling and row == 0 and col == 20:
                            continue

                        if needslogohandling and row == 6:
                            if col != 11:
                                continue

                            # To account for logo placement
                            for cirlce_centre_row, circle_centre_col, rad, colour in radiuses:
                                radius = math.sqrt(math.pow(cirlce_centre_row - row, 2) + math.pow(circle_centre_col - col, 2))
                                if rad >= radius >= rad - 2:
                                    # Again, (0, 20) is the logical location of the logo led
                                    self._keyboard_grid.set_key_colour(0, 20, colour)
                                    break
                        else:
                            for cirlce_centre_row, circle_centre_col, rad, colour in radiuses:
                                radius = math.sqrt(math.pow(cirlce_centre_row - row, 2) + math.pow(circle_centre_col - col, 2))
                                if rad >= radius >= rad - 2:
                                    if self._mode == 2:
                                        colour = self.get_color_for_key(row, col)
                                    self._keyboard_grid.set_key_colour(row, col, colour)
                                    break

                # Set the colors on the device
                payload = self._keyboard_grid.get_total_binary()

                self._parent.set_rgb_matrix(payload)
                self._parent.refresh_keyboard()

            # Sleep until the next ripple refresh
            time.sleep(self._refresh_rate)

