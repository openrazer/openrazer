"""
Monitor class
"""

from openrazer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as _RazerDeviceBrightnessSuspend


class RazerRaptor27(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Raptor 27 2560x1440
    """
    USB_VID = 0x1532
    USB_PID = 0x0F12
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 12]
    METHODS = ['get_device_type_accessory', 'set_static_effect', 'set_wave_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row']

    DEVICE_IMAGE = "https://assets3.razerzone.com/lUNSB5GzMBqJ8sWx2nsfm6u97zo=/1500x1000/https%3A%2F%2Fhybrismediaprod.blob.core.windows.net%2Fsys-master-phoenix-images-container%2Fha0%2Fh53%2F9081460162590%2Fraptor-27-gallery-1.jpg"
