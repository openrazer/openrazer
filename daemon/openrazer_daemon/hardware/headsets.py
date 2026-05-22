# SPDX-License-Identifier: GPL-2.0-or-later

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
               'set_static_effect', 'set_none_effect']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/229/229_kraken_71.png"

    def _suspend_device(self):
        self.suspend_args.clear()
        self.suspend_args['effect'] = self.zone["backlight"]["effect"]

        _dbus_chroma.set_none_effect(self)

    def _resume_device(self):
        effect = self.suspend_args.get('effect', '')
        if effect == 'static':  # Static on classic is only 1 colour
            _dbus_chroma.set_static_effect(self, 0x00, 0x00, 0x00)


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
               'set_custom_kraken']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/280/280_kraken_71_chroma.png"

    def _suspend_device(self):
        self.suspend_args.clear()
        self.suspend_args['effect'] = self.zone["backlight"]["effect"]
        self.suspend_args['args'] = self.zone["backlight"]["colors"][0:3]

        _dbus_chroma.set_none_effect(self)

    def _resume_device(self):
        effect = self.suspend_args.get('effect', '')
        args = self.suspend_args.get('args', [])

        if effect == 'spectrum':
            _dbus_chroma.set_spectrum_effect(self)
        elif effect == 'static':
            _dbus_chroma.set_static_effect(self, *args)
        elif effect == 'breathSingle':
            _dbus_chroma.set_breath_single_effect(self, *args)


class RazerKraken71V2(__RazerDevice):
    """
    Class for the Razer Kraken 7.1 V2
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_7\.1_V2_0+-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0510
    METHODS = ['get_device_type_headset',
               'set_static_effect', 'set_spectrum_effect', 'set_none_effect', 'set_breath_single_effect', 'set_breath_dual_effect', 'set_breath_triple_effect',
               'set_custom_kraken']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/729/729_kraken_71_v2.png"

    def _suspend_device(self):
        self.suspend_args.clear()
        self.suspend_args['effect'] = self.zone["backlight"]["effect"]
        if self.suspend_args['effect'] == "breathDual":
            self.suspend_args['args'] = self.zone["backlight"]["colors"][0:6]
        elif self.suspend_args['effect'] == "breathTriple":
            self.suspend_args['args'] = self.zone["backlight"]["colors"][0:9]
        else:
            self.suspend_args['args'] = self.zone["backlight"]["colors"][0:3]

        _dbus_chroma.set_none_effect(self)

    def _resume_device(self):
        effect = self.suspend_args.get('effect', '')
        args = self.suspend_args.get('args', [])

        if effect == 'spectrum':
            _dbus_chroma.set_spectrum_effect(self)
        elif effect == 'static':
            _dbus_chroma.set_static_effect(self, *args)
        elif effect == 'breathSingle':
            _dbus_chroma.set_breath_single_effect(self, *args)
        elif effect == 'breathDual':
            _dbus_chroma.set_breath_dual_effect(self, *args)
        elif effect == 'breathTriple':
            _dbus_chroma.set_breath_triple_effect(self, *args)


class RazerKrakenTournamentEdition(__RazerDevice):
    """
    Class for the Razer Kraken Tournament Edition
    """
    EVENT_FILE_REGEX = re.compile(r'.*RAZER_KRAKEN_TE+-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0520
    METHODS = ['get_device_type_headset',
               'set_static_effect', 'set_spectrum_effect', 'set_none_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_breath_triple_effect',
               'set_custom_kraken']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1399/1399_razerkrakente.png"

    def _suspend_device(self):
        self.suspend_args.clear()
        self.suspend_args['effect'] = self.zone["backlight"]["effect"]
        if self.suspend_args['effect'] == "breathDual":
            self.suspend_args['args'] = self.zone["backlight"]["colors"][0:6]
        elif self.suspend_args['effect'] == "breathTriple":
            self.suspend_args['args'] = self.zone["backlight"]["colors"][0:9]
        else:
            self.suspend_args['args'] = self.zone["backlight"]["colors"][0:3]

        _dbus_chroma.set_none_effect(self)

    def _resume_device(self):
        effect = self.suspend_args.get('effect', '')
        args = self.suspend_args.get('args', [])

        if effect == 'spectrum':
            _dbus_chroma.set_spectrum_effect(self)
        elif effect == 'static':
            _dbus_chroma.set_static_effect(self, *args)
        elif effect == 'breathSingle':
            _dbus_chroma.set_breath_single_effect(self, *args)
        elif effect == 'breathDual':
            _dbus_chroma.set_breath_dual_effect(self, *args)
        elif effect == 'breathTriple':
            _dbus_chroma.set_breath_triple_effect(self, *args)


class RazerKrakenUltimate(__RazerDevice):
    """
    Class for the Razer Kraken Ultimate
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_Ultimate_0+-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0527
    METHODS = ['get_device_type_headset',
               'set_static_effect', 'set_spectrum_effect', 'set_none_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_breath_triple_effect',
               'set_custom_kraken']

    DEVICE_IMAGE = "https://assets.razerzone.com/eeimages/support/products/1603/rzr_kraken_ultimate_render01_2019_resized.png"

    def _suspend_device(self):
        self.suspend_args.clear()
        self.suspend_args['effect'] = self.zone["backlight"]["effect"]
        if self.suspend_args['effect'] == "breathDual":
            self.suspend_args['args'] = self.zone["backlight"]["colors"][0:6]
        elif self.suspend_args['effect'] == "breathTriple":
            self.suspend_args['args'] = self.zone["backlight"]["colors"][0:9]
        else:
            self.suspend_args['args'] = self.zone["backlight"]["colors"][0:3]

        _dbus_chroma.set_none_effect(self)

    def _resume_device(self):
        effect = self.suspend_args.get('effect', '')
        args = self.suspend_args.get('args', [])

        if effect == 'spectrum':
            _dbus_chroma.set_spectrum_effect(self)
        elif effect == 'static':
            _dbus_chroma.set_static_effect(self, *args)
        elif effect == 'breathSingle':
            _dbus_chroma.set_breath_single_effect(self, *args)
        elif effect == 'breathDual':
            _dbus_chroma.set_breath_dual_effect(self, *args)
        elif effect == 'breathTriple':
            _dbus_chroma.set_breath_triple_effect(self, *args)


class RazerKrakenKittyEdition(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Kraken Kitty Edition
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_Kitty_Chroma_Control-event-if00')

    USB_VID = 0x1532
    USB_PID = 0x0F19
    METHODS = ['get_device_type_headset',
               'set_none_effect', 'set_static_effect', 'set_breath_random_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_starlight_random_effect', 'set_starlight_single_effect',
               'set_starlight_dual_effect', 'set_wave_effect', 'set_spectrum_effect',
               'set_custom_effect', 'set_key_row',
               'set_brightness', 'get_brightness']
    HAS_MATRIX = True
    MATRIX_DIMS = [1, 4]

    DEVICE_IMAGE = "https://assets2.razerzone.com/images/pnx.assets/1c503aa176bc82d999299aba0d6c7d2c/kraken-kitty-quartz.png"


class RazerKrakenKittyV2(__RazerDevice):
    """
    Class for the Razer Kraken Kitty V2
    """
    EVENT_FILE_REGEX = re.compile(r'.*Razer_Kraken_Kitty_V2_00000000-event-if03')

    USB_VID = 0x1532
    USB_PID = 0x0560
    METHODS = ['get_device_type_headset',
               'set_static_effect', 'set_spectrum_effect', 'set_none_effect', 'set_breath_single_effect',
               'set_breath_dual_effect', 'set_breath_triple_effect',
               'set_custom_kraken']

    DEVICE_IMAGE = "https://medias-p1.phoenix.razer.com/sys-master-phoenix-images-container/hcc/h6b/9631977570334/kraken-kitty-v2-quartz-500x500.png"


class RazerNariUltimate(__RazerDeviceBrightnessSuspend):
    """
    Class for the Razer Nari Ultimate (wireless dongle variant).

    Exposes two zones:
        - main zone: mapped to the haptic motor intensity (0..100),
          driven through the standard matrix_brightness attribute.
        - logo zone: the Razer logo LED on each cup (binary on/off),
          driven through logo_led_brightness / logo_led_state and the
          logo_matrix_effect_none / logo_matrix_effect_static pair.

    Microphone mute and volume are standard USB Audio Class Feature Unit
    controls handled by snd-usb-audio, not by this driver.
    """
    USB_VID = 0x1532
    USB_PID = 0x051A
    METHODS = ['get_device_type_headset',
               # Main zone: haptic mapped through the brightness slider.
               # No effect picker is exposed on this zone because haptic has
               # no meaningful "effect" concept (set_none_effect would clash
               # with the slider's own 0-value-means-off semantics).
               'get_brightness', 'set_brightness',
               # Logo zone: status LED
               'get_logo_brightness', 'set_logo_brightness',
               'get_logo_active', 'set_logo_active',
               'set_logo_static', 'set_logo_none']

    DEVICE_IMAGE = "https://assets2.razerzone.com/images/razer-nari-ultimate/shop/nariultimate-ch5-v1.png"


class RazerNariUltimateWired(RazerNariUltimate):
    """Razer Nari Ultimate (wired USB variant)."""
    USB_PID = 0x051B


class RazerNari(__RazerDeviceBrightnessSuspend):
    """
    Class for the non-Ultimate Razer Nari (no haptic motors).

    The protocol is identical to the Ultimate, but the headset lacks
    the bass-response haptic drivers so the matrix_brightness zone is
    effectively a no-op on the hardware; we still expose it to keep
    the daemon/Polychromatic UI consistent with the Ultimate class.
    """
    USB_VID = 0x1532
    USB_PID = 0x051C
    METHODS = ['get_device_type_headset',
               'get_logo_brightness', 'set_logo_brightness',
               'get_logo_active', 'set_logo_active',
               'set_logo_static', 'set_logo_none']

    DEVICE_IMAGE = "https://assets2.razerzone.com/images/razer-nari/shop/nari-ch5-v1.png"


class RazerNariWired(RazerNari):
    """Razer Nari (wired USB variant)."""
    USB_PID = 0x051D
