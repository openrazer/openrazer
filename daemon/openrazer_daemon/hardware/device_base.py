"""
Hardware base class
"""
import re
import os
import types
import logging
import time
import json
import random

from openrazer_daemon.dbus_services.service import DBusService
import openrazer_daemon.dbus_services.dbus_methods
from openrazer_daemon.misc import effect_sync


# pylint: disable=too-many-instance-attributes
class RazerDevice(DBusService):
    """
    Base class

    Sets up the logger, sets up DBus
    """
    BUS_PATH = 'org.razer'
    OBJECT_PATH = '/org/razer/device/'
    METHODS = []

    EVENT_FILE_REGEX = None

    USB_VID = None
    USB_PID = None
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [-1, -1]

    WAVE_DIRS = (1, 2)

    RAZER_URLS = {
        "top_img": None,
        "side_img": None,
        "perspective_img": None
    }

    def __init__(self, device_path, device_number, config, testing=False, additional_interfaces=None, additional_methods=[]):

        self.logger = logging.getLogger('razer.device{0}'.format(device_number))
        self.logger.info("Initialising device.%d %s", device_number, self.__class__.__name__)

        # Serial cache
        self._serial = None

        self._observer_list = []
        self._effect_sync_propagate_up = False
        self._disable_notifications = False
        self.additional_interfaces = []
        if additional_interfaces is not None:
            self.additional_interfaces.extend(additional_interfaces)

        self.config = config
        self._testing = testing
        self._parent = None
        self._device_path = device_path
        self._device_number = device_number
        self.serial = self.get_serial()

        self.current_effect = 'spectrum'
        self.current_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
        self.current_effect_speed = 1
        self.current_wave_dir = 1
        self.has_normal_effects = False

        self.current_scroll_effect = 'spectrum'
        self.current_scroll_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
        self.current_scroll_effect_speed = 1
        self.current_scroll_wave_dir = 1
        self.has_scroll_effects = False

        self.current_logo_effect = 'spectrum'
        self.current_logo_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
        self.current_logo_effect_speed = 1
        self.current_logo_wave_dir = 1
        self.has_logo_effects = False

        self.current_left_effect = 'spectrum'
        self.current_left_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
        self.current_left_effect_speed = 1
        self.current_left_wave_dir = 1
        self.has_left_effects = False

        self.current_right_effect = 'spectrum'
        self.current_right_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
        self.current_right_effect_speed = 1
        self.current_right_wave_dir = 1
        self.has_right_effects = False

        self._effect_sync = effect_sync.EffectSync(self, device_number)

        self._is_closed = False

        # device methods available in all devices
        self.methods_internal = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name']
        self.methods_internal.extend(additional_methods)

        # Find event files in /dev/input/by-id/ by matching against regex
        self.event_files = []

        if self._testing:
            search_dir = os.path.join(device_path, 'input')
        else:
            search_dir = '/dev/input/by-id/'

        if os.path.exists(search_dir):
            for event_file in os.listdir(search_dir):
                if self.EVENT_FILE_REGEX is not None and self.EVENT_FILE_REGEX.match(event_file) is not None:
                    self.event_files.append(os.path.join(search_dir, event_file))

        object_path = os.path.join(self.OBJECT_PATH, self.serial)
        DBusService.__init__(self, self.BUS_PATH, object_path)

        # Set up methods to suspend and restore device operation
        self.suspend_args = {}
        self.method_args = {}

        methods = {
            # interface, method, callback, in-args, out-args
            ('razer.device.misc', 'getSerial', self.get_serial, None, 's'),
            ('razer.device.misc', 'suspendDevice', self.suspend_device, None, None),
            ('razer.device.misc', 'getDeviceMode', self.get_device_mode, None, 's'),
            ('razer.device.misc', 'getRazerUrls', self.get_image_json, None, 's'),
            ('razer.device.misc', 'setDeviceMode', self.set_device_mode, 'yy', None),
            ('razer.device.misc', 'resumeDevice', self.resume_device, None, None),
            ('razer.device.misc', 'getVidPid', self.get_vid_pid, None, 'ai'),
            ('razer.device.misc', 'getDriverVersion', openrazer_daemon.dbus_services.dbus_methods.version, None, 's'),
            ('razer.device.misc', 'hasDedicatedMacroKeys', self.dedicated_macro_keys, None, 'b'),
        }

        effect_methods = {
            ('razer.device.misc', 'getCurrentEffect', self.get_current_effect, None, 's'),
            ('razer.device.misc', 'getCurrentEffectColors', self.get_current_effect_colors, None, 'ay'),
            ('razer.device.misc', 'getCurrentEffectSpeed', self.get_current_effect_speed, None, 'i'),
            ('razer.device.misc', 'getCurrentWaveDir', self.get_current_wave_dir, None, 'i'),
        }

        logo_effect_methods = {
            ('razer.device.misc', 'getCurrentLogoEffect', self.get_current_logo_effect, None, 's'),
            ('razer.device.misc', 'getCurrentLogoEffectColors', self.get_current_logo_effect_colors, None, 'ay'),
            ('razer.device.misc', 'getCurrentLogoEffectSpeed', self.get_current_logo_effect_speed, None, 'i'),
        }

        scroll_effect_methods = {
            ('razer.device.misc', 'getCurrentScrollEffect', self.get_current_scroll_effect, None, 's'),
            ('razer.device.misc', 'getCurrentScrollEffectColors', self.get_current_scroll_effect_colors, None, 'ay'),
            ('razer.device.misc', 'getCurrentScrollEffectSpeed', self.get_current_scroll_effect_speed, None, 'i'),
        }

        left_effect_methods = {
            ('razer.device.misc', 'getCurrentLeftEffect', self.get_current_left_effect, None, 's'),
            ('razer.device.misc', 'getCurrentLeftEffectColors', self.get_current_left_effect_colors, None, 'ay'),
            ('razer.device.misc', 'getCurrentLeftEffectSpeed', self.get_current_left_effect_speed, None, 'i'),
        }

        right_effect_methods = {
            ('razer.device.misc', 'getCurrentRightEffect', self.get_current_right_effect, None, 's'),
            ('razer.device.misc', 'getCurrentRightEffectColors', self.get_current_right_effect_colors, None, 'ay'),
            ('razer.device.misc', 'getCurrentRightEffectSpeed', self.get_current_right_effect_speed, None, 'i'),
        }

        for m in methods:
            self.logger.debug("Adding {}.{} method to DBus".format(m[0], m[1]))
            self.add_dbus_method(m[0], m[1], m[2], in_signature=m[3], out_signature=m[4])

        if 'set_static_effect' in self.METHODS:
            self.has_normal_effects = True
            for m in effect_methods:
                self.logger.debug("Adding {}.{} method to DBus".format(m[0], m[1]))
                self.add_dbus_method(m[0], m[1], m[2], in_signature=m[3], out_signature=m[4])

        if 'set_logo_static' in self.METHODS or 'set_logo_static_naga_hex_v2' in self.METHODS:
            self.has_logo_effects = True
            for m in logo_effect_methods:
                self.logger.debug("Adding {}.{} method to DBus".format(m[0], m[1]))
                self.add_dbus_method(m[0], m[1], m[2], in_signature=m[3], out_signature=m[4])

        if 'set_scroll_static' in self.METHODS or 'set_logo_static_naga_hex_v2' in self.METHODS:
            self.has_scroll_effects = True
            for m in scroll_effect_methods:
                self.logger.debug("Adding {}.{} method to DBus".format(m[0], m[1]))
                self.add_dbus_method(m[0], m[1], m[2], in_signature=m[3], out_signature=m[4])

        if 'set_left_static' in self.METHODS:
            self.has_left_effects = True
            for m in left_effect_methods:
                self.logger.debug("Adding {}.{} method to DBus".format(m[0], m[1]))
                self.add_dbus_method(m[0], m[1], m[2], in_signature=m[3], out_signature=m[4])

        if 'set_right_static' in self.METHODS:
            self.has_right_effects = True
            for m in right_effect_methods:
                self.logger.debug("Adding {}.{} method to DBus".format(m[0], m[1]))
                self.add_dbus_method(m[0], m[1], m[2], in_signature=m[3], out_signature=m[4])

        # Load additional DBus methods
        self.load_methods()

        # load last effect
        if self.has_normal_effects:
            if self.config.has_section(self._serial):
                try:
                    self.current_effect = self.config[self._serial]['effect']
                except KeyError:
                    self.current_effect = 'spectrum'
                    pass
                try:
                    for index, item in enumerate(self.config[self._serial]['colors'].split(" ")):
                        self.current_effect_colors[index] = int(item)
                        if not 0 <= self.current_effect_colors[index] <= 255:
                            raise ValueError('Color out of range')

                    if len(self.current_effect_colors) != 9:
                        raise ValueError('There must be exactly 9 colors')

                except ValueError:
                    # invalid colors. reinitialize
                    self.current_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
                    self.logger.info("%s: Invalid colors; restoring to defaults.", self.__class__.__name__)
                    pass

                self.current_effect_speed = int(self.config[self._serial]['speed'])
                self.current_wave_dir = int(self.config[self._serial]['wave_dir'])
                effect_func_name = 'set' + self.current_effect[0].upper() + self.current_effect[1:]
            else:
                self.current_effect = 'spectrum'
                effect_func_name = 'setSpectrum'

            effect_func = getattr(self, effect_func_name, None)

            if effect_func == None:
                self.logger.info("%s: Invalid effect name %s; restoring to Spectrum.", self.__class__.__name__, effect_func_name)
                effect_func_name = 'setSpectrum'
                effect_func = getattr(self, effect_func_name, None)

            if not effect_func == None:
                if effect_func_name == 'setNone' or effect_func_name == 'setSpectrum' or effect_func_name == 'setBlinking' or effect_func_name == 'setBreathRandom':
                    effect_func()
                elif effect_func_name == 'setStatic' or effect_func_name == 'setBlinking' or effect_func_name == 'setBreathSingle':
                    effect_func(self.current_effect_colors[0], self.current_effect_colors[1], self.current_effect_colors[2])
                elif effect_func_name == 'setReactive':
                    effect_func(self.current_effect_colors[0], self.current_effect_colors[1], self.current_effect_colors[2], self.current_effect_speed)
                elif effect_func_name == 'setBreathDual':
                    effect_func(self.current_effect_colors[0], self.current_effect_colors[1], self.current_effect_colors[2], self.current_effect_colors[3], self.current_effect_colors[4], self.current_effect_colors[5])
                elif effect_func_name == 'setBreathTriple':
                    effect_func(self.current_effect_colors[0], self.current_effect_colors[1], self.current_effect_colors[2], self.current_effect_colors[3], self.current_effect_colors[4], self.current_effect_colors[5], self.current_effect_colors[6], self.current_effect_colors[7], self.current_effect_colors[8])
                elif effect_func_name == 'setWave':
                    effect_func(self.current_wave_dir)
                elif effect_func_name == 'setStarlightRandom':
                    effect_func(self.current_effect_speed)
                elif effect_func_name == 'setStarlightSingle':
                    effect_func(self.current_effect_speed, self.current_effect_colors[0], self.current_effect_colors[1], self.current_effect_colors[2])
                elif effect_func_name == 'setStarlightDual':
                    effect_func(self.current_effect_speed, self.current_effect_colors[0], self.current_effect_colors[1], self.current_effect_colors[2], self.current_effect_colors[3], self.current_effect_colors[4], self.current_effect_colors[5])

        # load last logo effect
        if self.has_logo_effects:
            if self.config.has_section(self._serial):
                try:
                    self.current_logo_effect = self.config[self._serial]['logo_effect']
                except KeyError:
                    self.current_logo_effect = 'spectrum'
                    pass
                try:
                    for index, item in enumerate(self.config[self._serial]['logo_colors'].split(" ")):
                        self.current_logo_effect_colors[index] = int(item)
                        if not 0 <= self.current_logo_effect_colors[index] <= 255:
                            raise ValueError('Color out of range')

                    if len(self.current_logo_effect_colors) != 9:
                        raise ValueError('There must be exactly 9 colors')

                except ValueError:
                    # invalid colors. reinitialize
                    self.current_logo_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
                    self.logger.info("%s: Invalid logo colors; restoring to defaults.", self.__class__.__name__)
                    pass

                except KeyError:
                    self.current_logo_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
                    pass

                try:
                    self.current_logo_effect_speed = int(self.config[self._serial]['logo_speed'])
                    self.current_logo_wave_dir = int(self.config[self._serial]['logo_wave_dir'])

                except KeyError:
                    pass

                effect_func_name = 'setLogo' + self.current_logo_effect[0].upper() + self.current_logo_effect[1:]
            else:
                self.current_logo_effect = 'spectrum'
                effect_func_name = 'setLogoSpectrum'

            effect_func = getattr(self, effect_func_name, None)

            if effect_func == None:
                self.logger.info("%s: Invalid logo effect name %s; restoring to Spectrum.", self.__class__.__name__, effect_func_name)
                effect_func_name = 'setLogoSpectrum'
                effect_func = getattr(self, effect_func_name, None)

            if not effect_func == None:
                if effect_func_name == 'setLogoNone' or effect_func_name == 'setLogoSpectrum' or effect_func_name == 'setLogoBlinking' or effect_func_name == 'setLogoBreathRandom':
                    effect_func()
                elif effect_func_name == 'setLogoStatic' or effect_func_name == 'setLogoBlinking' or effect_func_name == 'setLogoPulsate' or effect_func_name == 'setLogoBreathSingle':
                    effect_func(self.current_logo_effect_colors[0], self.current_logo_effect_colors[1], self.current_logo_effect_colors[2])
                elif effect_func_name == 'setLogoReactive':
                    effect_func(self.current_logo_effect_colors[0], self.current_logo_effect_colors[1], self.current_logo_effect_colors[2], self.current_logo_effect_speed)
                elif effect_func_name == 'setLogoBreathDual':
                    effect_func(self.current_logo_effect_colors[0], self.current_logo_effect_colors[1], self.current_logo_effect_colors[2], self.current_logo_effect_colors[3], self.current_logo_effect_colors[4], self.current_logo_effect_colors[5])
                elif effect_func_name == 'setLogoWave':
                    effect_func(self.current_logo_wave_dir)
                else:
                    raise Exception("we can't handle this effect?")

        # load last scroll effect
        if self.has_scroll_effects:
            if self.config.has_section(self._serial):
                try:
                    self.current_scroll_effect = self.config[self._serial]['scroll_effect']
                except KeyError:
                    self.current_scroll_effect = 'spectrum'
                    pass
                try:
                    for index, item in enumerate(self.config[self._serial]['scroll_colors'].split(" ")):
                        self.current_scroll_effect_colors[index] = int(item)
                        if not 0 <= self.current_scroll_effect_colors[index] <= 255:
                            raise ValueError('Color out of range')

                    if len(self.current_scroll_effect_colors) != 9:
                        raise ValueError('There must be exactly 9 colors')

                except ValueError:
                    # invalid colors. reinitialize
                    self.current_scroll_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
                    self.logger.info("%s: Invalid scroll colors; restoring to defaults.", self.__class__.__name__)
                    pass

                except KeyError:
                    self.current_scroll_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
                    pass

                try:
                    self.current_scroll_effect_speed = int(self.config[self._serial]['scroll_speed'])
                    self.current_scroll_wave_dir = int(self.config[self._serial]['scroll_wave_dir'])

                except KeyError:
                    pass

                effect_func_name = 'setScroll' + self.current_scroll_effect[0].upper() + self.current_scroll_effect[1:]
            else:
                self.current_scroll_effect = 'spectrum'
                effect_func_name = 'setScrollSpectrum'

            effect_func = getattr(self, effect_func_name, None)

            if effect_func == None:
                self.logger.info("%s: Invalid scroll effect name %s; restoring to Spectrum.", self.__class__.__name__, effect_func_name)
                effect_func_name = 'setScrollSpectrum'
                effect_func = getattr(self, effect_func_name, None)

            if not effect_func == None:
                if effect_func_name == 'setScrollNone' or effect_func_name == 'setScrollSpectrum' or effect_func_name == 'setScrollBlinking' or effect_func_name == 'setScrollBreathRandom':
                    effect_func()
                elif effect_func_name == 'setScrollStatic' or effect_func_name == 'setScrollBlinking' or effect_func_name == 'setScrollPulsate' or effect_func_name == 'setScrollBreathSingle':
                    effect_func(self.current_scroll_effect_colors[0], self.current_scroll_effect_colors[1], self.current_scroll_effect_colors[2])
                elif effect_func_name == 'setScrollReactive':
                    effect_func(self.current_scroll_effect_colors[0], self.current_scroll_effect_colors[1], self.current_scroll_effect_colors[2], self.current_scroll_effect_speed)
                elif effect_func_name == 'setScrollBreathDual':
                    effect_func(self.current_scroll_effect_colors[0], self.current_scroll_effect_colors[1], self.current_scroll_effect_colors[2], self.current_scroll_effect_colors[3], self.current_scroll_effect_colors[4], self.current_scroll_effect_colors[5])
                elif effect_func_name == 'setScrollWave':
                    effect_func(self.current_scroll_wave_dir)
                else:
                    raise Exception("we can't handle this effect?")

        # load last left effect
        if self.has_left_effects:
            if self.config.has_section(self._serial):
                try:
                    self.current_left_effect = self.config[self._serial]['left_effect']
                except KeyError:
                    self.current_left_effect = 'spectrum'
                    pass
                try:
                    for index, item in enumerate(self.config[self._serial]['left_colors'].split(" ")):
                        self.current_left_effect_colors[index] = int(item)
                        if not 0 <= self.current_left_effect_colors[index] <= 255:
                            raise ValueError('Color out of range')

                    if len(self.current_left_effect_colors) != 9:
                        raise ValueError('There must be exactly 9 colors')

                except ValueError:
                    # invalid colors. reinitialize
                    self.current_left_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
                    self.logger.info("%s: Invalid left colors; restoring to defaults.", self.__class__.__name__)
                    pass

                except KeyError:
                    self.current_left_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
                    pass

                try:
                    self.current_left_effect_speed = int(self.config[self._serial]['left_speed'])
                    self.current_left_wave_dir = int(self.config[self._serial]['left_wave_dir'])

                except KeyError:
                    pass

                effect_func_name = 'setLeft' + self.current_left_effect[0].upper() + self.current_left_effect[1:]
            else:
                self.current_left_effect = 'spectrum'
                effect_func_name = 'setLeftSpectrum'

            effect_func = getattr(self, effect_func_name, None)

            if effect_func == None:
                self.logger.info("%s: Invalid left effect name %s; restoring to Spectrum.", self.__class__.__name__, effect_func_name)
                effect_func_name = 'setLeftSpectrum'
                effect_func = getattr(self, effect_func_name, None)

            if not effect_func == None:
                if effect_func_name == 'setLeftNone' or effect_func_name == 'setLeftSpectrum' or effect_func_name == 'setLeftBlinking' or effect_func_name == 'setLeftBreathRandom':
                    effect_func()
                elif effect_func_name == 'setLeftStatic' or effect_func_name == 'setLeftBlinking' or effect_func_name == 'setLeftPulsate' or effect_func_name == 'setLeftBreathSingle':
                    effect_func(self.current_left_effect_colors[0], self.current_left_effect_colors[1], self.current_left_effect_colors[2])
                elif effect_func_name == 'setLeftReactive':
                    effect_func(self.current_left_effect_colors[0], self.current_left_effect_colors[1], self.current_left_effect_colors[2], self.current_left_effect_speed)
                elif effect_func_name == 'setLeftBreathDual':
                    effect_func(self.current_left_effect_colors[0], self.current_left_effect_colors[1], self.current_left_effect_colors[2], self.current_left_effect_colors[3], self.current_left_effect_colors[4], self.current_left_effect_colors[5])
                elif effect_func_name == 'setLeftWave':
                    effect_func(self.current_left_wave_dir)
                else:
                    raise Exception("we can't handle this effect?")

        # load last right effect
        if self.has_right_effects:
            if self.config.has_section(self._serial):
                try:
                    self.current_right_effect = self.config[self._serial]['right_effect']
                except KeyError:
                    self.current_right_effect = 'spectrum'
                    pass
                try:
                    for index, item in enumerate(self.config[self._serial]['right_colors'].split(" ")):
                        self.current_right_effect_colors[index] = int(item)
                        if not 0 <= self.current_right_effect_colors[index] <= 255:
                            raise ValueError('Color out of range')

                    if len(self.current_right_effect_colors) != 9:
                        raise ValueError('There must be exactly 9 colors')

                except ValueError:
                    # invalid colors. reinitialize
                    self.current_right_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
                    self.logger.info("%s: Invalid right colors; restoring to defaults.", self.__class__.__name__)
                    pass

                except KeyError:
                    self.current_right_effect_colors = [0, 255, 0, 0, 255, 255, 0, 0, 255]
                    pass

                try:
                    self.current_right_effect_speed = int(self.config[self._serial]['right_speed'])
                    self.current_right_wave_dir = int(self.config[self._serial]['right_wave_dir'])

                except KeyError:
                    pass

                effect_func_name = 'setRight' + self.current_right_effect[0].upper() + self.current_right_effect[1:]
            else:
                self.current_right_effect = 'spectrum'
                effect_func_name = 'setRightSpectrum'

            effect_func = getattr(self, effect_func_name, None)

            if effect_func == None:
                self.logger.info("%s: Invalid right effect name %s; restoring to Spectrum.", self.__class__.__name__, effect_func_name)
                effect_func_name = 'setRightSpectrum'
                effect_func = getattr(self, effect_func_name, None)

            if not effect_func == None:
                if effect_func_name == 'setRightNone' or effect_func_name == 'setRightSpectrum' or effect_func_name == 'setRightBlinking' or effect_func_name == 'setRightBreathRandom':
                    effect_func()
                elif effect_func_name == 'setRightStatic' or effect_func_name == 'setRightBlinking' or effect_func_name == 'setRightPulsate' or effect_func_name == 'setRightBreathSingle':
                    effect_func(self.current_right_effect_colors[0], self.current_right_effect_colors[1], self.current_right_effect_colors[2])
                elif effect_func_name == 'setRightReactive':
                    effect_func(self.current_right_effect_colors[0], self.current_right_effect_colors[1], self.current_right_effect_colors[2], self.current_right_effect_speed)
                elif effect_func_name == 'setRightBreathDual':
                    effect_func(self.current_right_effect_colors[0], self.current_right_effect_colors[1], self.current_right_effect_colors[2], self.current_right_effect_colors[3], self.current_right_effect_colors[4], self.current_right_effect_colors[5])
                elif effect_func_name == 'setRightWave':
                    effect_func(self.current_right_wave_dir)
                else:
                    raise Exception("we can't handle this effect?")

    def send_effect_event(self, effect_name, *args):
        """
        Send effect event

        :param effect_name: Effect name
        :type effect_name: str

        :param args: Effect arguments
        :type args: list
        """
        payload = ['effect', self, effect_name]
        payload.extend(args)

        self.notify_observers(tuple(payload))

    def dedicated_macro_keys(self):
        """
        Returns if the device has dedicated macro keys

        :return: Macro keys
        :rtype: bool
        """
        return self.DEDICATED_MACRO_KEYS

    def get_current_effect(self):
        """
        Get the device's current effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_effect")

        return self.current_effect

    def get_current_effect_colors(self):
        """
        Get the device's current effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_effect_colors")

        return self.current_effect_colors

    def get_current_effect_speed(self):
        """
        Get the device's current effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_effect_speed")

        return self.current_effect_speed

    def get_current_wave_dir(self):
        """
        Get the device's current wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_wave_dir")

        return self.current_wave_dir

    def get_current_logo_effect(self):
        """
        Get the device's current logo effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_logo_effect")

        return self.current_logo_effect

    def get_current_logo_effect_colors(self):
        """
        Get the device's current logo effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_logo_effect_colors")

        return self.current_logo_effect_colors

    def get_current_logo_effect_speed(self):
        """
        Get the device's current logo effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_logo_effect_speed")

        return self.current_logo_effect_speed

    def get_current_logo_wave_dir(self):
        """
        Get the device's current logo wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_logo_wave_dir")

        return self.current_logo_wave_dir

    def get_current_scroll_effect(self):
        """
        Get the device's current scroll effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_scroll_effect")

        return self.current_scroll_effect

    def get_current_scroll_effect_colors(self):
        """
        Get the device's current scroll effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_scroll_effect_colors")

        return self.current_scroll_effect_colors

    def get_current_scroll_effect_speed(self):
        """
        Get the device's current scroll effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_scroll_effect_speed")

        return self.current_scroll_effect_speed

    def get_current_scroll_wave_dir(self):
        """
        Get the device's current scroll wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_scroll_wave_dir")

        return self.current_scroll_wave_dir

    def get_current_left_effect(self):
        """
        Get the device's current left effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_left_effect")

        return self.current_left_effect

    def get_current_left_effect_colors(self):
        """
        Get the device's current left effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_left_effect_colors")

        return self.current_left_effect_colors

    def get_current_left_effect_speed(self):
        """
        Get the device's current left effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_left_effect_speed")

        return self.current_left_effect_speed

    def get_current_left_wave_dir(self):
        """
        Get the device's current left wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_left_wave_dir")

        return self.current_left_wave_dir

    def get_current_right_effect(self):
        """
        Get the device's current right effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_right_effect")

        return self.current_right_effect

    def get_current_right_effect_colors(self):
        """
        Get the device's current right effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_right_effect_colors")

        return self.current_right_effect_colors

    def get_current_right_effect_speed(self):
        """
        Get the device's current right effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_right_effect_speed")

        return self.current_right_effect_speed

    def get_current_right_wave_dir(self):
        """
        Get the device's current right wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_right_wave_dir")

        return self.current_right_wave_dir

    @property
    def effect_sync(self):
        """
        Propagate the obsever call upwards, used for syncing effects

        :return: Effects sync flag
        :rtype: bool
        """
        return self._effect_sync_propagate_up

    @effect_sync.setter
    def effect_sync(self, value):
        """
        Setting to true will propagate observer events upwards

        :param value: Effect sync
        :type value: bool
        """
        self._effect_sync_propagate_up = value

    @property
    def disable_notify(self):
        """
        Disable notifications flag

        :return: Flag
        :rtype: bool
        """
        return self._disable_notifications

    @disable_notify.setter
    def disable_notify(self, value):
        """
        Set the disable notifications flag

        :param value: Disable
        :type value: bool
        """
        self._disable_notifications = value

    def get_driver_path(self, driver_filename):
        """
        Get the path to a driver file

        :param driver_filename: Name of driver file
        :type driver_filename: str

        :return: Full path to driver
        :rtype: str
        """
        return os.path.join(self._device_path, driver_filename)

    def get_serial(self):
        """
        Get serial number for device

        :return: String of the serial number
        :rtype: str
        """
        # TODO raise exception if serial can't be got and handle during device add
        if self._serial is None:
            serial_path = os.path.join(self._device_path, 'device_serial')
            count = 0
            serial = ''
            while len(serial) == 0:
                if count >= 5:
                    break

                try:
                    with open(serial_path, 'r') as f:
                        serial = f.read().strip()
                except (PermissionError, OSError) as err:
                    self.logger.warning('getting serial: {0}'.format(err))
                    serial = ''
                except UnicodeDecodeError as err:
                    self.logger.warning('malformed serial: {0}'.format(err))
                    serial = ''

                count += 1
                time.sleep(0.1)

                if len(serial) == 0:
                    self.logger.debug('getting serial: {0} count:{1}'.format(serial, count))

            if serial == '' or serial == 'Default string' or serial == 'empty (NULL)' or serial == 'As printed in the D cover':
                serial = 'UNKWN{0:012}'.format(random.randint(0, 4096))

            self._serial = serial.replace(' ', '_')

        return self._serial

    def get_device_mode(self):
        """
        Get device mode

        :return: String of device mode and arg separated by colon, e.g. 0:0 or 3:0
        :rtype: str
        """
        device_mode_path = os.path.join(self._device_path, 'device_mode')
        with open(device_mode_path, 'r') as mode_file:
            count = 0
            mode = mode_file.read().strip()
            while len(mode) == 0:
                if count >= 3:
                    break
                mode = mode_file.read().strip()

                count += 1
                time.sleep(0.1)

            return mode

    def set_device_mode(self, mode_id, param):
        """
        Set device mode

        :param mode_id: Device mode ID
        :type mode_id: int

        :param param: Device mode parameter
        :type param: int
        """
        device_mode_path = os.path.join(self._device_path, 'device_mode')
        with open(device_mode_path, 'wb') as mode_file:

            # Do some validation (even though its in the driver)
            if mode_id not in (0, 3):
                mode_id = 0
            if param != 0:
                param = 0

            mode_file.write(bytes([mode_id, param]))

    def get_vid_pid(self):
        """
        Get the usb VID PID

        :return: List of VID PID
        :rtype: list of int
        """
        result = [self.USB_VID, self.USB_PID]
        return result

    def get_image_json(self):
        return json.dumps(self.RAZER_URLS)

    def load_methods(self):
        """
        Load DBus methods

        Goes through the list in self.methods_internal and self.METHODS and loads each effect and adds it to DBus
        """
        available_functions = {}
        methods = dir(openrazer_daemon.dbus_services.dbus_methods)
        for method in methods:
            potential_function = getattr(openrazer_daemon.dbus_services.dbus_methods, method)
            if isinstance(potential_function, types.FunctionType) and hasattr(potential_function, 'endpoint') and potential_function.endpoint:
                available_functions[potential_function.__name__] = potential_function

        self.methods_internal.extend(self.METHODS)
        for method_name in self.methods_internal:
            try:
                new_function = available_functions[method_name]
                self.logger.debug("Adding %s.%s method to DBus", new_function.interface, new_function.name)
                self.add_dbus_method(new_function.interface, new_function.name, new_function, new_function.in_sig, new_function.out_sig, new_function.byte_arrays)
            except KeyError as e:
                self.logger.warning("Couldn't add method to DBus: %s", e)
                pass

    def suspend_device(self):
        """
        Suspend device
        """
        self.logger.info("Suspending %s", self.__class__.__name__)
        self._suspend_device()

    def resume_device(self):
        """
        Resume device
        """
        self.logger.info("Resuming %s", self.__class__.__name__)
        self._resume_device()

    def _suspend_device(self):
        """
        Suspend device
        """
        raise NotImplementedError()

    def _resume_device(self):
        """
        Resume device
        """
        raise NotImplementedError()

    def _close(self):
        """
        To be overridden by any subclasses to do cleanup
        """
        # Clear observer list
        self._observer_list.clear()

    def close(self):
        """
        Close any resources opened by subclasses
        """
        if not self._is_closed:
            self._close()

            self._is_closed = True

    def register_observer(self, observer):
        """
        Observer design pattern, register

        :param observer: Observer
        :type observer: object
        """
        if observer not in self._observer_list:
            self._observer_list.append(observer)

    def register_parent(self, parent):
        """
        Register the parent as an observer to be optionally notified (sends to other devices)

        :param parent: Observer
        :type parent: object
        """
        self._parent = parent

    def remove_observer(self, observer):
        """
        Obsever design pattern, remove

        :param observer: Observer
        :type observer: object
        """
        try:
            self._observer_list.remove(observer)
        except ValueError:
            pass

    def notify_observers(self, msg):
        """
        Notify observers with msg

        :param msg: Tuple with first element a string
        :type msg: tuple
        """
        if not self._disable_notifications:
            self.logger.debug("Sending observer message: %s", str(msg))

            if self._effect_sync_propagate_up and self._parent is not None:
                self._parent.notify_parent(msg)

            for observer in self._observer_list:
                observer.notify(msg)

    def notify(self, msg):
        """
        Receive observer messages

        :param msg: Tuple with first element a string
        :type msg: tuple
        """
        self.logger.debug("Got observer message: %s", str(msg))

        for observer in self._observer_list:
            observer.notify(msg)

    @classmethod
    def match(cls, device_id, dev_path):
        """
        Match against the device ID

        :param device_id: Device ID like 0000:0000:0000.0000
        :type device_id: str

        :param dev_path: Device path. Normally '/sys/bus/hid/devices/0000:0000:0000.0000'
        :type dev_path: str

        :return: True if its the correct device ID
        :rtype: bool
        """
        pattern = r'^[0-9A-F]{4}:' + '{0:04X}'.format(cls.USB_VID) + ':' + '{0:04X}'.format(cls.USB_PID) + r'\.[0-9A-F]{4}$'

        if re.match(pattern, device_id) is not None:
            if 'device_type' in os.listdir(dev_path):
                return True

        return False

    def __del__(self):
        self.close()

    def __repr__(self):
        return "{0}:{1}".format(self.__class__.__name__, self.serial)


class RazerDeviceSpecialBrightnessSuspend(RazerDevice):
    """
    Class for suspend using brightness

    Suspend functions
    """

    def __init__(self, device_path, device_number, config, testing=False, additional_interfaces=None, additional_methods=[]):
        super().__init__(device_path, device_number, config, testing, additional_interfaces, additional_methods)

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = openrazer_daemon.dbus_services.dbus_methods.get_brightness(self)

        # Todo make it context?
        self.disable_notify = True
        openrazer_daemon.dbus_services.dbus_methods.set_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        brightness = self.suspend_args.get('brightness', 100)

        self.disable_notify = True
        openrazer_daemon.dbus_services.dbus_methods.set_brightness(self, brightness)
        self.disable_notify = False


class RazerDeviceBrightnessSuspend(RazerDeviceSpecialBrightnessSuspend):
    """
    Class for devices that have get_brightness and set_brightness
    Inherits from RazerDeviceSpecialBrightnessSuspend
    """

    def __init__(self, device_path, device_number, config, testing=False, additional_interfaces=None, additional_methods=[]):
        additional_methods.extend(['get_brightness', 'set_brightness'])
        super().__init__(device_path, device_number, config, testing, additional_interfaces, additional_methods)
