import threading
import json
import logging

# pylint: disable=import-error
from openrazer_daemon.keyboard import KeyboardColour
from openrazer_daemon.misc.utils import capitalize_first_char


def hex2rgb(hx):
    if len(hx) != 7:
        return (255, 0, 0)
    if hx[0] != '#':
        return (255, 0, 0)
    try:
        return (
            int(hx[1:3], 16), int(hx[3:5], 16), int(hx[5:7], 16)
        )
    except:
        return (255, 0, 0)


def transmute_keyboard_matrix_from_polychromatic(raw_matrix):
    # Iterate over the columns
    for column in raw_matrix.values():
        # Iterate over the rows
        for i, entry in column.items():
            # Convert hex to tuple of rgb
            column[i] = hex2rgb(entry)
    return raw_matrix


class ComplexEffectThread(threading.Thread):
    """
    Ripple thread.

    This thread contains the run loop which performs all the circle calculations and generating of the binary payload
    """

    # Abstract
    EFFECT_NAME = ""

    def __init__(self, parent, device_number):
        super().__init__()

        self._logger = logging.getLogger(f'razer.device{device_number}.{self.EFFECT_NAME}thread')
        self._parent = parent

        self._colour = (0, 255, 0)
        self._refresh_rate = 0.040

        self._shutdown = False
        self._active = False

        self._rows, self._cols = self._parent._parent.MATRIX_DIMS

        self._keyboard_grid = KeyboardColour(self._rows, self._cols)
        self._matrix_map = None

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

    def get_color_for_key(self, row, col):
        if self._matrix_map:
            try:
                return self._matrix_map[str(col)][str(row)]
            except:
                return self._colour
        else:
            return self._colour

    # Abstract
    def load_config_file(self, json_data):
        pass

    def enable(self, parameters):
        """
        Enable the ripple effect

        If the colour tuple contains None then it will set the ripple to random colours
        :param colour: Colour tuple like (0, 255, 255) or effect name
        :type colour: tuple

        :param refresh_rate: Refresh rate in seconds
        :type refresh_rate: float
        """
        print(f"Loading {parameters}")
        match parameters:
            case (speed, ):
                self._colour = None
                self._matrix_map = None
                self._refresh_rate = speed
            case (r, g, b, speed):
                self._colour = (r, g, b)
                self._matrix_map = None
                self._refresh_rate = speed
            case (r, g, b, config_file, speed):
                self._colour = (r, g, b)
                with open(config_file, 'r') as file_source:
                    self.load_config_file(json.load(file_source))
                self._refresh_rate = speed
        self._active = True

    def disable(self):
        """
        Disable the ripple effect
        """
        self._active = False

    def run(self):
        """
        Event loop (ABSTRACT)
        """
        pass


class ComplexEffectManager(object):
    """
    Class which manages the overall process of performing a complex (matrix) effect
    """

    def __init__(self, parent, device_number, complex_effect_thread_constructor):
        self._logger = logging.getLogger(f'razer.device{device_number}.{complex_effect_thread_constructor.EFFECT_NAME}manager')
        self._parent = parent
        self._parent.register_observer(self)

        self._is_closed = False
        self._effect_name = complex_effect_thread_constructor.EFFECT_NAME
        self._function = 'set' + capitalize_first_char(complex_effect_thread_constructor.EFFECT_NAME)

        self._thread = complex_effect_thread_constructor(self, device_number)
        self._thread.start()

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
        self._parent._set_key_row(payload)

    def refresh_keyboard(self):
        """
        Refresh the keyboard
        """
        self._parent._set_custom_effect()

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
            # FIXME: Race condition
            if msg[2] == self._function:
                # Get (red, green, blue) tuple (args 3:6), and refreshrate arg 6
                self._parent.key_manager.temp_key_store_state = True
                self._thread.enable(msg[3:])
            elif self._thread._active:
                # Effect other than ripple so stop
                self._thread.disable()
                self._parent.key_manager.temp_key_store_state = False

    def close(self):
        """
        Close the manager, stop ripple thread
        """
        if not self._is_closed:
            self._logger.debug("Closing Complex Effect Manager")
            self._is_closed = True

            self._thread.shutdown = True
            self._thread.join(timeout=2)
            if self._thread.is_alive():
                self._logger.error("Could not stop Complex Effect thread")

    def __del__(self):
        self.close()
