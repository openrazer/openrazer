"""
Contains the functions and classes to perform ripple effects
"""
import datetime
import logging
import math
import threading
import multiprocessing
import time
import os
import selectors
from selectors import DefaultSelector, EVENT_READ



import asyncio
import evdev
import evdev.ecodes

# pylint: disable=import-error
from openrazer_daemon.keyboard import KeyboardColour, EVENT_MAPPING, KEY_MAPPING
import numpy as _np
import numpy.ma as _np_ma
import random

import collections
import colorsys
import math


COLOUR_CHOICES = (
    (255, 0, 0),    # Red
    (0, 255, 0),    # Green
    (0, 0, 255),    # Blue
    (255, 255, 0),  # Yellow
    (0, 255, 255),  # Cyan
    (255, 0, 255),  # Magenta
)


class Frame(object):
    """
    Class to represent the RGB matrix of the keyboard. So to animate you'd use multiple frames
    """
    def __init__(self, dimensions):
        self._rows, self._cols = dimensions
        self._components = 3

        self._matrix = None
        self._fb1 = None
        self.reset()

    # Index with row, col OR y, x
    def __getitem__(self, key:tuple) -> tuple:
        """
        Method to allow a slice to get an RGB tuple

        :param key: Key, must be y,x tuple
        :type key: tuple

        :return: RGB tuple
        :rtype: tuple

        :raises AssertionError: If key is invalid
        """
        assert isinstance(key, tuple), "Key is not a tuple"
        assert 0 <= key[0] < self._rows, "Row out of bounds"
        assert 0 <= key[1] < self._cols, "Column out of bounds"

        return tuple(self._matrix[:, key[0], key[1]])

    # Index with row, col OR y, x
    def __setitem__(self, key:tuple, rgb:tuple):
        """
        Method to allow a slice to set an RGB tuple

        :param key: Key, must be y,x tuple
        :type key: tuple

        :param rgb: RGB tuple
        :type rgb: tuple

        :raises AssertionError: If key is invalid
        """
        assert isinstance(key, tuple), "Key is not a tuple"
        assert 0 <= key[0] < self._rows, "Row out of bounds"
        assert 0 <= key[1] < self._cols, "Column out of bounds"
        assert isinstance(rgb, (list, tuple)) and len(rgb) == 3, "Value must be a tuple,list of 3 RGB components"

        self._matrix[:, key[0], key[1]] = rgb

    def __bytes__(self) -> bytes:
        """
        When bytes() is ran on the class will return a binary capable of being sent to the driver

        :return: Driver binary payload
        :rtype: bytes
        """
        return b''.join([self.row_binary(row_id) for row_id in range(0, self._rows)])

    def reset(self):
        """
        Init/Clear the matrix
        """
        if self._matrix is None:
            self._matrix = _np_ma.zeros((self._components, self._rows, self._cols), 'uint8')
            self._matrix.mask = False
            self._fb1 = _np.copy(self._matrix)
        else:
            self._matrix.fill(0)

    def set(self, y: int, x: int, rgb: tuple):
        """
        Method to allow a slice to set an RGB tuple

        :param y: Row
        :type y: int

        :param x: Column
        :type x: int

        :param rgb: RGB tuple
        :type rgb: tuple

        :raises AssertionError: If key is invalid
        """
        self.__setitem__((y, x), rgb)

    def get(self, y: int, x: int) -> list:
        """
        Method to allow a slice to get an RGB tuple

        :param y: Row
        :type y: int

        :param x: Column
        :type x: int

        :return rgb: RGB tuple
        :return rgb: tuple

        :raises AssertionError: If key is invalid
        """
        return self.__getitem__((y, x))

    def row_binary(self, row_id: int) -> bytes:
        """
        Get binary payload for 1 row which is compatible with the driver

        :param row_id: Row ID
        :type row_id: int

        :return: Binary payload
        :rtype: bytes
        """
        assert 0 <= row_id < self._rows, "Row out of bounds"

        start = 0
        end = self._cols - 1

        return row_id.to_bytes(1, byteorder='big') + start.to_bytes(1, byteorder='big') + end.to_bytes(1, byteorder='big') + self._matrix[:,row_id].tobytes(fill_value=0, order='F')

    def to_binary(self):
        """
        Get the whole binary for the keyboard to be sent to the driver.

        :return: Driver binary payload
        :rtype: bytes
        """
        return bytes(self)

    def apply_unmasked(self, array):
        indicies = _np.where(array.raw.mask == False)
        self._matrix[indicies] = array.raw[indicies]



    # Simple FB
    def to_framebuffer(self):
        self._fb1 = _np.copy(self._matrix)

    def to_framebuffer_or(self):
        self._fb1 = _np.bitwise_or(self._fb1, self._matrix)

    def draw_with_fb_or(self):
        self._matrix = _np.bitwise_or(self._fb1, self._matrix)
        return bytes(self)

    @property
    def rows(self):
        return self._rows

    @property
    def cols(self):
        return self._cols

    @property
    def raw(self):
        return self._matrix

    @property
    def mask(self):
        return self._matrix.mask


class BaseEffect(multiprocessing.Process):
    """
    Base effect

    EFFECT_NAME: This is the name of the effect
    EFFECT_METHOD_NAME: Name of DBus method
    ARG_SPEC: This dictionary will be expected arguments: dbus_type - https://dbus.freedesktop.org/doc/dbus-specification.html#basic-types
      The arguments will be stored as a dict in self.args, if none are specified it'll be None
    INTERVAL: Loop interval. If None then looping will not be enabled and things must rely on key events.
    NEED_KEYS: Boolean to enable key thread basically
    WANT_KEY_DOWN_EVENT: Boolean to denote that key down events are wanted
    WANT_KEY_UP_EVENT: Boolean to denota that key up events are wanted
    WANT_KEY_AUTOREPEAT_EVENT: Boolean to denota that autorepeat events are wanted
    WRITE_EFFECT: Boolean for writing effect to device, not useful yet but will be when layering comes in
    """
    # ARG_SPEC = {'red': 'b', 'green': 'b', 'blue: 'b'}
    EFFECT_NAME = 'Unknown'
    EFFECT_METHOD_NAME = 'NotImplementedEffect'
    ARG_SPEC = {}

    INTERVAL = None
    NEED_KEYS = True
    WANT_KEY_DOWN_EVENT = True
    WANT_KEY_UP_EVENT = False
    WANT_KEY_AUTOREPEAT_EVENT = False

    WRITE_EFFECT = True

    @staticmethod
    def match(vid, pid, device_type):
        """
        Filter out what devices this effect can run on

        :param vid: Device USB VID
        :type vid: int

        :param pid: Device USB PID
        :type pid: int

        :param device_type: Device Type
        :type device_type: str

        :return: True if effect is allowed
        :rtype: bool
        """
        return True

    def __init__(self, device_number, dimensions, device_base, event_files, args=None):
        super(BaseEffect, self).__init__()

        self._logger = logging.getLogger('razer.device{0}.ripplethread'.format(device_number))
        self._event_files = event_files

        self._effect = None

        # (Rows, Cols)
        self._dims = dimensions

        self._rgb_file = os.path.join(device_base, 'matrix_custom_frame')
        self._custom_file = os.path.join(device_base, 'matrix_effect_custom')

        self.args = args

        self.matrix = Frame(self._dims)

    def _key_event_callback(self):
        """
        Uses the high-level selectors library to basically run select() on the device's event files.

        Uses WANT_KEY_DOWN_EVENT, WANT_KEY_UP_EVENT, WANT_KEY_AUTOREPEAT_EVENT to determine what events are requried and if said events are needed then it will
        call _on_key_event() which should do what it needs with the key event, be it store it in an array or ring buffer, or just run an algorithm to generate an
        effect based on key input.

        This function is ran as a thread so be wary.
        """
        selector = selectors.DefaultSelector()

        for device_path in self._event_files:
            dev = evdev.InputDevice(device_path)
            selector.register(dev, selectors.EVENT_READ)

        while True:
            for key, mask in selector.select():
                for event in key.fileobj.read():
                    if event.type == evdev.ecodes.EV_KEY:
                        if self.WANT_KEY_DOWN_EVENT and event.value == 1:
                            self._on_key_event(event.timestamp(), 'press', event.code)

                        elif self.WANT_KEY_UP_EVENT and event.value == 0:
                            self._on_key_event(event.timestamp(), 'release', event.code)

                        elif self.WANT_KEY_AUTOREPEAT_EVENT and event.value == 2:
                            self._on_key_event(event.timestamp(), 'autorepeat', event.code)

    def _on_key_event(self, key_time, key_action, key_code):
        """
        This is called whenever a key event is received and is wanted. Its called from the key watching thread which may OR may not be the only thread.

        Main idea of this function is to either just action an effect based on key input, or store key events into arrays/ring buffers. Be careful when
        modifying objects that other threads can use. Employ locking or code in a way where corruption wont happen.
        :param key_time: Unix timestamp with millisecond accuract of the key event
        :type key_time: float

        :param key_action: Will be press, release or autorepeat
        :type key_action: str

        :param key_code: Key code
        :type key_code: int
        """
        pass

    def _run(self, matrix):
        """
        Loop function

        :param matrix: Matrix object
        :type matrix: Frame
        """
        pass

    def run(self):
        """
        Main thread
        """

        keys_thread = threading.Thread(target=self._key_event_callback)

        need_keys = self.NEED_KEYS if self._effect is None else self._effect.NEED_KEYS
        interval = self.INTERVAL if self._effect is None else self._effect.INTERVAL
        write_effect = self.WRITE_EFFECT if self._effect is None else self._effect.WRITE_EFFECT

        # Key thread
        if need_keys:
            keys_thread.start()

        # If interval is set then infinite-loop
        if interval is not None and isinstance(interval, (float, int)):
            rgb_file = open(self._rgb_file, 'wb', 0)
            custom_file = open(self._custom_file, 'wb', 0)

            while True:
                self._run(self.matrix)

                if write_effect:
                    rgb_file.write(self.matrix.to_binary())
                    custom_file.write(b'1')

                time.sleep(interval)

        # If no loop and keys are running then just wait on that
        elif need_keys:
            keys_thread.join()

        # Nothing valid so rage and quit
        else:
            self._logger.warning("Key thread is not running and no interval is set. Exiting.")


class LayeredEffect(BaseEffect):
    EFFECT_NAME = 'Spectrum Effect'
    EFFECT_METHOD_NAME = 'spectrumEffect'
    INTERVAL = 0.05
    NEED_KEYS = False

    def __init__(self, *args, effects, **kwargs):
        super(LayeredEffect, self).__init__(*args, **kwargs)

        self.effects = []
        for effect in effects:
            self.effects.append((Frame(self._dims), effect))

    def run(self):
        """
        Main thread
        """

        keys_thread = threading.Thread(target=self._key_event_callback)

        need_keys = self.NEED_KEYS
        interval = self.INTERVAL
        write_effect = self.WRITE_EFFECT

        # Key thread
        if need_keys:
            keys_thread.start()

        # If interval is set then infinite-loop
        if interval is not None and isinstance(interval, (float, int)):
            rgb_file = open(self._rgb_file, 'wb', 0)
            custom_file = open(self._custom_file, 'wb', 0)

            while True:
                self.matrix.reset()

                for effect_matrix, effect in self.effects:
                    effect._run(effect_matrix)
                    self.matrix.apply_unmasked(effect_matrix)

                if write_effect:
                    rgb_file.write(self.matrix.to_binary())
                    custom_file.write(b'1')

                time.sleep(interval)

        # If no loop and keys are running then just wait on that
        elif need_keys:
            keys_thread.join()

        # Nothing valid so rage and quit
        else:
            self._logger.warning("Key thread is not running and no interval is set. Exiting.")


class MaskedEffect(BaseEffect):
    EFFECT_NAME = 'Spectrum Effect'
    EFFECT_METHOD_NAME = 'spectrumEffect'
    INTERVAL = 0.05
    NEED_KEYS = False

    def __init__(self, *args, effect_cls, mask=None, invert_mask=False, **kwargs):
        super(MaskedEffect, self).__init__(*args, **kwargs)

        self._effect = effect_cls(*args, **kwargs)

        self.mask(mask)
        if invert_mask:
            self.matrix.raw.mask = ~self.matrix.raw.mask

        self._mask = _np.copy(self.matrix.mask)  # Tried matrix.raw.harden_mask() but then writing to it dies

    def mask(self, mask):
        if isinstance(mask, str):
            if mask == 'm_keys':
                self.mask_m_keys()

            elif mask == 'f_row':
                self.mask_f_row()

            elif mask == 'wsad':
                self.mask_wsad()

            elif mask == 'arrow':
                self.mask_arrow()

            elif mask == 'numpad':
                self.mask_numpad()

        elif isinstance(mask, (list, tuple)):
            for item in mask:
                if isinstance(item, str):
                    self.mask(item)

                elif isinstance(item, tuple):
                    self.matrix.mask[:, item[0], item[1]] = True

    def mask_numpad(self):
        for row in range(1, self.matrix.rows):
            for col in range(18, 22):
                self.matrix.mask[:, row, col] = True

    def mask_wsad(self):
        for row, col in ((2, 3), (3, 2), (3, 3), (3, 4)):
            self.matrix.mask[:, row, col] = True

    def mask_arrow(self):
        for row, col in ((5, 15), (5, 16), (5, 17), (4, 16)):
            self.matrix.mask[:, row, col] = True

    def mask_arrow_keys(self):
        pass

    def mask_f_row(self):
        for col in range(3, 15):
                self.matrix.mask[:, 0, col] = True

    def mask_m_keys(self):
        for row in range(0, self.matrix.rows):
            self.matrix.mask[:, row, 0] = True

    def _run(self, matrix):
        self._effect._run(matrix)
        matrix.raw.mask = self._mask


class WaveEffect(BaseEffect):
    EFFECT_NAME = 'Wave Effect'
    EFFECT_METHOD_NAME = 'waveEffect'
    INTERVAL = 0.05
    NEED_KEYS = False

    def __init__(self, *args, **kwargs):
        super(WaveEffect, self).__init__(*args, **kwargs)

        self.offset = 0

    def _run(self, matrix):
        """
        Loop function

        :param matrix: Matrix object
        :type matrix: Frame
        """

        for column in range(0, matrix.cols):
            value = math.fmod(self.offset + (time.time() + (500*(column/matrix.cols))), 360) / 360
            rgb = colorsys.hls_to_rgb(value, 0.5, 1.0)

            matrix.raw[0, :, column] = rgb[0] * 255
            matrix.raw[1, :, column] = rgb[1] * 255
            matrix.raw[2, :, column] = rgb[2] * 255

        # TODO fix for non keyboard, this reassignes the proper colour to the logo
        matrix[0, 20] = matrix[5, 11]

        self.offset += 20


class SpectrumEffect(BaseEffect):
    EFFECT_NAME = 'Spectrum Effect'
    EFFECT_METHOD_NAME = 'spectrumEffect'
    INTERVAL = 0.05
    NEED_KEYS = False

    def __init__(self, *args, **kwargs):
        super(SpectrumEffect, self).__init__(*args, **kwargs)

    def _run(self, matrix):
        """
        Loop function

        :param matrix: Matrix object
        :type matrix: Frame
        """

        value = math.fmod(time.time()*10, 360) / 360

        rgb = colorsys.hls_to_rgb(value, 0.5, 1.0)

        matrix.raw[0] = rgb[0] * 255
        matrix.raw[1] = rgb[1] * 255
        matrix.raw[2] = rgb[2] * 255


class ReactiveEffect(BaseEffect):
    EFFECT_NAME = 'Reactive Effect'
    EFFECT_METHOD_NAME = 'reactiveEffect'
    INTERVAL = 0.05
    NEED_KEYS = True


    @staticmethod
    def get_colour(current):
        """
        Get a colour that isnt the current

        :return: RGB Tuple
        :rtype: tuple[int]
        """
        result = random.choice(COLOUR_CHOICES)

        while result == current:
            result = random.choice(COLOUR_CHOICES)

        return result

    def __init__(self, *args, **kwargs):
        super(ReactiveEffect, self).__init__(*args, **kwargs)

        self.key_events = []

        self.colour = random.choice(COLOUR_CHOICES)

        self.expire = 2

    def _on_key_event(self, key_time, key_action, key_code):
        """
        This is called whenever a key event is received and is wanted. Its called from the key watching thread which may OR may not be the only thread.

        Main idea of this function is to either just action an effect based on key input, or store key events into arrays/ring buffers. Be careful when
        modifying objects that other threads can use. Employ locking or code in a way where corruption wont happen.
        :param key_time: Unix timestamp with millisecond accuract of the key event
        :type key_time: float

        :param key_action: Will be press, release or autorepeat
        :type key_action: str

        :param key_code: Key code
        :type key_code: int
        """
        try:
            row, col = KEY_MAPPING[EVENT_MAPPING[key_code]]
            self.colour = self.get_colour(self.colour)
            self.key_events.append((key_time, row, col, self.colour))
        except KeyError:
            pass


    def _run(self, matrix):
        """
        Loop function

        :param matrix: Matrix object
        :type matrix: Frame
        """
        # Credit to Sean Voisen for the equation - http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
        matrix.reset()
        now = time.time()
        expire = now - self.expire

        while len(self.key_events) > 0 and self.key_events[0][0] < expire:
            self.key_events.pop(0)

        for key_time, row, col, rgb in self.key_events:
            diff = (self.expire - (now - key_time)) / self.expire
            matrix[row, col] = (rgb[0] * diff, rgb[1] * diff, rgb[2] * diff)


        # matrix.raw[0] = self.colour[0] * value
        # matrix.raw[1] = self.colour[1] * value
        # matrix.raw[2] = self.colour[2] * value


class BreathingEffect(BaseEffect):
    EFFECT_NAME = 'Breathing Effect'
    EFFECT_METHOD_NAME = 'breathingEffect'
    INTERVAL = 0.05
    NEED_KEYS = False

    @staticmethod
    def get_colour(current):
        """
        Get a colour that isnt the current

        :return: RGB Tuple
        :rtype: tuple[int]
        """
        result = random.choice(COLOUR_CHOICES)

        while result == current:
            result = random.choice(COLOUR_CHOICES)

        return result

    def __init__(self, *args, **kwargs):
        super(BreathingEffect, self).__init__(*args, **kwargs)

        self.last_change = time.time()
        self.colour = random.choice(COLOUR_CHOICES)

    def _change_colour(self):
        """
        Change the colour if the time since the last change was greater than 0.5secs ago
        :return:
        """
        cur_time = time.time()
        if self.last_change < cur_time - 0.5:
            self.colour = self.get_colour(self.colour)
            self.last_change = cur_time

    def _run(self, matrix):
        """
        Loop function

        :param matrix: Matrix object
        :type matrix: Frame
        """
        # Credit to Sean Voisen for the equation - http://sean.voisen.org/blog/2011/10/breathing-led-with-arduino/
        value = ((_np.exp(_np.sin(time.time())) - 0.36787944) * 108.0)
        value /= 255

        matrix.raw[0] = self.colour[0] * value
        matrix.raw[1] = self.colour[1] * value
        matrix.raw[2] = self.colour[2] * value

        if value < 0.001:
            self._change_colour()


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
                                    radius = math.sqrt(math.pow(cirlce_centre_row-row, 2) + math.pow(circle_centre_col-col, 2))
                                    if rad >= radius >= rad-1:
                                        self._kerboard_grid.set_key_colour(0, 20, colour)
                                        break
                        else:
                            for cirlce_centre_row, circle_centre_col, rad, colour in radiuses:
                                radius = math.sqrt(math.pow(cirlce_centre_row-row, 2) + math.pow(circle_centre_col-col, 2))
                                if rad >= radius >= rad-1:
                                    self._kerboard_grid.set_key_colour(row, col, colour)
                                    break

                payload = self._kerboard_grid.get_total_binary()

                self._parent.set_rgb_matrix(payload)
                self._parent.refresh_keyboard()

            time.sleep(self._refresh_rate)


class RippleEffectProcess(multiprocessing.Process):
    def __init__(self, device_number, dimensions, device_base, event_files):
        super(RippleEffectProcess, self).__init__()

        #self.wait = multiprocessing.Event()

        self._logger = logging.getLogger('razer.device{0}.ripplethread'.format(device_number))

        self._event_files = event_files

        # (Rows, Cols)
        self._dims = dimensions

        self._rgb_file = os.path.join(device_base, 'matrix_custom_frame')
        self._custom_file = os.path.join(device_base, 'matrix_effect_custom')

        self.stop = False

        self.key_events = []


        # Key filtering
        self.press = True
        self.run_press_event = False
        self.release = False
        self.auto = False

    def _event_callback(self):
        selector = selectors.DefaultSelector()

        for device_path in self._event_files:
            dev = evdev.InputDevice(device_path)
            selector.register(dev, selectors.EVENT_READ)

        while True:
            for key, mask in selector.select():
                device = key.fileobj
                for event in device.read():

                    if event.type == evdev.ecodes.EV_KEY:
                        if self.press and event.value == 1:
                            self.key_events.append((event.timestamp(), 'press', event.code))
                        elif self.release and event.value == 0:
                            self.key_events.append((event.timestamp(), 'release', event.code))
                        elif self.auto and event.value == 2:
                            self.key_events.append((event.timestamp(), 'autorepeat', event.code))

    def _key_events_clean(self):
        while True:
            expire = time.time() - 2

            while len(self.key_events) > 0 and self.key_events[0][0] < expire:
                self.key_events.pop(0)

            time.sleep(1)

    def run(self):
        keys_thread = threading.Thread(target=self._event_callback)
        clean_thread = threading.Thread(target=self._key_events_clean)
        clean_thread.start()
        keys_thread.start()

        count = 0

        matrix = Frame(self._dims)

        rgb_file = open(self._rgb_file, 'wb', 0)
        custom_file = open(self._custom_file, 'wb', 0)

        while True:
            now = time.time()

            radiuses = []

            for event_time, action, key_code in self.key_events:

                try:
                    key_row, key_col = KEY_MAPPING[EVENT_MAPPING[key_code]]
                except KeyError:
                    print("unknown key {0}".format(key_code))
                    break

                now_diff = now - event_time

                # Current radius is based off a time metric
                colour = (255, 0, 0)
                radiuses.append((key_row, key_col, now_diff*12, colour))

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
                                    matrix[0, 20] = colour
                                    break
                    else:
                        for cirlce_centre_row, circle_centre_col, rad, colour in radiuses:
                            radius = math.sqrt(math.pow(cirlce_centre_row - row, 2) + math.pow(circle_centre_col - col, 2))
                            if rad >= radius >= rad - 1:
                                matrix[row, col] = colour
                                break

            binary = matrix.to_binary()
            rgb_file.write(binary)
            custom_file.write(b'1')
            matrix.reset()
            time.sleep(0.01)


class LightBlastEffectProcess(multiprocessing.Process):
    def __init__(self, device_number, dimensions, device_base, event_files):
        super(LightBlastEffectProcess, self).__init__()

        #self.wait = multiprocessing.Event()

        self._logger = logging.getLogger('razer.device{0}.ripplethread'.format(device_number))

        self._event_files = event_files

        # (Rows, Cols)
        self._dims = dimensions

        self._rgb_file = os.path.join(device_base, 'matrix_custom_frame')
        self._custom_file = os.path.join(device_base, 'matrix_effect_custom')

        self.stop = False

        self.key_events = []


        # Key filtering
        self.store_keys = False
        self.press = True
        self.release = False
        self.auto = True

        self.press_event = threading.Event()
        self.run_press_event = True
        self.event = None


    def _event_callback(self):
        selector = selectors.DefaultSelector()

        for device_path in self._event_files:
            dev = evdev.InputDevice(device_path)
            selector.register(dev, selectors.EVENT_READ)

        while True:
            for key, mask in selector.select():
                device = key.fileobj
                for event in device.read():

                    if event.type == evdev.ecodes.EV_KEY:
                        if self.press and event.value == 1:
                            if self.store_keys:
                                self.key_events.append((event.timestamp(), 'press', event.code))

                            if self.run_press_event:
                                self.event = (event.timestamp(), 'press', event.code)
                                self.press_event.set()

                        elif self.release and event.value == 0:
                            if self.store_keys:
                                self.key_events.append((event.timestamp(), 'release', event.code))
                        elif self.auto and event.value == 2:
                            if self.store_keys:
                                self.key_events.append((event.timestamp(), 'autorepeat', event.code))

                            if self.run_press_event:
                                self.event = (event.timestamp(), 'press', event.code)
                                self.press_event.set()

    def run(self):
        keys_thread = threading.Thread(target=self._event_callback)
        keys_thread.start()
        matrix = Frame(self._dims)

        rgb_file = open(self._rgb_file, 'wb', 0)
        custom_file = open(self._custom_file, 'wb', 0)

        ring_buffer = collections.deque(maxlen=20)

        while True:
            self.press_event.wait()

            try:
                key_row, key_col = KEY_MAPPING[EVENT_MAPPING[self.event[2]]]
            except KeyError:
                print("unknown key {0}".format(self.event.code))
                self.press_event.clear()
                continue

            ring_buffer.append((key_row, key_col))

            kdist = 0.0
            for col in range(0, 22):
                for row in range(0, 6):

                    dist = 0.0
                    knum = 0

                    for key_row, key_col in ring_buffer:
                        knum += 1  # ??????????????????????????????????????????????????????????????
                        dx = col - key_col
                        dy = row - key_row

                        if dx or dy:
                            kdist = 2.4 / _np.sqrt((dx**2) + (dy**2))
                        else:
                            kdist = 2.4

                        if kdist > 0.1:
                            dist += kdist

                    dist /= 14.0

                    colour = rgb_from_hue(dist, 0.3, 0.0)
                    matrix[row, col] = colour

            binary = matrix.to_binary()
            rgb_file.write(binary)
            custom_file.write(b'1')

            self.press_event.clear()


def clamp_u8(value):
    if value > 255:
        return 255
    if value < 0:
        return 0
    return value


def hue2rgb(p, q, t):
    tt = t
    if tt < 0.0:
        tt += 1.0
    if tt > 1.0:
        tt -= 1.0

    if tt < 1.0/6.0:
        return p+(q-p)*6.0*tt
    if tt < 1.0/2.0:
        return q
    if tt < 2.0/3.0:
        return q+(q-p)*((2.0/3.0)-tt)*6.0

    return p


def rgb_from_hue(percentage, start_hue, end_hue):
    hue = percentage * (end_hue - start_hue) + start_hue
    lightness = 0.5
    saturation = 1.0
    return hsl2rgb(hue, saturation, lightness)


def hsl2rgb(hue, saturation, lightness):
    if saturation == 0.0:
        red = green = blue = lightness * 255.0
    else:
        if lightness < 0.5:
            q = lightness * (1.0 + saturation)
        else:
            q = lightness + saturation - lightness * saturation

        p = 2.0 * lightness - q

        red = hue2rgb(p, q, hue+(1.0/3.0))*255.0
        green = hue2rgb(p, q, hue) * 255.0
        blue = hue2rgb(p, q, hue - (1.0 / 3.0)) * 255.0

    return int(clamp_u8(red)), int(clamp_u8(green)), int(clamp_u8(blue))





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


if __name__ == '__main__':
    #a = RippleEffectProcess(str(0), (6, 22), '/sys/bus/hid/drivers/razerkbd/0003:1532:0203.000A', ('/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-if01-event-kbd', '/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd'))
    #a = LightBlastEffectProcess(str(0), (6, 22), '/sys/bus/hid/drivers/razerkbd/0003:1532:0203.000A', ('/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-if01-event-kbd', '/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd'))

    # e1 = BreathingEffect(device_number=str(0),
    #                      dimensions=(6, 22),
    #                      device_base='/sys/bus/hid/drivers/razerkbd/0003:1532:0203.000A',
    #                      event_files=('/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-if01-event-kbd', '/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd'))
    # e2 = MaskedEffect(device_number=str(0),
    #                   dimensions=(6, 22),
    #                   device_base='/sys/bus/hid/drivers/razerkbd/0003:1532:0203.000A',
    #                   event_files=('/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-if01-event-kbd', '/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd'),
    #                   mask=('m_keys', 'f_row', 'wsad', 'arrow'),
    #                   invert_mask=True,
    #                   effect_cls=SpectrumEffect)
    # e3 = MaskedEffect(device_number=str(0),
    #                   dimensions=(6, 22),
    #                   device_base='/sys/bus/hid/drivers/razerkbd/0003:1532:0203.000A',
    #                   event_files=('/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-if01-event-kbd', '/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd'),
    #                   mask='numpad',
    #                   invert_mask=True,
    #                   effect_cls=WaveEffect)
    # a = LayeredEffect(device_number=str(0),
    #                   dimensions=(6, 22),
    #                   device_base='/sys/bus/hid/drivers/razerkbd/0003:1532:0203.000A',
    #                   event_files=('/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-if01-event-kbd', '/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd'),
    #                   effects=(e1, e2, e3))
    a = ReactiveEffect(device_number=str(0),
                   dimensions=(6, 22),
                   device_base='/sys/bus/hid/drivers/razerkbd/0003:1532:0203.000A',
                   event_files=('/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-if01-event-kbd', '/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd'))
    a.start()
    print("started")

    time.sleep(3000)

    a.terminate()
    time.sleep(1)
    print(a.is_alive())
