"""
Receives events from /dev/input/by-id/somedevice

Events are 24 bytes long and are emitted from a character device in 24 byte chunks.
As the keyboards have "2" keyboard interfaces we need to listen on both of them incase some n00b
switches to game mode.

Each event is in the format of
* signed int of seconds
* signed int of microseconds
* unsigned short of event type
* unsigned short code
* signed int value
"""
import datetime
import fcntl
import json
import logging
import os
import random
import select
import struct
import subprocess
import threading
import time

# pylint: disable=import-error
from razer_daemon.keyboard import KEY_MAPPING, TARTARUS_KEY_MAPPING, EVENT_MAPPING, TARTARUS_EVENT_MAPPING, NAGA_HEX_V2_EVENT_MAPPING, NAGA_HEX_V2_KEY_MAPPING, ORBWEAVER_EVENT_MAPPING, ORBWEAVER_KEY_MAPPING
from .macro import MacroKey, MacroRunner, macro_dict_to_obj

import multiprocessing
import selectors
import evdev
import json

EVENT_FORMAT = '@llHHI'
EVENT_SIZE = struct.calcsize(EVENT_FORMAT)

EPOLL_TIMEOUT = 0.01
SPIN_SLEEP = 0.01

EVIOCGRAB = 0x40044590

COLOUR_CHOICES = (
    (255, 0, 0),    # Red
    (0, 255, 0),    # Green
    (0, 0, 255),    # Blue
    (255, 255, 0),  # Yellow
    (0, 255, 255),  # Cyan
    (255, 0, 255),  # Magenta
)


def random_colour_picker(last_choice, iterable):
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
    while result == last_choice:
        result = random.choice(iterable)
    return result


class KeyWatcher(threading.Thread):
    """
    Thread to watch keyboard event files and return keypresses
    """
    @staticmethod
    def parse_event_record(data):
        """
        Parse Input event record

        :param data: Binary data
        :type data: bytes

        :return: Tuple of event time, key_action, key_code
        :rtype: tuple
        """
        # Event Seconds, Event Microseconds, Event Type, Event Code, Event Value
        ev_sec, ev_usec, ev_type, ev_code, ev_value = struct.unpack(EVENT_FORMAT, data)

        if ev_type != 0x01: # input-event-codes.h EV_KEY 0x01
            return None, None, None

        if ev_value == 0:
            key_action = 'release'
        elif ev_value == 1:
            key_action = 'press'
        elif ev_value == 2:
            key_action = 'autorepeat'
        else:
            key_action = 'unknown'

        seconds = ev_sec + (ev_usec * 0.000001)
        date = datetime.datetime.fromtimestamp(seconds)

        result = (date, key_action, ev_code)

        if ev_type == ev_code == ev_value == 0:
            return None, None, None

        return result

    def __init__(self, device_id, event_files, parent, use_epoll=True):
        super(KeyWatcher, self).__init__()

        self._logger = logging.getLogger('razer.device{0}.keywatcher'.format(device_id))
        self._event_files = event_files
        self._shutdown = False
        self._use_epoll = use_epoll
        self._parent = parent

        self.open_event_files = [open(event_file, 'rb') for event_file in self._event_files]
        # Set open files to non blocking mode
        if not use_epoll:
            for event_file in self.open_event_files:
                flags = fcntl.fcntl(event_file.fileno(), fcntl.F_GETFL)
                fcntl.fcntl(event_file.fileno(), fcntl.F_SETFL, flags | os.O_NONBLOCK)

    def run(self):
        """
        Main event loop
        """
        # Create dict of Event File Descriptor: Event File Object
        event_file_map = {event_file.fileno(): event_file for event_file in self.open_event_files}

        # Create epoll object
        poll_object = select.epoll()

        # Register files with select
        for event_fd in event_file_map.keys():
            poll_object.register(event_fd, select.EPOLLIN | select.EPOLLPRI)

        # Loop
        while not self._shutdown:
            # epoll is nice but it wasn't getting events properly :(
            if self._use_epoll:
                self._poll_epoll(poll_object, event_file_map)
            else:
                self._poll_read()

            time.sleep(SPIN_SLEEP)

        # Unbind files and close them
        for event_fd, event_file in event_file_map.items():
            poll_object.unregister(event_fd)
            event_file.close()

        poll_object.close()

    def _poll_epoll(self, poll_object, event_file_map):
        events = poll_object.poll(EPOLL_TIMEOUT)

        if len(events) != 0:
            # pylint: disable=unused-variable
            for event_fd, mask in events:
                key_data = event_file_map[event_fd].read(EVENT_SIZE)

                date, key_action, key_code = self.parse_event_record(key_data)

                # Skip if date, key_action and key_code is none as thats a spacer record
                if date is None:
                    continue

                # Now if key is pressed then we record
                self._parent.key_action(date, key_code, key_action)

    def _poll_read(self):
        for event_file in self.open_event_files:
            key_data = event_file.read(EVENT_SIZE)

            if key_data is None:
                continue

            date, key_action, key_code = self.parse_event_record(key_data)

            # Skip if date, key_action and key_code is none as thats a spacer record
            if date is None:
                continue

            # Now if key is pressed then we record
            self._parent.key_action(date, key_code, key_action)

    @property
    def shutdown(self):
        """
        Thread shutdown condition

        :return: Shutdown condition
        :rtype: bool
        """
        return self._shutdown
    @shutdown.setter
    def shutdown(self, value):
        """
        Set thread shutdown condition

        :param value: Boolean, normally only True would be used
        :type value: str
        """
        self._shutdown = value


# class NagaHexV2KeyManager(KeyboardMacroV2):
#     KEY_MAP = NAGA_HEX_V2_KEY_MAPPING
#     EVENT_MAP = NAGA_HEX_V2_EVENT_MAPPING
#
#
# class GamepadKeyManager(KeyboardMacroV2):
#     GAMEPAD_EVENT_MAPPING = TARTARUS_EVENT_MAPPING
#     GAMEPAD_KEY_MAPPING = TARTARUS_KEY_MAPPING
#
#     def __init__(self, device_id, event_files, parent, use_epoll=True, testing=False):
#         super(GamepadKeyManager, self).__init__(device_id, event_files, parent, use_epoll, testing=testing)
#
#         self._mode_modifier = False
#         self._mode_modifier_combo = []
#         self._mode_modifier_key_down = False
#
#     def key_action(self, event_time, key_id, key_press=True):
#         """
#         Process a key press event
#
#         Ok an attempt to explain the logic
#         * The function sets a value _fn_down depending on the state of FN.
#         * Adds keypress and release events to a macro list if recording a macro.
#         * Pressing FN+F9 starts recording a macro, then selecting any key marks that as a macro key,
#           then it will record keys, then pressing FN+F9 will save macro.
#         * Pressing any macro key will run macro.
#         * Pressing FN+F10 will toggle game mode.
#         * Pressing any key will increment a statistical number in a dictionary used for generating
#           heatmaps.
#         :param event_time: Time event occured
#         :type event_time: datetime.datetime
#
#         :param key_id: Key Event ID
#         :type key_id: int
#
#         :param key_press: If true then its a press, else its a release
#         :type key_press: bool
#         """
#         # Disable pylints complaining for this part, #PerformanceOverNeatness
#         # pylint: disable=too-many-branches,too-many-statements
#         self._access_lock.acquire()
#
#         if not self._event_files_locked:
#             self.grab_event_files(True)
#
#
#         now = datetime.datetime.now()
#
#         # Remove expired keys from store
#         try:
#             # Get date and if its less than now its expired
#             while self._temp_key_store[0][0] < now:
#                 self._temp_key_store.pop(0)
#         except IndexError:
#             pass
#
#         # Clean up any threads
#         if self._clean_counter > 20 and len(self._threads) > 0:
#             self._clean_counter = 0
#             self.clean_macro_threads()
#
#         try:
#             # Convert event ID to key name
#
#             key_name = self.GAMEPAD_EVENT_MAPPING[key_id]
#             # Key press
#
#             # This is the key for storing stats, by generating hour timestamps it will bucket data nicely.
#             storage_bucket = event_time.strftime('%Y%m%d%H')
#
#             try:
#                 # Try and increment key in bucket
#                 self._stats[storage_bucket][key_name] += 1
#                 # self._logger.debug("Increased key %s", key_name)
#             except KeyError:
#                 # Create bucket
#                 self._stats[storage_bucket] = dict.fromkeys(self.GAMEPAD_KEY_MAPPING, 0)
#                 try:
#                     # Increment key
#                     self._stats[storage_bucket][key_name] += 1
#                     # self._logger.debug("Increased key %s", key_name)
#                 except KeyError as err:
#                     self._logger.exception("Got key error. Couldn't store in bucket", exc_info=err)
#
#             if self._temp_key_store_active:
#                 colour = random_colour_picker(self._last_colour_choice, COLOUR_CHOICES)
#                 self._last_colour_choice = colour
#                 self._temp_key_store.append((now + self._temp_expire_time, self.GAMEPAD_KEY_MAPPING[key_name], colour))
#
#             # if self._testing:
#             #if key_press:
#                 #self._logger.debug("Got Key: {0} Down".format(key_name))
#             #else:
#                 #self._logger.debug("Got Key: {0} Up".format(key_name))
#
#             # Logic for mode switch modifier
#             if self._mode_modifier:
#                 if key_name == 'MODE_SWITCH' and key_press:
#                     # Start the macro string
#                     self._mode_modifier_key_down = True
#                     self._mode_modifier_combo.clear()
#                     self._mode_modifier_combo.append('MODE')
#
#                 elif key_name == 'MODE_SWITCH' and not key_press:
#                     # Release mode_switch
#                     self._mode_modifier_key_down = False
#
#                 elif key_press and self._mode_modifier_key_down:
#                     # Any keys pressed whilst mode_switch is down
#                     self._mode_modifier_combo.append(key_name)
#
#                     # Override keyname so it now equals a macro
#                     key_name = '+'.join(self._mode_modifier_combo)
#
#             self._logger.debug("Macro String: {0}".format(key_name))
#
#             if key_name in self._macros and key_press:
#                 self.play_macro(key_name)
#
#         except KeyError as err:
#             self._logger.exception("Got key error. Couldn't convert event to key name", exc_info=err)
#
#         self._access_lock.release()
#
#     @property
#     def mode_modifier(self):
#         """
#         Get if the MODE_SWTICH key is to act as a modifier
#
#         :return: True if a modifier, false if not
#         :rtype: bool
#         """
#         return self._mode_modifier
#
#     @mode_modifier.setter
#     def mode_modifier(self, value):
#         """
#         Set MODE_SWITCH modifier state
#
#         :param value: Modifier state
#         :type value: bool
#         """
#         self._mode_modifier = True if value else False
#
#
# class OrbweaverKeyManager(GamepadKeyManager):
#     GAMEPAD_EVENT_MAPPING = ORBWEAVER_EVENT_MAPPING
#     GAMEPAD_KEY_MAPPING = ORBWEAVER_KEY_MAPPING
#

class MediaKeyPress(threading.Thread):
    """
    Class to run xdotool to execute media/volume keypresses
    """
    def __init__(self, media_key):
        super(MediaKeyPress, self).__init__()
        if media_key == 'sleep':
            self._media_key = media_key
        else:
            self._media_key = MEDIA_KEY_MAP[media_key]

    def run(self):
        if self._media_key == 'sleep':
            subprocess.call(['dbus-send', '--system', '--print-reply', '--dest=org.freedesktop.login1',
                             '/org/freedesktop/login1', 'org.freedesktop.login1.Manager.Suspend', 'boolean:true'])
        else:
            proc = subprocess.Popen(['xdotool', 'key', self._media_key], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            proc.communicate()

















