"""
Macro stuff

Has objects representing key events
Launching programs etc...
"""
import bisect
import logging
import subprocess
import multiprocessing
import os
import json
import selectors
import evdev
# pylint: disable=import-error


class MacroV2Base(multiprocessing.Process):
    KEY_UP = 0
    KEY_DOWN = 1
    KEY_AUTOREPEAT = 2

    def __init__(self, serial, device_number, event_files, config, parent, grab_event_files=False, non_grab_files=None):
        super(MacroV2Base, self).__init__()

        self._serial = serial
        self._config = config
        self._macro_file = os.path.join(self._config['General']['DataDir'], 'macros-{0}.json'.format(serial))
        self._device_number = device_number
        self._logger = logging.getLogger('razer.device{0}.macroV2'.format(device_number))
        self._event_files = event_files
        self._event_files_nograb = non_grab_files
        self._parent = parent
        self._grab_event_files = grab_event_files

        self._macro_sets = []
        self._macros = {}

        self.uinput = evdev.UInput()

        self._load_macros()

    @classmethod
    def _state_to_string(cls, state):
        if state == cls.KEY_DOWN:
            return 'DOWN'
        elif state == cls.KEY_UP:
            return 'UP'
        else:
            return 'AUTOREPEAT'

    def _load_macros(self):
        json_obj = None

        if os.path.exists(self._macro_file):
            try:
                with open(self._macro_file, 'r') as open_fp:
                    json_obj = json.load(open_fp)
            except Exception:
                pass

        self._deserialise(json_obj)

    def _deserialise(self, json_obj=None):
        """
        Take a json list and covnert it to macros

        :param json_obj: Json list
        :type json_obj: list or dict or None
        """

        if json_obj is not None:
            self._macro_sets = []

            loaded_macros = 0
            for macro_set in json_obj:
                current_set = {}
                for macro_bind, macro_list in macro_set.items():
                    current_set[int(macro_bind)] = macro_dict_to_obj(macro_list)
                    loaded_macros += 1
                self._macro_sets.append(current_set)

            if loaded_macros == 1:
                self._logger.info("Loaded {0} macro".format(loaded_macros))
            else:
                self._logger.info("Loaded {0} macros".format(loaded_macros))

            self._macros = self._macro_sets[0]

        else:
            macro_set = {}
            self._macro_sets.append(macro_set)
            self._macros = macro_set

    def _serialise(self):
        """
        Get macros and return a json list of them

        :return: Json List
        :rtype: List
        """
        macro_list = []
        for macro_set in self._macro_sets:
            tmp = {}
            for macro_key, macro_obj_list in macro_set.items():
                tmp[macro_key] = macro_obj_to_dict(macro_obj_list)
            macro_list.append(tmp)

        return macro_list

    def _persist_macros(self):
        with open(self._macro_file, 'w') as open_fp:
            json.dump(self._serialise(), open_fp)

    def _key_event_callback(self):
        """
        Uses the high-level selectors library to basically run select() on the device's event files.

        This function is ran as a thread so be wary.
        """
        selector = selectors.DefaultSelector()

        for device_path in self._event_files:
            dev = evdev.InputDevice(device_path)
            if self._grab_event_files:
                dev.grab()
            selector.register(dev, selectors.EVENT_READ)

        # Hack for reading mouse event file just for keypresses
        if self._event_files_nograb is not None:
            for device_path in self._event_files_nograb:
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
        raise NotImplementedError()

    def _key_autorepeat(self, timestamp, key_code):
        """
        Triggered on key autorepeat

        :param timestamp: Key Event Timestamp
        :type timestamp: time.time

        :param key_code: Key Code
        :type key_code: int
        """
        raise NotImplementedError()

    def _key_release(self, timestamp, key_code):
        """
        Triggered on key release

        :param timestamp: Key Event Timestamp
        :type timestamp: time.time

        :param key_code: Key Code
        :type key_code: int
        """
        raise NotImplementedError()

    def run(self):
        self._logger.debug('Started Macro thread')
        self._key_event_callback()

    def play_macro(self, macro_key):
        """
        Play macro for a given key

        Launches a thread and adds it to the pool
        :param macro_key: Macro Key
        :type macro_key: int
        """
        self._logger.debug("Running macro - " + str(self._macros[macro_key]))
        for macro_obj in self._macros[macro_key]:
            macro_obj.exec(uinput=self.uinput, caller=self, parent=self._parent)

        # macro_obj.join()

    # DBus methods
    def dbus_delete_macro(self, key_code):
        """
        Delete a macro from a key

        :param key_code: Key Name
        :type key_code: int
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
        for macro_key, macro_list in self._macros.items():
            result_dict[str(macro_key)] = macro_obj_to_dict(macro_list)

        return json.dumps(result_dict)

    def dbus_add_macro(self, macro_key, macro_json):
        """
        Add macro from JSON

        The macro_json will be a list of macro objects which is then converted into JSON
        :param macro_key: Macro bind key
        :type macro_key: int

        :param macro_json: Macro JSON
        :type macro_json: str
        """
        try:
            macro_list = macro_dict_to_obj(json.loads(macro_json))
            self._macros[macro_key] = macro_list
        except Exception as err:
            self._logger.error("Failed to load macro {0}. Got {1}".format(macro_key, err))

    # Observer
    def notify(self, msg):
        pass


class KeyboardMacroV2(MacroV2Base):
    """
    TODO
     * Handle storing key metrics for heatmap
    """

    MACROMODE_KEY = 188  # EVENT_MAPPING.get('MACROMODE')
    GAMEMODE_KEY = 189  # EVENT_MAPPING.get('GAMEMODE')
    BRIGHTNESS_DOWN_KEY = 190  # EVENT_MAPPING.get('BRIGHTNESSDOWN')
    BRIGHTNESS_UP_KEY = 194  # EVENT_MAPPING.get('BRIGHTNESSUP')
    BRIGHTNESS_DELTA = 10  # Speed at which brightness changes

    def __init__(self, serial, device_number, event_files, config, parent, grab_event_files=False):
        super(KeyboardMacroV2, self).__init__(serial, device_number, event_files, config, parent, grab_event_files)

        # State variables
        self._recording_macro = False
        self._current_macro_bind = None
        self._current_macro_combo = []

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
                self._current_macro_combo.append((key_code, timestamp, self.KEY_DOWN))

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
            self._current_macro_combo.append((key_code, timestamp, self.KEY_UP))

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
        # Macros can be a sequence of macros
        self._macros[self._current_macro_bind] = [MacroKeyString(self._current_macro_combo)]
        self._persist_macros()


class NagaMacroV2(MacroV2Base):
    # LEFT = 272
    # RIGHT = 273
    # MIDDLE = 274
    BTN_DPI_UP = 264
    BTN_DPI_DOWN = 265

    BTN_SCROLL_LEFT = 262
    BTN_SCROLL_RIGHT = 263

    KEY_1 = 2
    KEY_2 = 3
    KEY_3 = 4
    KEY_4 = 5
    KEY_5 = 6
    KEY_6 = 7
    KEY_7 = 8

    # Not used
    WANTED_KEYS = (KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, BTN_DPI_UP, BTN_DPI_DOWN, BTN_SCROLL_LEFT, BTN_SCROLL_RIGHT)

    DEFUALT_DPI_LIST = [(500, 500), (800, 800), (1000, 1000), (1200, 1200), (1500, 1500), (2000, 2000)]

    def __init__(self, serial, device_number, event_files, config, parent, grab_event_files=False, non_grab_files=None):
        self._dpi_list = []
        self._current_dpi = None
        self._has_dpi = hasattr(parent, 'setDPI')

        # Call parent method, which calls load_macros() -> _deserialise()
        super(NagaMacroV2, self).__init__(serial, device_number, event_files, config, parent, grab_event_files, non_grab_files)

        # Set DPI as by now it wont be none, dpi might not exist though
        if self._has_dpi:
            dpi_x, dpi_y = self._current_dpi
            self._logger.debug('Restoring last known DPI ({0},{1})'.format(dpi_x, dpi_y))
            parent.setDPI(dpi_x, dpi_y)

        # Register observer with parent
        parent.register_observer(self)

    def notify(self, msg):
        #                   0         1                               2         3                  4
        # <class 'tuple'>: ('effect', RazerNagaHexV2:PM1642H00601138, 'setDPI', dbus.UInt16(1000), dbus.UInt16(1000))

        #                                                                     by now we know [2] will be method name
        if isinstance(msg, tuple) and len(msg) > 0 and msg[0] == 'effect' and msg[2] == 'setDPI':
            self._current_dpi = (int(msg[3]), int(msg[4]))
            self._parent.dpiChanged()
            self._persist_macros()

    def _serialise(self):
        return {
            'version': '1.0',
            'current_dpi': self._current_dpi,
            'dpi_list': self._dpi_list,
            'macros': super(NagaMacroV2, self)._serialise()
        }

    def _deserialise(self, json_obj=None):
        if not isinstance(json_obj, dict):
            self._dpi_list = self.DEFUALT_DPI_LIST
            self._current_dpi = self._dpi_list[0]
            super(NagaMacroV2, self)._deserialise(json_obj=None)
        else:
            self._dpi_list = json_obj.get('dpi_list', self.DEFUALT_DPI_LIST)
            self._current_dpi = json_obj.get('current_dpi', self._dpi_list[0])

            macro_list = json_obj.get('macros', None)
            super(NagaMacroV2, self)._deserialise(json_obj=macro_list)

        #
        # Setup default dpi handlers
        if self._has_dpi:
            modified = False

            if self.BTN_DPI_UP not in self._macros:
                modified = True
                self._macros[self.BTN_DPI_UP] = [MacroDPIChange('up')]

            if self.BTN_DPI_DOWN not in self._macros:
                modified = True
                self._macros[self.BTN_DPI_DOWN] = [MacroDPIChange('down')]

            if modified:
                self._persist_macros()

    def _key_press(self, timestamp, key_code):
        """
        Triggered on key press

        :param timestamp: Key Event Timestamp
        :type timestamp: time.time

        :param key_code: Key Code
        :type key_code: int
        """
        if key_code in self.WANTED_KEYS:
            if key_code in self._macros:
                self._logger.debug("Play macro {0}: {1}".format(key_code, self._macros[key_code]))
                self.play_macro(key_code)
            else:
                self._logger.debug('Macro Key {0}'.format(key_code))

    def _key_autorepeat(self, timestamp, key_code):
        """
        Triggered on key autorepeat

        :param timestamp: Key Event Timestamp
        :type timestamp: time.time

        :param key_code: Key Code
        :type key_code: int
        """
        pass

    def _key_release(self, timestamp, key_code):
        """
        Triggered on key release

        :param timestamp: Key Event Timestamp
        :type timestamp: time.time

        :param key_code: Key Code
        :type key_code: int
        """
        pass

    def dpi_up(self):
        # Can assume that people arnt using different dpi_x to dpi_y
        if self._has_dpi:
            total_dpis = len(self._dpi_list)
            index = bisect.bisect_right([x[0] for x in self._dpi_list], self._current_dpi[0])

            if index == total_dpis:
                # dpi is equal to or higher than max level
                pass
            else:
                dpi_x, dpi_y = self._dpi_list[index]
                self._logger.debug('DPI UP: Current:({0[0]}, {0[1]}) Next ({1}, {2})'.format(self._current_dpi, dpi_x, dpi_y))
                self._parent.setDPI(dpi_x, dpi_y)
                self._logger.debug('TODO: DPI_UP DBus SIGNAL')

    def dpi_down(self):
        # Can assume that people arnt using different dpi_x to dpi_y
        if self._has_dpi:
            index = bisect.bisect_left([x[0] for x in self._dpi_list], self._current_dpi[0]) - 1

            if index <= 0:
                # dpi is equal to or higher than max level
                pass
            else:
                dpi_x, dpi_y = self._dpi_list[index]
                self._logger.debug('DPI DOWN: Current:({0[0]}, {0[1]}) Next ({1}, {2})'.format(self._current_dpi, dpi_x, dpi_y))
                self._parent.setDPI(dpi_x, dpi_y)
                self._logger.debug('TODO: DPI_DOWN DBus SIGNAL')



















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

    def exec(self, **kwargs):
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


class MacroKeyString(MacroObject):
    def __init__(self, key_list):
        """
        Runs a list of keys

        :param key_list: Is a list of lists. Each element should be (event_time, key_code, up/down)
        :type key_list: list or tuple
        """
        self.macro_string = []

        # Basically just normalise the time so event 1 is 0, event 2 is the time since event 1, etc...
        start_time = key_list[0][0]
        for key_code, event_time, state in key_list:
            delay = event_time - start_time
            start_time = event_time

            self.macro_string.append((key_code, delay, state))

    def __repr__(self):
        return '<MacroString ({0})>'.format(len(self.macro_string))

    def to_dict(self):
        return {
            'type': 'MacroKeyString',
            'key_list': self.macro_string
        }

    def exec(self, uinput=None, **kwargs):
        """
        Loop over keys, send to UInput
        """
        if uinput is None:
            uinput = evdev.UInput()

        for keycode, event_time, state in self.macro_string:
            uinput.write(evdev.ecodes.EV_KEY, keycode, state)
        uinput.syn()


# If it only opens a new tab in chroma - https://askubuntu.com/questions/540939/xdg-open-only-opens-a-new-tab-in-a-new-chromium-window-despite-passing-it-a-url
class MacroURL(MacroObject):
    """
    Is an object of a key event used in macros
    """
    def __init__(self, url):
        self.url = url

    def __repr__(self):
        return '<MacroURL {0}>'.format(self.url)

    def to_dict(self):
        return {
            'type': 'MacroURL',
            'url': self.url,
        }

    def exec(self, **kwargs):
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
        return '<MacroScript {0}>'.format(self.script)

    def to_dict(self):
        return {
            'type': 'MacroScript',
            'script': self.script,
            'args': self.args
        }

    def exec(self, **kwargs):
        """
        Run script
        """
        proc = subprocess.Popen(self.script + self.args, shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        proc.communicate()


class MacroDPIChange(MacroObject):
    def __init__(self, direction):
        self.direction = direction

    def __repr__(self):
        return '<MacroDPIChange {0}>'.format(self.direction)

    def to_dict(self):
        return {
            'type': 'MacroDPIChange',
            'direction': self.direction,
        }

    def exec(self, caller, **kwargs):
        """
        Open URL in the browser
        """

        if self.direction == 'up':
            caller.dpi_up()
        else:
            caller.dpi_down()


def macro_dict_to_obj(macro_dict):
    """
    Converts a macro string to its relevant object

    :param macro_dict: Macro string
    :type macro_dict: dict or list or tuple

    :return: Macro Object
    :rtype: object

    :raises ValueError: When a type isn't known
    """

    # pylint: disable=redefined-variable-type
    if isinstance(macro_dict, (tuple, list)):
        return [macro_dict_to_obj(item) for item in macro_dict]
    if macro_dict['type'] == 'MacroKeyString':
        return MacroKeyString.from_dict(macro_dict)
    elif macro_dict['type'] == 'MacroURL':
        return MacroURL.from_dict(macro_dict)
    elif macro_dict['type'] == 'MacroScript':
        return MacroScript.from_dict(macro_dict)
    elif macro_dict['type'] == 'MacroDPIChange':
        return MacroDPIChange.from_dict(macro_dict)
    else:
        raise ValueError("unknown type {0}".format(type(macro_dict)))


def macro_obj_to_dict(macro_list):
    if isinstance(macro_list, (tuple, list)):
        return [macro_obj_to_dict(item) for item in macro_list]
    elif isinstance(macro_list, MacroObject):
        return macro_list.to_dict()
    else:
        raise ValueError("Unknown type {0}".format(type(macro_list)))
