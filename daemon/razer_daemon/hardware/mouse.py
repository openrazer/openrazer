"""
Mouse class
"""
from razer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend, RazerDevice
from razer_daemon.misc.battery_notifier import BatteryManager


class RazerMambaChromaWireless(RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Mamba Chroma (Wireless)
    """
    USB_VID = 0x1532
    USB_PID = 0x0045
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 15]  # 1 Row, 15 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_brightness', 'set_brightness', 'get_battery', 'is_charging', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'set_charge_effect', 'set_charge_colour', 'set_idle_time', 'set_low_battery_threshold', 'set_dpi_xy']

    def __init__(self, *args, **kwargs):
        super(RazerMambaChromaWireless, self).__init__(*args, **kwargs)

        self._battery_manager = BatteryManager(self, self._device_number, 'Razer Mamba')

    def _close(self):
        """
        Close the key manager
        """
        super(RazerMambaChromaWireless, self)._close()

        self._battery_manager.close()

class RazerMambaChromaWired(RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Mamba Chroma (Wired)
    """
    USB_VID = 0x1532
    USB_PID = 0x0044
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 15]  # 1 Row, 15 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_brightness', 'set_brightness', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'set_dpi_xy']

    def __init__(self, *args, **kwargs):
        super(RazerMambaChromaWired, self).__init__(*args, **kwargs)

class RazerMambaChromaTE(RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Mamba Chroma (Wired)
    """
    USB_VID = 0x1532
    USB_PID = 0x0046
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 15]  # 1 Row, 15 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_brightness', 'set_brightness', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'set_dpi_xy', 'set_te_logo']

    def __init__(self, *args, **kwargs):
        super(RazerMambaChromaTE, self).__init__(*args, **kwargs)

class RazerAbyssus(RazerDevice):
    """
    Class for the Razer Abyssus
    """

    USB_VID = 0x1532
    USB_PID = 0x0042
    HAS_MATRIX = False
    MATRIX_DIMS = [-1, -1]  # 1 Row, 15 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'set_logo_active', 'get_logo_active']

    def __init__(self, *args, **kwargs):
        super(RazerAbyssus, self).__init__(*args, **kwargs)

    def _resume_device(self):
        self.logger.debug("Abyssus doesnt have suspend/resume")

    def _suspend_device(self):
        self.logger.debug("Abyssus doesnt have suspend/resume")

class RazerImperiator(RazerDevice):
    """
    Class for the Razer Imperiator 2012
    """

    USB_VID = 0x1532
    USB_PID = 0x002F
    HAS_MATRIX = False
    MATRIX_DIMS = [-1, -1]  # 1 Row, 15 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'set_logo_active', 'get_logo_active', 'set_dpi_xy',
               'set_poll_rate', 'set_scroll_active', 'get_scroll_active']

    def __init__(self, *args, **kwargs):
        super(RazerImperiator, self).__init__(*args, **kwargs)

    def _resume_device(self):
        self.logger.debug("Imperiator doesnt have suspend/resume")

    def _suspend_device(self):
        self.logger.debug("Imperiator doesnt have suspend/resume")
