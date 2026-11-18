# SPDX-License-Identifier: GPL-2.0-or-later
"""Onboard audio controls for the MediaTek BlackShark V2 HyperSpeed."""
from openrazer_daemon.dbus_services import endpoint


def _read(self, name):
    with open(self.get_driver_path(name), 'r') as driver_file:
        return driver_file.read().strip()


def _write(self, name, value):
    with open(self.get_driver_path(name), 'w') as driver_file:
        # dbus.Byte.__str__ is a character, unlike Python's integer formatting.
        driver_file.write(str(int(value)) if isinstance(value, int) else str(value))


@endpoint('razer.device.audio.blacksharkv2', 'getSidetoneEnabled', out_sig='b')
def get_headset_sidetone_enabled(self):
    return bool(int(_read(self, 'sidetone_enabled')))


@endpoint('razer.device.audio.blacksharkv2', 'setSidetoneEnabled', in_sig='b')
def set_headset_sidetone_enabled(self, enabled):
    _write(self, 'sidetone_enabled', int(enabled))


@endpoint('razer.device.audio.blacksharkv2', 'getSidetoneLevel', out_sig='y')
def get_headset_sidetone_level(self):
    return int(_read(self, 'sidetone_level'))


@endpoint('razer.device.audio.blacksharkv2', 'setSidetoneLevel', in_sig='y')
def set_headset_sidetone_level(self, level):
    if not 0 <= level <= 15:
        raise ValueError('Sidetone level must be between 0 and 15')
    _write(self, 'sidetone_level', level)


@endpoint('razer.device.audio.blacksharkv2', 'getEqualizerEnabled', out_sig='b')
def get_headset_equalizer_enabled(self):
    return bool(int(_read(self, 'equalizer_enabled')))


@endpoint('razer.device.audio.blacksharkv2', 'setEqualizerEnabled', in_sig='b')
def set_headset_equalizer_enabled(self, enabled):
    _write(self, 'equalizer_enabled', int(enabled))


@endpoint('razer.device.audio.blacksharkv2', 'getEqualizerPreset', out_sig='y')
def get_headset_equalizer_preset(self):
    return int(_read(self, 'equalizer_preset'))


@endpoint('razer.device.audio.blacksharkv2', 'setEqualizerPreset', in_sig='y')
def set_headset_equalizer_preset(self, preset):
    if preset not in (0, 7, 8, 9, 255):
        raise ValueError('Preset must be Flat (0), Game (7), Music (8), Movie (9), or Custom (255)')
    _write(self, 'equalizer_preset', preset)


@endpoint('razer.device.audio.blacksharkv2', 'getEqualizerBands', out_sig='ai')
def get_headset_equalizer_bands(self):
    return [int(value) for value in _read(self, 'equalizer_bands').split()]


@endpoint('razer.device.audio.blacksharkv2', 'setEqualizerBands', in_sig='ai')
def set_headset_equalizer_bands(self, bands):
    if len(bands) != 10 or any(not isinstance(value, int) or not -9 <= value <= 6 for value in bands):
        raise ValueError('Expected ten integer EQ gains between -9 and +6 dB')
    _write(self, 'equalizer_bands', ' '.join(str(value) for value in bands))


@endpoint('razer.device.audio.blacksharkv2', 'getDnd', out_sig='b')
def get_headset_do_not_disturb(self):
    return bool(int(_read(self, 'do_not_disturb')))


@endpoint('razer.device.audio.blacksharkv2', 'setDnd', in_sig='b')
def set_headset_do_not_disturb(self, enabled):
    _write(self, 'do_not_disturb', int(enabled))


@endpoint('razer.device.audio.blacksharkv2', 'getMicrophoneMuted', out_sig='b')
def get_headset_microphone_muted(self):
    return bool(int(_read(self, 'microphone_muted')))


@endpoint('razer.device.audio.blacksharkv2', 'getDongleLedMode', out_sig='y')
def get_headset_dongle_led_mode(self):
    return int(_read(self, 'dongle_led_mode'))


@endpoint('razer.device.audio.blacksharkv2', 'setDongleLedMode', in_sig='y')
def set_headset_dongle_led_mode(self, mode):
    if not 0 <= mode <= 3:
        raise ValueError('Dongle LED mode must be Off (0), Link (1), Battery (2), or Warning (3)')
    _write(self, 'dongle_led_mode', mode)


@endpoint('razer.device.audio.blacksharkv2', 'getWirelessConnected', out_sig='b')
def get_headset_wireless_connected(self):
    return bool(int(_read(self, 'wireless_connected')))


@endpoint('razer.device.audio.blacksharkv2', 'getHeadsetSerial', out_sig='s')
def get_headset_serial(self):
    return _read(self, 'headset_serial')


@endpoint('razer.device.audio.blacksharkv2', 'getHeadsetFirmware', out_sig='s')
def get_headset_firmware(self):
    return _read(self, 'headset_firmware_version')
