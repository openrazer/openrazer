"""
Mouse class
"""
import re
from openrazer_daemon.hardware.device_base import RazerDeviceBrightnessSuspend as __RazerDeviceBrightnessSuspend, RazerDevice as __RazerDevice
from openrazer_daemon.misc.battery_notifier import BatteryManager as _BatteryManager
# TODO replace with plain import
from openrazer_daemon.dbus_services.dbus_methods.deathadder_chroma import get_logo_brightness as _da_get_logo_brightness, set_logo_brightness as _da_set_logo_brightness, \
    get_scroll_brightness as _da_get_scroll_brightness, set_scroll_brightness as _da_set_scroll_brightness, set_logo_active as _da_set_logo_active, \
    set_scroll_active as _da_set_scroll_active, get_scroll_active as _da_get_scroll_active, get_logo_active as _da_get_logo_active, set_backlight_active as _da_set_backlight_active, \
    get_backlight_active as _da_get_backlight_active
from openrazer_daemon.dbus_services.dbus_methods.chroma_keyboard import get_brightness as _get_backlight_brightness, set_brightness as _set_backlight_brightness
from openrazer_daemon.misc.key_event_management import NagaHexV2KeyManager as _NagaHexV2KeyManager


class RazerMambaChromaWireless(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Mamba Chroma (Wireless)
    """
    USB_VID = 0x1532
    USB_PID = 0x0045
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 15]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_brightness', 'set_brightness', 'get_battery', 'is_charging', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'set_charge_effect', 'set_charge_colour', 'set_idle_time', 'set_low_battery_threshold', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/22343/razer-mamba-gallery-03.png",
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
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_brightness', 'set_brightness', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row', 'max_dpi',
               'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate', 'set_idle_time', 'set_low_battery_threshold', 'get_battery', 'is_charging']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/22343/razer-mamba-gallery-03.png",
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
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_brightness', 'set_brightness', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row', 'max_dpi',
               'get_dpi_xy', 'set_dpi_xy']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/22294/mambategallery-800x800-1.png",
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
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'set_logo_active', 'get_logo_active']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/17026/abyssus2014_gallery_1.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17026/abyssus2014_gallery_4.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17079/abyssus2014_gallery_3_2.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerAbyssus, self).__init__(*args, **kwargs)

    def _resume_device(self):
        self.logger.debug("Abyssus doesnt have suspend/resume")

    def _suspend_device(self):
        self.logger.debug("Abyssus doesnt have suspend/resume")


class RazerImperator(__RazerDevice):
    """
    Class for the Razer Imperator 2012
    """
    USB_VID = 0x1532
    USB_PID = 0x002F
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'set_logo_active', 'get_logo_active', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy',
               'get_poll_rate', 'set_poll_rate', 'set_scroll_active', 'get_scroll_active']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/37/razer-imperator-gallery-5.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/37/razer-imperator-gallery-2.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/37/razer-imperator-gallery-1.png"
    }

    DPI_MAX = 6400

    def _resume_device(self):
        self.logger.debug("Imperator doesnt have suspend/resume")

    def _suspend_device(self):
        self.logger.debug("Imperator doesnt have suspend/resume")


class RazerOuroboros(__RazerDevice):
    """
    Class for the Razer Ouroboros
    """
    USB_VID = 0x1532
    USB_PID = 0x0032
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy',
               'get_poll_rate', 'set_poll_rate', 'set_scroll_active', 'get_scroll_active', 'get_scroll_brightness', 'set_scroll_brightness',
               'get_battery', 'is_charging', 'set_idle_time', 'set_low_battery_threshold']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/752/razer-ouroboros-gallery-1.png",
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
        scroll_brightness = self.suspend_args.get('brightness', 100)[0]

        self.disable_notify = True
        _da_set_scroll_brightness(self, scroll_brightness)
        self.disable_notify = False


class RazerOrochi2013(__RazerDevice):
    """
    Class for the Razer Orochi 2013
    """
    USB_VID = 0x1532
    USB_PID = 0x0039
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_dpi_xy', 'set_dpi_xy',
               'get_poll_rate', 'set_poll_rate', 'set_scroll_active', 'get_scroll_active', 'max_dpi']

    DPI_MAX = 6400

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/6713/razer-orochi-2013-gallery-1.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/6713/razer-orochi-2013-gallery-3.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/6713/razer-orochi-2013-gallery-5.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerOrochi2013, self).__init__(*args, **kwargs)

    def _resume_device(self):
        self.logger.debug("Orochi doesnt have suspend/resume")

    def _suspend_device(self):
        self.logger.debug("Orochi doesnt have suspend/resume")


class RazerOrochiWired(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Orochi (Wired)
    """
    USB_VID = 0x1532
    USB_PID = 0x0048
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_brightness', 'set_brightness',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_idle_time', 'set_low_battery_threshold', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'set_scroll_active', 'get_scroll_active']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/22770/razer-orochi-05-01.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22770/razer-orochi-07-01.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22770/razer-orochi-08-01.png"
    }

    DPI_MAX = 8200


class RazerDeathAdderChroma(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer DeathAdder Chroma
    """
    USB_VID = 0x1532
    USB_PID = 0x0043
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse',
               'set_logo_active', 'get_logo_active', 'get_logo_effect', 'get_logo_brightness', 'set_logo_brightness', 'set_logo_static', 'set_logo_pulsate', 'set_logo_blinking', 'set_logo_spectrum',
               'set_scroll_active', 'get_scroll_active', 'get_scroll_effect', 'get_scroll_brightness', 'set_scroll_brightness', 'set_scroll_static', 'set_scroll_pulsate', 'set_scroll_blinking', 'set_scroll_spectrum',
               'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/17531/deathadder_chroma_gallery_2.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17963/deathadder_chroma_gallery_5.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17963/deathadder_chroma_gallery_4.png"
    }

    DPI_MAX = 10000

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
    Class for the Razer Naga Hex V2
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Naga_Hex_V2-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0050
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [1, 3]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness', 'get_brightness', 'set_brightness',
               # Thumbgrid is technically backlight ID
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect','set_breath_single_effect', 'set_breath_dual_effect',
               # Logo
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # #Macros
               'get_macros', 'delete_macro', 'add_macro',
               # Can set Logo, Scroll and thumbgrid with custom
               'set_custom_effect', 'set_key_row']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/25031/nagahexv2-gallery-2.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/25031/nagahexv2-gallery-6.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/25031/nagahexv2-gallery-3.png"
    }

    DPI_MAX = 16000

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


class RazerNagaChroma(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Naga Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Naga_Chroma-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0053
    HAS_MATRIX = True
    DEDICATED_MACRO_KEYS = True
    MATRIX_DIMS = [1, 3]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness',
               # Thumbgrid is technically backlight ID
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect','set_breath_single_effect', 'set_breath_dual_effect',
               # Logo
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # #Macros
               'get_macros', 'delete_macro', 'add_macro',
               # Can set Logo, Scroll and thumbgrid with custom
               'set_custom_effect', 'set_key_row']

    RAZER_URLS = {
        "top_img": "https://assets2.razerzone.com/images/razer-naga-chroma/f7b87fa2737556bbeedb7352ee5cdb67-razer-naga-chroma-gallery-06.jpg",
        "side_img": "https://assets2.razerzone.com/images/razer-naga-chroma/28b8a7b0bf18c4bb0ffe6fe22d070dba-razer-naga-chroma-gallery-02.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/razer-naga-chroma/15fc24f88b577a6d080ed9e0e6a1b219-razer-naga-chroma-gallery-03.jpg"
    }

    DPI_MAX = 16000

    def __init__(self, *args, **kwargs):
        super(RazerNagaChroma, self).__init__(*args, **kwargs)

        self.key_manager = _NagaHexV2KeyManager(self._device_number, self.event_files, self, use_epoll=True, testing=self._testing, should_grab_event_files=True)

    def _close(self):
        """
        Close the key manager
        """
        super(RazerNagaChroma, self)._close()

        self.key_manager.close()


class RazerNagaHex(__RazerDevice):
    """
    Class for the Razer Naga Hex
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Naga_Hex-if01-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0041
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'max_dpi', 'get_dpi_xy_byte', 'set_dpi_xy_byte', 'get_poll_rate', 'set_poll_rate',
               'get_logo_active', 'set_logo_active', 'get_scroll_active', 'set_scroll_active']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/12/razer-naga-hex-gallery-6.png",
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
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'max_dpi', 'get_dpi_xy_byte', 'set_dpi_xy_byte', 'get_poll_rate', 'set_poll_rate',
               'get_logo_active', 'set_logo_active', 'get_scroll_active', 'set_scroll_active']

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
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_active', 'set_logo_active', 'get_scroll_active', 'set_scroll_active']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/293/razer-taipan-gallery-2-black__store_gallery.png",
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


class RazerDeathAdderElite(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer DeathAdder Elite
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_DeathAdder_Elite-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x005c
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 2]
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_brightness', 'set_logo_brightness', 'get_scroll_brightness', 'set_scroll_brightness',
               # Logo
               'set_logo_static_naga_hex_v2', 'set_logo_spectrum_naga_hex_v2', 'set_logo_none_naga_hex_v2', 'set_logo_reactive_naga_hex_v2', 'set_logo_breath_random_naga_hex_v2', 'set_logo_breath_single_naga_hex_v2', 'set_logo_breath_dual_naga_hex_v2',
               # Scroll wheel
               'set_scroll_static_naga_hex_v2', 'set_scroll_spectrum_naga_hex_v2', 'set_scroll_none_naga_hex_v2', 'set_scroll_reactive_naga_hex_v2', 'set_scroll_breath_random_naga_hex_v2', 'set_scroll_breath_single_naga_hex_v2', 'set_scroll_breath_dual_naga_hex_v2',
               # Can set LOGO and Scrol with custom
               'set_custom_effect', 'set_key_row']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/25919/daelite_gallery01.png",
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
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_brightness', 'set_brightness', 'set_wave_effect',
               'set_static_effect', 'set_spectrum_effect', 'set_reactive_effect', 'set_none_effect', 'set_breath_random_effect',
               'set_breath_single_effect', 'set_breath_dual_effect', 'set_custom_effect', 'set_key_row',
               'max_dpi', 'get_dpi_xy', 'set_dpi_xy']

    DPI_MAX = 16000

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/22772/rzr_diamondback_01__store_gallery.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/22772/rzr_diamondback_04__store_gallery.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/22772/rzr_diamondback_02__store_gallery.png"
    }


class RazerMamba2012Wireless(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Mamba 2012 (Wireless)
    """
    USB_VID = 0x1532
    USB_PID = 0x0025
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'get_battery', 'is_charging',
               'set_idle_time', 'set_low_battery_threshold', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'set_scroll_active', 'get_scroll_active', 'get_scroll_effect', 'get_scroll_brightness', 'set_scroll_brightness', 'set_scroll_static', 'set_scroll_spectrum']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/press/mamba-4g-2.jpg",
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


class RazerMamba2012Wired(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Mamba 2012 (Wired)
    """
    USB_VID = 0x1532
    USB_PID = 0x0024
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse',
               'set_idle_time', 'set_low_battery_threshold', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'set_scroll_active', 'get_scroll_active', 'get_scroll_effect', 'get_scroll_brightness', 'set_scroll_brightness', 'set_scroll_static', 'set_scroll_spectrum',
               'get_battery', 'is_charging']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/press/mamba-4g-2.jpg",
        "side_img": None,
        "perspective_img": "https://assets.razerzone.com/eeimages/support/products/18/mamba1.jpg"
    }

    DPI_MAX = 6400


class RazerNaga2014(__RazerDevice):
    """
    Class for the Razer Naga 2014
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Naga_2014-if0(1|2)-event-kbd')

    USB_VID = 0x1532
    USB_PID = 0x0040
    DEDICATED_MACRO_KEYS = True
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate',
               'get_logo_active', 'set_logo_active', 'get_scroll_active', 'set_scroll_active', 'set_backlight_active', 'get_backlight_active']

    RAZER_URLS = {
        "top_img": "https://assets2.razerzone.com/images/razer-naga-2014/6267ff992d7535b16817e551545d37df-razer-naga-2014-gallery-01.jpg",
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
        _da_get_backlight_active(self, False)
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


class RazerOrochi2011(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Orochi 2011
    """
    USB_VID = 0x1532
    USB_PID = 0x0013
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Orochi-if01-event-kbd')

    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse', 'set_logo_active', 'get_logo_active', 'set_scroll_active', 'get_scroll_active',
               'max_dpi', 'get_dpi_xy_byte', 'set_dpi_xy_byte', 'get_poll_rate', 'set_poll_rate']

    RAZER_URLS = {
        "top_img": "https://assets2.razerzone.com/images/razer-orochi/778bd94896bc70d1c18fc7e53d44abdc-Orochi-Base_gallery_4.jpg",
        "side_img": "https://assets2.razerzone.com/images/razer-orochi/60d430d24a21c8b7bb8b9aba65b0a84e-Orochi-Base_gallery_2.jpg",
        "perspective_img": "https://assets2.razerzone.com/images/razer-orochi/d6dd3766e7d8b5e937b4bffc8e4f9dee-Orochi-Base_gallery_7.jpg"
    }

    MAX_DPI = 4000

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


class RazerAbyssusV2(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Abyssus V2
    """
    USB_VID = 0x1532
    USB_PID = 0x005B
    METHODS = ['get_firmware', 'get_matrix_dims', 'has_matrix', 'get_device_name', 'get_device_type_mouse',
               'set_logo_active', 'get_logo_active', 'get_logo_effect', 'get_logo_brightness', 'set_logo_brightness', 'set_logo_static', 'set_logo_pulsate', 'set_logo_blinking', 'set_logo_spectrum',
               'set_scroll_active', 'get_scroll_active', 'get_scroll_effect', 'get_scroll_brightness', 'set_scroll_brightness', 'set_scroll_static', 'set_scroll_pulsate', 'set_scroll_blinking', 'set_scroll_spectrum',
               'max_dpi', 'get_dpi_xy', 'set_dpi_xy', 'get_poll_rate', 'set_poll_rate']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/25266/abyssus-v2-gallery-01.png",
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


