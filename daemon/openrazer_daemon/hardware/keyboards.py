"""
Keyboards class
"""
import re

from openrazer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as _RazerDeviceBrightnessSuspend
from openrazer_daemon.misc.key_event_management import KeyboardKeyManager as _KeyboardKeyManager, GamepadKeyManager as _GamepadKeyManager, OrbweaverKeyManager as _OrbweaverKeyManager
from openrazer_daemon.misc.ripple_effect import RippleManager as _RippleManager


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


class RazerNostromo(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Nostromo
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Nostromo-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0111
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_keypad',
               'tartarus_get_profile_led_red', 'tartarus_set_profile_led_red', 'tartarus_get_profile_led_green', 'tartarus_set_profile_led_green', 'tartarus_get_profile_led_blue', 'tartarus_set_profile_led_blue',
               'get_macros', 'delete_macro', 'add_macro',

               # ?
               'tartarus_get_mode_modifier', 'tartarus_set_mode_modifier']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/59/razer-nostromo-gallery-1.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/59/razer-nostromo-gallery-1.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/59/razer-nostromo-gallery-2.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerNostromo, self).__init__(*args, **kwargs)
        # Methods are loaded into DBus by this point

        self.key_manager = _GamepadKeyManager(self._device_number, self.event_files, self, testing=self._testing)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerNostromo, self)._close()

        # TODO look into saving stats in /var/run maybe
        self.key_manager.close()


class RazerTartarus(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Tartarus
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Tartarus(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0201
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_tartarus',
               'set_static_effect', 'bw_set_pulsate', 'tartarus_get_profile_led_red', 'tartarus_set_profile_led_red', 'tartarus_get_profile_led_green',
               'tartarus_set_profile_led_green', 'tartarus_get_profile_led_blue', 'tartarus_set_profile_led_blue', 'get_macros', 'delete_macro', 'add_macro', 'tartarus_get_mode_modifier', 'tartarus_set_mode_modifier']

    RAZER_URLS = {
        "top_img": "https://assets2.razerzone.com/images/tartarus-classic/b0535b8924b38f53cb8b853d536798ed-Tartarus-Classic-Base_gallery04.jpg",
        "side_img": "https://assets2.razerzone.com/images/tartarus-classic/f89a70c59f993f08e95a8060ee2623da-Tartarus-Classic-Base_gallery02.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/tartarus-classic/b3a11ddda103b2473c3253a0a82af389-Tartarus-Classic-Base_gallery03.jpg"
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


class RazerTartarusChroma(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Tartarus Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Tartarus_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0208
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_tartarus', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_static_effect', 'set_spectrum_effect', 'tartarus_get_profile_led_red', 'tartarus_set_profile_led_red', 'tartarus_get_profile_led_green',
               'tartarus_set_profile_led_green', 'tartarus_get_profile_led_blue', 'tartarus_set_profile_led_blue', 'get_macros', 'delete_macro', 'add_macro', 'tartarus_get_mode_modifier', 'tartarus_set_mode_modifier']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/22356/razer-tartarus-chroma-01-02.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22356/razer-tartarus-chroma-02.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22356/razer-tartarus-chroma-03.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerTartarusChroma, self).__init__(*args, **kwargs)
        # Methods are loaded into DBus by this point

        self.key_manager = _GamepadKeyManager(self._device_number, self.event_files, self, testing=self._testing)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerTartarusChroma, self)._close()

        # TODO look into saving stats in /var/run maybe
        self.key_manager.close()


class RazerOrbweaver(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Orbweaver
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Orbweaver(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0113
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_orbweaver',
               'tartarus_get_profile_led_red', 'tartarus_set_profile_led_red', 'tartarus_get_profile_led_green', 'tartarus_set_profile_led_green', 'tartarus_get_profile_led_blue', 'tartarus_set_profile_led_blue',
               'get_macros', 'delete_macro', 'add_macro', 'tartarus_get_mode_modifier', 'tartarus_set_mode_modifier',

               'bw_set_pulsate', 'bw_set_static']

    RAZER_URLS = {
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


class RazerOrbweaverChroma(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Orbweaver Chroma
    """

    EVENT_FILE_REGEX = re.compile(r'.*Razer_Orbweaver_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0207
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_tartarus', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_static_effect', 'set_spectrum_effect', 'tartarus_get_profile_led_red', 'tartarus_set_profile_led_red', 'tartarus_get_profile_led_green',
               'tartarus_set_profile_led_green', 'tartarus_get_profile_led_blue', 'tartarus_set_profile_led_blue', 'get_macros', 'delete_macro', 'add_macro', 'tartarus_get_mode_modifier', 'tartarus_set_mode_modifier']

    RAZER_URLS = {
        "top_img": "https://assets2.razerzone.com/images/orbweaver-chroma/370604e681b07ee0ffc2047f569e438e-orbweaver-crhoma-gallery-02.jpg",
        "side_img": "https://assets2.razerzone.com/images/orbweaver-chroma/8def7438b6f8d4faf24c9218daa07ad0-orbweaver-crhoma-gallery-03.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/orbweaver-chroma/518c021598fd22a51a714a1b276d1e9e-orbweaver-crhoma-gallery-04.jpg"
    }


    def __init__(self, *args, **kwargs):
        super(RazerOrbweaverChroma, self).__init__(*args, **kwargs)
        # Methods are loaded into DBus by this point

        self.key_manager = _OrbweaverKeyManager(self._device_number, self.event_files, self, testing=self._testing)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerOrbweaverChroma, self)._close()

        # TODO look into saving stats in /var/run maybe
        self.key_manager.close()


class RazerBlackWidowUltimate2012(_MacroKeyboard):
    """
    Class for the Razer BlackWidow Ultimate 2012
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow_Ultimate_2012(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x010D
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/22212/razer-blackwidow-ultimate-classic-gallery-4.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22212/razer-blackwidow-ultimate-classic-gallery-1.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22212/razer-blackwidow-ultimate-classic-gallery-2.png"
    }


class RazerBlackWidowClassic(_MacroKeyboard):
    """
    Class for the Razer BlackWidow (Classic)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x011B
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-01.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-02.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-04.png"
    }


class RazerBlackWidowClassicAlternate(_MacroKeyboard):
    """
    Class for the Razer BlackWidow (Classic)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x010E
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-01.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-02.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-04.png"
    }


class RazerBlackWidowUltimate2013(_MacroKeyboard):
    """
    Class for the Razer BlackWidow Ultimate 2013
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow_Ultimate_2013(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x011A
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/17561/razer-blackwidow-ultimate-gallery-02.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17561/razer-blackwidow-ultimate-gallery-01.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17561/razer-blackwidow-ultimate-gallery-04.png"
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
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/17557/razer-blackwidow-ultimate-gallery-01.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17557/razer-blackwidow-ultimate-gallery-02.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17557/razer-blackwidow-ultimate-gallery-04.png"
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
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
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
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix',  'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "top_img": "https://assets2.razerzone.com/images/blackwidow-te-chroma/508721b4948304fe823e0d84b2ca114f-Blackwidow-TE-Chroma-Base_gallery2.jpg",
        "side_img": "https://assets2.razerzone.com/images/blackwidow-te-chroma/87f7492792c72241c6d5bc302e36d46f-Blackwidow-TE-Chroma-Base_gallery3.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/blackwidow-te-chroma/918fc196cb8aec3e140316650d97a075-Blackwidow-TE-Chroma-Base_gallery5.jpg"
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
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/24325/razer-blackwidow-x-chroma-redo-1.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/24325/razer-blackwidow-x-chroma-redo-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/24325/razer-blackwidow-x-chroma-redo-4.png"
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


class RazerBlackWidowXTournamentEditionChroma(_MacroKeyboard):
    """
    Class for the Razer BlackWidow X Tournament Edition Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_X_Tournament_Edition_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x021a
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/24362/razer-blackwidow-te-chroma-gallery-01.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/24362/razer-blackwidow-te-chroma-gallery-03.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/24362/razer-blackwidow-te-chroma-gallery-04.png"
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
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row'

               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-05-v2.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-08-v2.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-01-v2.png"
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
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row',

               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-15__store_gallery.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-22__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-02__store_gallery.png"
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
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-07__store_gallery.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-13__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-01__store_gallery.png"
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
    Class for the Razer Blade (Late 2016)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0224
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 15]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-07__store_gallery.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-13__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-01__store_gallery.png"
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
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',

               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/25684/rzrblade14-07__store_gallery.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/25684/rzrblade14-13__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/25684/rzrblade14-02__store_gallery.png"
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


class RazerBlackWidowUltimate2016(_MacroKeyboard):
    """
    Class for the Razer BlackWidow Ultimate 2016
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Ultimate_2016(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0214
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro', 'set_starlight_random_effect',

               'set_ripple_effect']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/22916/razer-blackwidow-gallery-01.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22916/razer-blackwidow-gallery-07.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22916/razer-blackwidow-gallery-02.png"
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
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro', 'set_starlight_random_effect',

               'set_ripple_effect']

    RAZER_URLS = {
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
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/25713/razer-ornata-chroma-gallery-05.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/25713/razer-ornata-chroma-gallery-07.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/25713/razer-ornata-chroma-gallery-08.png"
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
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_single_effect'
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode', 'set_breath_single_effect',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_single_effect', 'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
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
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_firmware', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness',
               'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode', 'get_macro_effect',
               'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro', 'set_static_effect',
               'set_spectrum_effect', 'has_matrix', 'get_matrix_dims', 'set_none_effect']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/58/razer-anansi-gallery-5.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/58/razer-anansi-gallery-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/58/razer-anansi-gallery-2.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerAnansi, self).__init__(*args, **kwargs)

    def _close(self):
        super(RazerAnansi, self)._close()


class RazerDeathStalkerExpert(_MacroKeyboard):
    """
    Class for the Razer DeathStalker Expert
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_DeathStalker(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0202
    DEDICATED_MACRO_KEYS = False
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/771/razer-dstalk-gallery-5.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/771/razer-dstalk-gallery-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/771/razer-dstalk-gallery-2.png"
    }


class RazerDeathStalkerChroma(_MacroKeyboard):
    """
    Class for the Razer DeathStalker Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*DeathStalker_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0204
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [1, 6]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro']

    RAZER_URLS = {
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


class RazerBlackWidowChromaOverwatch(_MacroKeyboard):
    """
    Class for the Razer BlackWidow Chroma (Overwatch)
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0211
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/23326/overwatch-razer-gallery-5.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/23326/overwatch-razer-gallery-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/23326/overwatch-razer-gallery-1.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBlackWidowChromaOverwatch, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidowChromaOverwatch, self)._close()

        self.ripple_manager.close()


class RazerBladeStealthMid2017(_MacroKeyboard):
    """
    Class for the Razer Blade Stealth (Mid 2017)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Stealth(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x022D
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row',

               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-15__store_gallery.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-22__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-02__store_gallery.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBladeStealthMid2017, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBladeStealthLate2016, self)._close()

        self.ripple_manager.close()


class RazerBladePro2017(_MacroKeyboard):
    """
    Class for the Razer Blade Pro (2017)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Pro(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0225
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 25]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour', 'blade_get_logo_active', 'blade_set_logo_active']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-07__store_gallery.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-13__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-01__store_gallery.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBladePro2017, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBladePro2017, self)._close()

        self.ripple_manager.close()


class RazerBladeStealthLate2017(_MacroKeyboard):
    """
    Class for the Razer Blade Stealth (Late 2017)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Stealth(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0232
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row',

               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-15__store_gallery.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-22__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-02__store_gallery.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerBladeStealthLate2017, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBladeStealthLate2017, self)._close()

        self.ripple_manager.close()
