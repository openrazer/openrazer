# SPDX-License-Identifier: GPL-2.0-or-later

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
               'set_reactive_effect', 'trigger_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_device_type_core']


class RazerCoreXChroma(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Core X Chroma
    """
    USB_VID = 0x1532
    USB_PID = 0x0F1A
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 16]

    DEVICE_IMAGE = "https://hybrismediaprod.blob.core.windows.net/sys-master-phoenix-images-container/h44/h3d/9084457943070/core-x-chroma-gallery1.jpg"

    METHODS = ['set_wave_effect', 'set_static_effect', 'set_spectrum_effect',
               'set_reactive_effect', 'trigger_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               'set_starlight_random_effect', 'set_starlight_single_effect', 'set_starlight_dual_effect',
               'set_custom_effect', 'set_key_row', 'get_device_type_core']
