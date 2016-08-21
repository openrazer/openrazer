"""
Mousemat class
"""
from razer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend


class RazerFireFly(RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Firefly 2013
    """
    USB_VID = 0x1532
    USB_PID = 0x0C00
    HAS_MATRIX = True
    MATRIX_DIMS = [1, -1]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_firefly', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row']
