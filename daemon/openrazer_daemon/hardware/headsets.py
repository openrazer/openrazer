"""
Headsets class
"""
import re

from openrazer_daemon.hardware.device_base import RazerDevice as __RazerDevice, RazerDeviceBrightnessSuspend as __RazerDeviceBrightnessSuspend
from openrazer_daemon.dbus_services.dbus_methods import kraken as _dbus_kraken, chroma_keyboard as _dbus_chroma


class RazerKraken71(__RazerDevice):
    """
    Class for the Razer Kraken 7.1
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_7\.1_000000000000-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0501
    METHODS = ['get_device_type_headset',
               'set_static_effect', 'set_none_effect', 'get_current_effect_kraken']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/229/229_kraken_71.png"

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


class RazerKraken71Alternate(RazerKraken71):
    """
    Class for the Razer Kraken 7.1 (Alternate)
    """
    USB_PID = 0x0506


class RazerKraken71Chroma(__RazerDevice):
    """
    Class for the Razer Kraken 7.1 Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_7\.1_Chroma-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0504
    METHODS = ['get_device_type_headset',
               'set_static_effect', 'set_spectrum_effect', 'set_none_effect', 'set_breath_single_effect',
               'get_current_effect_kraken', 'get_static_effect_args_kraken', 'get_breath_effect_args_kraken', 'set_custom_kraken']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/280/280_kraken_71_chroma.png"

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


class RazerKraken71V2(__RazerDevice):
    """
    Class for the Razer Kraken 7.1 V2
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_7\.1_V2_0+-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0510
    METHODS = ['get_device_type_headset',
               'set_static_effect', 'set_spectrum_effect', 'set_none_effect', 'set_breath_single_effect', 'set_breath_dual_effect', 'set_breath_triple_effect',
               'get_current_effect_kraken', 'get_static_effect_args_kraken', 'get_breath_effect_args_kraken', 'set_custom_kraken']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/729/729_kraken_71_v2.png"

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


class RazerKrakenUltimate(__RazerDevice):
    """
    Class for the Razer Kraken Ultimate
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_Ultimate_0+-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0527
    METHODS = ['get_device_type_headset',
               'set_static_effect', 'set_spectrum_effect', 'set_none_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_breath_triple_effect', 'get_current_effect_kraken',
               'get_static_effect_args_kraken', 'get_breath_effect_args_kraken', 'set_custom_kraken']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1603/rzr_kraken_ultimate_render01_2019_resized.png"

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


class RazerKrakenKittyEdition(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Kraken Kitty Edition
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_Kitty_Chroma_Control-event-if00')

    USB_VID = 0x1532
    USB_PID = 0x0F19
    METHODS = ['get_device_type_headset',
               'set_none_effect', 'set_static_effect', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect',
               'set_custom_effect', 'set_key_row',
               'set_brightness', 'get_brightness']
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 4]

    DEVICE_IMAGE = "https://hybrismediaprod.blob.core.windows.net/sys-master-phoenix-images-container/hda/h57/9055450103838/Kraken-Kitty-Edition-Black-gallery-Hero.jpg "
