"""
Mouse class
"""
import re
from razer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as __RazerDeviceBrightnessSuspend, RazerDevice as __RazerDevice
from razer_daemon.misc.battery_notifier import BatteryManager as _BatteryManager
from razer_daemon.dbus_services.dbus_methods.deathadder_chroma import get_logo_brightness as _da_get_logo_brightness, set_logo_brightness as _da_set_logo_brightness, get_scroll_brightness as _da_get_scroll_brightness, set_scroll_brightness as _da_set_scroll_brightness, set_logo_active as _da_set_logo_active, set_scroll_active as _da_set_scroll_active
from razer_daemon.dbus_services.dbus_methods.chroma_keyboard import get_brightness as _get_backlight_brightness, set_brightness as _set_backlight_brightness
from razer_daemon.misc.key_event_management import NagaHexV2KeyManager as _NagaHexV2KeyManager

class RazerMambaChromaWireless(__RazerDeviceBrightnessSuspend):
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
               'set_charge_effect', 'set_charge_colour', 'set_idle_time', 'set_low_battery_threshold', 'get_dpi_xy', 'set_dpi_xy']

    def __init__(self, *args, **kwargs):
        super(RazerMambaChromaWireless, self).__init__(*args, **kwargs)

        self._battery_manager = _BatteryManager(self, self._device_number, 'Razer Mamba')

    def _close(self):
        """
        Close the key manager
        """
        super(RazerMambaChromaWireless, self)._close()

        self._battery_manager.close()


class RazerMambaChromaWired(__RazerDeviceBrightnessSuspend):
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
               'get_dpi_xy', 'set_dpi_xy']

    def __init__(self, *args, **kwargs):
        super(RazerMambaChromaWired, self).__init__(*args, **kwargs)


class RazerMambaChromaTE(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Mamba Chroma (Wired)
    """
    USB_VID = 0x1532
    USB_PID = 0x0046
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 16]  # 1 Row, 16 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_brightness', 'set_brightness', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'get_dpi_xy', 'set_dpi_xy']

    def __init__(self, *args, **kwargs):
        super(RazerMambaChromaTE, self).__init__(*args, **kwargs)


class RazerAbyssus(__RazerDevice):
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


class RazerImperiator(__RazerDevice):
    """
    Class for the Razer Imperiator 2012
    """

    USB_VID = 0x1532
    USB_PID = 0x002F
    HAS_MATRIX = False
    MATRIX_DIMS = [-1, -1]  # 1 Row, 15 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'set_logo_active', 'get_logo_active', 'get_dpi_xy', 'set_dpi_xy',
               'get_poll_rate', 'set_poll_rate', 'set_scroll_active', 'get_scroll_active']

    def __init__(self, *args, **kwargs):
        super(RazerImperiator, self).__init__(*args, **kwargs)

    def _resume_device(self):
        self.logger.debug("Imperiator doesnt have suspend/resume")

    def _suspend_device(self):
        self.logger.debug("Imperiator doesnt have suspend/resume")


class RazerOrochiWired(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Mamba Chroma (Wireless)
    """
    USB_VID = 0x1532
    USB_PID = 0x0048
    HAS_MATRIX = False
    MATRIX_DIMS = [-1, -1]  # 1 Row, 15 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_brightness', 'set_brightness',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_idle_time', 'set_low_battery_threshold', 'get_dpi_xy', 'set_dpi_xy', 'set_scroll_active', 'get_scroll_active']


class RazerDeathadderChroma(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Deathadder Chroma
    """
    USB_VID = 0x1532
    USB_PID = 0x0043
    HAS_MATRIX = False
    MATRIX_DIMS = [-1, -1]  # 1 Row, 15 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse',
               'set_logo_active', 'get_logo_active', 'get_logo_effect', 'get_logo_brightness', 'set_logo_brightness', 'set_logo_static', 'set_logo_pulsate', 'set_logo_blinking', 'set_logo_spectrum',
               'set_scroll_active', 'get_scroll_active', 'get_scroll_effect', 'get_scroll_brightness', 'set_scroll_brightness', 'set_scroll_static', 'set_scroll_pulsate', 'set_scroll_blinking', 'set_scroll_spectrum',
               'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate']

    def __init__(self, *args, **kwargs):
        super(RazerDeathadderChroma, self).__init__(*args, **kwargs)

        # Set brightness to max and LEDs to on, on startup
        _da_set_logo_brightness(self, 100)
        _da_set_scroll_brightness(self, 100)
        _da_set_logo_active(self, True)
        _da_set_scroll_active(self, True)


    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = (_da_get_logo_brightness(self), _da_get_scroll_brightness(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_brightness(self, 0)
        _da_set_scroll_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_brightness = self.suspend_args.get('brightness', (100, 100))[0]
        scroll_brightness = self.suspend_args.get('brightness', (100, 100))[1]

        self.disable_notify = True
        _da_set_logo_brightness(self, logo_brightness)
        _da_set_scroll_brightness(self, scroll_brightness)
        self.disable_notify = False


class RazerNagaHexV2(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Deathadder Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Naga_Hex_V2-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0050
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [-1, -1]  # 1 Row, 15 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness',
               # Thumbgrid is technically backlight ID
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect','set_breath_single_effect', 'set_breath_dual_effect',
               # Logo
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # #Macros
               'get_macros', 'delete_macro', 'add_macro']

    def __init__(self, *args, **kwargs):
        super(RazerNagaHexV2, self).__init__(*args, **kwargs)

        self.key_manager = _NagaHexV2KeyManager(self._device_number, self.event_files, self, use_epoll=True, testing=self._testing, should_grab_event_files=True)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerNagaHexV2, self)._close()

        self.key_manager.close()

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = (_da_get_logo_brightness(self), _da_get_scroll_brightness(self), _get_backlight_brightness(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_brightness(self, 0)
        _da_set_scroll_brightness(self, 0)
        _set_backlight_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_brightness = self.suspend_args.get('brightness', (100, 100, 100))[0]
        scroll_brightness = self.suspend_args.get('brightness', (100, 100, 100))[1]
        backlight_brightness = self.suspend_args.get('brightness', (100, 100, 100))[2]

        self.disable_notify = True
        _da_set_logo_brightness(self, logo_brightness)
        _da_set_scroll_brightness(self, scroll_brightness)
        _set_backlight_brightness(self, backlight_brightness)
        self.disable_notify = False


class RazerDeathadderElite(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Deathadder Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_DeathAdder_Elite-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x005c
    HAS_MATRIX = False
    MATRIX_DIMS = [-1, -1]  # 1 Row, 15 Cols
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness',
               # Logo
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2']

    def __init__(self, *args, **kwargs):
        super(RazerDeathadderElite, self).__init__(*args, **kwargs)

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = (_da_get_logo_brightness(self), _da_get_scroll_brightness(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_brightness(self, 0)
        _da_set_scroll_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_brightness = self.suspend_args.get('brightness', (100, 100))[0]
        scroll_brightness = self.suspend_args.get('brightness', (100, 100))[1]

        self.disable_notify = True
        _da_set_logo_brightness(self, logo_brightness)
        _da_set_scroll_brightness(self, scroll_brightness)
        self.disable_notify = False
