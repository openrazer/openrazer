# SPDX-License-Identifier: GPL-2.0-or-later
"""Onboard headset controls, advertised individually through capabilities."""
from typing import Any
import dbus as _dbus  # type: ignore
from openrazer.client.devices import RazerDevice


class RazerBlackSharkV2HyperSpeed(RazerDevice):
    def __init__(self, serial: str) -> None:
        super().__init__(serial)
        self._audio = _dbus.Interface(self._dbus, 'razer.device.audio.blacksharkv2')

    def _get_audio(self, capability: str, method: str) -> Any:
        if not self.has(capability):
            raise NotImplementedError()
        return getattr(self._audio, method)()

    def _set_audio(self, capability: str, method: str, value: Any) -> None:
        if not self.has(capability):
            raise NotImplementedError()
        getattr(self._audio, method)(value)

    @property
    def sidetone_enabled(self) -> bool:
        return bool(self._get_audio('sidetone', 'getSidetoneEnabled'))

    @sidetone_enabled.setter
    def sidetone_enabled(self, enabled: bool) -> None:
        self._set_audio('sidetone', 'setSidetoneEnabled', enabled)

    @property
    def sidetone_level(self) -> int:
        """Sidetone volume, 0..15."""
        return int(self._get_audio('sidetone', 'getSidetoneLevel'))

    @sidetone_level.setter
    def sidetone_level(self, level: int) -> None:
        if not 0 <= level <= 15:
            raise ValueError('Sidetone level must be between 0 and 15')
        self._set_audio('sidetone', 'setSidetoneLevel', level)

    @property
    def equalizer_enabled(self) -> bool:
        return bool(self._get_audio('equalizer', 'getEqualizerEnabled'))

    @equalizer_enabled.setter
    def equalizer_enabled(self, enabled: bool) -> None:
        self._set_audio('equalizer', 'setEqualizerEnabled', enabled)

    @property
    def equalizer_preset(self) -> int:
        """Flat=0, Game=7, Music=8, Movie=9, Custom=255."""
        return int(self._get_audio('equalizer', 'getEqualizerPreset'))

    @equalizer_preset.setter
    def equalizer_preset(self, preset: int) -> None:
        if preset not in (0, 7, 8, 9, 255):
            raise ValueError('Unsupported equalizer preset')
        self._set_audio('equalizer', 'setEqualizerPreset', preset)

    @property
    def equalizer_bands(self) -> tuple[int, ...]:
        """Ten gains (-9..+6 dB) at 31, 63, 125, 250, 500, 1k, 2k, 4k, 8k, 16k Hz."""
        return tuple(int(value) for value in self._get_audio('equalizer', 'getEqualizerBands'))

    @equalizer_bands.setter
    def equalizer_bands(self, bands: tuple[int, ...]) -> None:
        if len(bands) != 10 or any(not isinstance(value, int) or not -9 <= value <= 6 for value in bands):
            raise ValueError('Expected ten integer EQ gains between -9 and +6 dB')
        self._set_audio('equalizer', 'setEqualizerBands', bands)

    @property
    def do_not_disturb(self) -> bool:
        return bool(self._get_audio('do_not_disturb', 'getDnd'))

    @do_not_disturb.setter
    def do_not_disturb(self, enabled: bool) -> None:
        self._set_audio('do_not_disturb', 'setDnd', enabled)

    @property
    def microphone_muted(self) -> bool:
        return bool(self._get_audio('microphone_mute', 'getMicrophoneMuted'))

    @property
    def dongle_led_mode(self) -> int:
        """Off=0, Link=1, Battery=2, Warning=3; receiver only."""
        return int(self._get_audio('dongle_led_mode', 'getDongleLedMode'))

    @dongle_led_mode.setter
    def dongle_led_mode(self, mode: int) -> None:
        if not 0 <= mode <= 3:
            raise ValueError('Dongle LED mode must be between 0 and 3')
        self._set_audio('dongle_led_mode', 'setDongleLedMode', mode)

    @property
    def wireless_connected(self) -> bool:
        return bool(self._get_audio('wireless_connected', 'getWirelessConnected'))

    @property
    def headset_serial(self) -> str:
        return str(self._get_audio('headset_identity', 'getHeadsetSerial'))

    @property
    def headset_firmware_version(self) -> str:
        return str(self._get_audio('headset_identity', 'getHeadsetFirmware'))
