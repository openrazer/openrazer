"""
Keyboards class
"""
from razer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend


class RazerBlackWidow2013(RazerDeviceBrightnessSuspend):
    """
    Class for the BlackWidow Ultimate 2013
    """
    USB_VID = 0x1532
    USB_PID = 0x011A
    METHODS = ['get_firmware', 'get_brightness', 'enable_macro_keys', 'set_brightness', 'get_device_type', 'get_game_mode', 'set_game_mode', 'set_macro_mode', 'get_macro_mode',
               'get_macro_effect', 'set_macro_effect', 'bw_get_effect', 'bw_set_pulsate', 'bw_set_static']
