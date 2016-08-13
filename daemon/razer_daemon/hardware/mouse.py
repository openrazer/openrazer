"""
Mouse class
"""
from razer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend
from razer_daemon.misc.battery_notifier import BatteryManager


class RazerMambaChromaWireless(RazerDeviceBrightnessSuspend):
    """
    Class for the BlackWidow Chroma
    """
    USB_VID = 0x1532
    USB_PID = 0x0045
    HAS_MATRIX = True
    MATRIX_DIMS = [1, -1]  # 6 Rows, 22 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_brightness', 'set_brightness', 'get_battery', 'is_charging', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'set_charge_effect', 'set_charge_colour', 'set_idle_time', 'set_low_battery_threshold', 'set_dpi_xy']

    def __init__(self, *args):
        super(RazerMambaChromaWireless, self).__init__(*args)

        self._battery_manager = BatteryManager(self, self._device_number, 'Razer Mamba')

    def _close(self):
        """
        Close the key manager
        """
        super(RazerMambaChromaWireless, self)._close()

        self._battery_manager.close()