"""
Hardware base class
"""
import re
import os
import types
import inspect
import logging
import time
import json
import random

from openrazer_daemon.dbus_services.service import DBusService
import openrazer_daemon.dbus_services.dbus_methods
from openrazer_daemon.misc import effect_sync


# pylint: disable=too-many-instance-attributes
# pylint: disable=E1102
# See https://github.com/PyCQA/pylint/issues/1493
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

    ZONES = ('backlight', 'logo', 'scroll', 'left', 'right', 'charging', 'fast_charging', 'fully_charged')

    DEVICE_IMAGE = None

    def __init__(self, device_path, device_number, config, persistence, testing=False, additional_interfaces=None, additional_methods=[]):

        self.logger = logging.getLogger('razer.device{0}'.format(device_number))
        self.logger.info("Initialising device.%d %s", device_number, self.__class__.__name__)

        # Serial cache
        self._serial = None

        # Local storage key name
        self.storage_name = "UnknownDevice"

        self._observer_list = []
        self._effect_sync_propagate_up = False
        self._disable_notifications = False
        self.additional_interfaces = []
        if additional_interfaces is not None:
            self.additional_interfaces.extend(additional_interfaces)

        self.config = config
        self.persistence = persistence
        self._testing = testing
        self._parent = None
        self._device_path = device_path
        self._device_number = device_number
        self.serial = self.get_serial()

        if self.USB_PID == 0x0f07:
            self.storage_name = "ChromaMug"
        elif self.USB_PID == 0x0013:
            self.storage_name = "Orochi2011"
        elif self.USB_PID == 0x0016:
            self.storage_name = "DeathAdder35G"
        elif self.USB_PID == 0x0024 or self.USB_PID == 0x0025:
            self.storage_name = "Mamba2012"
        else:
            self.storage_name = self.serial

        self.zone = dict()

        for i in self.ZONES:
            self.zone[i] = {
                "present": False,
                "active": True,
                "brightness": 75.0,
                "effect": 'spectrum',
                "colors": [0, 255, 0, 0, 255, 255, 0, 0, 255],
                "speed": 1,
                "wave_dir": 1,
            }

        self.dpi = [1800, 1800]
        self.poll_rate = 500

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
            ('razer.device.misc', 'getDeviceImage', self.get_device_image, None, 's'),
            ('razer.device.misc', 'setDeviceMode', self.set_device_mode, 'yy', None),
            ('razer.device.misc', 'resumeDevice', self.resume_device, None, None),
            ('razer.device.misc', 'getVidPid', self.get_vid_pid, None, 'ai'),
            ('razer.device.misc', 'getDriverVersion', openrazer_daemon.dbus_services.dbus_methods.version, None, 's'),
            ('razer.device.misc', 'hasDedicatedMacroKeys', self.dedicated_macro_keys, None, 'b'),
            # Deprecated API, but kept for backwards compatibility
            ('razer.device.misc', 'getRazerUrls', self.get_image_json, None, 's'),

            ('razer.device.lighting.chroma', 'restoreLastEffect', self.restore_effect, None, None),
        }

        effect_methods = {
            "backlight": {
                ('razer.device.lighting.chroma', 'getEffect', self.get_current_effect, None, 's'),
                ('razer.device.lighting.chroma', 'getEffectColors', self.get_current_effect_colors, None, 'ay'),
                ('razer.device.lighting.chroma', 'getEffectSpeed', self.get_current_effect_speed, None, 'i'),
                ('razer.device.lighting.chroma', 'getWaveDir', self.get_current_wave_dir, None, 'i'),
            },

            "logo": {
                ('razer.device.lighting.logo', 'getLogoEffect', self.get_current_logo_effect, None, 's'),
                ('razer.device.lighting.logo', 'getLogoEffectColors', self.get_current_logo_effect_colors, None, 'ay'),
                ('razer.device.lighting.logo', 'getLogoEffectSpeed', self.get_current_logo_effect_speed, None, 'i'),
                ('razer.device.lighting.logo', 'getLogoWaveDir', self.get_current_logo_wave_dir, None, 'i'),
            },

            "scroll": {
                ('razer.device.lighting.scroll', 'getScrollEffect', self.get_current_scroll_effect, None, 's'),
                ('razer.device.lighting.scroll', 'getScrollEffectColors', self.get_current_scroll_effect_colors, None, 'ay'),
                ('razer.device.lighting.scroll', 'getScrollEffectSpeed', self.get_current_scroll_effect_speed, None, 'i'),
                ('razer.device.lighting.scroll', 'getScrollWaveDir', self.get_current_scroll_wave_dir, None, 'i'),
            },

            "left": {
                ('razer.device.lighting.left', 'getLeftEffect', self.get_current_left_effect, None, 's'),
                ('razer.device.lighting.left', 'getLeftEffectColors', self.get_current_left_effect_colors, None, 'ay'),
                ('razer.device.lighting.left', 'getLeftEffectSpeed', self.get_current_left_effect_speed, None, 'i'),
                ('razer.device.lighting.left', 'getLeftWaveDir', self.get_current_left_wave_dir, None, 'i'),
            },

            "right": {
                ('razer.device.lighting.right', 'getRightEffect', self.get_current_right_effect, None, 's'),
                ('razer.device.lighting.right', 'getRightEffectColors', self.get_current_right_effect_colors, None, 'ay'),
                ('razer.device.lighting.right', 'getRightEffectSpeed', self.get_current_right_effect_speed, None, 'i'),
                ('razer.device.lighting.right', 'getRightWaveDir', self.get_current_right_wave_dir, None, 'i'),
            },

            "charging": {
                ('razer.device.lighting.charging', 'getChargingEffect', self.get_current_charging_effect, None, 's'),
                ('razer.device.lighting.charging', 'getChargingEffectColors', self.get_current_charging_effect_colors, None, 'ay'),
                ('razer.device.lighting.charging', 'getChargingEffectSpeed', self.get_current_charging_effect_speed, None, 'i'),
                ('razer.device.lighting.charging', 'getChargingWaveDir', self.get_current_charging_wave_dir, None, 'i'),
            },

            "fast_charging": {
                ('razer.device.lighting.fast_charging', 'getFastChargingEffect', self.get_current_fast_charging_effect, None, 's'),
                ('razer.device.lighting.fast_charging', 'getFastChargingEffectColors', self.get_current_fast_charging_effect_colors, None, 'ay'),
                ('razer.device.lighting.fast_charging', 'getFastChargingEffectSpeed', self.get_current_fast_charging_effect_speed, None, 'i'),
                ('razer.device.lighting.fast_charging', 'getFastChargingWaveDir', self.get_current_fast_charging_wave_dir, None, 'i'),
            },

            "fully_charged": {
                ('razer.device.lighting.fully_charged', 'getFullyChargedEffect', self.get_current_fully_charged_effect, None, 's'),
                ('razer.device.lighting.fully_charged', 'getFullyChargedEffectColors', self.get_current_fully_charged_effect_colors, None, 'ay'),
                ('razer.device.lighting.fully_charged', 'getFullyChargedEffectSpeed', self.get_current_fully_charged_effect_speed, None, 'i'),
                ('razer.device.lighting.fully_charged', 'getFullyChargedWaveDir', self.get_current_fully_charged_wave_dir, None, 'i'),
            }
        }

        for m in methods:
            self.logger.debug("Adding {}.{} method to DBus".format(m[0], m[1]))
            self.add_dbus_method(m[0], m[1], m[2], in_signature=m[3], out_signature=m[4])

        # this check is separate from the rest because backlight effects don't have prefixes in their names
        if 'set_static_effect' in self.METHODS or 'bw_set_static' in self.METHODS:
            self.zone["backlight"]["present"] = True
            for m in effect_methods["backlight"]:
                self.logger.debug("Adding {}.{} method to DBus".format(m[0], m[1]))
                self.add_dbus_method(m[0], m[1], m[2], in_signature=m[3], out_signature=m[4])

        for i in self.ZONES[1:]:
            if 'set_' + i + '_static' in self.METHODS or 'set_' + i + '_static_naga_hex_v2' in self.METHODS or '`set_' + i + 'active' in self.METHODS:
                self.zone[i]["present"] = True
                for m in effect_methods[i]:
                    self.logger.debug("Adding {}.{} method to DBus".format(m[0], m[1]))
                    self.add_dbus_method(m[0], m[1], m[2], in_signature=m[3], out_signature=m[4])

        # Load additional DBus methods
        self.load_methods()

        # load last DPI/poll rate state
        if self.persistence.has_section(self.storage_name):
            if 'set_dpi_xy' in self.METHODS or 'set_dpi_xy_byte' in self.METHODS:
                try:
                    self.dpi[0] = int(self.persistence[self.storage_name]['dpi_x'])
                    self.dpi[1] = int(self.persistence[self.storage_name]['dpi_y'])
                except KeyError:
                    pass

            if 'set_poll_rate' in self.METHODS:
                try:
                    self.poll_rate = int(self.persistence[self.storage_name]['poll_rate'])
                except KeyError:
                    pass

        dpi_func = getattr(self, "setDPI", None)
        if dpi_func is not None:
            dpi_func(self.dpi[0], self.dpi[1])

        poll_rate_func = getattr(self, "setPollRate", None)
        if poll_rate_func is not None:
            poll_rate_func(self.poll_rate)

        # load last effects
        for i in self.ZONES:
            if self.zone[i]["present"]:
                # check if we have the device in the persistence file
                if self.persistence.has_section(self.storage_name):
                    # try reading the effect name from the persistence
                    try:
                        self.zone[i]["effect"] = self.persistence[self.storage_name][i + '_effect']
                    except KeyError:
                        pass

                    # zone active status
                    try:
                        self.zone[i]["active"] = bool(self.persistence[self.storage_name][i + '_active'])
                    except KeyError:
                        pass

                    # brightness
                    try:
                        self.zone[i]["brightness"] = float(self.persistence[self.storage_name][i + '_brightness'])
                    except KeyError:
                        pass

                    # colors.
                    # these are stored as a string that must contain 9 numbers, separated with spaces.
                    try:
                        for index, item in enumerate(self.persistence[self.storage_name][i + '_colors'].split(" ")):
                            self.zone[i]["colors"][index] = int(item)
                            # check if the color is in range
                            if not 0 <= self.zone[i]["colors"][index] <= 255:
                                raise ValueError('Color out of range')

                        # check if we have exactly 9 colors
                        if len(self.zone[i]["colors"]) != 9:
                            raise ValueError('There must be exactly 9 colors')

                    except ValueError:
                        # invalid colors. reinitialize
                        self.zone[i]["colors"] = [0, 255, 0, 0, 255, 255, 0, 0, 255]
                        self.logger.info("%s: Invalid colors; restoring to defaults.", self.__class__.__name__)
                        pass

                    except KeyError:
                        pass

                    # speed
                    try:
                        self.zone[i]["speed"] = int(self.persistence[self.storage_name][i + '_speed'])

                    except KeyError:
                        pass

                    # wave direction
                    try:
                        self.zone[i]["wave_dir"] = int(self.persistence[self.storage_name][i + '_wave_dir'])

                    except KeyError:
                        pass

                if 'set_' + i + '_active' in self.METHODS:
                    active_func = getattr(self, "set" + self.capitalize_first_char(i) + "Active", None)
                    if active_func is not None:
                        active_func(self.zone[i]["active"])

                # load brightness level
                bright_func = None
                if i == "backlight":
                    bright_func = getattr(self, "setBrightness", None)
                elif 'set_' + i + '_brightness' in self.METHODS:
                    bright_func = getattr(self, "set" + self.capitalize_first_char(i) + "Brightness", None)

                if bright_func is not None:
                    bright_func(self.zone[i]["brightness"])

        self.restore_effect()

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

    def restore_effect(self):
        """
        Set the device to the current effect

        This is used at launch time and can be called by applications
        that use custom matrix frames after they exit
        """
        for i in self.ZONES:
            if self.zone[i]["present"]:
                # prepare the effect method name
                # yes, we need to handle the backlight zone separately too.
                # the backlight effect methods don't have a prefix.
                if i == "backlight":
                    effect_func_name = 'set' + self.capitalize_first_char(self.zone[i]["effect"])
                else:
                    effect_func_name = 'set' + self.handle_underscores(self.capitalize_first_char(i)) + self.capitalize_first_char(self.zone[i]["effect"])

                # find the effect method
                effect_func = getattr(self, effect_func_name, None)

                # check if the effect method exists only if we didn't look for spectrum (because resetting to Spectrum when the effect is Spectrum is in vain)
                if effect_func == None and not self.zone[i]["effect"] == "spectrum":
                    # not found. restoring to Spectrum
                    self.logger.info("%s: Invalid effect name %s; restoring to Spectrum.", self.__class__.__name__, effect_func_name)
                    self.zone[i]["effect"] = 'spectrum'
                    if i == "backlight":
                        effect_func_name = 'setSpectrum'
                    else:
                        effect_func_name = 'set' + self.capitalize_first_char(i) + 'Spectrum'
                    effect_func = getattr(self, effect_func_name, None)

                # we check again here because there is a possibility the device may not even have Spectrum
                if effect_func is not None:
                    effect = self.zone[i]["effect"]
                    colors = self.zone[i]["colors"]
                    speed = self.zone[i]["speed"]
                    wave_dir = self.zone[i]["wave_dir"]
                    if self.get_num_arguments(effect_func) == 0:
                        effect_func()
                    elif self.get_num_arguments(effect_func) == 1:
                        # there are 2 effects which require 1 argument.
                        # these are: Starlight (Random) and Wave.
                        if effect == 'starlightRandom':
                            effect_func(speed)
                        elif effect == 'wave':
                            effect_func(wave_dir)
                        elif effect == 'rippleRandomColour':
                            # do nothing. this is handled in the ripple manager.
                            pass
                        else:
                            self.logger.error("%s: Effect requires 1 argument but don't know how to handle it!", self.__class__.__name__)
                    elif self.get_num_arguments(effect_func) == 3:
                        effect_func(colors[0], colors[1], colors[2])
                    elif self.get_num_arguments(effect_func) == 4:
                        # starlight/reactive have different arguments.
                        if effect == 'starlightSingle' or effect == 'reactive':
                            effect_func(colors[0], colors[1], colors[2], speed)
                        elif effect == 'ripple':
                            # do nothing. this is handled in the ripple manager.
                            pass
                        else:
                            self.logger.error("%s: Effect requires 4 arguments but don't know how to handle it!", self.__class__.__name__)
                    elif self.get_num_arguments(effect_func) == 6:
                        effect_func(colors[0], colors[1], colors[2], colors[3], colors[4], colors[5])
                    elif self.get_num_arguments(effect_func) == 7:
                        effect_func(colors[0], colors[1], colors[2], colors[3], colors[4], colors[5], speed)
                    elif self.get_num_arguments(effect_func) == 9:
                        effect_func(colors[0], colors[1], colors[2], colors[3], colors[4], colors[5], colors[6], colors[7], colors[8])
                    else:
                        self.logger.error("%s: Couldn't detect effect argument count!", self.__class__.__name__)

    def set_persistence(self, zone, key, value):
        """
        Set a device's current state for persisting across sessions.

        :param zone: Zone
        :type zone: string

        :param key: Key
        :type key: string

        :param value: Value
        :type value: string
        """
        self.persistence.status["changed"] = True

        if zone:
            self.zone[zone][key] = value
        else:
            self.zone[key] = value

    def get_current_effect(self):
        """
        Get the device's current effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_effect")

        return self.zone["backlight"]["effect"]

    def get_current_effect_colors(self):
        """
        Get the device's current effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_effect_colors")

        return self.zone["backlight"]["colors"]

    def get_current_effect_speed(self):
        """
        Get the device's current effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_effect_speed")

        return self.zone["backlight"]["speed"]

    def get_current_wave_dir(self):
        """
        Get the device's current wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_wave_dir")

        return self.zone["backlight"]["wave_dir"]

    def get_current_logo_effect(self):
        """
        Get the device's current logo effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_logo_effect")

        return self.zone["logo"]["effect"]

    def get_current_logo_effect_colors(self):
        """
        Get the device's current logo effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_logo_effect_colors")

        return self.zone["logo"]["colors"]

    def get_current_logo_effect_speed(self):
        """
        Get the device's current logo effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_logo_effect_speed")

        return self.zone["logo"]["speed"]

    def get_current_logo_wave_dir(self):
        """
        Get the device's current logo wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_logo_wave_dir")

        return self.zone["logo"]["wave_dir"]

    def get_current_scroll_effect(self):
        """
        Get the device's current scroll effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_scroll_effect")

        return self.zone["scroll"]["effect"]

    def get_current_scroll_effect_colors(self):
        """
        Get the device's current scroll effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_scroll_effect_colors")

        return self.zone["scroll"]["colors"]

    def get_current_scroll_effect_speed(self):
        """
        Get the device's current scroll effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_scroll_effect_speed")

        return self.zone["scroll"]["speed"]

    def get_current_scroll_wave_dir(self):
        """
        Get the device's current scroll wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_scroll_wave_dir")

        return self.zone["scroll"]["wave_dir"]

    def get_current_left_effect(self):
        """
        Get the device's current left effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_left_effect")

        return self.zone["left"]["effect"]

    def get_current_left_effect_colors(self):
        """
        Get the device's current left effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_left_effect_colors")

        return self.zone["left"]["colors"]

    def get_current_left_effect_speed(self):
        """
        Get the device's current left effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_left_effect_speed")

        return self.zone["left"]["speed"]

    def get_current_left_wave_dir(self):
        """
        Get the device's current left wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_left_wave_dir")

        return self.zone["left"]["wave_dir"]

    def get_current_right_effect(self):
        """
        Get the device's current right effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_right_effect")

        return self.zone["right"]["effect"]

    def get_current_right_effect_colors(self):
        """
        Get the device's current right effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_right_effect_colors")

        return self.zone["right"]["colors"]

    def get_current_right_effect_speed(self):
        """
        Get the device's current right effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_right_effect_speed")

        return self.zone["right"]["speed"]

    def get_current_right_wave_dir(self):
        """
        Get the device's current right wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_right_wave_dir")

        return self.zone["right"]["wave_dir"]

    def get_current_charging_effect(self):
        """
        Get the device's current charging effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_charging_effect")

        return self.zone["charging"]["effect"]

    def get_current_charging_effect_colors(self):
        """
        Get the device's current charging effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_charging_effect_colors")

        return self.zone["charging"]["colors"]

    def get_current_charging_effect_speed(self):
        """
        Get the device's current charging effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_charging_effect_speed")

        return self.zone["charging"]["speed"]

    def get_current_charging_wave_dir(self):
        """
        Get the device's current charging wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_charging_wave_dir")

        return self.zone["charging"]["wave_dir"]

    def get_current_fast_charging_effect(self):
        """
        Get the device's current fast_charging effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_fast_charging_effect")

        return self.zone["fast_charging"]["effect"]

    def get_current_fast_charging_effect_colors(self):
        """
        Get the device's current fast_charging effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_fast_charging_effect_colors")

        return self.zone["fast_charging"]["colors"]

    def get_current_fast_charging_effect_speed(self):
        """
        Get the device's current fast_charging effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_fast_charging_effect_speed")

        return self.zone["fast_charging"]["speed"]

    def get_current_fast_charging_wave_dir(self):
        """
        Get the device's current fast_charging wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_fast_charging_wave_dir")

        return self.zone["fast_charging"]["wave_dir"]

    def get_current_fully_charged_effect(self):
        """
        Get the device's current fully_charged effect

        :return: Effect
        :rtype: string
        """
        self.logger.debug("DBus call get_current_fully_charged_effect")

        return self.zone["fully_charged"]["effect"]

    def get_current_fully_charged_effect_colors(self):
        """
        Get the device's current fully_charged effect's colors

        :return: 3 colors
        :rtype: list of byte
        """
        self.logger.debug("DBus call get_current_fully_charged_effect_colors")

        return self.zone["fully_charged"]["colors"]

    def get_current_fully_charged_effect_speed(self):
        """
        Get the device's current fully_charged effect's speed

        :return: Speed
        :rtype: int
        """
        self.logger.debug("DBus call get_current_fully_charged_effect_speed")

        return self.zone["fully_charged"]["speed"]

    def get_current_fully_charged_wave_dir(self):
        """
        Get the device's current fully_charged wave direction

        :return: Direction
        :rtype: int
        """
        self.logger.debug("DBus call get_current_fully_charged_wave_dir")

        return self.zone["fully_charged"]["wave_dir"]

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

                if len(serial) == 0:
                    time.sleep(0.1)
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
        # Deprecated API, but kept for backwards compatibility
        return json.dumps({
            "top_img": self.get_device_image(),
            "side_img": self.get_device_image(),
            "perspective_img": self.get_device_image()
        })

    def get_device_image(self):
        return self.DEVICE_IMAGE

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
                raise RuntimeError("Couldn't add method to DBus: " + str(e)) from None

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
            # If this is a mouse, retrieve current DPI for local storage
            # in case the user has changed the DPI on-the-fly
            # (e.g. the DPI buttons)
            if 'get_dpi_xy' in self.METHODS:
                dpi_func = getattr(self, "getDPI", None)
                if dpi_func is not None:
                    self.dpi = dpi_func()

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

    @staticmethod
    def get_num_arguments(func):
        """
        Get number of arguments in a function

        :param func: Function
        :type func: callable

        :return: Number of arguments
        :rtype: int
        """
        func_sig = inspect.signature(func)
        return len(func_sig.parameters)

    @staticmethod
    def handle_underscores(string):
        return re.sub(r'[_]+(?P<first>[a-z])', lambda m: m.group('first').upper(), string)

    @staticmethod
    def capitalize_first_char(string):
        return string[0].upper() + string[1:]

    def __del__(self):
        self.close()

    def __repr__(self):
        return "{0}:{1}".format(self.__class__.__name__, self.serial)


class RazerDeviceSpecialBrightnessSuspend(RazerDevice):
    """
    Class for suspend using brightness

    Suspend functions
    """

    def __init__(self, device_path, device_number, config, persistence, testing=False, additional_interfaces=None, additional_methods=[]):
        super().__init__(device_path, device_number, config, persistence, testing, additional_interfaces, additional_methods)

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

    def __init__(self, device_path, device_number, config, persistence, testing=False, additional_interfaces=None, additional_methods=[]):
        additional_methods.extend(['get_brightness', 'set_brightness'])
        super().__init__(device_path, device_number, config, persistence, testing, additional_interfaces, additional_methods)
