"""
Receives events from /dev/input/by-id/somedevice
"""

import datetime
import logging
import os
import select
import threading
import time
import sys
from openrazer_daemon.keyboard import KEY_MAPPING
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))  # TODO: figure out a better way to handle this
from evdev import ecodes, InputDevice

EPOLL_TIMEOUT = 0.01
SPIN_SLEEP = 0.005


class KeyWatcher(threading.Thread):
    """
    Thread to watch keyboard event files and return keypresses
    """

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
                self._logger.info("Grabbed device %s", str(device.path))
            except (IOError, OSError) as err:
                self._logger.exception("Error grabbing device %s", str(device.path), exc_info=err)

        # Loop
        while not self._shutdown:
            try:
                self._poll(self._event_file_map)
            except (OSError, IOError) as err:
                self._logger.exception("Error reading from device, stopping key watcher", exc_info=err)
                self._shutdown = True

            time.sleep(SPIN_SLEEP)

        self._logger.debug("Closing keywatcher")

        # Close files
        for device in self._event_file_map.values():
            device.close()

    # pylint: disable=no-member
    def _poll(self, event_file_map):
        r, _, _ = select.select(event_file_map, [], [], EPOLL_TIMEOUT)
        for fd in r:
            events = event_file_map[fd].read()
            if not events:
                break

            scan_code = None
            _ecodes = ecodes
            for event in events:
                if event.type == event.code == _ecodes.MSC_SCAN:  # Both EV_MSC and MSC_SCAN are 0x4, lucky us
                    scan_code = event.value  # Grab the scan code of the key
                    continue  # Skip that event

                if event.type == _ecodes.EV_KEY:  # Ignore other non-key events
                    self._parent.key_action(event.code, event.value, scan_code)
                    scan_code = None

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


class KeyboardKeyManager():
    """
    Key management class.

    This class deals with anything to do with keypresses. Currently it does:
    * Receiving keypresses from the KeyWatcher
    * Logic to deal with GameMode shortcut not working when macro's not enabled
    * Logic to deal with recording on the fly macros and replaying them
    * Send key press to binding manager
    """
    _thread = threading.Thread

    def __init__(self, device_id, event_files, parent, testing=False):
        self._device_id = device_id
        self._logger = logging.getLogger('razer.device{0}.keymanager'.format(device_id))
        self._parent = parent
        # self._parent.register_observer(self)
        self._testing = testing

        self._event_files = event_files
        self._access_lock = threading.Lock()
        self._keywatcher = KeyWatcher(device_id, event_files, self)
        if len(event_files) > 0:
            self._logger.debug("Starting KeyWatcher")
            self._keywatcher.start()
        else:
            self._logger.warning("No event files for KeyWatcher")

        self._temp_key_store_active = False
        self._temp_key_store = []
        self._temp_expire_time = datetime.timedelta(seconds=2)

        self.KEY_MAP = KEY_MAPPING

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

    def key_action(self, key_code, key_action, scan_code=None):
        """
        Process a key press event

        * Adds keypresses to the temporary key store (for ripple effect)
        * Sends key to the binding manager

        :param key_code: Key Event ID
        :type key_code: int

        :param key_action: Can either be 0, 1, 2: Up, Down, Autorepeat
        :type key_action: int

        :param scan_code: The value of MSC_SCAN
        :type scan_code: int
        """
        # self._logger.debug("Got key: {0}, state: {1}".format(key_name, key_action))
        _binding_manager = self._parent.binding_manager

        # Sets up game mode as when enabling macro keys it stops the key working
        if key_action == 1:
            if key_code == 189:  # GAMEMODE
                self._logger.debug("Got gamemode combo")
                self._parent.setGameMode(not self._parent.getGameMode())
                return

            if key_code == 194:  # BRIGHTNESSUP
                self._parent.setBrightness(max((self._parent.getBrightness() + 10), 0))
                return

            if key_code == 190:  # BRIGHTNESSDOWN
                self._parent.setBrightness(max((self._parent.getBrightness() - 10), 0))
                return

            if key_code == 188:  # MACROMODE
                _binding_manager.macro_mode = not _binding_manager.macro_mode
                return

        if _binding_manager.macro_mode:
            if key_code in (183, 184, 185, 186, 187):  # M1, M2, M3, M4, M5
                if _binding_manager.macro_key is None:
                    _binding_manager.macro_key = key_code

            elif _binding_manager.macro_key:
                profile = self._parent.getActiveProfile()
                mapping = self._parent.getActiveMap()
                key = _binding_manager.macro_key
                _str = str

                if key_action == 1:
                    self._parent.addAction(profile, mapping, _str(key), "key", _str(key_code))
                elif key_action == 0:
                    self._parent.addAction(profile, mapping, _str(key), "release", _str(key_code))

            else:
                self._logger.warning("On-the-fly macros are only supported for macro keys, please use a client for configuring other keys")
                _binding_manager.macro_mode = False

        else:
            x = self._thread(target=_binding_manager.key_action, args=(key_code, key_action, scan_code))
            x.start()

        now = datetime.datetime.now()
        # Remove expired keys from store
        try:
            # Get date and if its less than now its expired
            while self._temp_key_store[0][0] < now:
                self._temp_key_store.pop(0)
        except IndexError:
            pass

        if key_action == 1 and self.temp_key_store_state:
            self._temp_key_store.append((now + self._temp_expire_time, self.KEY_MAP[key_code]))

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
