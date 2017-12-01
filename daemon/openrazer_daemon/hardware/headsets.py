"""
Headsets class
"""
import re
import time
import struct

from openrazer_daemon.hardware.device_base import RazerDevice as __RazerDevice
from openrazer_daemon.dbus_services.dbus_methods import kraken as _dbus_kraken, chroma_keyboard as _dbus_chroma


# TODO convert to complete bytearray
class RazerReport(object):
    def __init__(self, report_id, destination, length, address):
        self.report_id = report_id
        self.destination = destination
        self.length = length
        self.addr_h = address >> 8
        self.addr_l = address & 0xFF
        self.arguments = bytearray(32)

    def bytes(self):
        return struct.pack('>BBBBB32s', self.report_id, self.destination, self.length, self.addr_h, self.addr_l, self.arguments)


class EffectByte(object):
    def __init__(self):
        self.value = 0x00

    def _get_bit(self, pos):
        return (self.value >> pos) & 0x01 == 0x01

    def _set_bit(self, pos, val):
        val = 0x01 if val else 0x00

        self.value |= val << pos

    # TODO programatically generate the methods
    @property
    def on_off_static(self):  # Bit 0
        return self._get_bit(0)

    @on_off_static.setter
    def on_off_static(self, value):  # Bit 0
        self._set_bit(0, value)

    @property
    def single_colour_breathing(self):  # Bit 1
        return self._get_bit(1)

    @single_colour_breathing.setter
    def single_colour_breathing(self, value):  # Bit 1
        self._set_bit(1, value)

    @property
    def spectrum_cycling(self):  # Bit 2
        return self._get_bit(2)

    @spectrum_cycling.setter
    def spectrum_cycling(self, value):  # Bit 2
        self._set_bit(2, value)

    @property
    def sync(self):  # Bit 3
        return self._get_bit(3)

    @sync.setter
    def sync(self, value):  # Bit 3
        self._set_bit(3, value)

    @property
    def two_colour_breathing(self):  # Bit 4
        return self._get_bit(4)

    @two_colour_breathing.setter
    def two_colour_breathing(self, value):  # Bit 4
        self._set_bit(4, value)

    @property
    def three_colour_breathing(self):  # Bit 5
        return self._get_bit(5)

    @three_colour_breathing.setter
    def three_colour_breathing(self, value):  # Bit 5
        self._set_bit(5, value)


# Need to talk to devs, once we send OutputReport, headset replies with InterruptReport, doesnt reach dev files
# class _RazerKrakenBase(__RazerDevice):
#     LED_MODE_ADDRESS = 0x172D
#     CUSTOM_ADDRESS = 0x1189
#     BREATHING1_ADDRESS = 0x1741
#     BREATHING2_ADDRESS = 0x1745
#     BREATHING3_ADDRESS = 0x174D
#
#     def __init__(self, *args, **kwargs):
#         super(_RazerKrakenBase, self).__init__(*args, **kwargs)
#
#         self.send_fw()
#         # self.set_led_effect_none()
#
#         print()
#
#     def razer_get_report(self, report_id, dest, length, address):
#         return RazerReport(report_id, dest, length, address)
#
#     def razer_get_usb_response(self, data):
#         #
#         with open(self.hidraw_path, 'wb') as open_file:
#             data = data.bytes()
#             open_file.write(data)
#
#         self.hid_node.sendFeatureReport(data, report_num=4)
#         time.sleep(0.01)
#         answer = self.hid_node.getFeatureReport(report_num=4, length=37)
#         return RazerReport(answer[1:])
#
#     def razer_send_payload(self, request_report):
#         try:
#             response_report = self.razer_get_usb_response(request_report)
#         except Exception as err:
#             print()
#             response_report = '\x00' * 37
#
#         return response_report
#
#     # get_kraken_request_report(0x04, 0x40, 0x01, device->led_mode_address);
#     def set_led_effect_none(self):
#         report = self.razer_get_report(0x04, 0x40, 0x01, self.LED_MODE_ADDRESS)
#
#         effect_byte = EffectByte()
#         effect_byte.on_off_static = 0
#         effect_byte.spectrum_cycling = 0
#
#         report.arguments[0] = effect_byte.value
#
#         self.razer_send_payload(report)
#
#     def send_fw(self):
#         report = self.razer_get_report(0x04, 0x20, 0x02, 0x0030)
#
#         self.razer_send_payload(report)


class RazerKrakenClassic(__RazerDevice):
    """
    Class for the Razer Kraken 7.1 Chroma
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_7\.1_000000000000-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0501
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

    @property
    def hid_request_index(self):
        return 4

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

    @property
    def hid_request_index(self):
        return 4

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

    @property
    def hid_request_index(self):
        return 4

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
