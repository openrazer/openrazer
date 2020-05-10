"""
Receives events from /dev/input/by-id/somedevice

Events are 24 bytes long and are emitted from a character device in 24 byte chunks.
As the keyboards have "2" keyboard interfaces we need to listen on both of them in case some n00b
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
import threading
import time
import sys
from openrazer_daemon.keyboard import KEY_MAPPING, TARTARUS_KEY_MAPPING, EVENT_MAPPING, TARTARUS_EVENT_MAPPING, NAGA_HEX_V2_EVENT_MAPPING, NAGA_HEX_V2_KEY_MAPPING, ORBWEAVER_EVENT_MAPPING, ORBWEAVER_KEY_MAPPING

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))  # TODO: figure out a better way to handle this
from evdev import UInput, ecodes, InputDevice
from evdev.events import event_factory
# pylint: disable=import-error
EVENT_FORMAT = '@llHHI'
EVENT_SIZE = struct.calcsize(EVENT_FORMAT)

EPOLL_TIMEOUT = 0.01
SPIN_SLEEP = 0.005

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
    @staticmethod  # TODO remove
    def parse_event_record(event):
        """
        Parse Input event record

        :param data: Binary data
        :type data: bytes

        :return: Tuple of event time, key_action, key_code
        :rtype: tuple
        """

        if event.type != ecodes.ecodes['EV_KEY']:  # input-event-codes.h EV_KEY 0x01
            return None, None, None

        if event.value == 0:
            key_action = 'release'
        elif event.value == 1:
            key_action = 'press'
        elif event.value == 2:
            key_action = 'autorepeat'
        else:
            key_action = 'unknown'

        date = datetime.datetime.fromtimestamp(event.timestamp())

        result = (date, key_action, event.code)

        if event.type == event.code == event.value == 0:
            return None, None, None

        return result

    def __init__(self, device_id, event_files, parent):
        super(KeyWatcher, self).__init__()

        self._logger = logging.getLogger('razer.device{0}.keywatcher'.format(device_id))
        self._event_files = event_files
        self._shutdown = False
        self._parent = parent

        event_file_map = map(InputDevice, (self._event_files))
        self._event_file_map = {event_file.fd: event_file for event_file in event_file_map}

    def run(self):
        """
        Main event loop
        """
        # Grab device event file
        for device in self._event_file_map.values():
            try:
                device.grab()
                self._logger.debug("Grabbed device {0}".format(device.path))
            except (IOError, OSError) as err:
                self._logger.exception("Error grabbing device {0}".format(device.path), exc_info=err)

        # Loop
        while not self._shutdown:
            try:
                self.poll(self._event_file_map)
            except (OSError, IOError) as err:
                self._logger.exception("Error reading from device, stopping key watcher", exc_info=err)
                self._shutdown = True

            time.sleep(SPIN_SLEEP)

        self._logger.debug("Closing keywatcher")

        # Ungrab files and close them
        for device in self._event_file_map.values():
            try:
                device.ungrab()
            except:
                pass  # If the device is unplugged we don't care

            try:  # Try once for each
                device.close()
            except:
                pass

    def poll(self, event_file_map):
        r, w, x = select.select(event_file_map, [], [], EPOLL_TIMEOUT)
        for fd in r:
            events = event_file_map[fd].read()
            if not events:
                break
            for event in events:
                date, key_action, key_code = self.parse_event_record(event)

                # Skip if date, key_action and key_code is none as that's a spacer record
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


class KeyboardKeyManager(object):
    """
    Key management class.

    This class deals with anything to do with keypresses. Currently it does:
    * Receiving keypresses from the KeyWatcher
    * Logic to deal with GameMode shortcut not working when macro's not enabled
    * Logic to deal with recording on the fly macros and replaying them
    * Stores number of keypresses / key / hour, stats are used for heatmaps / time-series

    It will be used to store keypresses in a list (for at most 2 seconds) if enabled for the ripple effect, when I
    get round to making the effect.
    """

    # pylint: disable=too-many-instance-attributes
    def __init__(self, device_id, event_files, parent, testing=False, should_grab_event_files=False):

        self._device_id = device_id
        self._logger = logging.getLogger('razer.device{0}.keymanager'.format(device_id))
        self._parent = parent
        self._parent.register_observer(self)
        self._testing = testing

        self._event_files = event_files
        self._access_lock = threading.Lock()
        self._keywatcher = KeyWatcher(device_id, event_files, self)
        if len(event_files) > 0:
            self._logger.debug("Starting KeyWatcher")
            self._keywatcher.start()
        else:
            self._logger.warning("No event files for KeyWatcher")

        self._record_stats = parent.config.get('Statistics', 'key_statistics')
        self._stats = {}

        self._temp_key_store_active = False
        self._temp_key_store = []
        self._temp_expire_time = datetime.timedelta(seconds=2)

        self._last_colour_choice = None

        self._should_grab_event_files = should_grab_event_files
        self._event_files_locked = False

        self.KEY_MAP = KEY_MAPPING
        self.EVENT_MAP = EVENT_MAPPING

    # TODO add property for enabling key stats?

    @property
    def temp_key_store(self):
        """
        Get the temporary key store

        :return: List of keys
        :rtype: list
        """
        # Locking so it doesn't mutate whilst copying
        self._access_lock.acquire()
        now = datetime.datetime.now()

        # Remove expired keys from store
        try:
            # Get date and if its less than now its expired
            while self._temp_key_store[0][0] < now:
                self._temp_key_store.pop(0)
        except IndexError:

            pass

        # Creating a copy as then it doesn't mutate whilst iterating
        result = self._temp_key_store[:]
        self._access_lock.release()
        return result

    @property
    def temp_key_store_state(self):
        """
        Get the state of the temporary key store

        :return: Active state
        :rtype: bool
        """
        return self._temp_key_store_active

    @temp_key_store_state.setter
    def temp_key_store_state(self, value):
        """
        Set the state of the temporary key store

        :param value: Active state
        :type value: bool
        """
        self._temp_key_store_active = value

    def key_action(self, event_time, key_id, key_press):
        """
        Process a key press event

        Ok an attempt to explain the logic
        * Adds keypresses to the temporary key store (for ripple effect)
        * Sends key to the binding manager
        * Pressing any key will increment a statistical number in a dictionary used for generating
          heatmaps.
        :param event_time: Time event occurred
        :type event_time: datetime.datetime

        :param key_id: Key Event ID
        :type key_id: int

        :param key_press: Can either be press, release, autorepeat
        :type key_press: bool
        """
        # Disable pylints complaining for this part, #PerformanceOverNeatness
        # pylint: disable=too-many-branches,too-many-statements

        now = datetime.datetime.now()

        # Remove expired keys from store
        try:
            # Get date and if its less than now its expired
            while self._temp_key_store[0][0] < now:
                self._temp_key_store.pop(0)
        except IndexError:
            pass

        try:
            # Convert event ID to key name
            key_name = self.EVENT_MAP[key_id]

            # self._logger.debug("Got key: {0}, state: {1}".format(key_name, key_press))

            # This is the key for storing stats, by generating hour timestamps it will bucket data nicely.
            storage_bucket = event_time.strftime('%Y%m%d%H')

            try:
                # Try and increment key in bucket
                self._stats[storage_bucket][key_name] += 1
            #    self._logger.debug("Increased key %s", key_name)
            except KeyError:
                # Create bucket
                self._stats[storage_bucket] = dict.fromkeys(self.KEY_MAP, 0)
                try:
                    # Increment key
                    self._stats[storage_bucket][key_name] += 1
            #        self._logger.debug("Increased key %s", key_name)
                except KeyError as err:
                    self._logger.exception("Got key error. Couldn't store in bucket", exc_info=err)

            if key_press == 'press' and self.temp_key_store_state:
                colour = random_colour_picker(self._last_colour_choice, COLOUR_CHOICES)
                self._last_colour_choice = colour
                self._temp_key_store.append((now + self._temp_expire_time, self.KEY_MAP[key_name], colour))

            # Sets up game mode as when enabling macro keys it stops the key working
            if key_name == 'GAMEMODE':
                self._logger.info("Got game mode combo")

                game_mode = self._parent.getGameMode()
                self._parent.setGameMode(not game_mode)

            # Brightness logic
            elif key_name == 'BRIGHTNESSDOWN':
                # Get brightness value
                current_brightness = self._parent.method_args.get('brightness', None)
                if current_brightness is None:
                    current_brightness = self._parent.getBrightness()

                if current_brightness > 0:
                    current_brightness -= 10
                    if current_brightness < 0:
                        current_brightness = 0

                    self._parent.setBrightness(current_brightness)
                    #self._parent.method_args['brightness'] = current_brightness
            elif key_name == 'BRIGHTNESSUP':
                # Get brightness value
                current_brightness = self._parent.method_args.get('brightness', None)
                if current_brightness is None:
                    current_brightness = self._parent.getBrightness()

                if current_brightness < 100:
                    current_brightness += 10
                    if current_brightness > 100:
                        current_brightness = 100

                    self._parent.setBrightness(current_brightness)
                    #self._parent.method_args['brightness'] = current_brightness

            else:
                x = threading.Thread(target=self._parent.binding_manager.key_press, args=(key_id, key_press))
                x.start()

        except KeyError as err:
            self._logger.exception("Got key error. Couldn't convert event to key name", exc_info=err)

    def close(self):
        """
        Cleanup function
        """
        if self._keywatcher.is_alive():
            self._parent.remove_observer(self)

            self._logger.debug("Stopping key manager")
            self._keywatcher.shutdown = True
            self._keywatcher.join(timeout=2)
            if self._keywatcher.is_alive():
                self._logger.error("Could not stop KeyWatcher thread")

    def __del__(self):
        self.close()

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
                #     self.temp_key_store_state = True
                # else:
                #     self.temp_key_store_state = False
                pass


class NagaHexV2KeyManager(KeyboardKeyManager):
    KEY_MAP = NAGA_HEX_V2_KEY_MAPPING
    EVENT_MAP = NAGA_HEX_V2_EVENT_MAPPING


class GamepadKeyManager(KeyboardKeyManager):
    EVENT_MAP = TARTARUS_EVENT_MAPPING
    KEY_MAP = TARTARUS_KEY_MAPPING


class OrbweaverKeyManager(KeyboardKeyManager):
    EVENT_MAP = ORBWEAVER_EVENT_MAPPING
    KEY_MAP = ORBWEAVER_KEY_MAPPING
