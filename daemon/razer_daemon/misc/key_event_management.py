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
import asyncio
import datetime
import json
import logging

import evdev

# pylint: disable=import-error
from razer_daemon.keyboard import KEY_MAPPING, TARTARUS_KEY_MAPPING, EVENT_MAPPING, TARTARUS_EVENT_MAPPING
from .macro import MacroKey, MacroRunner, macro_dict_to_obj


class KeyWatcher():
    """
    Task to watch keyboard event files and return keypresses
    """
    @staticmethod
    def parse_event_record(ev):
        """
        Parse Input event record

        :param event: evdev event
        :type event: evdev.events.KeyEvent

        :return: Tuple of event time, action, key_code
        :rtype: tuple
        """
        if not isinstance(ev, evdev.KeyEvent):
            return None, None, None

        if ev.keystate == ev.key_up:
            action = 'release'
        elif ev.keystate == ev.key_down:
            action = 'press'
        elif ev.keystate == ev.key_hold:
            action = 'autorepeat'
        else:
            action = 'unknown'

        seconds = ev.event.sec + (ev.event.usec * 0.000001)
        date = datetime.datetime.fromtimestamp(seconds)

        result = (date, action, ev.scancode)

        return result

    def __init__(self, device_id, event_files, parent, testing=False):
        super(KeyWatcher, self).__init__()

        self._logger = logging.getLogger('razer.device{0}.keywatcher'.format(device_id))
        self._event_files = event_files
        self._parent = parent
        self._testing = testing

        self._event_devices_locked = False
        self._key_actions = []

        self._event_devices = [evdev.InputDevice(event_file) for event_file in self._event_files]
        for device in self._event_devices:
            asyncio.ensure_future(self._event_callback(device))

    def add_key_action(self, key_action):
        if key_action not in self._key_actions:
            self._key_actions.append(key_action)

    def remove_key_action(self, key_action):
        if key_action in self._key_actions:
            self._key_actions.remove(key_action)

    def grab_event_devices(self, grab):
        """
        Grab the event files exclusively

        :param grab: True to grab, False to release
        :type grab: bool
        """
        if self._event_devices_locked == grab:
            return

        if not self._testing:
            for event_device in self._event_devices:
                if grab is True:
                    event_device.grab()
                else:
                    event_device.ungrab()
        self._event_devices_locked = grab

    async def _event_callback(self, device):
        async for event in device.async_read_loop():
            try:
                if len(self._key_actions) == 0:
                    continue

                date, action, keycode = self.parse_event_record(evdev.categorize(event))

                # Skip if date, key_action and key_code is none as thats a spacer record
                if date is None:
                    continue

                # Call all actions for the keypress
                for key_action in self._key_actions:
                    await key_action(date, keycode, action)
            except (OSError, IOError) as err:
                self._logger.error("Error from event device: %s", err)
                return


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
    def __init__(self, device_id, event_files, parent, testing=False):

        self._device_id = device_id
        self._logger = logging.getLogger('razer.device{0}.keymanager'.format(device_id))
        self._parent = parent
        self._parent.register_observer(self)
        self._testing = testing

        self._event_files = event_files
        self._keywatcher = KeyWatcher(device_id, event_files, self, testing)

        self._record_stats = parent.config.get('Statistics', 'key_statistics')
        self._stats = {}

        self._recording_macro = False
        self._macros = {}

        self._current_macro_bind_key = None
        self._current_macro_combo = []

        self._clean_counter = 0

        self._event_files_locked = False

        self._key_mapping = KEY_MAPPING
        self._event_mapping = EVENT_MAPPING

        if len(event_files) > 0:
            self._logger.debug("Starting KeyWatcher")
        else:
            self._logger.warning("No event files for KeyWatcher")

        self.add_key_action(self.key_action)

    #TODO add property for enabling key stats?


    @property
    def key_mapping(self):
        return self._key_mapping

    @key_mapping.setter
    def key_mapping(self, mapping):
        self._key_mapping = mapping

    @property
    def event_mapping(self):
        return self._event_mapping

    @event_mapping.setter
    def event_mapping(self, mapping):
        self._event_mapping = mapping

    def map_event(self, keycode):
        result = None
        if self.event_mapping is None:
            result = evdev.ecodes.KEY[keycode][4:]
        else:
            result = self.event_mapping[keycode]
        return result

    def add_key_action(self, action):
        self._keywatcher.add_key_action(action)

    def remove_key_action(self, action):
        self._keywatcher.remove_key_action(action)

    async def key_action(self, event_time, key_id, key_press='press'):
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

        :param key_press: Can either be press, release, autorepeat
        :type key_press: bool
        """
        # Disable pylints complaining for this part, #PerformanceOverNeatness
        # pylint: disable=too-many-branches,too-many-statements

        if key_press == 'autorepeat': # TODO not done right yet
            # If its brightness then convert autorepeat to key presses
            if key_id in (190, 194):
                key_press = 'press'
            else:
                # Quit out early
                return

        now = datetime.datetime.now()

        try:
            # Convert event ID to key name
            key_name = self.map_event(key_id)

            #self._logger.info("Got key: {0}, state: {1}".format(key_name, 'DOWN' if key_press else 'UP'))

            # Key release
            if key_press == 'release':
                if self._recording_macro:
                    # Skip as dont care about releasing macro bind key
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
                    self._stats[storage_bucket] = dict.fromkeys(self.key_mapping, 0)
                    try:
                        # Increment key
                        self._stats[storage_bucket][key_name] += 1
                        # self._logger.debug("Increased key %s", key_name)
                    except KeyError as err:
                        self._logger.exception("Got key error. Couldn't store in bucket", exc_info=err)

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
                        current_brightness -= 20
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
                        current_brightness += 20
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
                    # Don't want no recursion, cancel macro, dont let one call macro in a macro
                    elif key_name == self._current_macro_bind_key:
                        self._logger.warning("Skipping macro assignment as would cause recursion")
                        self._recording_macro = False
                        self._parent.setMacroMode(False)
                    # Anything else just record it
                    else:
                        self._current_macro_combo.append((event_time, key_name, 'DOWN'))
                # Not recording anything so if a macro key is pressed then run
                else:
                    # If key has a macro, play it
                    if key_name in self._macros:
                        await self.play_macro(key_name)

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

    async def play_macro(self, macro_key):
        """
        Play macro for a given key

        :param macro_key: Macro Key
        :type macro_key: str
        """
        self._logger.info("Running Macro %s:%s", macro_key, str(self._macros[macro_key]))
        macro = MacroRunner(self._device_id, macro_key, self._macros[macro_key])
        task = asyncio.ensure_future(macro.run())
        await task

    async def play_media_key(self, media_key):
        """
        Execute a media keys function

        :param media_key: Media key name
        :type media_key: str
        """
        media_key = MediaKeyPress(media_key)
        await media_key.run()

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
        type key which determins what type of action it will perform.
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
        self._parent.remove_observer(self)

        self._logger.debug("Stopping key manager")

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


class TartarusKeyManager(KeyboardKeyManager):
    def __init__(self, device_id, event_files, parent, testing=False):
        super(TartarusKeyManager, self).__init__(device_id, event_files, parent, testing=testing)

        self._mode_modifier = False
        self._mode_modifier_combo = []
        self._mode_modifier_key_down = False
        self._key_mapping = TARTARUS_KEY_MAPPING
        self._event_mapping = TARTARUS_EVENT_MAPPING

    async def key_action(self, event_time, key_id, key_press='press'):
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
        self._keywatcher.grab_event_devices(True)

        try:
            # Convert event ID to key name

            key_name = self.map_event(key_id)
            # Key press

            # This is the key for storing stats, by generating hour timestamps it will bucket data nicely.
            storage_bucket = event_time.strftime('%Y%m%d%H')

            try:
                # Try and increment key in bucket
                self._stats[storage_bucket][key_name] += 1
                # self._logger.debug("Increased key %s", key_name)
            except KeyError:
                # Create bucket
                self._stats[storage_bucket] = dict.fromkeys(self.key_mapping, 0)
                try:
                    # Increment key
                    self._stats[storage_bucket][key_name] += 1
                    # self._logger.debug("Increased key %s", key_name)
                except KeyError as err:
                    self._logger.exception("Got key error. Couldn't store in bucket", exc_info=err)

            # if self._testing:
            if key_press:
                self._logger.debug("Got Key: %s Down", key_name)
            else:
                self._logger.debug("Got Key: %s Up", key_name)

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

            self._logger.debug("Macro String: %s", key_name)

            if key_name in self._macros and key_press:
                self.play_macro(key_name)

        except KeyError as err:
            self._logger.exception("Got key error. Couldn't convert event to key name", exc_info=err)

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

class MediaKeyPress():
    """
    Class to run xdotool to execute media/volume keypresses
    """
    def __init__(self, media_key):
        if media_key == 'sleep':
            self._media_key = media_key
        #else:
        #    self._media_key = MEDIA_KEY_MAP[media_key]

    async def run(self):
        if self._media_key == 'sleep':
            task = await asyncio.create_subprocess_exec(['dbus-send', '--system', '--print-reply',
                    '--dest=org.freedesktop.login1', '/org/freedesktop/login1',
                    'org.freedesktop.login1.Manager.Suspend', 'boolean:true'])
            await task.wait()
        else:
            task = await asyncio.create_subprocess_exec(['xdotool', 'key', self._media_key],
                    stdout=asyncio.subprocess.DEVNULL, stderr=asyncio.subprocess.DEVNULL)
            await task.communicate()
