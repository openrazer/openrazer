"""
Keyboards class
"""
import re

from razer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend
from razer_daemon.misc.key_event_management import KeyManager
from razer_daemon.misc.ripple_effect import RippleManager


class MacroKeyboard(RazerDeviceBrightnessSuspend):
    """
    Keyboard class

    Has macro functionality and brightness based suspend
    """

    def __init__(self, *args):
        super(MacroKeyboard, self).__init__(*args)
        # Methods are loaded into DBus by this point

        self.key_manager = KeyManager(self._device_number, self.event_files, self)

    def _close(self):
        """
        Close the key manager
        """
        super(MacroKeyboard, self)._close()

        # TODO look into saving in /var/run maybe
        self.key_manager.close()


class RazerBlackWidow2013(MacroKeyboard):
    """
    Class for the BlackWidow Ultimate 2013
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_BlackWidow_Ultimate_2013(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x011A
    METHODS = ['get_firmware', 'get_brightness', 'enable_macro_keys', 'set_brightness', 'get_device_type', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static', 'get_macros', 'delete_macro', 'add_macro']

class RazerBlackWidowChroma(MacroKeyboard):
    """
    Class for the BlackWidow Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*BlackWidow_Chroma(-if01)?-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0203
    METHODS = ['get_firmware', 'get_device_type', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'enable_macro_keys', 'get_game_mode', 'set_game_mode', 'get_macro_mode', 'set_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'get_macros', 'delete_macro', 'add_macro',

               'set_ripple_effect']

    def __init__(self, *args):
        super(RazerBlackWidowChroma, self).__init__(*args)

        self.ripple_manager = RippleManager(self, self._device_number)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerBlackWidowChroma, self)._close()

        self.ripple_manager.close()


