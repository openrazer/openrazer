"""
Contains the functions and classes to perform ripple effects
"""
import asyncio
import datetime
import logging
import math
import random

# pylint: disable=import-error
from razer_daemon.keyboard import KeyboardColour

COLOUR_CHOICES = (
    (255, 0, 0),    # Red
    (0, 255, 0),    # Green
    (0, 0, 255),    # Blue
    (255, 255, 0),  # Yellow
    (0, 255, 255),  # Cyan
    (255, 0, 255),  # Magenta
)


class RippleEffect():
    """
    Ripple task.

    This class contains the run loop which performs all the
    circle calculations and generating of the binary payload
    """
    def __init__(self, parent, device_number, height, width):
        self._logger = logging.getLogger('razer.device{0}.rippleeffect'.format(device_number))
        self._parent = parent
        self._height = height
        self._width = width

        self._colour = (0, 255, 0)
        self._refresh_rate = 0.100

        self._shutdown = False
        self._active = False

        self._keyboard_grid = KeyboardColour(width, height)
        self._interactive = asyncio.Event()

    async def interaction(self):
        self._interactive.set()

    @property
    def key_list(self):
        return self._parent.key_list

    def stop(self):
        self._active = False
        self._interactive.set()

    def start(self, colour, refresh_rate):
        """
        Enable the ripple effect

        If the colour tuple contains None then it will set the ripple to random colours
        :param colour: Colour tuple like (0, 255, 255)
        :type colour: tuple

        :param refresh_rate: Refresh rate in seconds
        :type refresh_rate: float
        """
        if self._active:
            return

        if colour[0] is None:
            self._colour = None
        else:
            self._colour = colour
        self._refresh_rate = refresh_rate

        self._active = True
        asyncio.ensure_future(self._start())

    async def _start(self):
        while self._active:
            await self._interactive.wait()

            # Recheck
            if not self._active:
                break

            expire_diff = datetime.timedelta(seconds=2)

            # TODO time execution and then sleep for _refresh_rate - time_taken
            while len(self.key_list) > 0:
                # Clear keyboard
                self._keyboard_grid.reset_rows()

                now = datetime.datetime.now()

                radiuses = []

                for expire_time, (key_row, key_col), colour in self.key_list:
                    event_time = expire_time - expire_diff

                    now_diff = now - event_time

                    # Current radius is based off a time metric
                    if self._colour is not None:
                        colour = self._colour
                    radiuses.append((key_row, key_col, now_diff.total_seconds() * 12, colour))

                for row in range(0, self._height):
                    for col in range(0, self._width):
                        if row == 0 and col == 20:
                            continue
                        if row == 6:
                            if col != 11:
                                continue
                            else:
                                # To account for logo placement
                                for circle_centre_row, circle_centre_col, rad, colour in radiuses:
                                    radius = math.sqrt(math.pow(circle_centre_row-row, 2) + math.pow(circle_centre_col-col, 2))
                                    if rad >= radius >= rad-1:
                                        self._keyboard_grid.set_key_colour(0, 20, colour)
                                        break
                        else:
                            for circle_centre_row, circle_centre_col, rad, colour in radiuses:
                                radius = math.sqrt(math.pow(circle_centre_row-row, 2) + math.pow(circle_centre_col-col, 2))
                                if rad >= radius >= rad-1:
                                    self._keyboard_grid.set_key_colour(row, col, colour)
                                    break

                payload = self._keyboard_grid.get_total_binary()

                self._parent.set_rgb_matrix(payload)
                self._parent.refresh_keyboard()

                await asyncio.sleep(self._refresh_rate)
            self._interactive.clear()

class RippleManager(object):
    """
    Class which manages the overall process of performing a ripple effect
    """
    def __init__(self, parent, device_number):
        self._logger = logging.getLogger('razer.device{0}.ripplemanager'.format(device_number))
        self._parent = parent
        self._parent.register_observer(self)
        self._key_expire_time = datetime.timedelta(seconds=2)
        self._last_colour_choice = None
        self._is_closed = False

        dims = self._parent.MATRIX_DIMS

        self._ripple_effect = RippleEffect(self, device_number, dims[0], dims[1])

        self._key_list = []

    def random_colour_picker(self, iterable):
        """
        Chose a random choice but not the last one

        :param last_choice: Last choice
        :type last_choice: object

        :param iterable: Iterable object
        :type iterable: iterable

        :return: Choice
        :rtype: object
        """
        result = random.choice(iterable)
        while result == self._last_colour_choice:
            result = random.choice(iterable)
        return result

    async def key_action(self, event_time, key_id, key_press='press'):
        now = datetime.datetime.now()

        self._expire_keys(now)

        if key_press is not 'press':
            return

        key_name = self.event_mapping[key_id]
        colour = self.random_colour_picker(COLOUR_CHOICES)
        self._last_colour_choice = colour
        self._key_list.append((now + self._key_expire_time, self.key_mapping[key_name], colour))

        await self._ripple_effect.interaction()

    def _expire_keys(self, time=None):
        """
        Get the temporary key store

        :return: List of keys
        :rtype: list
        """
        if time is None:
            time = datetime.datetime.now()

        # Remove expired keys from store
        try:
            # Get date and if its less than now its expired
            while self._key_list[0][0] < time:
                self._key_list.pop(0)
        except IndexError:
            pass

    @property
    def key_list(self):
        self._expire_keys()
        return self._key_list[:]

    @property
    def key_mapping(self):
        result = []
        if hasattr(self._parent.key_manager, 'key_mapping'):
            result = self._parent.key_manager.key_mapping

        return result

    @property
    def event_mapping(self):
        result = []
        if hasattr(self._parent.key_manager, 'event_mapping'):
            result = self._parent.key_manager.event_mapping

        return result

    def set_rgb_matrix(self, payload):
        """
        Set the LED matrix on the keyboard

        :param payload: Binary payload
        :type payload: bytes
        """
        self._parent.setKeyRow(payload)

    def refresh_keyboard(self):
        """
        Refresh the keyboard
        """
        self._parent.setCustom()

    def notify(self, msg):
        """
        Receive notificatons from the device (we only care about effects)

        :param msg: Notification
        :type msg: tuple
        """
        if not isinstance(msg, tuple):
            self._logger.warning("Got msg that was not a tuple")
        elif msg[0] == 'effect':
            # We have a message directed at us
            # MSG format
            #  0         1       2             3
            # ('effect', Device, 'effectName', 'effectparams'...)
            # Device is the device the msg originated from (could be parent device)
            if msg[2] == 'setRipple':
                # Get (red, green, blue) tuple (args 3:6), and refreshrate arg 6
                self._ripple_effect.start(msg[3:6], msg[6])
                self._parent.key_manager.add_key_action(self.key_action)

            else:
                # Effect other than ripple so stop
                self._parent.key_manager.remove_key_action(self.key_action)
                self._ripple_effect.stop()

    def close(self):
        """
        Close the manager, stop ripple effect
        """
        if not self._is_closed:
            self._ripple_effect.stop()
            self._logger.debug("Closing Ripple Manager")
            self._is_closed = True

    def __del__(self):
        self.close()
