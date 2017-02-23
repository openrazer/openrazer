"""
Mousemat class
"""
from razer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as __RazerDeviceBrightnessSuspend


class RazerFireFly(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Firefly (2013)
    """
    USB_VID = 0x1532
    USB_PID = 0x0C00
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 15]  # 6 Rows, 22 Cols

    RAZER_URLS = {
        "store": "http://www.razerzone.com/gb-en/store/razer-firefly",
        "top_img": "http://assets.razerzone.com/eeimages/products/21936/rzr_firefly_gallery-2.png",
        "side_img": "http://assets.razerzone.com/eeimages/products/21936/fireflycloth-gallery-6.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/21936/fireflycloth-gallery-3.png"
    }

    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_firefly', 'get_brightness', 'set_brightness', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row']
