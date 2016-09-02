"""
Keyboards class
"""
import re

from razer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend
from razer_daemon.misc.key_event_management import KeyboardKeyManager, TartarusKeyManager
from razer_daemon.misc.ripple_effect import RippleManager


class MacroKeyboard(RazerDeviceBrightnessSuspend):
    """
    Keyboard class

    Has macro functionality and brightness based suspend
    """

    def __init__(self, *args):
        super(MacroKeyboard, self).__init__(*args)
        # Methods are loaded into DBus by this point

        self.key_manager = KeyboardKeyManager(self._device_number, self.event_files, self)

    def _close(self):
        """
        Close the key manager
        """
        super(MacroKeyboard, self)._close()

        # TODO look into saving stats in /var/run maybe
        self.key_manager.close()


class RazerTartarus(RazerDeviceBrightnessSuspend):
    """
        Keyboard class

        Has macro functionality and brightness based suspend
        """

    EVENT_FILE_REGEX = re.compile(r'.*Razer_Tartarus_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0208
    HAS_MATRIX = False
    MATRIX_DIMS = [-1, -1]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'set_brightness', 'get_device_name', 'get_device_type_tartarus', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_static_effect', 'set_spectrum_effect', 'tartaris_get_profile_led_red', 'tartaris_set_profile_led_red', 'tartaris_get_profile_led_green',
               'tartaris_set_profile_led_green', 'tartaris_get_profile_led_blue', 'tartaris_set_profile_led_blue', 'get_macros', 'delete_macro', 'add_macro']

    def __init__(self, *args):
        super(RazerTartarus, self).__init__(*args)
        # Methods are loaded into DBus by this point

        self.key_manager = TartarusKeyManager(self._device_number, self.event_files, self)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerTartarus, self)._close()

        # TODO look into saving stats in /var/run maybe
        self.key_manager.close()


class RazerBlackWidow2013(MacroKeyboard):
    """
    Class for the BlackWidow Ultimate 2013
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow_Ultimate_2013(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x011A
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22] # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_brightness', 'enable_macro_keys', 'set_brightness', 'get_device_name', 'get_device_type_keyboard', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']


class RazerBlackWidowChroma(MacroKeyboard):
    """
    Class for the BlackWidow Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0203
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'enable_macro_keys', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    def __init__(self, *args):
        super(RazerBlackWidowChroma, self).__init__(*args)

        self.ripple_manager = RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidowChroma, self)._close()

        self.ripple_manager.close()


class RazerBlackWidowChromaTournamentEdition(MacroKeyboard):
    """
    Class for the BlackWidow Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Tournament_Edition_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0209
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix',  'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'enable_macro_keys', 'get_game_mode', 'set_game_mode',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    def __init__(self, *args):
        super(RazerBlackWidowChromaTournamentEdition, self).__init__(*args)

        self.ripple_manager = RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidowChromaTournamentEdition, self)._close()

        self.ripple_manager.close()


class RazerBlackWidowChromaX(MacroKeyboard):
    """
    Class for the BlackWidow Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_X_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0203
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'enable_macro_keys', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    def __init__(self, *args):
        super(RazerBlackWidowChroma, self).__init__(*args)

        self.ripple_manager = RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidowChromaX, self)._close()

        self.ripple_manager.close()


class RazerBladeStealth(MacroKeyboard):
    """
    Class for the BlackWidow Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Blade_Stealth(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0205
    HAS_MATRIX = True
    MATRIX_DIMS = [6, 22]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_keyboard', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_game_mode', 'set_game_mode',

               'set_ripple_effect', 'set_ripple_effect_random_colour']

    def __init__(self, *args):
        super(RazerBlackWidowChroma, self).__init__(*args)

        self.ripple_manager = RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBladeStealth, self)._close()

        self.ripple_manager.close()