"""
Mug class
"""
import re

from razer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as _RazerDeviceBrightnessSuspend
from razer_daemon.misc.key_event_management import KeyboardKeyManager as _KeyboardKeyManager, TartarusKeyManager as _TartarusKeyManager

class RazerChromaMugHolder(_RazerDeviceBrightnessSuspend):

    """
    Class for the Kraken
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Chroma_Mug_Holder-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0F07
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [-1, -1]
    METHODS = ['get_firmware', 'get_device_name', 'get_device_type_mug', 'has_matrix', 'get_matrix_dims',
               'set_static_effect', 'set_spectrum_effect', 'set_wave_effect', 'set_none_effect', 'set_breath_single_effect', 'set_breath_dual_effect', 'set_breath_random_effect', 'set_blinking_effect',
               'get_brightness', 'set_brightness', 'is_mug_present']

    def __init__(self, *args, **kwargs):
        super(RazerChromaMugHolder, self).__init__(*args, **kwargs)

    def _close(self):
        super(RazerChromaMugHolder, self)._close()
