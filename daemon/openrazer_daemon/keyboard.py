"""
Module to handle custom colours
"""

import gi
gi.require_version('Gdk', '3.0')
from gi.repository import Gdk
import struct
import subprocess


# KEY_UP = 0x0
# KEY_DOWN = 0x1
# KEY_HOLD = 0x2

KEY_MAPPING = {
    # Row 0
    1: (0, 1), 59: (0, 3), 60: (0, 4), 61: (0, 5), 62: (0, 6), 63: (0, 7), 64: (0, 8), 65: (0, 9), 66: (0, 10), 67: (0, 11), 68: (0, 12), 70: (0, 16), 87: (0, 13), 88: (0, 14), 99: (0, 15), 119: (0, 17),
    # Row 1
    2: (1, 2), 3: (1, 3), 4: (1, 4), 5: (1, 5), 6: (1, 6), 7: (1, 7), 8: (1, 8), 9: (1, 9), 10: (1, 10), 11: (1, 11), 12: (1, 12), 13: (1, 13), 14: (1, 14), 41: (1, 1), 55: (1, 20), 69: (1, 18), 74: (1, 21), 98: (1, 19), 102: (1, 16), 104: (1, 17), 110: (1, 15), 183: (1, 0),
    # Row 2
    15: (2, 1), 16: (2, 2), 17: (2, 3), 18: (2, 4), 19: (2, 5), 20: (2, 6), 21: (2, 7), 22: (2, 8), 23: (2, 9), 24: (2, 10), 25: (2, 11), 26: (2, 12), 27: (2, 13), 71: (2, 18), 72: (2, 19), 73: (2, 20), 78: (2, 21), 107: (2, 16), 109: (2, 17), 111: (2, 15), 184: (2, 0),
    # Row 3
    28: (3, 14), 30: (3, 2), 31: (3, 3), 32: (3, 4), 33: (3, 5), 34: (3, 6), 35: (3, 7), 36: (3, 8), 37: (3, 9), 38: (3, 10), 39: (3, 11), 40: (3, 12), 43: (3, 13), 58: (3, 1), 75: (3, 18), 76: (3, 19), 77: (3, 20), 185: (3, 0),
    # Row 4
    42: (4, 1), 44: (4, 3), 45: (4, 4), 46: (4, 5), 47: (4, 6), 48: (4, 7), 49: (4, 8), 50: (4, 9), 51: (4, 10), 52: (4, 11), 53: (4, 12), 54: (4, 14), 79: (4, 18), 80: (4, 19), 81: (4, 20), 86: (4, 2), 96: (4, 21), 103: (4, 16), 186: (4, 0),
    # Row 5
    29: (5, 1), 56: (5, 3), 57: (5, 7), 82: (5, 19), 83: (5, 20), 97: (5, 14), 100: (5, 11), 105: (5, 15), 106: (5, 17), 108: (5, 16), 125: (5, 2), 127: (5, 13), 187: (5, 0),

    # Additional mappings
    113: (0, 3), 114: (0, 4), 115: (0, 5), 163: (0, 9), 164: (0, 8), 165: (0, 7), 188: (0, 11), 189: (0, 12), 190: (0, 13), 194: (0, 14)
}


#      0   1   2   3   4    5   6   7
#    ------------------------------------
# 0 |  1   2   3   4   5        UP
# 1 |  6   7   8   9   10  LEFT    RIGHT
# 2 |  11  12  13  14  15  TMB DOWN MS
#
#
#

TARTARUS_KEY_MAPPING = {
    15: (0, 0),
    16: (0, 1),
    17: (0, 2),
    18: (0, 3),
    19: (0, 4),
    103: (0, 6),

    58: (1, 0),
    30: (1, 1),
    31: (1, 2),
    32: (1, 3),
    33: (1, 4),
    105: (1, 5),
    106: (1, 7),

    42: (2, 0),
    44: (2, 1),
    45: (2, 2),
    46: (2, 3),
    47: (2, 4),
    57: (2, 5),
    108: (2, 6),
    56: (2, 7)

}

#      0   1   2   3   4    5   6   7    8
#    ---------------------------------------
# 0 |
# 1 |      1   2   3   4   5   MS   UP
# 2 |      6   7   8   9   10  LEFT    RIGHT
# 3 |      11  12  13  14  15      DOWN  TMB
# 4 |      16      17  18  19  20
#

ORBWEAVER_KEY_MAPPING = {
    41: (1, 1),
    2: (1, 2),
    3: (1, 3),
    4: (1, 4),
    5: (1, 5),
    56: (1, 6),
    103: (1, 7),

    15: (2, 1),
    16: (2, 2),
    17: (2, 3),
    18: (2, 4),
    19: (2, 5),
    105: (2, 6),
    106: (2, 7),

    58: (3, 1),
    30: (3, 2),
    31: (3, 3),
    32: (3, 4),
    33: (3, 5),
    108: (3, 7),
    57: (3, 8),

    42: (4, 1),
    44: (4, 3),
    45: (4, 4),
    46: (4, 5),
    47: (4, 6)
}

# Naga hex buttons are normal 1-7 keys
#      0 1 2 3 4
#   --------------
# 0 |    1   2
# 1 |  4       3
# 2 |    5   7
# 3 |      6
NAGA_HEX_V2_KEY_MAPPING = {
    2: (0, 1), 3: (0, 3),
    4: (1, 0), 5: (1, 4),
    6: (2, 1), 7: (2, 3),
    8: (3, 2)
}


class KeyDoesNotExistError(Exception):
    """
    Simple custom error
    """
    pass


class NoBackupError(Exception):
    pass


class RGB(object):

    @staticmethod
    def clamp(value):
        """
        Clamp a value to 0-255

        :param value: Value to be clamped
        :type value: integer or float

        :return: Integer in the range of 0-255
        :rtype: int
        """
        result = int(value)

        if value > 255:
            result = 255
        elif value < 0:
            result = 0

        return result

    def __init__(self, red=0, green=0, blue=0):
        self._red = red
        self._green = green
        self._blue = blue

    @property
    def red(self):
        """
        Getter for red element

        :return: Red element
        :rtype: int
        """
        return self._red

    @red.setter
    def red(self, value):
        """
        Setter for red value

        :param value: Red value
        :type value: int or float
        """
        self._red = RGB.clamp(value)

    @property
    def green(self):
        """
        Getter for green element

        :return: Green element
        :rtype: int
        """
        return self._green

    @green.setter
    def green(self, value):
        """
        Setter for green value

        :param value: Green value
        :type value: int or float
        """
        self._green = RGB.clamp(value)

    @property
    def blue(self):
        """
        Getter for blue element

        :return: Blue element
        :rtype: int
        """
        return self._blue

    @blue.setter
    def blue(self, value):
        """
        Setter for blue value

        :param value: Blue value
        :type value: int or float
        """
        self._blue = RGB.clamp(value)

    def set(self, colour_tuple):
        """
        Sets all the colours at once

        :param colour_tuple: Tuple of R,G,B elements
        :type colour_tuple: tuple
        """
        # Shortcut to clamp all parameters and assign to the 3 variables
        self._red, self._green, self._blue = list(map(RGB.clamp, colour_tuple))

    def get(self):
        """
        Gets all the colours as a tuple

        :return: RGB tuple
        :rtype: tuple
        """
        return self._red, self._green, self._blue

    def __bytes__(self):
        """
        Convert to bytes

        :return: Byte string
        :rtype: bytearray
        """
        return bytes((self._red, self._green, self._blue))

    def __repr__(self):
        """
        String representation

        :return: String
        :rtype: str
        """
        return "RGB Object (#{0:02X}{1:02X}{2:02X})".format(self._red, self._green, self._blue)


class KeyboardColour(object):
    """
    Keyboard class which represents the colour state of the keyboard.
    """

    @staticmethod
    def gdk_colour_to_rgb(gdk_color):
        """
        Converts GDK colour to (R,G,B) tuple

        :param gdk_color: GDK colour
        :type gdk_color: Gdk.Color or tuple

        :return: Tuple of 3 ints
        :rtype: tuple
        """
        if isinstance(gdk_color, (list, tuple)):
            return gdk_color

        assert type(gdk_color) is Gdk.Color, "Is not of type Gdk.Color"

        red = int(gdk_color.red_float * 255)
        green = int(gdk_color.green_float * 255)
        blue = int(gdk_color.blue_float * 255)

        return red, green, blue

    def __init__(self, rows, columns):
        self.rows = rows
        self.columns = columns

        # Two-dimensional array to hold color data
        self.colors = []

        # Backup object (currently not used)
        self.backup = None

        # Initialize array with empty values
        self.reset_rows()

    def backup_configuration(self):
        """
        Backs up the current configuration
        """
        self.backup = KeyboardColour(self.rows, self.columns)
        self.backup.get_from_total_binary(self.get_total_binary())

    def restore_configuration(self):
        """
        Restores the previous configuration
        """
        if self.backup is None:
            raise NoBackupError()

        self.colors = self.backup.colors
        self.backup = None

    def get_rows_raw(self):
        """
        Gets the raw representation of the rows

        :return: Rows
        :rtype: list
        """
        return self.colors

    def reset_rows(self):
        """
        Reset the rows of the keyboard
        """
        self.colors.clear()

        for row in range(0, self.rows):
            # Create 22 rgb values
            self.colors.append([RGB() for _ in range(0, self.columns)])

    def set_key_colour(self, row, col, colour):
        """
        Set the colour of a key

        :param row: Row ID
        :type row: int

        :param col: Column ID
        :type col: int

        :param colour: Colour to set
        :type colour: Gdk.Color or tuple

        :raises KeyDoesNotExistError: If given key does not exist
        """
        self.colors[row][col].set(KeyboardColour.gdk_colour_to_rgb(colour))

    def get_key_colour(self, key):
        """
        Get the colour of a key

        :param key: Key to set the colour of
        :type key: str

        :raises KeyDoesNotExistError: If given key does not exist
        """
        if key not in KEY_MAPPING:
            raise KeyDoesNotExistError("The key \"{0}\" does not exist".format(key))

        row_id, col_id = KEY_MAPPING[key]
        return self.colors[row_id][col_id].get()

    def reset_key(self, row, col):
        """
        Reset the colour of a key

        :param row: Row ID
        :type row: int

        :param col: Column ID
        :type col: int

        :raises KeyDoesNotExistError: If given key does not exist
        """
        self.colors[row][col].set((0, 0, 0))

    def get_row_binary(self, row_id):
        """
        Gets the binary payload for a given row

        :param row_id: Row ID
        :type row_id: int

        :return: Byte string of 67 bytes, Row ID byte then 22 RGB bytes
        :rtype: bytearray
        """
        assert isinstance(row_id, int), "Row ID is not an int"

        payload = bytes([row_id, 0x00, len(self.colors[row_id]) - 1])

        for rgb in self.colors[row_id]:
            payload += bytes(rgb)

        return payload

    def get_total_binary(self):
        """
        Gets the binary payload for the whole keyboard

        :return: Byte string of 6*67 bytes, (Row ID byte then 22 RGB bytes) * 6
        :rtype: bytearray
        """
        payload = b''

        for row in range(0, self.rows):
            payload += self.get_row_binary(row)

        return payload

    def get_from_total_binary(self, binary_blob):
        """
        Load in a binary blob which is the output from get_total_binary

        :param binary_blob: Binary blob
        :type binary_blob: bytearray
        """
        self.reset_rows()

        for row_id in range(0, self.rows):
            binary_blob = binary_blob[1:]  # Skip first byte

            for col_id, binary_rgb in enumerate([binary_blob[i:i + 3] for i in range(0, 66, 3)]):
                rgb = struct.unpack('=BBB', binary_rgb)
                self.colors[row_id][col_id].set(rgb)

            binary_blob = binary_blob[66:]  # Skip the current row


def get_keyboard_layout():
    """
    Function to get the keyboard layout

    I can see this becoming an ugly mess as it'll need to support multiple keyboard layouts which have different names
    on different systems

    :return: Keyboard layout
    :rtype: str
    """
    cmd = ['setxkbmap', '-query']
    output = subprocess.check_output(cmd)

    result = 'gb'

    if output:
        output = output.decode('utf-8').splitlines()
        layout = 'gb'
        variant = ''
        for line in output:
            if line.startswith('layout'):
                layout = line.split(':', 1)[1].strip()
                if layout.find(',') > -1:
                    layout = layout.split(',')[0]

            elif line.startswith('variant'):
                variant = line.split(':', 1)[1].strip().split(',')[0]

                if 'latin9' in variant:  # Removes some rubbish from ubuntu
                    variant = 'latin9'

        if variant == '':
            result = layout
        else:
            result = layout + '-' + variant

    # If the user has an international layout variant ignore that part
    result = result.replace('-altgr-intl', '')

    return result
