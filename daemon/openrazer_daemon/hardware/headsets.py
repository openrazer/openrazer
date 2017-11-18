"""
Headsets class
"""
import re

from openrazer_daemon.hardware.device_base import RazerDevice as __RazerDevice
from openrazer_daemon.dbus_services.dbus_methods import kraken as _dbus_kraken, chroma_keyboard as _dbus_chroma


class RazerKrakenClassic(__RazerDevice):
    """
    Class for the Razer Kraken 7.1 Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_7\.1_000000000000-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0501
    DEDICATED_MACRO_KEYS = False
    METHODS = ['get_firmware', 'get_device_name', 'get_device_type_headset', 'has_matrix', 'get_matrix_dims',
               'set_static_effect', 'set_none_effect', 'get_current_effect_kraken']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/17519/02.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17519/03.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17519/01.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerKrakenClassic, self).__init__(*args, **kwargs)

    def _close(self):
        super(RazerKrakenClassic, self)._close()

    @staticmethod
    def decode_bitfield(bitfield):
        return {
            'state': (bitfield & 0x01) == 0x01,
            'breathing1': (bitfield & 0x02) == 0x02,
            'spectrum': (bitfield & 0x04) == 0x04,
            'sync': (bitfield & 0x08) == 0x08,
            'breathing2': (bitfield & 0x10) == 0x10,
            'breathing3': (bitfield & 0x20) == 0x20,
        }

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()

        current_effect = _dbus_kraken.get_current_effect_kraken(self)
        dec = self.decode_bitfield(current_effect)

        if dec['state'] == 0x00:
            self.suspend_args['effect'] = 'none'
        elif dec['state'] == 0x01:
            self.suspend_args['effect'] = 'static'

        self.disable_notify = True
        _dbus_chroma.set_none_effect(self)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """

        effect = self.suspend_args.get('effect', '')

        self.disable_notify = True
        if effect == 'static':  # Static on classic is only 1 colour
            _dbus_chroma.set_static_effect(self, 0x00, 0x00, 0x00)

        self.disable_notify = False


class RazerKraken(__RazerDevice):
    """
    Class for the Razer Kraken 7.1 Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_7\.1_Chroma-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0504
    DEDICATED_MACRO_KEYS = False
    METHODS = ['get_firmware', 'get_device_name', 'get_device_type_headset', 'has_matrix', 'get_matrix_dims',
               'set_static_effect', 'set_spectrum_effect', 'set_none_effect', 'set_breath_single_effect',
               'get_current_effect_kraken', 'get_static_effect_args_kraken', 'get_breath_effect_args_kraken', 'set_custom_kraken']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/17519/02.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17519/03.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/17519/01.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerKraken, self).__init__(*args, **kwargs)

    def _close(self):
        super(RazerKraken, self)._close()

    @staticmethod
    def decode_bitfield(bitfield):
        return {
            'state': (bitfield & 0x01) == 0x01,
            'breathing1': (bitfield & 0x02) == 0x02,
            'spectrum': (bitfield & 0x04) == 0x04,
            'sync': (bitfield & 0x08) == 0x08,
            'breathing2': (bitfield & 0x10) == 0x10,
            'breathing3': (bitfield & 0x20) == 0x20,
        }

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()

        current_effect = _dbus_kraken.get_current_effect_kraken(self)
        dec = self.decode_bitfield(current_effect)

        if dec['breathing1']:
            self.suspend_args['effect'] = 'breathing1'
            self.suspend_args['args'] = _dbus_kraken.get_breath_effect_args_kraken(self)
        elif dec['spectrum']:
            self.suspend_args['effect'] = 'spectrum'
        elif dec['state']:
            self.suspend_args['effect'] = 'static'
            self.suspend_args['args'] = _dbus_kraken.get_static_effect_args_kraken(self)

        self.disable_notify = True
        _dbus_chroma.set_none_effect(self)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """

        effect = self.suspend_args.get('effect', '')
        args = self.suspend_args.get('args', [])

        self.disable_notify = True
        if effect == 'spectrum':
            _dbus_chroma.set_spectrum_effect(self)
        elif effect == 'static':
            _dbus_chroma.set_static_effect(self, *args)
        elif effect == 'breathing1':
            _dbus_chroma.set_breath_single_effect(self, *args)

        self.disable_notify = False


class RazerKrakenV2(__RazerDevice):
    """
    Class for the Razer Kraken 7.1 V2
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_7\.1_V2_0+-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0510
    DEDICATED_MACRO_KEYS = False
    METHODS = ['get_firmware', 'get_device_name', 'get_device_type_headset', 'has_matrix', 'get_matrix_dims',
               'set_static_effect', 'set_spectrum_effect', 'set_none_effect', 'set_breath_single_effect', 'set_breath_dual_effect', 'set_breath_triple_effect',
               'get_current_effect_kraken', 'get_static_effect_args_kraken', 'get_breath_effect_args_kraken', 'set_custom_kraken']

    RAZER_URLS = {
        "top_img": "https://assets.razerzone.com/eeimages/products/26005/kraken71v2_gallery04-v2.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/26005/kraken71v2_gallery01-v2.png",
        "perspective_img": "https://assets.razerzone.com/eeimages/products/26005/kraken71v2_gallery03-v2.png"
    }

    def __init__(self, *args, **kwargs):
        super(RazerKrakenV2, self).__init__(*args, **kwargs)

    def _close(self):
        super(RazerKrakenV2, self)._close()

    @staticmethod
    def decode_bitfield(bitfield):
        return {
            'state': (bitfield & 0x01) == 0x01,
            'breathing1': (bitfield & 0x02) == 0x02,
            'spectrum': (bitfield & 0x04) == 0x04,
            'sync': (bitfield & 0x08) == 0x08,
            'breathing2': (bitfield & 0x10) == 0x10,
            'breathing3': (bitfield & 0x20) == 0x20,
        }

    def _suspend_device(self):
        """
        Suspend the device

        Get the current brightness level, store it for later and then set the brightness to 0
        """
        self.suspend_args.clear()

        current_effect = _dbus_kraken.get_current_effect_kraken(self)
        dec = self.decode_bitfield(current_effect)

        if dec['breathing1']:
            self.suspend_args['effect'] = 'breathing1'
            self.suspend_args['args'] = _dbus_kraken.get_breath_effect_args_kraken(self)
        elif dec['breathing2']:
            self.suspend_args['effect'] = 'breathing2'
            self.suspend_args['args'] = _dbus_kraken.get_breath_effect_args_kraken(self)
        elif dec['breathing3']:
            self.suspend_args['effect'] = 'breathing3'
            self.suspend_args['args'] = _dbus_kraken.get_breath_effect_args_kraken(self)
        elif dec['spectrum']:
            self.suspend_args['effect'] = 'spectrum'
        elif dec['state']:
            self.suspend_args['effect'] = 'static'
            self.suspend_args['args'] = _dbus_kraken.get_static_effect_args_kraken(self)

        self.disable_notify = True
        _dbus_chroma.set_none_effect(self)
        self.disable_notify = False

    def _resume_device(self):
        """
        Resume the device

        Get the last known brightness and then set the brightness
        """

        effect = self.suspend_args.get('effect', '')
        args = self.suspend_args.get('args', [])

        self.disable_notify = True
        if effect == 'spectrum':
            _dbus_chroma.set_spectrum_effect(self)
        elif effect == 'static':
            _dbus_chroma.set_static_effect(self, *args)
        elif effect == 'breathing1':
            _dbus_chroma.set_breath_single_effect(self, *args)
        elif effect == 'breathing2':
            _dbus_chroma.set_breath_dual_effect(self, *args)
        elif effect == 'breathing3':
            _dbus_chroma.set_breath_triple_effect(self, *args)

        self.disable_notify = False
