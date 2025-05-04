# SPDX-License-Identifier: GPL-2.0-or-later

"""
Monitor class
"""
from openrazer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as _RazerDeviceBrightnessSuspend


class RazerRaptor27(_RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Raptor 27
    """
    USB_VID = 0x1532
    USB_PID = 0x0F12
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 12]
    METHODS = ['get_device_type_accessory', 'set_static_effect', 'set_wave_effect', 'set_spectrum_effect',
               'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row']

    DEVICE_IMAGE = "https://hybrismediaprod.blob.core.windows.net/sys-master-phoenix-images-container/ha0/h53/9081460162590/raptor-27-gallery-1.jpg"
