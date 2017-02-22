"""
Macro stuff

Has objects representing key events
Launching programs etc...
"""
import logging
import subprocess
import threading
import multiprocessing
import os
import json
import selectors
import evdev

# pylint: disable=import-error
from razer_daemon.keyboard import XTE_MAPPING
import razer_daemon.misc.key_mapping as razer_key_mapping


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

        self._keymaps = razer_key_mapping.KeymapManager(self._device_number)

        self._macro_sets = []
        self._macros = {}

        self._load_macros()

    @classmethod
    def _state_to_string(cls, state):
        if state == cls.KEY_DOWN:
            return 'DOWN'
        elif state == cls.KEY_UP:
            return 'UP'
        else:
            return 'AUTOREPEAT'

    def _convert_keycode_to_keysym(self, keycode):
        """
        Converts a keycode to a keysym

        :param keycode: Keycode
        :type keycode: int

        :return: Keysym
        :rtype: str

        :raises KeyError: If no keymaps are loaded
        :raises ValueError: If no keycode -> keysym is found
        """
        return self._keymaps.keycode_to_keysym(keycode)

    def _convert_keysym_to_keycode(self, keysym):
        """
        Converts a keysym to a keycode

        :param keysym: keysym
        :type keysym: str

        :return: keycode
        :rtype: int

        :raises KeyError: If no keymaps are loaded
        :raises ValueError: If no keysym -> keycode is found
        """
        return self._keymaps.keysym_to_keycode(keysym)

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

    # TODO check recordign macros store keycodes, ------------------------------------------------------->
    # Then on send to client it does keycode -> keysym
    # Then on receive it does keysym <- keycode



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









# This determins if the macro keys are executed with their natural spacing
XTE_SLEEP = False

class MacroObject(object):
    """
    Macro base object
    """
    def to_dict(self):
        """
        Convert the object to a dict to be sent over DBus

        :return: Dictionary
        :rtype: dict
        """
        raise NotImplementedError()

    @classmethod
    def from_dict(cls, values_dict):
        """
        Create class from dict

        :param values_dict: Dictionary of values (and a type key)
        :type values_dict: dict

        :return: Class object
        """
        del values_dict['type']
        return cls(**values_dict)


class MacroKey(MacroObject):
    """
    Is an object of a key event used in macros
    """
    def __init__(self, key_id, pre_pause, state):
        self.key_id = key_id
        self.pre_pause = pre_pause
        self.state = state

    def __repr__(self):
        return '{0} {1}'.format(self.key_id, self.state)

    def __str__(self):
        return 'MacroKey|{0}|{1}|{2}'.format(self.key_id, self.pre_pause, self.state)

    def to_dict(self):
        return {
            'type': 'MacroKey',
            'key_id': self.key_id,
            'pre_pause': self.pre_pause,
            'state': self.state
        }

    @property
    def xte_key(self):
        """
        Convert key to XTE compatible name

        :return: XTE Name
        :rtype: str
        """
        return XTE_MAPPING.get(self.key_id, self.key_id)

# If it only opens a new tab in chroma - https://askubuntu.com/questions/540939/xdg-open-only-opens-a-new-tab-in-a-new-chromium-window-despite-passing-it-a-url
class MacroURL(MacroObject):
    """
    Is an object of a key event used in macros
    """
    def __init__(self, url):
        self.url = url

    def __repr__(self):
        return '{0}'.format(self.url)

    def __str__(self):
        return 'MacroURL|{0}'.format(self.url)

    def to_dict(self):
        return {
            'type': 'MacroURL',
            'url': self.url,
        }

    def execute(self):
        """
        Open URL in the browser
        """
        proc = subprocess.Popen(['xdg-open', self.url], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        proc.communicate()


class MacroScript(MacroObject):
    """
    Is an object of a key event used in macros
    """
    def __init__(self, script, args=None):
        self.script = script
        if isinstance(args, str):
            self.args = ' ' + args
        else:
            self.args = ''

    def __repr__(self):
        return '{0}'.format(self.script)

    def __str__(self):
        return 'MacroScript|{0}'.format(self.script)

    def to_dict(self):
        return {
            'type': 'MacroScript',
            'script': self.script,
            'args': self.args
        }

    def execute(self):
        """
        Run script
        """
        proc = subprocess.Popen(self.script + self.args, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        proc.communicate()


class MacroRunner(threading.Thread):
    """
    Thread to run macros
    """
    def __init__(self, device_id, macro_bind, macro_data):
        super(MacroRunner, self).__init__()

        self._logger = logging.getLogger('razer.device{0}.macro{1}'.format(device_id, macro_bind))
        self._macro_data = macro_data
        self._macro_bind = macro_bind

    @staticmethod
    def xte_line(key_event):
        """
        Generate a line to be fet into XTE

        :param key_event: Key event object
        :type key_event: MacroKey

        :return: String XTE script
        :rtype: str
        """
        # Save key here to prevent 5odd dictionary lookups
        key = key_event.xte_key

        cmd = ''

        if key is not None:
            if XTE_SLEEP:
                cmd += 'usleep {0}\n'.format(key_event.pre_pause)

            if key_event.state == 'UP':
                cmd += 'keyup {0}\n'.format(key)
            else:
                cmd += 'keydown {0}\n'.format(key)

        return cmd

    def run(self):
        """
        Main thread function
        """

        # TODO move the xte-munging to the init
        xte = ''

        for event in self._macro_data:
            if isinstance(event, MacroKey):
                xte += self.xte_line(event)
            else:
                if xte != '':
                    proc = subprocess.Popen(['xte'], stdin=subprocess.PIPE)
                    proc.communicate(input=xte.encode('ascii'))
                    xte = ''

                # Now run everything else (this just allows for less calls to xte
                if not isinstance(event, MacroKey):
                    event.execute()

        if xte != '':
            proc = subprocess.Popen(['xte'], stdin=subprocess.PIPE)
            proc.communicate(input=xte.encode('ascii'))

        self._logger.debug("Finished running macro %s", self._macro_bind)


def macro_dict_to_obj(macro_dict):
    """
    Converts a macro string to its relevant object

    :param macro_dict: Macro string
    :type macro_dict: dict

    :return: Macro Object
    :rtype: object

    :raises ValueError: When a type isn't known
    """

    # pylint: disable=redefined-variable-type
    if macro_dict['type'] == 'MacroKey':
        result = MacroKey.from_dict(macro_dict)
    elif macro_dict['type'] == 'MacroURL':
        result = MacroURL.from_dict(macro_dict)
    elif macro_dict['type'] == 'MacroScript':
        result = MacroScript.from_dict(macro_dict)
    else:
        raise ValueError("unknown type")

    return result
