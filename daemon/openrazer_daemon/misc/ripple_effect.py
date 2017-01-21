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
from openrazer_daemon.dbus_services.service import DBusService
from openrazer_daemon.keyboard import KeyboardColour, EVENT_MAPPING, KEY_MAPPING
import numpy as _np
import numpy.ma as _np_ma
import random
import signal
import sys

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
    CLEAR_ON_LOOP = False

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

        self._logger = logging.getLogger('razer.device{0}.customEffect'.format(device_number))
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
        signal.signal(signal.SIGTERM, lambda signum, stack_frame: sys.exit(0))

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

                if self.CLEAR_ON_LOOP:
                    self.matrix.reset()

                time.sleep(interval)

        # If no loop and keys are running then just wait on that
        elif need_keys:
            keys_thread.join()

        # Nothing valid so rage and quit
        else:
            self._logger.warning("Key thread is not running and no interval is set. Exiting.")


class LayeredEffect(BaseEffect):
    EFFECT_NAME = 'Layered Effect'
    EFFECT_METHOD_NAME = 'layeredEffect'
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
    EFFECT_NAME = 'Masked Effect'
    EFFECT_METHOD_NAME = 'maskedEffect'
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


class RippleEffect(BaseEffect):
    EFFECT_NAME = 'Ripple Effect'
    EFFECT_METHOD_NAME = 'rippleEffect'
    INTERVAL = 0.05
    NEED_KEYS = True
    WANT_KEY_DOWN_EVENT = True

    CLEAR_ON_LOOP = True

    def __init__(self, *args, **kwargs):
        super(RippleEffect, self).__init__(*args, **kwargs)

        self.key_events = []
        self.expire_time = 2

    def _on_key_event(self, key_time, key_action, key_code):
        """
        Append key to list
        :param key_time: Unix timestamp with millisecond accuract of the key event
        :type key_time: float

        :param key_action: Will be press, release or autorepeat
        :type key_action: str

        :param key_code: Key code
        :type key_code: int
        """
        try:
            key_row, key_col = KEY_MAPPING[EVENT_MAPPING[key_code]]
            self.key_events.append((key_time, key_row, key_col))
            #self._logger.debug("Appending key to list")
        except KeyError:
            pass

    def _run(self, matrix):
        """
        Loop function

        :param matrix: Matrix object
        :type matrix: Frame
        """

        now = time.time()

        #self._logger.debug("1")

        expire = now - self.expire_time

        #self._logger.debug("2")

        while len(self.key_events) > 0 and self.key_events[0][0] < expire:
            self.key_events.pop(0)
            #self._logger.debug("2.1")

        #self._logger.debug("3")

        # List of radii
        radiuses = []

        #self._logger.debug("4")

        for event_time, key_row, key_col in self.key_events:
            #self._logger.debug("4.1")
            now_diff = now - event_time

            #self._logger.debug("4.2")

            # Current radius is based off a time metric
            colour = (0, 255, 0)
            radiuses.append((key_row, key_col, now_diff * 12, colour))

            #self._logger.debug("4.3")

        #self._logger.debug("5")
        # 1 extra row to do the logo led
        if len(self.key_events) > 0:
            for row in range(0, matrix.rows + 1):
                for col in range(0, matrix.cols):

                    #self._logger.debug("5.1")

                    if row == 0 and col == 20:
                        continue
                    if row == 6:
                        #self._logger.debug("5.2.1")

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
                        #self._logger.debug("5.2.1")

                        for cirlce_centre_row, circle_centre_col, rad, colour in radiuses:
                            radius = math.sqrt(math.pow(cirlce_centre_row - row, 2) + math.pow(circle_centre_col - col, 2))
                            if rad >= radius >= rad - 1:
                                matrix[row, col] = colour
                                break


class LightBlastEffectEffect(BaseEffect):
    EFFECT_NAME = 'Light Blast Effect'
    EFFECT_METHOD_NAME = 'lightblastEffect'
    INTERVAL = 0.01
    NEED_KEYS = True
    WANT_KEY_DOWN_EVENT = True
    WANT_KEY_AUTOREPEAT_EVENT = True

    CLEAR_ON_LOOP = False

    def __init__(self, *args, **kwargs):
        super(LightBlastEffectEffect, self).__init__(*args, **kwargs)

        self.ring_buffer = collections.deque(maxlen=20)
        self.key_event = multiprocessing.Event()

        self.expire_time = 2

    def _on_key_event(self, key_time, key_action, key_code):
        """
        Append key to list
        :param key_time: Unix timestamp with millisecond accuract of the key event
        :type key_time: float

        :param key_action: Will be press, release or autorepeat
        :type key_action: str

        :param key_code: Key code
        :type key_code: int
        """
        try:
            key_row, key_col = KEY_MAPPING[EVENT_MAPPING[key_code]]
            self.ring_buffer.append((key_row, key_col))
            self.key_event.set()
        except KeyError:
            pass

    def _run(self, matrix):
        if self.key_event.is_set():

            for col in range(0, matrix.cols):
                for row in range(0, matrix.rows):

                    dist = 0.0
                    knum = 0

                    for key_row, key_col in list(self.ring_buffer):
                        knum += 2  # ??????????????????????????????????????????????????????????????
                        dx = col - key_col
                        dy = row - key_row

                        if dx or dy:
                            kdist = 2.4 / _np.sqrt((dx ** 2) + (dy ** 2))
                        else:
                            kdist = 2.4

                        if kdist > 0.1:
                            dist += kdist

                    dist /= 17.0

                    colour = rgb_from_hue(dist, 0.3, 0.0)
                    matrix[row, col] = colour

            self.key_event.clear()


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


class CustomEffectManager(DBusService):
    def __init__(self, device_number, dimensions, device_base, event_files, parent):
        """
        Custom Effect manager

        :param device_number: Device Number
        :type device_number: int

        :param dimensions: Dimensions (rows, cols)
        :type dimensions: tuple or list

        :param device_base: Driver base filepath
        :type device_base: str

        :param event_files: Device event files in /dev/
        :type event_files: tuple of str

        :param parent: So we can add DBus methods and observe
        :type parent: razer_daemon.hardware.device_base.RazerDevice
        """

        DBusService.__init__(self, 'org.razer', '/org/razer/device/' + parent.serial + '/custom')

        self._device_number = device_number
        self._logger = logging.getLogger('razer.device{0}.customEffectManager'.format(device_number))
        self._dims = dimensions
        self._device_base = device_base
        self._event_files = event_files
        self._parent = parent

        self._effects = {}
        self.load_effects()

        self._parent.register_observer(self)

        self._current_effect = None

        # TODO add logging
        self.add_dbus_method('org.razer.lighting.custom', 'isCustomEffectRunning', self.dbus_is_effect_running, out_signature='b')
        self.add_dbus_method('org.razer.lighting.custom', 'startCustomEffect', self.dbus_run_effect, in_signature='s')
        self.add_dbus_method('org.razer.lighting.custom', 'stopCustomEffect', self.stop_effect)
        self.add_dbus_method('org.razer.lighting.custom', 'listCustomEffects', self.dbus_list_effects, out_signature='as')

    @property
    def is_running(self):
        """
        If current effect is running

        :return: Boolean
        :rtype: bool
        """
        if self._current_effect is None:
            return False

        if self._current_effect.is_alive():
            return True
        else:
            self._current_effect = None
            return False

    def load_effects(self):
        """
        Currently goes through classes in this file and loaded in ones that are wanted.

        Split out from __init__ so that eventually it'll do more, scan other files etc...
        """
        for name, obj in globals().items():
            if isinstance(obj, type) and issubclass(obj, BaseEffect) and name not in ('BaseEffect', 'MaskedEffect', 'LayeredEffect'):
                self._effects[obj.EFFECT_METHOD_NAME] = obj

    def notify(self, msg):
        """
        observer Notify

        :param msg: Message
        :type msg: ???
        """
        msg_type, origin, args = msg

        if msg_type == 'effect':
            self.stop_effect()

    def get_base_args(self):
        """
        Create standard args for creating BaseEffect

        :return: Dictionary of args
        :rtype: dict
        """
        return {
            'device_number': self._device_number,
            'dimensions': self._dims,
            'device_base': self._device_base,
            'event_files': self._event_files,
        }

    def stop_effect(self):
        """
        Stop effect if running
        """
        if self.is_running:
            self._current_effect.terminate()
            self._current_effect = None

    def close(self):
        """
        Stop effect as closing down
        """
        self.stop_effect()

    def dbus_run_effect(self, effect_name):
        self.stop_effect()

        effect_cls = self._effects.get(effect_name, None)
        if effect_cls is not None:
            self._current_effect = effect_cls(**self.get_base_args())
            self._current_effect.start()

    def dbus_is_effect_running(self):
        """
        Is custom effect running

        :return: Boolean
        :rtype: bool
        """
        return self.is_running

    def dbus_list_effects(self):
        """
        List custom effects

        :return:
        """
        return list(self._effects.keys())


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

    # a = ReactiveEffect(device_number=str(0),
    #                dimensions=(6, 22),
    #                device_base='/sys/bus/hid/drivers/razerkbd/0003:1532:0203.000A',
    #                event_files=('/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-if01-event-kbd', '/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd'))
    # a.start()
    # print("started")
    #
    # time.sleep(3000)
    #
    # a.terminate()
    # time.sleep(1)
    # print(a.is_alive())

    a = CustomEffectManager(0, (6,22), '/sys/bus/hid/drivers/razerkbd/0003:1532:0203.000A', ('/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-if01-event-kbd', '/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd'), None)


    print()
