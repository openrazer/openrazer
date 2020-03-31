"""
Mouse class
"""
import re
from openrazer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as __RazerDeviceBrightnessSuspend, RazerDeviceSpecialBrightnessSuspend as __RazerDeviceSpecialBrightnessSuspend, RazerDevice as __RazerDevice
from openrazer_daemon.misc.battery_notifier import BatteryManager as _BatteryManager
# TODO replace with plain import
from openrazer_daemon.dbus_services.dbus_methods.deathadder_chroma import get_logo_brightness as _da_get_logo_brightness, set_logo_brightness as _da_set_logo_brightness, \
    get_scroll_brightness as _da_get_scroll_brightness, set_scroll_brightness as _da_set_scroll_brightness, set_logo_active as _da_set_logo_active, \
    set_scroll_active as _da_set_scroll_active, get_scroll_active as _da_get_scroll_active, get_logo_active as _da_get_logo_active, set_backlight_active as _da_set_backlight_active, \
    get_backlight_active as _da_get_backlight_active
from openrazer_daemon.dbus_services.dbus_methods.chroma_keyboard import get_brightness as _get_backlight_brightness, set_brightness as _set_backlight_brightness
from openrazer_daemon.misc.key_event_management import NagaHexV2KeyManager as _NagaHexV2KeyManager
from openrazer_daemon.dbus_services.dbus_methods.lanceheadte import get_left_brightness as _get_left_brightness, get_right_brightness as _get_right_brightness, \
    set_left_brightness as _set_left_brightness, set_right_brightness as _set_right_brightness


class RazerLanceheadWirelessWired(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Lancehead Wireless (Wired)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Lancehead_Wireless-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0070
    HAS_MATRIX = True
    WAVE_DIRS = (1, 2)
    MATRIX_DIMS = [1, 16]
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate', 'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness',
               'get_left_brightness', 'set_left_brightness', 'get_right_brightness', 'set_right_brightness',
               # Battery
               'get_battery', 'is_charging', 'set_idle_time', 'set_low_battery_threshold',
               # Logo
               'set_logo_wave', 'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_wave', 'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # Left side
               'set_left_wave', 'set_left_static', 'set_left_spectrum', 'set_left_none', 'set_left_reactive', 'set_left_breath_random', 'set_left_breath_single', 'set_left_breath_dual',
               # Right side
               'set_right_wave', 'set_right_static', 'set_right_spectrum', 'set_right_none', 'set_right_reactive', 'set_right_breath_random', 'set_right_breath_single', 'set_right_breath_dual',
               # Can set LOGO and Scroll with custom
               'set_custom_effect', 'set_key_row']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1205/1205_lancehead.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1205/1205_lancehead.png"
    }

    DPI_MAX = 16000

    def _suspend_device(self):
        """
        Suspend the device
        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = (_da_get_logo_brightness(self), _da_get_scroll_brightness(self), _get_left_brightness(self), _get_right_brightness(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_brightness(self, 0)
        _da_set_scroll_brightness(self, 0)
        _set_left_brightness(self, 0)
        _set_right_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device
        Get the last known brightness and then set the brightness
        """
        logo_brightness = self.suspend_args.get('brightness', (100, 100))[0]
        scroll_brightness = self.suspend_args.get('brightness', (100, 100))[1]
        left_row_brightness = self.suspend_args.get('brightness', (100, 100))[2]
        right_row_brightness = self.suspend_args.get('brightness', (100, 100))[3]
        self.disable_notify = True
        _da_set_logo_brightness(self, logo_brightness)
        _da_set_scroll_brightness(self, scroll_brightness)
        _set_left_brightness(self, left_row_brightness)
        _set_right_brightness(self, right_row_brightness)
        self.disable_notify = False


class RazerLanceheadWirelessReceiver(RazerLanceheadWirelessWired):
    """
    Class for the Razer Lancehead Wireless (Receiver)
    """
    USB_PID = 0x006F
    METHODS = RazerLanceheadWirelessWired.METHODS + ['set_charge_effect', 'set_charge_colour']

    def __init__(self, *args, **kwargs):
        super(RazerLanceheadWirelessReceiver, self).__init__(*args, **kwargs)

        self._battery_manager = _BatteryManager(self, self._device_number, 'Razer Lancehead Wireless')
        self._battery_manager.active = self.config.getboolean('Startup', 'mouse_battery_notifier', fallback=False)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerLanceheadWirelessReceiver, self)._close()

        self._battery_manager.close()


class RazerLanceheadWired(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Lancehead (Wired)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Lancehead-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0059
    HAS_MATRIX = True
    WAVE_DIRS = (1, 2)
    MATRIX_DIMS = [1, 16]
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate', 'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness',
               'get_left_brightness', 'set_left_brightness', 'get_right_brightness', 'set_right_brightness',
               # Battery
               'get_battery', 'is_charging', 'set_idle_time', 'set_low_battery_threshold',
               # Logo
               'set_logo_wave', 'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_wave', 'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # Left side
               'set_left_wave', 'set_left_static', 'set_left_spectrum', 'set_left_none', 'set_left_reactive', 'set_left_breath_random', 'set_left_breath_single', 'set_left_breath_dual',
               # Right side
               'set_right_wave', 'set_right_static', 'set_right_spectrum', 'set_right_none', 'set_right_reactive', 'set_right_breath_random', 'set_right_breath_single', 'set_right_breath_dual',
               # Can set LOGO and Scroll with custom
               'set_custom_effect', 'set_key_row']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1205/1205_lancehead.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1205/1205_lancehead.png"
    }

    DPI_MAX = 16000

    def _suspend_device(self):
        """
        Suspend the device
        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = (_da_get_logo_brightness(self), _da_get_scroll_brightness(self), _get_left_brightness(self), _get_right_brightness(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_brightness(self, 0)
        _da_set_scroll_brightness(self, 0)
        _set_left_brightness(self, 0)
        _set_right_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device
        Get the last known brightness and then set the brightness
        """
        logo_brightness = self.suspend_args.get('brightness', (100, 100))[0]
        scroll_brightness = self.suspend_args.get('brightness', (100, 100))[1]
        left_row_brightness = self.suspend_args.get('brightness', (100, 100))[2]
        right_row_brightness = self.suspend_args.get('brightness', (100, 100))[3]
        self.disable_notify = True
        _da_set_logo_brightness(self, logo_brightness)
        _da_set_scroll_brightness(self, scroll_brightness)
        _set_left_brightness(self, left_row_brightness)
        _set_right_brightness(self, right_row_brightness)
        self.disable_notify = False


class RazerLanceheadWireless(RazerLanceheadWired):
    """
    Class for the Razer Lancehead (Wireless)
    """
    USB_PID = 0x005A
    METHODS = RazerLanceheadWired.METHODS + ['set_charge_effect', 'set_charge_colour']

    def __init__(self, *args, **kwargs):
        super(RazerLanceheadWireless, self).__init__(*args, **kwargs)

        self._battery_manager = _BatteryManager(self, self._device_number, 'Razer Lancehead')
        self._battery_manager.active = self.config.getboolean('Startup', 'mouse_battery_notifier', fallback=False)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerLanceheadWireless, self)._close()

        self._battery_manager.close()


class RazerDeathAdderEssentialWhiteEdition(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer DeathAdder Essential (White Edition)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Razer_DeathAdder_Essential_White_Edition-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0071
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy',
               'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness',
               'get_scroll_brightness', 'set_scroll_brightness',
               # Logo
               'set_logo_static_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2',
               # Scroll wheel
               'set_scroll_static_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2']

    DEVICE_IMAGE = "https://assets2.razerzone.com/images/da10m/carousel/razer-death-adder-gallery-25.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets2.razerzone.com/images/da10m/carousel/razer-death-adder-gallery-25.png"
    }

    DPI_MAX = 6400

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


class RazerAbyssusEliteDVaEdition(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Abyssus Elite (D.Va Edition)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_D.Va_Razer_Abyssus_Elite-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x006A
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy',
               'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness',
               # Underglow/Logo use LOGO_LED
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2',
               'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1288/d.va_abyssus_elite.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1288/d.va_abyssus_elite.png"
    }

    DPI_MAX = 7200

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args['brightness'] = _da_get_logo_brightness(self)

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_brightness = self.suspend_args.get('brightness', 100)

        self.disable_notify = True
        _da_set_logo_brightness(self, logo_brightness)
        self.disable_notify = False


class RazerAbyssusEssential(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Abyssus Essential
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Razer_Abyssus_Essential-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x006B
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy',
               'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness',
               # Backlight/Logo...same
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2',
               'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1290/1290_abyssusessential.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1290/1290_abyssusessential.png"
    }

    DPI_MAX = 7200

    def _suspend_device(self):
        """
        Suspend the device
        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args['brightness'] = _da_get_logo_brightness(self)

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device
        Get the last known brightness and then set the brightness
        """
        logo_brightness = self.suspend_args.get('brightness', 100)

        self.disable_notify = True
        _da_set_logo_brightness(self, logo_brightness)
        self.disable_notify = False


class RazerLanceheadTE(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Lancehead Tournament Edition
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Lancehead_TE-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0060
    HAS_MATRIX = True
    WAVE_DIRS = (1, 2)
    MATRIX_DIMS = [1, 16]
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate', 'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness',
               'get_left_brightness', 'set_left_brightness', 'get_right_brightness', 'set_right_brightness',
               # Logo
               'set_logo_wave', 'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_wave', 'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # Left side
               'set_left_wave', 'set_left_static', 'set_left_spectrum', 'set_left_none', 'set_left_reactive', 'set_left_breath_random', 'set_left_breath_single', 'set_left_breath_dual',
               # Right side
               'set_right_wave', 'set_right_static', 'set_right_spectrum', 'set_right_none', 'set_right_reactive', 'set_right_breath_random', 'set_right_breath_single', 'set_right_breath_dual',
               # Can set LOGO and Scroll with custom
               'set_custom_effect', 'set_key_row']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1203/1206_lanceheadte.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1203/1206_lanceheadte.png"
    }

    DPI_MAX = 16000

    def _suspend_device(self):
        """
        Suspend the device
        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = (_da_get_logo_brightness(self), _da_get_scroll_brightness(self), _get_left_brightness(self), _get_right_brightness(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_brightness(self, 0)
        _da_set_scroll_brightness(self, 0)
        _set_left_brightness(self, 0)
        _set_right_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device
        Get the last known brightness and then set the brightness
        """
        logo_brightness = self.suspend_args.get('brightness', (100, 100))[0]
        scroll_brightness = self.suspend_args.get('brightness', (100, 100))[1]
        left_row_brightness = self.suspend_args.get('brightness', (100, 100))[2]
        right_row_brightness = self.suspend_args.get('brightness', (100, 100))[3]
        self.disable_notify = True
        _da_set_logo_brightness(self, logo_brightness)
        _da_set_scroll_brightness(self, scroll_brightness)
        _set_left_brightness(self, left_row_brightness)
        _set_right_brightness(self, right_row_brightness)
        self.disable_notify = False


class RazerMambaChromaWireless(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Mamba Chroma (Wireless)
    """
    USB_VID = 0x1532
    USB_PID = 0x0045
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 15]
    METHODS = ['get_device_type_mouse', 'get_battery', 'is_charging', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'set_charge_effect', 'set_charge_colour', 'set_idle_time', 'set_low_battery_threshold', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/609/609_mamba_500x500.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/609/609_mamba_500x500.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22343/razer-mamba-gallery-10.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22343/razer-mamba-gallery-04.png"
    }

    DPI_MAX = 16000

    def __init__(self, *args, **kwargs):
        super(RazerMambaChromaWireless, self).__init__(*args, **kwargs)

        self._battery_manager = _BatteryManager(self, self._device_number, 'Razer Mamba')
        self._battery_manager.active = self.config.getboolean('Startup', 'mouse_battery_notifier', fallback=False)

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
    MATRIX_DIMS = [1, 15]
    METHODS = ['get_device_type_mouse', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row', 'max_dpi',
               'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate', 'set_idle_time', 'set_low_battery_threshold', 'get_battery', 'is_charging']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/609/609_mamba_500x500.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/609/609_mamba_500x500.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22343/razer-mamba-gallery-01.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22343/razer-mamba-gallery-04.png"
    }

    DPI_MAX = 16000


class RazerMambaTE(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Mamba Tournament Edition
    """
    USB_VID = 0x1532
    USB_PID = 0x0046
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 16]
    METHODS = ['get_device_type_mouse', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row', 'max_dpi',
               'get_dpi_xy', 'set_dpi_xy']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/606/606_mambate_500x500.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/606/606_mambate_500x500.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22294/mambategallery-800x800-5.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22294/mambategallery-800x800-2.png"
    }

    DPI_MAX = 16000


class RazerAbyssus(__RazerDevice):
    """
    Class for the Razer Abyssus
    """
    USB_VID = 0x1532
    USB_PID = 0x0042
    METHODS = ['get_device_type_mouse', 'set_logo_active', 'get_logo_active', 'get_poll_rate', 'set_poll_rate']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/274/abyssus2014_500x500.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/274/abyssus2014_500x500.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17026/abyssus2014_gallery_4.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17079/abyssus2014_gallery_3_2.png"
    }

    def _resume_device(self):
        self.logger.debug("Abyssus doesn't have suspend/resume")

    def _suspend_device(self):
        self.logger.debug("Abyssus doesn't have suspend/resume")


class RazerImperator(__RazerDevice):
    """
    Class for the Razer Imperator 2012
    """
    USB_VID = 0x1532
    USB_PID = 0x002F
    METHODS = ['get_device_type_mouse', 'set_logo_active', 'get_logo_active', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy',
               'get_poll_rate', 'set_poll_rate', 'set_scroll_active', 'get_scroll_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/215/215_imperator.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/215/215_imperator.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/37/razer-imperator-gallery-2.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/37/razer-imperator-gallery-1.png"
    }

    DPI_MAX = 6400

    def _resume_device(self):
        self.logger.debug("Imperator doesn't have suspend/resume")

    def _suspend_device(self):
        self.logger.debug("Imperator doesn't have suspend/resume")


class RazerOuroboros(__RazerDevice):
    """
    Class for the Razer Ouroboros
    """
    USB_VID = 0x1532
    USB_PID = 0x0032
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy',
               'get_poll_rate', 'set_poll_rate', 'set_scroll_active', 'get_scroll_active', 'get_scroll_brightness', 'set_scroll_brightness',
               'get_battery', 'is_charging', 'set_idle_time', 'set_low_battery_threshold']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/26/26_ouroboros.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/26/26_ouroboros.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/752/razer-ouroboros-gallery-2.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/752/razer-ouroboros-gallery-2.png"
    }

    DPI_MAX = 8200

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = _da_get_scroll_brightness(self)

        # Todo make it context?
        self.disable_notify = True
        _da_set_scroll_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        scroll_brightness = self.suspend_args.get('brightness', 100)

        self.disable_notify = True
        _da_set_scroll_brightness(self, scroll_brightness)
        self.disable_notify = False


class RazerOrochi2013(__RazerDevice):
    """
    Class for the Razer Orochi 2013
    """
    USB_VID = 0x1532
    USB_PID = 0x0039
    METHODS = ['get_device_type_mouse', 'get_dpi_xy', 'set_dpi_xy',
               'get_poll_rate', 'set_poll_rate', 'set_scroll_active', 'get_scroll_active', 'max_dpi']

    DPI_MAX = 6400

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/612/612_orochi_2015.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/612/612_orochi_2015.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/6713/razer-orochi-2013-gallery-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/6713/razer-orochi-2013-gallery-5.png"
    }

    def _resume_device(self):
        self.logger.debug("Orochi doesn't have suspend/resume")

    def _suspend_device(self):
        self.logger.debug("Orochi doesn't have suspend/resume")


class RazerOrochiWired(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Orochi (Wired)
    """
    USB_VID = 0x1532
    USB_PID = 0x0048
    METHODS = ['get_device_type_mouse',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_idle_time', 'set_low_battery_threshold', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'set_scroll_active', 'get_scroll_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/612/612_orochi_2015.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/612/612_orochi_2015.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22770/razer-orochi-07-01.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22770/razer-orochi-08-01.png"
    }

    DPI_MAX = 8200


class RazerDeathAdderChroma(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer DeathAdder Chroma
    """
    USB_VID = 0x1532
    USB_PID = 0x0043
    METHODS = ['get_device_type_mouse',
               'set_logo_active', 'get_logo_active', 'get_logo_effect', 'get_logo_brightness', 'set_logo_brightness', 'set_logo_static', 'set_logo_pulsate', 'set_logo_blinking', 'set_logo_spectrum',
               'set_scroll_active', 'get_scroll_active', 'get_scroll_effect', 'get_scroll_brightness', 'set_scroll_brightness', 'set_scroll_static', 'set_scroll_pulsate', 'set_scroll_blinking', 'set_scroll_spectrum',
               'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/278/278_deathadder_chroma.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/278/278_deathadder_chroma.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17963/deathadder_chroma_gallery_5.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17963/deathadder_chroma_gallery_4.png"
    }

    DPI_MAX = 10000

    def __init__(self, *args, **kwargs):
        super(RazerDeathAdderChroma, self).__init__(*args, **kwargs)

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


class RazerDeathAdder2013(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer DeathAdder 2013
    """
    USB_VID = 0x1532
    USB_PID = 0x0037
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy_byte', 'set_dpi_xy_byte', 'get_poll_rate', 'set_poll_rate',
               'get_scroll_active', 'set_scroll_active', 'set_scroll_static', 'set_scroll_pulsate', 'set_scroll_blinking',
               'get_logo_active', 'set_logo_active', 'set_logo_static', 'set_logo_pulsate', 'set_logo_blinking']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/561/561_deathadder_classic.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/561/561_deathadder_classic.png",
        "side_img": None,
        "perspective_img": None
    }

    DPI_MAX = 6400

    def __init__(self, *args, **kwargs):
        super(RazerDeathAdder2013, self).__init__(*args, **kwargs)

        # Set brightness to max and LEDs to on, on startup
        _da_set_logo_active(self, True)
        _da_set_scroll_active(self, True)

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['active'] = (_da_get_logo_active(self), _da_get_scroll_active(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_active(self, False)
        _da_set_scroll_active(self, False)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_active = self.suspend_args.get('active', (True, True))[0]
        scroll_active = self.suspend_args.get('active', (True, True))[1]

        self.disable_notify = True
        _da_set_logo_active(self, logo_active)
        _da_set_scroll_active(self, scroll_active)
        self.disable_notify = False


class RazerNagaHexV2(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Naga Hex V2
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Naga_Hex_V2-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0050
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [1, 3]
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness',
               # Thumbgrid is technically backlight ID
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               # Logo
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # #Macros
               'get_macros', 'delete_macro', 'add_macro',
               # Can set Logo, Scroll and thumbgrid with custom
               'set_custom_effect', 'set_key_row']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/715/715_nagahexv2_500x500.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/715/715_nagahexv2_500x500.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/25031/nagahexv2-gallery-6.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/25031/nagahexv2-gallery-3.png"
    }

    DPI_MAX = 16000

    def __init__(self, *args, **kwargs):
        super(RazerNagaHexV2, self).__init__(*args, **kwargs)

        # self.key_manager = _NagaHexV2KeyManager(self._device_number, self.event_files, self, use_epoll=True, testing=self._testing, should_grab_event_files=True)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerNagaHexV2, self)._close()

        # self.key_manager.close()

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


class RazerNaga2012(__RazerDevice):
    """
    Class for the Razer Naga 2012
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Razer_Naga-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x002E
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy_byte', 'set_dpi_xy_byte', 'get_poll_rate', 'set_poll_rate',
               'get_logo_active', 'set_logo_active', 'get_scroll_active', 'set_scroll_active', 'set_backlight_active', 'get_backlight_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/products/39/razer-naga-gallery-1.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/39/razer-naga-gallery-4.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/39/razer-naga-gallery-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/39/razer-naga-gallery-1.png"
    }

    DPI_MAX = 5600

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['active'] = (_da_get_logo_active(self), _da_get_scroll_active(self), _da_get_backlight_active(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_active(self, False)
        _da_set_scroll_active(self, False)
        _da_set_backlight_active(self, False)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_active = self.suspend_args.get('active', (True, True, True))[0]
        scroll_active = self.suspend_args.get('active', (True, True, True))[1]
        backlight_active = self.suspend_args.get('active', (True, True, True))[2]

        self.disable_notify = True
        _da_set_logo_active(self, logo_active)
        _da_set_scroll_active(self, scroll_active)
        _da_set_backlight_active(self, backlight_active)
        self.disable_notify = False


class RazerNagaChroma(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Naga Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Naga_Chroma-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0053
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [1, 3]
    METHODS = ['get_device_type_mouse', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness', 'max_dpi',
               # Thumbgrid is technically backlight ID
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect', 'set_breath_single_effect', 'set_breath_dual_effect',
               # Logo
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # #Macros
               'get_macros', 'delete_macro', 'add_macro',
               # Can set Logo, Scroll and thumbgrid with custom
               'set_custom_effect', 'set_key_row']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/636/636_naga_chroma.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/636/636_naga_chroma.png",
        "side_img": "https://assets2.razerzone.com/images/razer-naga-chroma/28b8a7b0bf18c4bb0ffe6fe22d070dba-razer-naga-chroma-gallery-02.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/razer-naga-chroma/15fc24f88b577a6d080ed9e0e6a1b219-razer-naga-chroma-gallery-03.jpg"
    }

    DPI_MAX = 16000

    def __init__(self, *args, **kwargs):
        super(RazerNagaChroma, self).__init__(*args, **kwargs)

        # self.key_manager = _NagaHexV2KeyManager(self._device_number, self.event_files, self, use_epoll=True, testing=self._testing, should_grab_event_files=True)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerNagaChroma, self)._close()

        # self.key_manager.close()


class RazerNagaTrinity(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Naga Trinity
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Naga_Trinity-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0067
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [1, 3]
    METHODS = ['get_device_type_mouse', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_brightness', 'set_brightness', 'set_static_effect', 'max_dpi']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1251/1251_razer_naga_trinity.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1251/1251_razer_naga_trinity.png"
    }

    DPI_MAX = 16000

    def __init__(self, *args, **kwargs):
        super(RazerNagaTrinity, self).__init__(*args, **kwargs)

        # self.key_manager = _NagaHexV2KeyManager(self._device_number, self.event_files, self, use_epoll=True, testing=self._testing, should_grab_event_files=True)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerNagaTrinity, self)._close()

        # self.key_manager.close()


class RazerNagaHex(__RazerDevice):
    """
    Class for the Razer Naga Hex
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Naga_Hex-if01-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0041
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy_byte', 'set_dpi_xy_byte', 'get_poll_rate', 'set_poll_rate',
               'get_logo_active', 'set_logo_active', 'get_scroll_active', 'set_scroll_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/23/23_naga_hex.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/23/23_naga_hex.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/12/razer-naga-hex-gallery-5.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/12/razer-naga-hex-gallery-1.png"
    }

    DPI_MAX = 5600

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['active'] = (_da_get_logo_active(self), _da_get_scroll_active(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_active(self, False)
        _da_set_scroll_active(self, False)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_active = self.suspend_args.get('active', (True, True))[0]
        scroll_active = self.suspend_args.get('active', (True, True))[1]

        self.disable_notify = True
        _da_set_logo_active(self, logo_active)
        _da_set_scroll_active(self, scroll_active)
        self.disable_notify = False


class RazerNagaHexRed(__RazerDevice):
    """
    Class for the Razer Naga Hex (Red)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Naga_Hex-if01-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0036
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy_byte', 'set_dpi_xy_byte', 'get_poll_rate', 'set_poll_rate',
               'get_logo_active', 'set_logo_active', 'get_scroll_active', 'set_scroll_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/products/12/razer-naga-hex-gallery-12.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/12/razer-naga-hex-gallery-12.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/12/razer-naga-hex-gallery-11.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/12/razer-naga-hex-gallery-7.png"
    }

    DPI_MAX = 5600

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['active'] = (_da_get_logo_active(self), _da_get_scroll_active(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_active(self, False)
        _da_set_scroll_active(self, False)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_active = self.suspend_args.get('active', (True, True))[0]
        scroll_active = self.suspend_args.get('active', (True, True))[1]

        self.disable_notify = True
        _da_set_logo_active(self, logo_active)
        _da_set_scroll_active(self, scroll_active)
        self.disable_notify = False


class RazerTaipan(__RazerDevice):
    """
    Class for the Razer Taipan
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Taipan-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0034
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_active', 'set_logo_active', 'get_scroll_active', 'set_scroll_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/19/19_taipan.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/19/19_taipan.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/293/razer-taipan-gallery-3-black__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/293/razer-taipan-gallery-4-black__store_gallery.png"
    }

    DPI_MAX = 8200

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


class RazerDeathAdderElite(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer DeathAdder Elite
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_DeathAdder_Elite-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x005C
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 2]
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness',
               # Logo
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # Can set LOGO and Scroll with custom
               'set_custom_effect', 'set_key_row']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/724/724_deathadderelite_500x500.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/724/724_deathadderelite_500x500.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/25919/daelite_gallery03.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/25919/daelite_gallery02.png"
    }

    DPI_MAX = 16000

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


class RazerDiamondbackChroma(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Diamondback Chroma
    """
    USB_VID = 0x1532
    USB_PID = 0x004C
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 19]
    METHODS = ['get_device_type_mouse', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'max_dpi', 'get_dpi_xy', 'set_dpi_xy']

    DPI_MAX = 16000

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/613/613_diamondback.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/613/613_diamondback.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22772/rzr_diamondback_04__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22772/rzr_diamondback_02__store_gallery.png"
    }


class RazerDeathAdder3_5G(__RazerDevice):
    """
    Class for the Razer DeathAdder 3.5G
    """
    USB_VID = 0x1532
    USB_PID = 0x0016
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_device_type_mouse',
               'get_poll_rate', 'set_poll_rate', 'get_dpi_xy', 'set_dpi_xy', 'available_dpi', 'max_dpi',
               'get_logo_active', 'set_logo_active', 'get_scroll_active', 'set_scroll_active']

    AVAILABLE_DPI = [450, 900, 1800, 3500]
    DPI_MAX = 3500

    DEVICE_IMAGE = "https://assets2.razerzone.com/images/da10m/carousel/razer-death-adder-gallery-04.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets2.razerzone.com/images/da10m/carousel/razer-death-adder-gallery-04.png"
    }

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['active'] = (_da_get_logo_active(self), _da_get_scroll_active(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_active(self, False)
        _da_set_scroll_active(self, False)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_active = self.suspend_args.get('active', (True, True))[0]
        scroll_active = self.suspend_args.get('active', (True, True))[1]

        self.disable_notify = True
        _da_set_logo_active(self, logo_active)
        _da_set_scroll_active(self, scroll_active)
        self.disable_notify = False


class RazerMamba2012Wireless(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Mamba 2012 (Wireless)
    """
    USB_VID = 0x1532
    USB_PID = 0x0025
    METHODS = ['get_device_type_mouse', 'get_battery', 'is_charging',
               'set_idle_time', 'set_low_battery_threshold', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'set_scroll_active', 'get_scroll_active', 'get_scroll_effect', 'get_scroll_brightness', 'set_scroll_brightness', 'set_scroll_static', 'set_scroll_spectrum']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/192/192_mamba_2012.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/192/192_mamba_2012.png",
        "side_img": None,
        "perspective_img": "https://assets.razerzone.com/eeimages/support/products/18/mamba1.jpg"
    }

    DPI_MAX = 6400

    def __init__(self, *args, **kwargs):
        super(RazerMamba2012Wireless, self).__init__(*args, **kwargs)

        self._battery_manager = _BatteryManager(self, self._device_number, 'Razer Mamba')
        self._battery_manager.active = self.config.getboolean('Startup', 'mouse_battery_notifier', fallback=False)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerMamba2012Wireless, self)._close()

        self._battery_manager.close()


class RazerMamba2012Wired(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Mamba 2012 (Wired)
    """
    USB_VID = 0x1532
    USB_PID = 0x0024
    METHODS = ['get_device_type_mouse',
               'set_idle_time', 'set_low_battery_threshold', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'set_scroll_active', 'get_scroll_active', 'get_scroll_effect', 'get_scroll_brightness', 'set_scroll_brightness', 'set_scroll_static', 'set_scroll_spectrum',
               'get_battery', 'is_charging']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/192/192_mamba_2012.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/192/192_mamba_2012.png",
        "side_img": None,
        "perspective_img": "https://assets.razerzone.com/eeimages/support/products/18/mamba1.jpg"
    }

    DPI_MAX = 6400


class RazerMambaWirelessWired(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Mamba Wireless (Wired)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Mamba_Wireless_000000000000-if0(1|2)-event-kbd')
    USB_VID = 0x1532
    USB_PID = 0x0073
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 16]
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate', 'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness',
               # Battery
               'get_battery', 'is_charging', 'set_idle_time', 'set_low_battery_threshold',
               # Logo
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # Can set LOGO and Scroll with custom
               'set_custom_effect', 'set_key_row']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1404/1404_mamba_wireless.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1404/1404_mamba_wireless.png"
    }

    DPI_MAX = 16000

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


class RazerMambaWirelessReceiver(RazerMambaWirelessWired):
    """
    Class for the Razer Mamba Wireless (Receiver)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Mamba_Wireless_Receiver-if0(1|2)-event-kbd')
    USB_PID = 0x0072
    METHODS = RazerMambaWirelessWired.METHODS + ['set_charge_effect', 'set_charge_colour']

    def __init__(self, *args, **kwargs):
        super(RazerMambaWirelessReceiver, self).__init__(*args, **kwargs)

        self._battery_manager = _BatteryManager(self, self._device_number, 'Razer Mamba Wireless')
        self._battery_manager.active = self.config.getboolean('Startup', 'mouse_battery_notifier', fallback=False)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerMambaWirelessReceiver, self)._close()

        self._battery_manager.close()


class RazerNaga2014(__RazerDevice):
    """
    Class for the Razer Naga 2014
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Naga_2014-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0040
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_active', 'set_logo_active', 'get_scroll_active', 'set_scroll_active', 'set_backlight_active', 'get_backlight_active']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/227/227_razer_naga_2014.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/227/227_razer_naga_2014.png",
        "side_img": "https://assets2.razerzone.com/images/razer-naga-2014/1de23ed838df6362d79b02ff1f5d9efa-razer-naga-2014-gallery-05.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/razer-naga-2014/1de23ed838df6362d79b02ff1f5d9efa-razer-naga-2014-gallery-05.jpg"
    }

    DPI_MAX = 8200

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['active'] = (_da_get_logo_active(self), _da_get_scroll_active(self), _da_get_backlight_active(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_active(self, False)
        _da_set_scroll_active(self, False)
        _da_set_backlight_active(self, False)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_active = self.suspend_args.get('active', (True, True, True))[0]
        scroll_active = self.suspend_args.get('active', (True, True, True))[1]
        backlight_active = self.suspend_args.get('active', (True, True, True))[2]

        self.disable_notify = True
        _da_set_logo_active(self, logo_active)
        _da_set_scroll_active(self, scroll_active)
        _da_set_backlight_active(self, backlight_active)
        self.disable_notify = False


class RazerOrochi2011(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Orochi 2011
    """
    USB_VID = 0x1532
    USB_PID = 0x0013
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Orochi-if01-event-kbd')

    METHODS = ['get_device_type_mouse', 'set_logo_active', 'get_logo_active', 'set_scroll_active', 'get_scroll_active',
               'max_dpi', 'get_dpi_xy_byte', 'set_dpi_xy_byte', 'get_poll_rate', 'set_poll_rate']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/612/612_orochi_2015.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/612/612_orochi_2015.png",
        "side_img": "https://assets2.razerzone.com/images/razer-orochi/60d430d24a21c8b7bb8b9aba65b0a84e-Orochi-Base_gallery_2.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/razer-orochi/d6dd3766e7d8b5e937b4bffc8e4f9dee-Orochi-Base_gallery_7.jpg"
    }

    DPI_MAX = 4000

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['active'] = (_da_get_logo_active(self), _da_get_scroll_active(self))

        # Todo make it context?
        self.disable_notify = True
        _da_set_logo_active(self, False)
        _da_set_scroll_active(self, False)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_active = self.suspend_args.get('active', (True, True))[0]
        scroll_active = self.suspend_args.get('active', (True, True))[1]

        self.disable_notify = True
        _da_set_logo_active(self, logo_active)
        _da_set_scroll_active(self, scroll_active)
        self.disable_notify = False


class RazerAbyssusV2(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Abyssus V2
    """
    USB_VID = 0x1532
    USB_PID = 0x005B
    METHODS = ['get_device_type_mouse',
               'set_logo_active', 'get_logo_active', 'get_logo_effect', 'get_logo_brightness', 'set_logo_brightness', 'set_logo_static', 'set_logo_pulsate', 'set_logo_blinking', 'set_logo_spectrum',
               'set_scroll_active', 'get_scroll_active', 'get_scroll_effect', 'get_scroll_brightness', 'set_scroll_brightness', 'set_scroll_static', 'set_scroll_pulsate', 'set_scroll_blinking', 'set_scroll_spectrum',
               'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/721/721_abyssusv2.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/721/721_abyssusv2.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/25266/abyssus-v2-gallery-04.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/25266/abyssus-v2-gallery-03.png"
    }

    DPI_MAX = 5000

    def __init__(self, *args, **kwargs):
        super(RazerAbyssusV2, self).__init__(*args, **kwargs)

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


class RazerAbyssus1800(__RazerDevice):
    """
    Class for the Razer Abyssus 1800
    """
    USB_VID = 0x1532
    USB_PID = 0x0020
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy_byte', 'set_dpi_xy_byte', 'get_poll_rate', 'set_poll_rate',
               'get_logo_active', 'set_logo_active']

    DPI_MAX = 1800

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1277/1277_abyssus_2000.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1277/1277_abyssus_2000.png",
        "side_img": None,
        "perspective_img": None
    }


class RazerAbyssus2000(__RazerDevice):
    """
    Class for the Razer Abyssus 2000
    """
    USB_VID = 0x1532
    USB_PID = 0x005E
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_active', 'set_logo_active']

    DPI_MAX = 2000

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1277/1277_abyssus_2000.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1277/1277_abyssus_2000.png",
        "side_img": None,
        "perspective_img": None
    }


class RazerDeathAdder3500(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer DeathAdder 3500
    """
    USB_VID = 0x1532
    USB_PID = 0x0054
    METHODS = ['get_device_type_mouse',
               'get_logo_effect', 'get_logo_brightness', 'set_logo_brightness', 'set_logo_static', 'set_logo_pulsate', 'set_logo_blinking',
               'get_scroll_effect', 'get_scroll_brightness', 'set_scroll_brightness', 'set_scroll_static', 'set_scroll_pulsate', 'set_scroll_blinking',
               'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/561/561_deathadder_classic.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/561/561_deathadder_classic.png"
    }

    DPI_MAX = 3500

    def __init__(self, *args, **kwargs):
        super(RazerDeathAdder3500, self).__init__(*args, **kwargs)

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


class RazerViperUltimateWired(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Viper Ultimate (Wired)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Razer_Viper_Ultimate_000000000000-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x007A
    HAS_MATRIX = True
    WAVE_DIRS = (1, 2)
    MATRIX_DIMS = [1, 20]
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               # Logo
               'get_logo_brightness', 'set_logo_brightness', 'set_logo_wave',
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2',
               'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2',
               'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1577/ee_photo.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": DEVICE_IMAGE
    }

    DPI_MAX = 20000

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = _da_get_logo_brightness(self)

        self.disable_notify = True
        _da_set_logo_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_brightness = self.suspend_args.get('brightness', 100)

        self.disable_notify = True
        _da_set_logo_brightness(self, logo_brightness)
        self.disable_notify = False


class RazerViperUltimateWireless(RazerViperUltimateWired):
    """
    Class for the Razer Viper Ultimate (Wireless)
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Razer_Viper_Ultimate_Dongle-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x007B


class RazerViper(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Viper
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Razer_Viper-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0078
    HAS_MATRIX = True
    WAVE_DIRS = (1, 2)
    MATRIX_DIMS = [1, 16]
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               # Logo
               'get_logo_brightness', 'set_logo_brightness', 'set_logo_wave',
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2',
               'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2',
               'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1539/1539_viper.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1539/1539_viper.png"
    }

    DPI_MAX = 16000

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = _da_get_logo_brightness(self)

        self.disable_notify = True
        _da_set_logo_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """
        logo_brightness = self.suspend_args.get('brightness', 100)

        self.disable_notify = True
        _da_set_logo_brightness(self, logo_brightness)
        self.disable_notify = False


class RazerDeathAdderEssential(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer DeathAdder Essential
    """
    USB_VID = 0x1532
    USB_PID = 0x006E
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               # Logo
               'get_logo_brightness', 'set_logo_brightness',
               'set_logo_static_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2',
               # Scroll wheel
               'get_scroll_brightness', 'set_scroll_brightness',
               'set_scroll_static_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2']

    DPI_MAX = 6400

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1385/1385_deathadderessential.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1385/1385_deathadderessential.png"
    }

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = (_da_get_logo_brightness(self), _da_get_scroll_brightness(self))

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


class RazerMambaElite(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Mamba Elite
    """
    USB_VID = 0x1532
    USB_PID = 0x006C
    HAS_MATRIX = True
    WAVE_DIRS = (1, 2)
    MATRIX_DIMS = [1, 20]
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy',
               'get_poll_rate', 'set_poll_rate',
               # Logo logo_led_brightness/logo_matrix_effect_breath/...
               'get_logo_brightness', 'set_logo_brightness',
               'set_logo_wave', 'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel scroll_led_brightness/scroll_matrix_effect_breath/...
               'get_scroll_brightness', 'set_scroll_brightness',
               'set_scroll_wave', 'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # Left side left_led_brightness/left_matrix_effect_breath/...
               'get_left_brightness', 'set_left_brightness',
               'set_left_wave', 'set_left_static', 'set_left_spectrum', 'set_left_none', 'set_left_reactive', 'set_left_breath_random', 'set_left_breath_single', 'set_left_breath_dual',
               # Right side right_led_brightness/right_matrix_effect_breath/...
               'get_right_brightness', 'set_right_brightness',
               'set_right_wave', 'set_right_static', 'set_right_spectrum', 'set_right_none', 'set_right_reactive', 'set_right_breath_random', 'set_right_breath_single', 'set_right_breath_dual',
               # Custom frame
               'set_custom_effect', 'set_key_row']

    DPI_MAX = 16000

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1390/1390_mamba_elite.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": DEVICE_IMAGE
    }

    def _suspend_device(self):
        """
        Suspend the device
        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()
        self.suspend_args['brightness'] = (
            _da_get_logo_brightness(self),
            _da_get_scroll_brightness(self),
            _get_left_brightness(self),
            _get_right_brightness(self))

        # Todo make it context?
        self.disable_notify = True
        _set_backlight_brightness(self, 0)
        _da_set_logo_brightness(self, 0)
        _da_set_scroll_brightness(self, 0)
        _set_left_brightness(self, 0)
        _set_right_brightness(self, 0)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device
        Get the last known brightness and then set the brightness
        """
        logo_brightness = self.suspend_args.get('brightness', (100, 100))[0]
        scroll_brightness = self.suspend_args.get('brightness', (100, 100))[1]
        left_row_brightness = self.suspend_args.get('brightness', (100, 100))[2]
        right_row_brightness = self.suspend_args.get('brightness', (100, 100))[3]

        self.disable_notify = True
        _da_set_logo_brightness(self, logo_brightness)
        _da_set_scroll_brightness(self, scroll_brightness)
        _set_left_brightness(self, left_row_brightness)
        _set_right_brightness(self, right_row_brightness)
        self.disable_notify = False


class RazerDeathAdder1800(__RazerDevice):
    """
    Class for the Razer DeathAdder 1800
    """
    USB_VID = 0x1532
    USB_PID = 0x0038
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy_byte', 'set_dpi_xy_byte', 'get_poll_rate', 'set_poll_rate',
               'get_logo_active', 'set_logo_active']

    DPI_MAX = 1800

    DEVICE_IMAGE = "https://rzrwarranty.s3.amazonaws.com/a7daf40ad78c9584a693e310effa956019cdcd081391f93f71a7cd36d3dc577e.png"

    RAZER_URLS = {
        "top_img": DEVICE_IMAGE
    }


class RazerBasilisk(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer Basilisk
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Basilisk-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0064
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 2]
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness',
               # Logo
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # Can set LOGO and Scroll with custom
               'set_custom_effect', 'set_key_row']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1241/1241_basilisk.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": DEVICE_IMAGE
    }

    DPI_MAX = 16000

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


class RazerDeathAdderV2(__RazerDeviceSpecialBrightnessSuspend):
    """
    Class for the Razer DeathAdder V2
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_DeathAdder_V2-if0(1|2)-event-kbd')
    METHODS = ['get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness',
               # Logo
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # Can set LOGO and Scroll with custom
               'set_custom_effect', 'set_key_row']

    USB_VID = 0x1532
    USB_PID = 0x0084
    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1612/1612_razerdeathadderv2.png"

    # Deprecated - RAZER_URLS be removed in future.
    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/support/products/1612/1612_razerdeathadderv2.png"
    }

    DPI_MAX = 20000

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
