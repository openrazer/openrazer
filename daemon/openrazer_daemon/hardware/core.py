"""
Core class
"""
from openrazer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as __RazerDeviceBrightnessSuspend


class RazerCore(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Core
    """
    USB_VID = 0x1532
    USB_PID = 0x0215
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 9]

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/684/684_razer_core.png"

    METHODS = ['set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_device_type_core']
