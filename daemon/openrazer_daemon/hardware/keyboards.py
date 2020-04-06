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
        if 'additional_methods' in kwargs:
            kwargs['additional_methods'].extend(['get_keyboard_layout'])
        else:
            kwargs['additional_methods'] = ['get_keyboard_layout']
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


class _RippleKeyboard(_MacroKeyboard):
    """
    Keyboard class

    Inherits _MacroKeyboard and has a ripple manager
    """

    def __init__(self, *args, **kwargs):
        super(_RippleKeyboard, self).__init__(*args, **kwargs)

        self.ripple_manager = _RippleManager(self, self._device_number)

    def _close(self):
        super(_RippleKeyboard, self)._close()

        self.ripple_manager.close()


class RazerNostromo(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Nostromo
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Nostromo-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0111
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_device_type_keypad', 'keypad_get_profile_led_red', 'keypad_set_profile_led_red', 'keypad_get_profile_led_green', 'keypad_set_profile_led_green', 'keypad_get_profile_led_blue', 'keypad_set_profile_led_blue',
               'get_macros', 'delete_macro', 'add_macro',

               # ?
               'keypad_get_mode_modifier', 'keypad_set_mode_modifier']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/564/564_tartarus_classic.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/564/564_tartarus_classic.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/59/razer-nostromo-gallery-1.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/59/razer-nostromo-gallery-2.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerNostromo, self).__init__(*args, **kwargs)
        # Methods are loaded into DBus by this point

        # self.key_manager = _GamepadKeyManager(self._device_number, self.event_files, self, testing=self._testing)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerNostromo, self)._close()

        # TODO look into saving stats in /var/run maybe
        # self.key_manager.close()


class RazerTartarus(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Tartarus
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Tartarus(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0201
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_device_type_keypad',
               'set_static_effect', 'bw_set_pulsate', 'keypad_get_profile_led_red', 'keypad_set_profile_led_red', 'keypad_get_profile_led_green',
               'keypad_set_profile_led_green', 'keypad_get_profile_led_blue', 'keypad_set_profile_led_blue', 'get_macros', 'delete_macro', 'add_macro', 'keypad_get_mode_modifier', 'keypad_set_mode_modifier']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/228/228_tartarus.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/228/228_tartarus.png",
        "side_img": "https://assets2.razerzone.com/images/tartarus-classic/f89a70c59f993f08e95a8060ee2623da-Tartarus-Classic-Base_gallery02.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/tartarus-classic/b3a11ddda103b2473c3253a0a82af389-Tartarus-Classic-Base_gallery03.jpg"
    }

    def __init__(self, *args, **kwargs):
        super(RazerTartarus, self).__init__(*args, **kwargs)
        # Methods are loaded into DBus by this point

        # self.key_manager = _GamepadKeyManager(self._device_number, self.event_files, self, testing=self._testing)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerTartarus, self)._close()

        # TODO look into saving stats in /var/run maybe
        # self.key_manager.close()


class RazerTartarusChroma(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Tartarus Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Tartarus_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0208
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_device_type_keypad', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_static_effect', 'set_spectrum_effect', 'keypad_get_profile_led_red', 'keypad_set_profile_led_red', 'keypad_get_profile_led_green',
               'keypad_set_profile_led_green', 'keypad_get_profile_led_blue', 'keypad_set_profile_led_blue', 'get_macros', 'delete_macro', 'add_macro', 'keypad_get_mode_modifier', 'keypad_set_mode_modifier']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/598/598_tartarus_chroma.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/598/598_tartarus_chroma.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22356/razer-tartarus-chroma-02.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22356/razer-tartarus-chroma-03.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerTartarusChroma, self).__init__(*args, **kwargs)
        # Methods are loaded into DBus by this point

        # self.key_manager = _GamepadKeyManager(self._device_number, self.event_files, self, testing=self._testing)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerTartarusChroma, self)._close()

        # TODO look into saving stats in /var/run maybe
        # self.key_manager.close()


class RazerTartarusV2(_RippleKeyboard):
    """
    Class for Razer Tartarus V2
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Tartarus_V2(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x022B
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [4, 6]

    METHODS = ['get_device_type_keypad',
               'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_key_row',
               'set_reactive_effect',
               'set_none_effect',
               'set_custom_effect',
               'set_wave_effect',
               'set_static_effect',
               'set_spectrum_effect',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'keypad_get_profile_led_red', 'keypad_set_profile_led_red',
               'keypad_get_profile_led_green', 'keypad_set_profile_led_green',
               'keypad_get_profile_led_blue', 'keypad_set_profile_led_blue',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_ripple_effect', 'set_ripple_effect_random_colour',
               'get_game_mode', 'set_game_mode',
               'keypad_get_mode_modifier', 'keypad_set_mode_modifier']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1255/1255_tartarus_v2.png"

    RAZER_URLS = {
        "top_img": DEVICE_IMAGE
    }

    def __init__(self, *args, **kwargs):
        super(RazerTartarusV2, self).__init__(*args, **kwargs)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerTartarusV2, self)._close()

        # TODO look into saving stats in /var/run maybe
        # self.key_manager.close()


class RazerOrbweaver(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Orbweaver
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Orbweaver(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0113
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_device_type_keypad',
               'keypad_get_profile_led_red', 'keypad_set_profile_led_red', 'keypad_get_profile_led_green', 'keypad_set_profile_led_green', 'keypad_get_profile_led_blue', 'keypad_set_profile_led_blue',
               'get_macros', 'delete_macro', 'add_macro', 'keypad_get_mode_modifier', 'keypad_set_mode_modifier',

               'bw_set_pulsate', 'bw_set_static']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/56/56_orbweaver.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/56/56_orbweaver.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/7305/razer-orbweaver-latest-02.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/7305/razer-orbweaver-latest-03.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerOrbweaver, self).__init__(*args, **kwargs)
        # Methods are loaded into DBus by this point

        # self.key_manager = _OrbweaverKeyManager(self._device_number, self.event_files, self, testing=self._testing)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerOrbweaver, self)._close()

        # TODO look into saving stats in /var/run maybe
        # self.key_manager.close()


class RazerOrbweaverChroma(_RippleKeyboard):
    """
    Class for the Razer Orbweaver Chroma
    """

    EVENT_FILE_REGEX = re.compile(r'.*Razer_Orbweaver_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0207
    DEDICATED_MACRO_KEYS = True
    HAS_MATRIX = True
    MATRIX_DIMS = [5, 22]
    METHODS = ['get_device_type_keypad',
               'set_key_row',
               'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_wave_effect', 'set_custom_effect',
               'keypad_get_profile_led_red', 'keypad_set_profile_led_red',
               'keypad_get_profile_led_green', 'keypad_set_profile_led_green',
               'keypad_get_profile_led_blue', 'keypad_set_profile_led_blue',
               'get_macros', 'delete_macro', 'add_macro',
               'keypad_get_mode_modifier', 'keypad_set_mode_modifier',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/607/607_orbweaver_chroma.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/607/607_orbweaver_chroma.png",
        "side_img": "https://assets2.razerzone.com/images/orbweaver-chroma/8def7438b6f8d4faf24c9218daa07ad0-orbweaver-crhoma-gallery-03.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/orbweaver-chroma/518c021598fd22a51a714a1b276d1e9e-orbweaver-crhoma-gallery-04.jpg"
    }

    def __init__(self, *args, **kwargs):
        super(RazerOrbweaverChroma, self).__init__(*args, **kwargs)
        # Methods are loaded into DBus by this point

        # self.key_manager = _OrbweaverKeyManager(self._device_number, self.event_files, self, testing=self._testing)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerOrbweaverChroma, self)._close()

        # TODO look into saving stats in /var/run maybe
        # self.key_manager.close()


class RazerBlackWidowUltimate2012(_MacroKeyboard):
    """
    Class for the Razer BlackWidow Ultimate 2012
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow_Ultimate_2012(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x010D
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/563/563_blackwidow_ultimate_classic.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/563/563_blackwidow_ultimate_classic.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22212/razer-blackwidow-ultimate-classic-gallery-1.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22212/razer-blackwidow-ultimate-classic-gallery-2.png"
    }


class RazerBlackWidowStealth(_MacroKeyboard):
    """
    Class for the Razer BlackWidow (Classic)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x011B
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-01.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-01.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-02.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-04.png"
    }


class RazerBlackWidowStealthEdition(_MacroKeyboard):
    """
    Class for the Razer BlackWidow Stealth Edition
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x010E
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-01.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/17559/razer-blackwidow-gallery-01.png",
        "side_img": "https://drh1.img.digitalriver.com/DRHM/Storefront/Company/razerusa/images/product/gallery/razer-blackwidow-stealth-gallery1.jpg",
        "perspective_img": "https://drh2.img.digitalriver.com/DRHM/Storefront/Company/razerusa/images/product/gallery/razer-blackwidow-stealth-gallery2.jpg"
    }


class RazerBlackWidowUltimate2013(_MacroKeyboard):
    """
    Class for the Razer BlackWidow Ultimate 2013
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow_Ultimate(_2013)?(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x011A
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/245/438_blackwidow_ultimate_2014.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/245/438_blackwidow_ultimate_2014.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17561/razer-blackwidow-ultimate-gallery-01.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17561/razer-blackwidow-ultimate-gallery-04.png"
    }


class RazerBlackWidowChroma(_RippleKeyboard):
    """
    Class for the Razer BlackWidow Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0203
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/279/279_blackwidow_chroma.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/279/279_blackwidow_chroma.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17557/razer-blackwidow-ultimate-gallery-02.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17557/razer-blackwidow-ultimate-gallery-04.png"
    }


class RazerBlackWidowChromaV2(_RippleKeyboard):
    """
    Class for the BlackWidow Chroma V2
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Chroma_V2(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0221
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1179/1179_blackwidow_chroma_v2_alt.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1179/1179_blackwidow_chroma_v2_alt.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26600/razer-blackwidow-chroma-v2-gallery-02-wristrest-green.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26600/razer-blackwidow-chroma-v2-gallery-03-wristrest.png"
    }


class RazerBlackWidowChromaTournamentEdition(_RippleKeyboard):
    """
    Class for the Razer BlackWidow Tournament Edition Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Tournament_Edition_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0209
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/571/571_blackwidow_tournament_edition_chroma.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/571/571_blackwidow_tournament_edition_chroma.png",
        "side_img": "https://assets2.razerzone.com/images/blackwidow-te-chroma/87f7492792c72241c6d5bc302e36d46f-Blackwidow-TE-Chroma-Base_gallery3.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/blackwidow-te-chroma/918fc196cb8aec3e140316650d97a075-Blackwidow-TE-Chroma-Base_gallery5.jpg"
    }


class RazerBlackWidowXChroma(_RippleKeyboard):
    """
    Class for the Razer BlackWidow X Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_X_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0216
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/716/716_blackwidow_x_chroma.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/716/716_blackwidow_x_chroma.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/24325/razer-blackwidow-x-chroma-redo-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/24325/razer-blackwidow-x-chroma-redo-4.png"
    }


class RazerBlackWidowXTournamentEditionChroma(_RippleKeyboard):
    """
    Class for the Razer BlackWidow X Tournament Edition Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_X_Tournament_Edition_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x021A
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/717/717_blackwidow_x_tournament_edition_chroma.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/717/717_blackwidow_x_tournament_edition_chroma.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/24362/razer-blackwidow-te-chroma-gallery-03.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/24362/razer-blackwidow-te-chroma-gallery-04.png"
    }


class RazerBladeStealth(_RippleKeyboard):
    """
    Class for the Razer Blade Stealth
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Stealth(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0205
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/667/667_blade_stealth_2016_6500u.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/667/667_blade_stealth_2016_6500u.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-08-v2.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/23914/razer-blade-stealth-gallery-01-v2.png"
    }


class RazerBladeStealthLate2016(_RippleKeyboard):
    """
    Class for the Razer Blade Stealth (Late 2016)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Stealth(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0220
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/667/667_blade_stealth_2016_6500u.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/667/667_blade_stealth_2016_6500u.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-22__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-02__store_gallery.png"
    }


class RazerBladeProLate2016(_MacroKeyboard):
    """
    Class for the Razer Blade Pro (Late 2016)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Pro(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0210
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/736/736_blade_pro_2016.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/736/736_blade_pro_2016.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-13__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-01__store_gallery.png"
    }


class RazerBladeLate2016(_RippleKeyboard):
    """
    Class for the Razer Blade (Late 2016)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0224
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/736/736_blade_pro_2016.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/736/736_blade_pro_2016.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-13__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-01__store_gallery.png"
    }


class RazerBladeQHD(_RippleKeyboard):
    """
    Class for the Razer Blade (QHD)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x020F
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',

               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/736/736_blade_pro_2016.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/736/736_blade_pro_2016.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/25684/rzrblade14-13__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/25684/rzrblade14-02__store_gallery.png"
    }


class RazerBlackWidowUltimate2016(_RippleKeyboard):
    """
    Class for the Razer BlackWidow Ultimate 2016
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Ultimate_2016(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0214
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro', 'set_starlight_random_effect',

               'set_ripple_effect']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/616/616_blackwidow_ultimate_2016.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/616/616_blackwidow_ultimate_2016.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22916/razer-blackwidow-gallery-07.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22916/razer-blackwidow-gallery-02.png"
    }


class RazerBlackWidowXUltimate(_RippleKeyboard):
    """
    Class for the Razer BlackWidow X Ultimate
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_X_Ultimate(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0217
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro', 'set_starlight_random_effect',

               'set_ripple_effect']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/718/718_blackwidow_x_ultimate.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/718/718_blackwidow_x_ultimate.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/24363/razer-blackwidow-x-ultimate-redo-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/24363/razer-blackwidow-x-ultimate-redo-4.png"
    }


class RazerOrnataChroma(_RippleKeyboard):
    """
    Class for the Razer Ornata Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Ornata_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x021E
    HAS_MATRIX = True
    WAVE_DIRS = (0, 1)
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/727/727_ornata_chroma.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/727/727_ornata_chroma.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/25713/razer-ornata-chroma-gallery-07.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/25713/razer-ornata-chroma-gallery-08.png"
    }


class RazerHuntsmanElite(_RippleKeyboard):
    """
    Class for the Razer Huntsman Elite
    """
    EVENT_FILE_REGEX = re.compile(r'.*Huntsman_Elite(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0226
    HAS_MATRIX = True
    WAVE_DIRS = (0, 1)
    MATRIX_DIMS = [9, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1361/1361_huntsman_elite.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1361/1361_huntsman_elite.png"
    }


class RazerHuntsmanTournamentEdition(_RippleKeyboard):
    """
    Class for the Razer Huntsman Tournament Edition
    """
    EVENT_FILE_REGEX = re.compile(r'.*Huntsman_Tournament_Edition_00000000001A(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0243
    HAS_MATRIX = True
    WAVE_DIRS = (0, 1)
    MATRIX_DIMS = [6, 18]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1537/1537_huntsman_te.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1537/1537_huntsman_te.png"
    }


class RazerBlackWidowElite(_RippleKeyboard):
    """
    Class for the Razer BlackWidow Elite
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Elite(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0228
    HAS_MATRIX = True
    WAVE_DIRS = (0, 1)
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1398/1398_blackwidowelite.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1398/1398_blackwidowelite.png"
    }


class RazerHuntsman(_RippleKeyboard):
    """
    Class for the Razer Huntsman
    """
    EVENT_FILE_REGEX = re.compile(r'.*Huntsman(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0227
    HAS_MATRIX = True
    WAVE_DIRS = (0, 1)
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1360/1360_huntsman-3.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1360/1360_huntsman-3.png"
    }


class RazerCynosaChroma(_RippleKeyboard):
    """
    Class for the Razer Cynosa Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Cynosa_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x022A
    HAS_MATRIX = True
    WAVE_DIRS = (0, 1)
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1256/1256_cynosa_chroma.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1256/1256_cynosa_chroma.png"
    }


class RazerCynosaLite(_RippleKeyboard):
    """
    Class for the Razer Cynosa Lite
    """
    EVENT_FILE_REGEX = re.compile(r'.*Cynosa_Lite(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x023F
    METHODS = ['get_device_type_keyboard', 'set_static_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro']

    DEVICE_IMAGE = "https://assets2.razerzone.com/images/og-image/cynosa-lite-OGimage.jpg"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets2.razerzone.com/images/og-image/cynosa-lite-OGimage.jpg"
    }


class RazerBlackWidowLite(_RippleKeyboard):
    """
    Class for the Razer BlackWidow Lite
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Lite(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0235
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_static_effect',
               'set_none_effect', 'set_breath_single_effect',
               'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               ]

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1456/1456_blackwidowlite_-_2.png"


class RazerBlackWidow2019(_RippleKeyboard):
    """
    Class for the Razer BlackWidow 2019
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0241
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1501/1501-blackwidow2019.png"


class RazerBlackWidowEssential(_RippleKeyboard):
    """
    Class for the Razer BlackWidow Essential
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow_Essential(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0237
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_static_effect', 'set_none_effect',
               'set_breath_single_effect', 'set_key_row', 'get_game_mode',
               'set_game_mode', 'get_macro_mode', 'set_macro_mode', 'get_macro_effect',
               'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1501/1501-blackwidow2019.png"


class RazerOrnata(_RippleKeyboard):
    """
    Class for the Razer Ornata
    """
    EVENT_FILE_REGEX = re.compile(r'.*Ornata(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x021F
    HAS_MATRIX = True
    WAVE_DIRS = (0, 1)
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_single_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode', 'set_breath_single_effect',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',
               'set_starlight_single_effect', 'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/726/726_ornata.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/726/726_ornata.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/25675/razer_ornata_003.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/25675/razer_ornata_004.png"
    }


class RazerAnansi(_MacroKeyboard):
    """
    Class for the Razer Anansi
    """
    EVENT_FILE_REGEX = re.compile(r'.*Anansi(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x010F
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_device_type_keyboard',
               'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode', 'get_macro_effect',
               'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro', 'set_static_effect',
               'set_spectrum_effect', 'set_none_effect']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/54/54_anansi.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/54/54_anansi.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/58/razer-anansi-gallery-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/58/razer-anansi-gallery-2.png"
    }


class RazerDeathStalkerExpert(_MacroKeyboard):
    """
    Class for the Razer DeathStalker Expert
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_DeathStalker(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0202
    METHODS = ['get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/49/49_razer_deathstalker.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/49/49_razer_deathstalker.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/771/razer-dstalk-gallery-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/771/razer-dstalk-gallery-2.png"
    }


class RazerDeathStalkerChroma(_RippleKeyboard):
    """
    Class for the Razer DeathStalker Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*DeathStalker_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0204
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [1, 6]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/665/665_deathstalker_chroma.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/665/665_deathstalker_chroma.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22563/rzr_deathstalker_chroma_03.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22563/rzr_deathstalker_chroma_02.png"
    }


class RazerBlackWidowChromaOverwatch(_RippleKeyboard):
    """
    Class for the Razer BlackWidow Chroma (Overwatch)
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0211
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [6, 22]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/products/23326/overwatch-razer-gallery-5.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/23326/overwatch-razer-gallery-5.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/23326/overwatch-razer-gallery-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/23326/overwatch-razer-gallery-1.png"
    }


class RazerBladeStealthMid2017(_RippleKeyboard):
    """
    Class for the Razer Blade Stealth (Mid 2017)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Stealth(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x022D
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1213/1213_blade_stealth_2017_7500u.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1213/1213_blade_stealth_2017_7500u.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-22__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-02__store_gallery.png"
    }


class RazerBladePro2017(_RippleKeyboard):
    """
    Class for the Razer Blade Pro (2017)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Pro(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0225
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 25]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour', 'blade_get_logo_active', 'blade_set_logo_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1200/1200_blade_pro_2017.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1200/1200_blade_pro_2017.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-13__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-01__store_gallery.png"
    }


class RazerBladePro2017FullHD(_RippleKeyboard):
    """
    Class for the Razer Blade Pro FullHD (2017)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_ProFullHD(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x022F
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 25]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour', 'blade_get_logo_active', 'blade_set_logo_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1200/1200_blade_pro_2017.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1200/1200_blade_pro_2017.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-13__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-01__store_gallery.png"
    }


class RazerBladeStealthLate2017(_RippleKeyboard):
    """
    Class for the Razer Blade Stealth (Late 2017)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Stealth(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0232
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1213/1213_blade_stealth_2017_7500u.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1213/1213_blade_stealth_2017_7500u.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-22__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26727/rzrblade14-02__store_gallery.png"
    }


class RazerBlade2018(_RippleKeyboard):
    """
    Class for the Razer Blade 15 (2018)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0233
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour', 'get_logo_active', 'set_logo_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1418/1418_blade_2018__base.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1418/1418_blade_2018__base.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-13__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26227/razer-blade-pro-gallery-01__store_gallery.png"
    }


class RazerBlade2018Mercury(_RippleKeyboard):
    """
    Class for the Razer Blade 15 (2018) Mercury
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0240
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_custom_effect', 'set_key_row', 'set_starlight_random_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://d4kkpd69xt9l7.cloudfront.net/sys-master/images/h97/h78/9088805240862/RZ09-02386EM2-R3U1.png_300Wx300H"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://d4kkpd69xt9l7.cloudfront.net/sys-master/images/h97/h78/9088805240862/RZ09-02386EM2-R3U1.png_300Wx300H",
        "side_img": "https://assets2.razerzone.com/images/blade-15/shop/blade15-mercury-7.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/blade-15/shop/blade15-mercury-1.jpg"
    }


class RazerBlade2018Base(_RippleKeyboard):
    """
    Class for the Razer Blade 15 (2018) Base Model
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x023B
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'set_static_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1418/1418_blade_2018__base.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1418/1418_blade_2018__base.png",
        "side_img": "https://assets2.razerzone.com/images/blade-15/shop/blade15-d1-6.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/blade-15/shop/blade15-d1-1.jpg"
    }


class RazerBladeStealth2019(_RippleKeyboard):
    """
    Class for the Razer Blade Stealth (2019)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Stealth(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0239
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'set_static_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1475/1475_bladestealth13(2019).png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1475/1475_bladestealth13(2019).png",
        "side_img": "https://assets.razerzone.com/eeimages/hazel3/img/gallery/blade_stealth_h3_black_3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/support/products/1475/1475_bladestealth13(2019).png"
    }


class RazerBladeStealthLate2019(_RippleKeyboard):
    """
    Class for the Razer Blade Stealth (Late 2019)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Stealth(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x024A
    METHODS = ['get_device_type_keyboard', 'set_static_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect']

    DEVICE_IMAGE = "https://assets2.razerzone.com/images/blade-stealth-13/shop/stealth-l2p-1.jpg"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets2.razerzone.com/images/blade-stealth-13/shop/stealth-l2p-2.jpg",
        "side_img": "https://assets2.razerzone.com/images/blade-stealth-13/shop/stealth-l2p-3.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/blade-stealth-13/shop/stealth-l2p-1.jpg"
    }


class RazerBlade2019Adv(_RippleKeyboard):
    """
    Class for the Razer Blade 15 (2019) Advanced
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x023A
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1482/blade15.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1482/blade15.png",
        "side_img": "https://assets2.razerzone.com/images/blade-15/blade15-usp-ports-advanced-model.jpg",
        "perspective_img": "https://d4kkpd69xt9l7.cloudfront.net/sys-master/root/h78/h94/9126618628126/razer-blade-15-gallery07.jpg"
    }


class RazerBladeMid2019Mercury(_RippleKeyboard):
    """
    Class for the Razer Blade 15 (Mid 2019) Mercury
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0245
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'get_logo_active', 'set_logo_active', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets2.razerzone.com/images/blade-15/shop/blade15-mercury-1.jpg"


class RazerBlade2019Base(_RippleKeyboard):
    """
    Class for the Razer Blade 15 (Mid 2019) Base Model
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0246
    METHODS = ['get_device_type_keyboard', 'set_static_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1518/1518_blade15_mid2019-base.png"


class RazerBladeProLate2019(_RippleKeyboard):
    """
    Class for the Razer Blade Pro (Late 2019)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x024C
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'get_logo_active', 'set_logo_active', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets2.razerzone.com/images/razer-blade-pro-17/razer-blade-pro-17-2019-OGimage-1200x630.jpg"


class RazerBlade2019StudioEdition(_RippleKeyboard):
    """
    Class for the Razer Blade 15 Studio Edition (2019)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x024D
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets2.razerzone.com/images/blade-15/shop/studio-ch41-1.jpg"


class RazerBladePro2019(_RippleKeyboard):
    """
    Class for the Razer Blade Pro 17 (2019)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0234
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 16]
    METHODS = ['get_device_type_keyboard', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_ripple_effect', 'set_ripple_effect_random_colour']

    DEVICE_IMAGE = "https://assets2.razerzone.com/images/razer-blade-pro-17/razer-blade-pro-17-2019-OGimage-1200x630.jpg"
