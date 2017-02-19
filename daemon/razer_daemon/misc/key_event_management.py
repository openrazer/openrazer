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


class KeyboardMacroV2(multiprocessing.Process):
    """
    TODO
     * Handle Fn+F9 for on the fly macro'ing
     * Persist macros

     * Handle storing key metrics for heatmap


    Run loop
      keyname = lookup(keycode)

      on key_release:
        handle_modifiers(keyname)

        if recording_macro:
          if keyname not int (current_macro_bind_key, macromode_key):
            add_to_current_macro(key_event_time, keyname, 'release')

      on key_press:
        handle_modifiers(keyname)

        if keyname == gamemode_key:
          gamemode_current = get_game_mode_state()
          set_game_mode_state(!gamemode_current)

        elif keyname == brightness_down_key:
          kb_brightness_down()

        elif keyname == brightness_up_key:
          kb_brightness_up()

        elif keyname == macromode_key:
          if not recording_macro:
            recording_macro = True
            current_macro_bind_key = None
            current_macro_combo = []

            set_macro_effect(blinking)
            set_macro_led(True)

          else:
            logger.debug('finished recording macro')
            if current_macro_bind_key is not None:
              if len(current_macro_combo) > 0:
                add_kb_macro()
              else:
                delete_macro(current_macro_bind_key)

            recording_macro = False
            set_macro_mode(False)

        elif recording_macro == True:
          if current_macro_bind_key is None:
            if keyname not in (M1-M5):
              logger.warning("Macros are only for M1-M5 for now.")
              recording_macro = False
              set_macro_led(False)

            else:
              current_macro_bind_key == keyname
              set_macro_effect(static)

          elif keyname == current_macro_bind_key:
            warning("Skipping macro assignment as would cause recursion")
            recording_macro = False
            set_macro_led(False)

          else:
            current_macro_combo.append(key_time, keyname, 'press')

        else:
          if keyname in macro_list:
            play_macro(keyname)


    """

    KEY_UP = 0
    KEY_DOWN = 1
    KEY_AUTOREPEAT = 2

    MACROMODE_KEY = 188  # EVENT_MAPPING.get('MACROMODE')
    GAMEMODE_KEY = 189  # EVENT_MAPPING.get('GAMEMODE')
    BRIGHTNESS_DOWN_KEY = 190  # EVENT_MAPPING.get('BRIGHTNESSDOWN')
    BRIGHTNESS_UP_KEY = 194  # EVENT_MAPPING.get('BRIGHTNESSUP')
    BRIGHTNESS_DELTA = 10  # Speed at which brightness changes

    def __init__(self, device_number, event_files, config, parent):
        super(KeyboardMacroV2, self).__init__()

        self._config = config
        self._macro_file = os.path.join(self._config['General']['DataDir'], 'macros.json')
        self._device_number = device_number
        self._logger = logging.getLogger('razer.device{0}.macroV2'.format(device_number))
        self._event_files = event_files
        self._parent = parent

        # State variables
        self._recording_macro = False
        self._current_macro_bind = None
        self._current_macro_combo = []

        self._macro_sets = []
        self._macros = {}

        self._load_macros()

    @staticmethod
    def _convert_keycode_to_key(keycode):
        return EVENT_MAPPING[keycode]

    @classmethod
    def _state_to_string(cls, state):
        if state == cls.KEY_DOWN:
            return 'DOWN'
        elif state == cls.KEY_UP:
            return 'UP'
        else:
            return 'AUTOREPEAT'

    def _persist_macros(self):
        with open(self._macro_file, 'w') as open_fp:
            macro_list = []
            for macro_set in self._macro_sets:
                tmp = {}
                for macro_key, macro_obj_list in macro_set.items():
                    tmp[macro_key] = [macro_obj.to_dict() for macro_obj in macro_obj_list]
                macro_list.append(tmp)

            self._macro_sets = json.dump(macro_list, open_fp)

    def _load_macros(self):
        if os.path.exists(self._macro_file):
            try:
                with open(self._macro_file, 'r') as open_fp:
                    loaded_macros = 0
                    macro_list = json.load(open_fp)

                    self._macro_sets = []

                    for macro_set in macro_list:
                        current_set = {}
                        for macro_bind, macro_list in macro_set.items():
                            current_set[int(macro_bind)] = [macro_dict_to_obj(macro_dict) for macro_dict in macro_list]
                            loaded_macros += 1
                        self._macro_sets.append(current_set)

                    if loaded_macros == 1:
                        self._logger.info("Loaded {0} macro".format(loaded_macros))
                    else:
                        self._logger.info("Loaded {0} macros".format(loaded_macros))
                self._macros = self._macro_sets[0]
            except Exception:
                self._logger.warning("Failed to load macros.json")
                macro_set = {}
                self._macro_sets.append(macro_set)
                self._macros = macro_set

        else:
            macro_set = {}
            self._macro_sets.append(macro_set)
            self._macros = macro_set

    def _key_event_callback(self):
        """
        Uses the high-level selectors library to basically run select() on the device's event files.

        This function is ran as a thread so be wary.
        """
        selector = selectors.DefaultSelector()

        for device_path in self._event_files:
            dev = evdev.InputDevice(device_path)
            selector.register(dev, selectors.EVENT_READ)

        while True:
            try:
                for key, mask in selector.select():
                    for event in key.fileobj.read():
                        if event.type == evdev.ecodes.EV_KEY:
                            pass
                            if event.value == self.KEY_DOWN:
                                # self._logger.debug('Key {0} down'.format(event.code))
                                self._key_press(event.timestamp(), event.code)
                            elif event.value == self.KEY_UP:
                                # self._logger.debug('Key {0} up'.format(event.code))
                                self._key_release(event.timestamp(), event.code)
                            elif event.value == self.KEY_AUTOREPEAT:
                                # self._logger.debug('Key {0} autorepeat'.format(event.code))
                                self._key_autorepeat(event.timestamp(), event.code)
            except TypeError:
                break

    def _key_press(self, timestamp, key_code):
        """
        Triggered on key press

        :param timestamp: Key Event Timestamp
        :type timestamp: time.time

        :param key_code: Key Code
        :type key_code: int
        """
        if key_code == self.GAMEMODE_KEY:
            self._logger.info("Triggering game mode toggle")

            # No point accessing raw kernel file as the parent method will handle toggling win-key, altf4 etc... and sending of events.
            game_mode = self._parent.getGameMode()
            self._parent.setGameMode(not game_mode)

        elif key_code == self.BRIGHTNESS_DOWN_KEY:
            self._brightness_down()

        elif key_code == self.BRIGHTNESS_UP_KEY:
            self._brightness_up()

        elif key_code == self.MACROMODE_KEY:
            self._logger.debug("Macro key pressed")

            if not self._recording_macro:
                # Record macro time!

                self._recording_macro = True
                self._current_macro_bind = None
                self._current_macro_combo.clear()

                self._parent.setMacroEffect(0x01)
                self._parent.setMacroMode(True)

            else:
                self._logger.debug("Finished recording macro")
                # Finishing macro?
                if self._current_macro_bind is not None:
                    if len(self._current_macro_combo) > 0:
                        self._add_otf_macro()
                        self._logger.debug("Macro {0}: {1}".format(self._current_macro_bind, self._current_macro_combo))
                        pass
                    else:
                        # Empty macro clear
                        self._current_macro_combo.clear()

                self._recording_macro = False
                self._parent.setMacroMode(False)

        elif self._recording_macro:
            # Am recording macro

            if self._current_macro_bind is None:
                # Limit macros to M1-5
                if key_code not in (183, 184, 185, 186, 187):
                    self._logger.warning("On-the-fly macros are only M1-5 for now")
                    self._recording_macro = False
                    self._parent.setMacroMode(False)
                else:
                    self._logger.debug("Starting macro")
                    self._current_macro_bind = key_code
                    self._parent.setMacroEffect(0x00)

            elif key_code == self._current_macro_bind:
                # Recording current macro bind key, cancel it, #NoRecursion
                self._logger.warning("Skipping macro assignment as would cause recursion")
                self._recording_macro = False
                self._parent.setMacroMode(False)

            else:
                # Anything else, record it
                self._current_macro_combo.append((timestamp, key_code, self.KEY_DOWN))

        else:
            # If key is a macro, play it
            if key_code in self._macros:
                self._logger.debug("Play macro {0}: {1}".format(key_code, self._macros[key_code]))
                self.play_macro(key_code)

    def _key_autorepeat(self, timestamp, key_code):
        """
        Triggered on key autorepeat

        :param timestamp: Key Event Timestamp
        :type timestamp: time.time

        :param key_code: Key Code
        :type key_code: int
        """
        if key_code == self.BRIGHTNESS_DOWN_KEY:
            self._brightness_down()

        elif key_code == self.BRIGHTNESS_UP_KEY:
            self._brightness_up()

    def _key_release(self, timestamp, key_code):
        """
        Triggered on key release

        :param timestamp: Key Event Timestamp
        :type timestamp: time.time

        :param key_code: Key Code
        :type key_code: int
        """
        if self._recording_macro and key_code not in (self._current_macro_bind, self.MACROMODE_KEY):
            # Am recording macro, key is not macrokey or the key the macro is bound to
            # Store said key event
            self._current_macro_combo.append((timestamp, key_code, self.KEY_UP))

    def _brightness_down(self):
        self._logger.debug("Triggering brightness down")
        current_brightness = self._parent.method_args.get('brightness', None)
        if current_brightness is None:
            current_brightness = self._parent.getBrightness()

        if current_brightness > 0:
            current_brightness -= self.BRIGHTNESS_DELTA
            if current_brightness < 0:
                current_brightness = 0

            self._parent.setBrightness(current_brightness)

    def _brightness_up(self):
        self._logger.debug("Triggering brightness up")
        current_brightness = self._parent.method_args.get('brightness', None)
        if current_brightness is None:
            current_brightness = self._parent.getBrightness()

        if current_brightness < 100:
            current_brightness += self.BRIGHTNESS_DELTA
            if current_brightness > 100:
                current_brightness = 100

            self._parent.setBrightness(current_brightness)

    def _add_otf_macro(self):
        new_macro = []

        start_time = self._current_macro_combo[0][0]
        for event_time, key_code, state in self._current_macro_combo:
            delay = event_time - start_time
            start_time = event_time
            new_macro.append(MacroKey(self._convert_keycode_to_key(key_code), delay, self._state_to_string(state)))

        self._macros[self._current_macro_bind] = new_macro
        self._persist_macros()

    def play_macro(self, macro_key):
        """
        Play macro for a given key

        Launches a thread and adds it to the pool
        :param macro_key: Macro Key
        :type macro_key: int
        """
        self._logger.debug("Running macro - " + str(self._macros[macro_key]))
        macro_obj = MacroRunner(self._device_number, macro_key, self._macros[macro_key])
        macro_obj.daemon = True
        macro_obj.start()
        #macro_obj.join()

    def run(self):
        self._logger.debug('Started Macro thread')
        self._key_event_callback()

    # DBus methods
    def dbus_delete_macro(self, key_code):
        """
        Delete a macro from a key

        :param key_name: Key Name
        :type key_name: str
        """
        try:
            del self._macros[key_code]
        except KeyError:
            pass

    def dbus_get_macros(self):
        """
        Get macros in JSON format

        Returns a JSON blob of all active macros in the format of
        {BIND_KEY: [MACRO_DICT...]}

        MACRO_DICT is a dict representation of an action that can be performed. The dict will have a
        type key which determins what type of action it will perform.
        For example there are key press macros, URL opening macros, Script running macros etc...
        :return: JSON of macros
        :rtype: str
        """
        result_dict = {}
        for macro_key, macro_combo in self._macros.items():
            str_combo = [value.to_dict() for value in macro_combo]
            result_dict[str(macro_key)] = str_combo

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
        self._macros[int(macro_key)] = macro_list


class NagaHexV2KeyManager(KeyboardMacroV2):
    KEY_MAP = NAGA_HEX_V2_KEY_MAPPING
    EVENT_MAP = NAGA_HEX_V2_EVENT_MAPPING


class GamepadKeyManager(KeyboardMacroV2):
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
        :param event_time: Time event occured
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
            #if key_press:
                #self._logger.debug("Got Key: {0} Down".format(key_name))
            #else:
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

















