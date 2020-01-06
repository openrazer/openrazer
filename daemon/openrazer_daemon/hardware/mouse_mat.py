"""
Mousemat class
"""
from openrazer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as __RazerDeviceBrightnessSuspend


class RazerFirefly(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Firefly (2013)
    """
    USB_VID = 0x1532
    USB_PID = 0x0C00
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 15]
    METHODS = ['get_device_type_mousemat', 'set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'trigger_reactive_effect']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/594/594_firefly_500x500.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/594/594_firefly_500x500.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/21936/fireflycloth-gallery-6.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/21936/fireflycloth-gallery-3.png"
    }


class RazerFirefly_Hyperflux(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Firefly_Hyperflux (2018)
    """
    USB_VID = 0x1532
    USB_PID = 0x0068
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 1]
    METHODS = ['get_device_type_mousemat', 'set_static_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_key_row']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/594/594_firefly_500x500.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/594/594_firefly_500x500.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/21936/fireflycloth-gallery-6.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/21936/fireflycloth-gallery-3.png"
    }


class RazerGoliathus(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Goliathus (2018)
    """
    USB_VID = 0x1532
    USB_PID = 0x0C01
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 1]
    METHODS = ['get_device_type_mousemat', 'set_static_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1394/1394_goliathus_chroma.png"


class RazerGoliathusExtended(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Goliathus Extended (2018)
    """
    USB_VID = 0x1532
    USB_PID = 0x0C02
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 1]
    METHODS = ['get_device_type_mousemat', 'set_static_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1395/1395_goliathusextended.png"
