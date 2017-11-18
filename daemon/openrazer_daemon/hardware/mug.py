"""
Mug class
"""
import re

from openrazer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as _RazerDeviceBrightnessSuspend


class RazerChromaMugHolder(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Chroma Mug Holder
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Chroma_Mug_Holder-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0F07
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 15]
    DEDICATED_MACRO_KEYS = False
    METHODS = ['get_firmware', 'get_device_name', 'get_device_type_mug', 'has_matrix', 'get_matrix_dims',
               'set_static_effect', 'set_spectrum_effect', 'set_wave_effect', 'set_none_effect', 'set_breath_single_effect', 'set_breath_dual_effect', 'set_breath_random_effect', 'set_blinking_effect',
               'get_brightness', 'set_brightness', 'is_mug_present',
               'set_custom_effect', 'set_key_row']

    RAZER_URLS = {
        "top_img": "https://assets2.razerzone.com/images/mug-holder/e64e507b73e61c44789d996065fd9645-1500x1000mug_01.jpg",
        "side_img": None,
        "perspective_img": "https://assets2.razerzone.com/images/mug-holder/e64e507b73e61c44789d996065fd9645-1500x1000mug_01.jpg"
    }

    def __init__(self, *args, **kwargs):
        super(RazerChromaMugHolder, self).__init__(*args, **kwargs)

    def _close(self):
        super(RazerChromaMugHolder, self)._close()
