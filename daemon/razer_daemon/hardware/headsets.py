"""
Keyboards class
"""
import re

from razer_daemon.hardware.device_base import RazerDevice as __RazerDevice
from razer_daemon.dbus_services.dbus_methods import kraken as _dbus_kraken, chroma_keyboard as _dbus_chroma


class RazerKraken(__RazerDevice):

    """
    Class for the Kraken
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_7\.1_Chroma-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0504
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [-1, -1]
    METHODS = ['get_firmware', 'get_device_name', 'get_device_type_headset', 'has_matrix', 'get_matrix_dims',
               'set_static_effect', 'set_spectrum_effect', 'set_none_effect', 'set_breath_single_effect',
               'get_current_effect_kraken', 'get_static_effect_args_kraken', 'get_breath_effect_args_kraken']

    RAZER_URLS = {
        "store": "http://web.archive.org/web/20160826002356/http://www.razerzone.com/gaming-audio/razer-kraken-71-chroma",
        "top_img": "https://assets.razerzone.com/eeimages/products/17519/02.png",
        "side_img": "https://assets.razerzone.com/eeimages/products/17519/03.png",
        "perspective_img": "http://assets.razerzone.com/eeimages/products/17519/01.png"
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
    Class for the Kraken V2
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_7\.1_V2_0+-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0510
    HAS_MATRIX = False
    DEDICATED_MACRO_KEYS = False
    MATRIX_DIMS = [-1, -1]
    METHODS = ['get_firmware', 'get_device_name', 'get_device_type_headset', 'has_matrix', 'get_matrix_dims',
               'set_static_effect', 'set_spectrum_effect', 'set_none_effect', 'set_breath_single_effect', 'set_breath_dual_effect', 'set_breath_triple_effect',
               'get_current_effect_kraken', 'get_static_effect_args_kraken', 'get_breath_effect_args_kraken']

    RAZER_URLS = {
        "store": "http://www.razerzone.com/gaming-audio/razer-kraken-71-v2",
        "top_img": "http://assets.razerzone.com/eeimages/products/26005/kraken71v2_gallery03-v2.png",
        "side_img": None,
        "perspective_img": None
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
