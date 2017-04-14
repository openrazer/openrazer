"""
Keyboards class
"""
import re

from razer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as _RazerDeviceBrightnessSuspend
from razer_daemon.misc.key_event_management import KeyboardKeyManager as _KeyboardKeyManager, GamepadKeyManager as _GamepadKeyManager, OrbweaverKeyManager as _OrbweaverKeyManager
from razer_daemon.misc.ripple_effect import RippleManager as _RippleManager


class _MacroKeyboard(_RazerDeviceBrightnessSuspend):
    """
    Keyboard class

    Has macro functionality and brightness based suspend
    """

    def __init__(self, *args, **kwargs):
        super(_MacroKeyboard, self).__init__(*args, **kwargs)
        # Methods are loaded into DBus by this point

        self.key_manager = _KeyboardKeyManager(self._device_number, self.event_files, self, use_epoll=True, testing=self._testing)

        self.logger.info('Putting device into driver mode. Daemon will handle special functionality')
        self.set_device_mode(0x03, 0x00)  # Driver mode

    def _close(self):
        """
        Close the key manager
        """
        super(_MacroKeyboard, self)._close()

        try:
            self.set_device_mode(0x00, 0x00)  # Device mode
        except FileNotFoundError:  # Could be called when daemon is stopping or device is removed.
            pass

        # TODO look into saving stats in /var/run maybe
        self.key_manager.close()


class RazerTartarus(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Tartarus Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Tartarus_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0208
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [-1, -1]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_tartarus', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_static_effect', 'set_spectrum_effect', 'tartarus_get_profile_led_red', 'tartarus_set_profile_led_red', 'tartarus_get_profile_led_green',
               'tartarus_set_profile_led_green', 'tartarus_get_profile_led_blue', 'tartarus_set_profile_led_blue', 'get_macros', 'delete_macro', 'add_macro', 'tartarus_get_mode_modifier', 'tartarus_set_mode_modifier']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-tartarus-chroma",
        "top_img": "http://assets.razerzone.com/eeimages/products/22356/razer-tartarus-chroma-01-02.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/22356/razer-tartarus-chroma-02.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/22356/razer-tartarus-chroma-03.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerTartarus, self).__init__(*args, **kwargs)
        # Methods are loaded into DBus by this point

        self.key_manager = _GamepadKeyManager(self._device_number, self.event_files, self, testing=self._testing)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerTartarus, self)._close()

        # TODO look into saving stats in /var/run maybe
        self.key_manager.close()


class RazerOrbweaver(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Orbweaver
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Orbweaver(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0113
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [-1, -1]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_orbweaver',
               'tartarus_get_profile_led_red', 'tartarus_set_profile_led_red', 'tartarus_get_profile_led_green', 'tartarus_set_profile_led_green', 'tartarus_get_profile_led_blue', 'tartarus_set_profile_led_blue',
               'get_macros', 'delete_macro', 'add_macro', 'tartarus_get_mode_modifier', 'tartarus_set_mode_modifier',

               'bw_set_pulsate', 'bw_set_static']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-orbweaver-2012",
        "top_img": "https://assets.razerzone.com/eeimages/products/7305/razer-orbweaver-latest-04.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/7305/razer-orbweaver-latest-02.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/7305/razer-orbweaver-latest-03.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerOrbweaver, self).__init__(*args, **kwargs)
        # Methods are loaded into DBus by this point

        self.key_manager = _OrbweaverKeyManager(self._device_number, self.event_files, self, testing=self._testing)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerOrbweaver, self)._close()

        # TODO look into saving stats in /var/run maybe
        self.key_manager.close()


class RazerBlackWidow2012(_MacroKeyboard):
    """
    Class for the Razer BlackWidow Ultimate 2012
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow_Ultimate_2012(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x010D
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22] # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blackwidow-ultimate-classic",
        "top_img": "http://assets.razerzone.com/eeimages/products/22212/razer-blackwidow-ultimate-classic-gallery-4.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/22212/razer-blackwidow-ultimate-classic-gallery-1.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/22212/razer-blackwidow-ultimate-classic-gallery-2.png"
    }


class RazerBlackWidowClassic(_MacroKeyboard):
    """
    Class for the Razer BlackWidow (Classic)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x011B
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    RAZER_URLS = {
        "store": "http://www.razerzone.com/store/razer-blackwidow-old", # URL is dead
        "top_img": "http://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-01.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-02.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-04.png"
    }


class RazerBlackWidowClassicAlternate(_MacroKeyboard):
    """
    Class for the Razer BlackWidow (Classic)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x010E
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    RAZER_URLS = {
        "store": "http://www.razerzone.com/gb-en/store/razer-blackwidow-old", # URL is dead
        "top_img": "http://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-01.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-02.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-04.png"
    }


class RazerBlackWidow2013(_MacroKeyboard):
    """
    Class for the Razer BlackWidow Ultimate 2013
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow_Ultimate_2013(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x011A
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blackwidow-ultimate-2014",
        "top_img": "http://assets.razerzone.com/eeimages/products/17561/razer-blackwidow-ultimate-gallery-02.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/17561/razer-blackwidow-ultimate-gallery-01.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/17561/razer-blackwidow-ultimate-gallery-04.png"
    }


class RazerBlackWidowChroma(_MacroKeyboard):
    """
    Class for the Razer BlackWidow Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0203
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blackwidow-chroma-v1",
        "top_img": "http://assets.razerzone.com/eeimages/products/17557/razer-blackwidow-ultimate-gallery-01.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/17557/razer-blackwidow-ultimate-gallery-02.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/17557/razer-blackwidow-ultimate-gallery-04.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBlackWidowChroma, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidowChroma, self)._close()

        self.ripple_manager.close()


class RazerBlackWidowChromaV2(_MacroKeyboard):
    """
    Class for the BlackWidow Chroma V2
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Chroma_V2(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0221
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blackwidow-chroma-v2",
        "top_img": "https://assets.razerzone.com/eeimages/products/26600/razer-blackwidow-chroma-v2-gallery-01-wristrest.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26600/razer-blackwidow-chroma-v2-gallery-02-wristrest-green.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26600/razer-blackwidow-chroma-v2-gallery-03-wristrest.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBlackWidowChromaV2, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidowChromaV2, self)._close()

        self.ripple_manager.close()


class RazerBlackWidowChromaTournamentEdition(_MacroKeyboard):
    """
    Class for the Razer BlackWidow Tournament Edition Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Tournament_Edition_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0209
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix',  'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blackwidow-tournament-edition-chroma",
        "top_img": "http://assets.razerzone.com/eeimages/products/24362/razer-blackwidow-te-chroma-gallery-01.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/24362/razer-blackwidow-te-chroma-gallery-03.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/24362/razer-blackwidow-te-chroma-gallery-04.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBlackWidowChromaTournamentEdition, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidowChromaTournamentEdition, self)._close()

        self.ripple_manager.close()


class RazerBlackWidowXChroma(_MacroKeyboard):
    """
    Class for the Razer BlackWidow X Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_X_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0216
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blackwidow-x-chroma",
        "top_img": "http://assets.razerzone.com/eeimages/products/24325/razer-blackwidow-x-chroma-redo-1.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/24325/razer-blackwidow-x-chroma-redo-3.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/24325/razer-blackwidow-x-chroma-redo-4.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBlackWidowXChroma, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidowXChroma, self)._close()

        self.ripple_manager.close()


class RazerBlackWidowXChromaTournamentEdition(_MacroKeyboard):
    """
    Class for the Razer BlackWidow X Tournament Edition Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_X_Tournament_Edition_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x021a
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blackwidow-x-tournament-edition-chroma",
        "top_img": "http://assets.razerzone.com/eeimages/products/24362/razer-blackwidow-te-chroma-gallery-01.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/24362/razer-blackwidow-te-chroma-gallery-03.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/24362/razer-blackwidow-te-chroma-gallery-04.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBlackWidowXChromaTournamentEdition, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidowXChromaTournamentEdition, self)._close()

        self.ripple_manager.close()


class RazerBladeStealth(_MacroKeyboard):
    """
    Class for the Razer Blade Stealth
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Stealth(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0205
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row'

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blade-stealth",
        "top_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-05-v2.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-03-v2.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-04-v2.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBladeStealth, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBladeStealth, self)._close()

        self.ripple_manager.close()


class RazerBladeStealthLate2016(_MacroKeyboard):
    """
    Class for the Razer Blade Stealth (Late 2016)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Stealth(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0220
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 16]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blade-stealth",
        "top_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-05-v2.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-03-v2.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-04-v2.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBladeStealthLate2016, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBladeStealthLate2016, self)._close()

        self.ripple_manager.close()


class RazerBladeProLate2016(_MacroKeyboard):
    """
    Class for the Razer Blade Pro (Late 2016)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Pro(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0210
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blade-pro",
        "top_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-05-v2.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-03-v2.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-04-v2.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBladeProLate2016, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBladeProLate2016, self)._close()

        self.ripple_manager.close()


class RazerBladeLate2016(_MacroKeyboard):
    """
    Class for the Razer Blade Pro (Late 2016)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0224
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 15]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blade-pro",
        "top_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-05-v2.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-03-v2.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-04-v2.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBladeLate2016, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBladeLate2016, self)._close()

        self.ripple_manager.close()


class RazerBladeQHD(_MacroKeyboard):
    """
    Class for the Razer Blade (QHD)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x020F
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 16]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blade",
        "top_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-05-v2.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-03-v2.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-04-v2.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBladeQHD, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBladeQHD, self)._close()

        self.ripple_manager.close()


class RazerBlackWidow2016(_MacroKeyboard):
    """
    Class for the Razer BlackWidow Ultimate 2016
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Ultimate_2016(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0214
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro', 'set_starlight_random_effect',

               'set_ripple_effect']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blackwidow-ultimate-2016",
        "top_img": "http://assets.razerzone.com/eeimages/products/22916/razer-blackwidow-gallery-01.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/22916/razer-blackwidow-gallery-07.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/22916/razer-blackwidow-gallery-02.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBlackWidow2016, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidow2016, self)._close()

        self.ripple_manager.close()


class RazerBlackWidowXUltimate(_MacroKeyboard):
    """
    Class for the Razer BlackWidow X Ultimate
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_X_Ultimate(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0217
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro', 'set_starlight_random_effect',

               'set_ripple_effect']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-blackwidow-x-ultimate",
        "top_img": "https://assets.razerzone.com/eeimages/products/24363/razer-blackwidow-x-ultimate-redo-1.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/24363/razer-blackwidow-x-ultimate-redo-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/24363/razer-blackwidow-x-ultimate-redo-4.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBlackWidowXUltimate, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidowXUltimate, self)._close()

        self.ripple_manager.close()


class RazerOrnataChroma(_MacroKeyboard):
    """
    Class for the Razer Ornata Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Ornata_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x021e
    HAS_MATRIX = True
    WAVE_DIRS = (0, 1)
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-ornata-chroma",
        "top_img": "http://assets.razerzone.com/eeimages/products/25713/razer-ornata-chroma-gallery-05.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/25713/razer-ornata-chroma-gallery-07.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/25713/razer-ornata-chroma-gallery-08.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerOrnataChroma, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerOrnataChroma, self)._close()

        self.ripple_manager.close()


class RazerOrnata(_MacroKeyboard):
    """
    Class for the Razer Ornata
    """
    EVENT_FILE_REGEX = re.compile(r'.*Ornata(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x021f
    HAS_MATRIX = True
    WAVE_DIRS = (0, 1)
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_single_effect'
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode', 'set_breath_single_effect',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_single_effect', 'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-ornata",
        "top_img": "https://assets.razerzone.com/eeimages/products/25675/razer_ornata_001.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/25675/razer_ornata_003.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/25675/razer_ornata_004.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerOrnata, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerOrnata, self)._close()

        self.ripple_manager.close()


class RazerAnansi(_MacroKeyboard):
    """
    Class for the Razer Anansi
    """
    EVENT_FILE_REGEX = re.compile(r'.*Anansi(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x010f
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [-1, -1]
    METHODS = ['get_firmware', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness',
               'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode', 'get_macro_effect',
               'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro', 'set_static_effect',
               'set_spectrum_effect', 'has_matrix', 'get_matrix_dims', 'set_none_effect']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/store/razer-anansi",
        "top_img": "https://assets.razerzone.com/eeimages/products/58/razer-anansi-gallery-5.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/58/razer-anansi-gallery-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/58/razer-anansi-gallery-2.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerAnansi, self).__init__(*args, **kwargs)

    def _close(self):
        super(RazerAnansi, self)._close()


class RazerDeathStalkerChroma(_MacroKeyboard):
    """
    Class for the Razer DeathStalker Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*DeathStalker_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0204
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [1, 6]  # 1 Row, 6 zones
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro']

    RAZER_URLS = {
        "store": "https://www.razerzone.com/gb-en/store/razer-deathstalker-chroma",
        "top_img": "https://assets.razerzone.com/eeimages/products/22563/rzr_deathstalker_chroma_05.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22563/rzr_deathstalker_chroma_03.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22563/rzr_deathstalker_chroma_02.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerDeathStalkerChroma, self).__init__(*args, **kwargs)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidowChroma, self)._close()

        self.ripple_manager.close()
