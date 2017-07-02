"""
Core class
"""
from razer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as __RazerDeviceBrightnessSuspend


class RazerCore(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Core
    """
    USB_VID = 0x1532
    USB_PID = 0x0215
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 9]  # 6 Rows, 22 Cols

    RAZER_URLS = {
        "store": "https://www.razerzone.com/gaming-systems/razer-core",
        "top_img": "https://assets2.razerzone.com/images/razer-core/d0a4b25e594c263397b704ac9e8e0490-rzr-core-1500x1000-01.jpg",
        "side_img": "https://assets2.razerzone.com/images/razer-core/1b215ac8e728cd71225edf95b8a18f1c-rzr-core-1500x1000-05.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/razer-core/01689cb99a40585d574712f3ca82838a-rzr-core-1500x1000-04.jpg"
    }

    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_device_type_core']
