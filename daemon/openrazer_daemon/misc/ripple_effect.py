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


class RippleEffectThread(threading.Thread):
    """
    Ripple thread.

    This thread contains the run loop which performs all the circle calculations and generating of the binary payload
    """

    def __init__(self, parent, device_number):
        super(RippleEffectThread, self).__init__()

        self._logger = logging.getLogger('razer.device{0}.ripplethread'.format(device_number))
        self._parent = parent

        self._colour = (0, 255, 0)
        self._refresh_rate = 0.100

        self._shutdown = False
        self._active = False

        self._kerboard_grid = KeyboardColour()

    @property
    def shutdown(self):
        """
        Get the shutdown flag
        """
        return self._shutdown

    @shutdown.setter
    def shutdown(self, value):
        """
        Set the shutdown flag

        :param value: Shutdown
        :type value: bool
        """
        self._shutdown = value

    @property
    def active(self):
        """
        Get if the thread is active

        :return: Active
        :rtype: bool
        """
        return self._active

    @property
    def key_list(self):
        """
        Get key list

        :return: Key list
        :rtype: list
        """
        return self._parent.key_list

    def enable(self, colour, refresh_rate):
        """
        Enable the ripple effect

        If the colour tuple contains None then it will set the ripple to random colours
        :param colour: Colour tuple like (0, 255, 255)
        :type colour: tuple

        :param refresh_rate: Refresh rate in seconds
        :type refresh_rate: float
        """
        if colour[0] is None:
            self._colour = None
        else:
            self._colour = colour
        self._refresh_rate = refresh_rate
        self._active = True

    def disable(self):
        """
        Disable the ripple effect
        """
        self._active = False

    def run(self):
        """
        Event loop
        """
        # pylint: disable=too-many-nested-blocks,too-many-branches
        expire_diff = datetime.timedelta(seconds=2)

        # TODO time execution and then sleep for _refresh_rate - time_taken
        while not self._shutdown:
            if self._active:
                # Clear keyboard
                self._kerboard_grid.reset_rows()

                now = datetime.datetime.now()

                radiuses = []

                for expire_time, (key_row, key_col), colour in self.key_list:
                    event_time = expire_time - expire_diff

                    now_diff = now - event_time

                    # Current radius is based off a time metric
                    if self._colour is not None:
                        colour = self._colour
                    radiuses.append((key_row, key_col, now_diff.total_seconds() * 12, colour))

                for row in range(0, 7):
                    for col in range(0, 22):
                        if row == 0 and col == 20:
                            continue
                        if row == 6:
                            if col != 11:
                                continue
                            else:
                                # To account for logo placement
                                for cirlce_centre_row, circle_centre_col, rad, colour in radiuses:
                                    radius = math.sqrt(math.pow(cirlce_centre_row - row, 2) + math.pow(circle_centre_col - col, 2))
                                    if rad >= radius >= rad - 1:
                                        self._kerboard_grid.set_key_colour(0, 20, colour)
                                        break
                        else:
                            for cirlce_centre_row, circle_centre_col, rad, colour in radiuses:
                                radius = math.sqrt(math.pow(cirlce_centre_row - row, 2) + math.pow(circle_centre_col - col, 2))
                                if rad >= radius >= rad - 1:
                                    self._kerboard_grid.set_key_colour(row, col, colour)
                                    break

                payload = self._kerboard_grid.get_total_binary()

                self._parent.set_rgb_matrix(payload)
                self._parent.refresh_keyboard()

            time.sleep(self._refresh_rate)


class RippleManager(object):
    """
    Class which manages the overall process of performing a ripple effect
    """

    def __init__(self, parent, device_number):
        self._logger = logging.getLogger('razer.device{0}.ripplemanager'.format(device_number))
        self._parent = parent
        self._parent.register_observer(self)

        self._is_closed = False

        self._ripple_thread = RippleEffectThread(self, device_number)
        self._ripple_thread.start()

    @property
    def key_list(self):
        """
        Get the list of keys from the key manager

        :return: List of tuples (expire_time, (key_row, key_col), random_colour)
        :rtype: list of tuple
        """
        result = []
        if hasattr(self._parent, 'key_manager'):
            result = self._parent.key_manager.temp_key_store

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
                self._parent.key_manager.temp_key_store_state = True
                self._ripple_thread.enable(msg[3:6], msg[6])
            else:
                # Effect other than ripple so stop
                self._ripple_thread.disable()

                self._parent.key_manager.temp_key_store_state = False

    def close(self):
        """
        Close the manager, stop ripple thread
        """
        if not self._is_closed:
            self._logger.debug("Closing Ripple Manager")
            self._is_closed = True

            self._ripple_thread.shutdown = True
            self._ripple_thread.join(timeout=2)
            if self._ripple_thread.is_alive():
                self._logger.error("Could not stop RippleEffect thread")

    def __del__(self):
        self.close()
