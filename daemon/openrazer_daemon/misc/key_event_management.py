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
from evdev import UInput, ecodes, InputDevice

# pylint: disable=import-error
from openrazer_daemon.keyboard import KEY_MAPPING, TARTARUS_KEY_MAPPING, EVENT_MAPPING, TARTARUS_EVENT_MAPPING, NAGA_HEX_V2_EVENT_MAPPING, NAGA_HEX_V2_KEY_MAPPING, ORBWEAVER_EVENT_MAPPING, ORBWEAVER_KEY_MAPPING
from .macro import MacroKey, MacroRunner, macro_dict_to_obj
from .key_binding_manager import KeyBindingManager

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
    @staticmethod #TODO remove
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

    def __init__(self, device_id, event_files, parent, use_epoll=True):
        super(KeyWatcher, self).__init__()

        self._logger = logging.getLogger('razer.device{0}.keywatcher'.format(device_id))
        self._event_files = event_files
        self._shutdown = False
        self._parent = parent


    def run(self):
        """
        Main event loop
        """
        event_file_map = map(InputDevice, (self._event_files))
        event_file_map = {event_file.fd: event_file for event_file in event_file_map}

        for dev in event_file_map.values(): dev.grab()

        # Loop
        while not self._shutdown:
            r, w, x = select.select(event_file_map, [], [])
            for fd in r:
                for event in event_file_map[fd].read():
                    date, key_action, key_code = self.parse_event_record(event)

                    # Skip if date, key_action and key_code is none as that's a spacer record
                    if date is None:
                        continue

                    # Now if key is pressed then we record
                    self._parent.key_action(date, key_code, key_action)

#            time.sleep(SPIN_SLEEP)

        # Ungrab files and close them
        for dev in event_file_map.values(): dev.ungrab()


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
    KEY_MAP = KEY_MAPPING
    EVENT_MAP = EVENT_MAPPING

    # pylint: disable=too-many-instance-attributes
    def __init__(self, device_id, event_files, parent, use_epoll=False, testing=False, should_grab_event_files=False):

        self._device_id = device_id
        self._logger = logging.getLogger('razer.device{0}.keymanager'.format(device_id))
        self._parent = parent
        self._parent.register_observer(self)
        self._testing = testing

        self._event_files = event_files
        self._access_lock = threading.Lock()
        self._keywatcher = KeyWatcher(device_id, event_files, self, use_epoll=use_epoll)
        self._fake_device = UInput(name="{0} (mapped)".format(self._parent.get_device_name()))
        self._binding_manager = KeyBindingManager(device_id, self, self._fake_device)


        if len(event_files) > 0:
            self._logger.debug("Starting KeyWatcher")
            self._keywatcher.start()
        else:
            self._logger.warning("No event files for KeyWatcher")

        self._record_stats = parent.config.get('Statistics', 'key_statistics')
        self._stats = {}

        self._recording_macro = False
        self._macros = {}

        self._current_macro_bind_key = None
        self._current_macro_combo = []

        self._threads = set()
        self._clean_counter = 0

        self._temp_key_store_active = False
        self._temp_key_store = []
        self._temp_expire_time = datetime.timedelta(seconds=2)

        self._last_colour_choice = None

        self._should_grab_event_files = should_grab_event_files
        self._event_files_locked = False  

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

    def key_action(self, event_time, key_id, key_press='press'):
        """
        Process a key press event

        Ok an attempt to explain the logic
        * The function sets a value _fn_down depending on the state of FN.
        * Adds keypress and release events to a macro list if recording a macro.
        * Pressing FN+F9 starts recording a macro, then selecting any key marks that as a macro key,
          then it will record keys, then pressing FN+F9 will save macro.
        * Pressing any macro key will run macro.
        * Pressing FN+F10 will toggle game mode.
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

        if key_press == 'autorepeat':  # TODO probably should go
            # If its brightness then convert autorepeat to key presses
            # 190 (KEY_F20) and 194 (KEY_F24) are defined in razerkbd_driver.c
            if key_id in (190, 194):
                key_press = 'press'
            else:
                # Quit out early
                return

        now = datetime.datetime.now()

        # Remove expired keys from store
        try:
            # Get date and if its less than now its expired
            while self._temp_key_store[0][0] < now:
                self._temp_key_store.pop(0)
        except IndexError:
            pass

        # Clean up any threads
        if self._clean_counter > 20 and len(self._threads) > 0:
            self._clean_counter = 0
            self.clean_macro_threads()

        try:
            # Convert event ID to key name
            key_name = self.EVENT_MAP[key_id]

            # self._logger.info("Got key: {0}, state: {1}".format(key_name, 'DOWN' if key_press else 'UP'))

            # Key release
            if key_press == 'release':
                if self._recording_macro:
                    # Skip as don't care about releasing macro bind key
                    if key_name not in (self._current_macro_bind_key, 'MACROMODE'):
                        # Record key release events
                        self._current_macro_combo.append((event_time, key_name, 'UP'))

            else:
                # Key press

                # This is the key for storing stats, by generating hour timestamps it will bucket data nicely.
                storage_bucket = event_time.strftime('%Y%m%d%H')

                try:
                    # Try and increment key in bucket
                    self._stats[storage_bucket][key_name] += 1
                    # self._logger.debug("Increased key %s", key_name)
                except KeyError:
                    # Create bucket
                    self._stats[storage_bucket] = dict.fromkeys(self.KEY_MAP, 0)
                    try:
                        # Increment key
                        self._stats[storage_bucket][key_name] += 1
                        # self._logger.debug("Increased key %s", key_name)
                    except KeyError as err:
                        self._logger.exception("Got key error. Couldn't store in bucket", exc_info=err)

                if self._temp_key_store_active:
                    colour = random_colour_picker(self._last_colour_choice, COLOUR_CHOICES)
                    self._last_colour_choice = colour
                    self._temp_key_store.append((now + self._temp_expire_time, self.KEY_MAP[key_name], colour))

                # Macro FN+F9 logic
                if key_name == 'MACROMODE':
                    self._logger.info("Got macro combo")

                    if not self._recording_macro:
                        # Starting to record macro
                        self._recording_macro = True
                        self._current_macro_bind_key = None
                        self._current_macro_combo = []

                        self._parent.setMacroEffect(0x01)
                        self._parent.setMacroMode(True)

                    else:
                        self._logger.debug("Finished recording macro")
                        # Finish recording macro
                        if self._current_macro_bind_key is not None:
                            if len(self._current_macro_combo) > 0:
                                self.add_kb_macro()
                            else:
                                # Clear macro
                                self.dbus_delete_macro(self._current_macro_bind_key)
                        self._recording_macro = False
                        self._parent.setMacroMode(False)
                # Sets up game mode as when enabling macro keys it stops the key working
                elif key_name == 'GAMEMODE':
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

                elif self._recording_macro:

                    if self._current_macro_bind_key is None:
                        # Restrict macro bind keys to M1-M5
                        if key_name not in ('M1', 'M2', 'M3', 'M4', 'M5'):
                            self._logger.warning("Macros are only for M1-M5 for now.")
                            self._recording_macro = False
                            self._parent.setMacroMode(False)
                        else:
                            self._current_macro_bind_key = key_name
                            self._parent.setMacroEffect(0x00)
                    # Don't want no recursion, cancel macro, don't let one call macro in a macro
                    elif key_name == self._current_macro_bind_key:
                        self._logger.warning("Skipping macro assignment as would cause recursion")
                        self._recording_macro = False
                        self._parent.setMacroMode(False)
                    # Anything else just record it
                    else:
                        self._current_macro_combo.append((event_time, key_name, 'DOWN'))
                # Not recording anything so if a macro key is pressed then run
                elif not self._recording_macro:
                    # If key has a macro, play it
                    if key_name in self._macros:
                        self.play_macro(key_name)

                else:
                    self._binding_manager.key_press(key_id)


        except KeyError as err:
            self._logger.exception("Got key error. Couldn't convert event to key name", exc_info=err)

    def add_kb_macro(self):
        """
        Tidy up the recorded macro and add it to the store

        Goes through the macro and generated relative delays between key events
        """
        new_macro = []

        start_time = self._current_macro_combo[0][0]
        for event_time, key, state in self._current_macro_combo:
            delay = (event_time - start_time).microseconds
            start_time = event_time
            new_macro.append(MacroKey(key, delay, state))

        self._macros[self._current_macro_bind_key] = new_macro

    def clean_macro_threads(self):
        """
        Threadless-threadpool

        Goes though the threads (macro play jobs) and removed the threads if they have finished.
        #SetMagic
        """
        self._logger.debug("Cleaning up macro threads")
        to_remove = set()

        for macro_thread in self._threads:
            macro_thread.join(timeout=0.05)
            if not macro_thread.is_alive():
                to_remove.add(macro_thread)

        self._threads -= to_remove

    def play_macro(self, macro_key):
        """
        Play macro for a given key

        Launches a thread and adds it to the pool
        :param macro_key: Macro Key
        :type macro_key: str
        """
        self._logger.info("Running Macro %s:%s", macro_key, str(self._macros[macro_key]))
        macro_thread = MacroRunner(self._device_id, macro_key, self._macros[macro_key])
        macro_thread.start()
        self._threads.add(macro_thread)

    # Methods to be used with DBus
    def dbus_delete_macro(self, key_name):
        """
        Delete a macro from a key

        :param key_name: Key Name
        :type key_name: str
        """
        try:
            del self._macros[key_name]
        except KeyError:
            pass

    def dbus_get_macros(self):
        """
        Get macros in JSON format

        Returns a JSON blob of all active macros in the format of
        {BIND_KEY: [MACRO_DICT...]}

        MACRO_DICT is a dict representation of an action that can be performed. The dict will have a
        type key which determines what type of action it will perform.
        For example there are key press macros, URL opening macros, Script running macros etc...
        :return: JSON of macros
        :rtype: str
        """
        result_dict = {}
        for macro_key, macro_combo in self._macros.items():
            str_combo = [value.to_dict() for value in macro_combo]
            result_dict[macro_key] = str_combo

        return json.dumps(result_dict)

    def dbus_add_macro(self, macro_key, macro_json):
        """
        Add macro from JSON

        The macro_json will be a list of macro objects which is then converted into JSON
        :param macro_key: Macro bind key
        :type macro_key: str

        :param macro_json: Macro JSON
        :type macro_json: str
        """
        macro_list = [macro_dict_to_obj(macro_object_dict) for macro_object_dict in json.loads(macro_json)]
        self._macros[macro_key] = macro_list

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
            if msg[2] != 'setRipple':
                # If we are not doing ripple effect then disable the storing of keys
                #self.temp_key_store_state = False
                pass


class NagaHexV2KeyManager(KeyboardKeyManager):
    KEY_MAP = NAGA_HEX_V2_KEY_MAPPING
    EVENT_MAP = NAGA_HEX_V2_EVENT_MAPPING


class GamepadKeyManager(KeyboardKeyManager):
    GAMEPAD_EVENT_MAPPING = TARTARUS_EVENT_MAPPING
    GAMEPAD_KEY_MAPPING = TARTARUS_KEY_MAPPING

    def __init__(self, device_id, event_files, parent, use_epoll=True, testing=False):
        super(GamepadKeyManager, self).__init__(device_id, event_files, parent, use_epoll, testing=testing)

        self._mode_modifier = False
        self._mode_modifier_combo = []
        self._mode_modifier_key_down = False

    def key_action(self, event_time, key_id, key_press=True):
        """
        Process a key press event

        Ok an attempt to explain the logic
        * The function sets a value _fn_down depending on the state of FN.
        * Adds keypress and release events to a macro list if recording a macro.
        * Pressing FN+F9 starts recording a macro, then selecting any key marks that as a macro key,
          then it will record keys, then pressing FN+F9 will save macro.
        * Pressing any macro key will run macro.
        * Pressing FN+F10 will toggle game mode.
        * Pressing any key will increment a statistical number in a dictionary used for generating
          heatmaps.
        :param event_time: Time event occurred
        :type event_time: datetime.datetime

        :param key_id: Key Event ID
        :type key_id: int

        :param key_press: If true then its a press, else its a release
        :type key_press: bool
        """
        # Disable pylints complaining for this part, #PerformanceOverNeatness
        # pylint: disable=too-many-branches,too-many-statements
        self._access_lock.acquire()

        if not self._event_files_locked:
            self.grab_event_files(True)

        now = datetime.datetime.now()

        # Remove expired keys from store
        try:
            # Get date and if its less than now its expired
            while self._temp_key_store[0][0] < now:
                self._temp_key_store.pop(0)
        except IndexError:
            pass

        # Clean up any threads
        if self._clean_counter > 20 and len(self._threads) > 0:
            self._clean_counter = 0
            self.clean_macro_threads()

        try:
            # Convert event ID to key name

            key_name = self.GAMEPAD_EVENT_MAPPING[key_id]
            # Key press

            # This is the key for storing stats, by generating hour timestamps it will bucket data nicely.
            storage_bucket = event_time.strftime('%Y%m%d%H')

            try:
                # Try and increment key in bucket
                self._stats[storage_bucket][key_name] += 1
                # self._logger.debug("Increased key %s", key_name)
            except KeyError:
                # Create bucket
                self._stats[storage_bucket] = dict.fromkeys(self.GAMEPAD_KEY_MAPPING, 0)
                try:
                    # Increment key
                    self._stats[storage_bucket][key_name] += 1
                    # self._logger.debug("Increased key %s", key_name)
                except KeyError as err:
                    self._logger.exception("Got key error. Couldn't store in bucket", exc_info=err)

            if self._temp_key_store_active:
                colour = random_colour_picker(self._last_colour_choice, COLOUR_CHOICES)
                self._last_colour_choice = colour
                self._temp_key_store.append((now + self._temp_expire_time, self.GAMEPAD_KEY_MAPPING[key_name], colour))

            # if self._testing:
            # if key_press:
                #self._logger.debug("Got Key: {0} Down".format(key_name))
            # else:
                #self._logger.debug("Got Key: {0} Up".format(key_name))

            # Logic for mode switch modifier
            if self._mode_modifier:
                if key_name == 'MODE_SWITCH' and key_press:
                    # Start the macro string
                    self._mode_modifier_key_down = True
                    self._mode_modifier_combo.clear()
                    self._mode_modifier_combo.append('MODE')

                elif key_name == 'MODE_SWITCH' and not key_press:
                    # Release mode_switch
                    self._mode_modifier_key_down = False

                elif key_press and self._mode_modifier_key_down:
                    # Any keys pressed whilst mode_switch is down
                    self._mode_modifier_combo.append(key_name)

                    # Override keyname so it now equals a macro
                    key_name = '+'.join(self._mode_modifier_combo)

            self._logger.debug("Macro String: {0}".format(key_name))

            if key_name in self._macros and key_press:
                self.play_macro(key_name)

        except KeyError as err:
            self._logger.exception("Got key error. Couldn't convert event to key name", exc_info=err)

        self._access_lock.release()

    @property
    def mode_modifier(self):
        """
        Get if the MODE_SWTICH key is to act as a modifier

        :return: True if a modifier, false if not
        :rtype: bool
        """
        return self._mode_modifier

    @mode_modifier.setter
    def mode_modifier(self, value):
        """
        Set MODE_SWITCH modifier state

        :param value: Modifier state
        :type value: bool
        """
        self._mode_modifier = True if value else False


class OrbweaverKeyManager(GamepadKeyManager):
    GAMEPAD_EVENT_MAPPING = ORBWEAVER_EVENT_MAPPING
    GAMEPAD_KEY_MAPPING = ORBWEAVER_KEY_MAPPING
