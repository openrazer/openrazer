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

import asyncio
import evdev

# pylint: disable=import-error
from openrazer_daemon.keyboard import KeyboardColour
import numpy as _np


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
            self._matrix = _np.zeros((self._components, self._rows, self._cols), 'uint8')
            self._fb1 = _np.copy(self._matrix)
        else:
            self._matrix.fill(0)

    def set(self, y:int, x:int, rgb:tuple):
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

    def get(self, y:int, x:int) -> list:
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

    def row_binary(self, row_id:int) -> bytes:
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

        return row_id.to_bytes(1, byteorder='big') + start.to_bytes(1, byteorder='big') + end.to_bytes(1, byteorder='big') + self._matrix[:,row_id].tobytes(order='F')

    def to_binary(self):
        """
        Get the whole binary for the keyboard to be sent to the driver.

        :return: Driver binary payload
        :rtype: bytes
        """
        return bytes(self)

    # Simple FB
    def to_framebuffer(self):
        self._fb1 = _np.copy(self._matrix)

    def to_framebuffer_or(self):
        self._fb1 = _np.bitwise_or(self._fb1, self._matrix)

    def draw_with_fb_or(self):
        self._matrix = _np.bitwise_or(self._fb1, self._matrix)
        return bytes(self)


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

        self._logger = logging.getLogger('razer.device{0}.ripplethread'.format(device_number))

        self._event_files = event_files

        # (Rows, Cols)
        self._dims = dimensions

        self._rgb_file = os.path.join(device_base, 'matrix_custom_frame')
        self._custom_file = os.path.join(device_base, 'matrix_effect_custom')

        self.stop = False

        self.key_events = []

    @asyncio.coroutine
    def _event_callback(self, device):
        while True:
            events = yield from device.async_read()
            for event in events:
                self.key_events.append(event)

    @asyncio.coroutine
    def _run_loop(self):
        count = 0

        while True:
            print("Count = {0}, Len = {1}".format(count, len(self.key_events)))
            yield from asyncio.sleep(0.1)
            count += 1

    def run(self):
        for device_path in self._event_files:
            dev = evdev.InputDevice(device_path)
            asyncio.ensure_future(self._event_callback(dev))

        asyncio.ensure_future(self._run_loop())

        loop = asyncio.get_event_loop()
        loop.run_forever()

        # matrix = Frame(self._dims)
        #
        # rgb_file = open(self._rgb_file, 'wb', 0)
        # custom_file = open(self._custom_file, 'wb', 0)

        # while True:
        #     for row in range(0, self._dims[0]):
        #         for col in range(0, self._dims[1]):
        #             matrix[row, col] = (0, 255, 0)
        #
        #             binary = matrix.to_binary()
        #
        #             rgb_file.write(binary)
        #             #rgb_file.flush()
        #             custom_file.write(b'1')
        #             #custom_file.flush()
        #
        #
        #             time.sleep(0.1)
        #     matrix.reset()





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
    a = RippleEffectProcess(str(0), (6, 22), '/sys/bus/hid/drivers/razerkbd/0003:1532:0203.000A', ('/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-if01-event-kbd', '/dev/input/by-id/usb-Razer_Razer_BlackWidow_Chroma-event-kbd'))
    a.start()

    time.sleep(30)

    a.terminate()
    time.sleep(1)
    print(a.is_alive())
