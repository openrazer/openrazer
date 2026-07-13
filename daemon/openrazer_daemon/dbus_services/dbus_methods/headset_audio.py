# SPDX-License-Identifier: GPL-2.0-or-later

"""
DBus methods for headset audio settings (equalizer, presets, mic monitoring)
and related device toggles.

Currently used by the Razer BlackShark V2 Pro 2.4 (razerblackshark module),
whose DSP applies a 10-band equalizer on the device itself. See
BLACKSHARK_NOTES.md for the protocol.
"""

from openrazer_daemon.dbus_services import endpoint

#: Number of equalizer bands (31Hz, 63, 125, 250, 500, 1k, 2k, 4k, 8k, 16k).
EQUALIZER_BANDS = 10
#: Per-band gain limit in dB (matches the Razer Synapse slider range).
EQUALIZER_LIMIT = 5
#: EQ preset selector values used on the wire (driver equalizer_preset attr).
#: game1-game5 are Synapse's five fixed game-specific EQ profiles, numbered
#: in the order they appear in Synapse.
EQUALIZER_PRESETS = {'game': 7, 'music': 8, 'movie': 9,
                     'game1': 250, 'game2': 254, 'game3': 251,
                     'game4': 253, 'game5': 252, 'custom': 255}


@endpoint('razer.device.audio', 'setEqualizer', in_sig='ai')
def set_equalizer(self, bands):
    """
    Set the on-device custom equalizer curve (switches to the custom preset).

    :param bands: 10 per-band gains in dB, low to high frequency. Each is
        clamped to +/- 5 dB.
    :type bands: list of int
    """
    self.logger.debug("DBus call set_equalizer")

    if len(bands) != EQUALIZER_BANDS:
        raise ValueError("Equalizer requires exactly {0} bands".format(EQUALIZER_BANDS))

    clamped = [max(-EQUALIZER_LIMIT, min(EQUALIZER_LIMIT, int(b))) for b in bands]
    payload = bytes((b & 0xff) for b in clamped)

    driver_path = self.get_driver_path('equalizer')
    with open(driver_path, 'wb') as driver_file:
        driver_file.write(payload)


@endpoint('razer.device.audio', 'getEqualizer', out_sig='ai')
def get_equalizer(self):
    """
    Get the active preset's equalizer curve, read from the device.

    :return: 10 per-band gains in dB, low to high frequency
    :rtype: list of int
    """
    self.logger.debug("DBus call get_equalizer")

    driver_path = self.get_driver_path('equalizer')
    with open(driver_path, 'rb') as driver_file:
        raw = driver_file.read(EQUALIZER_BANDS)

    return [b - 256 if b > 127 else b for b in raw]


@endpoint('razer.device.audio', 'setEqualizerPreset', in_sig='s')
def set_equalizer_preset(self, preset):
    """
    Select the device EQ preset.

    "game", "music" and "movie" use curves stored on the device; "custom"
    applies the bands set via setEqualizer; "game1".."game5" are the five
    fixed game-specific EQ profiles.

    :param preset: One of game / music / movie / game1..game5 / custom
    :type preset: str
    """
    self.logger.debug("DBus call set_equalizer_preset")

    preset = str(preset).lower()
    if preset not in EQUALIZER_PRESETS:
        raise ValueError("Preset must be one of: " + ", ".join(EQUALIZER_PRESETS))

    driver_path = self.get_driver_path('equalizer_preset')
    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(EQUALIZER_PRESETS[preset]))


@endpoint('razer.device.audio', 'getEqualizerPreset', out_sig='s')
def get_equalizer_preset(self):
    """
    Get the active device EQ preset.

    :return: One of game / music / movie / game1..game5 / custom
    :rtype: str
    """
    self.logger.debug("DBus call get_equalizer_preset")

    driver_path = self.get_driver_path('equalizer_preset')
    with open(driver_path, 'r') as driver_file:
        value = int(driver_file.read().strip())

    for name, wire in EQUALIZER_PRESETS.items():
        if wire == value:
            return name
    return 'custom'


@endpoint('razer.device.audio', 'setMicMonitoring', in_sig='b')
def set_mic_monitoring(self, enabled):
    """
    Toggle mic monitoring (sidetone): hear your own mic in the headset.

    :param enabled: True to enable
    :type enabled: bool
    """
    self.logger.debug("DBus call set_mic_monitoring")

    driver_path = self.get_driver_path('mic_monitoring')
    with open(driver_path, 'w') as driver_file:
        driver_file.write('1' if enabled else '0')


@endpoint('razer.device.audio', 'getMicMonitoring', out_sig='b')
def get_mic_monitoring(self):
    """
    Get whether mic monitoring (sidetone) is enabled.

    :rtype: bool
    """
    self.logger.debug("DBus call get_mic_monitoring")

    driver_path = self.get_driver_path('mic_monitoring')
    with open(driver_path, 'r') as driver_file:
        return bool(int(driver_file.read().strip()))


@endpoint('razer.device.audio', 'setMicMonitoringLevel', in_sig='y')
def set_mic_monitoring_level(self, level):
    """
    Set the mic monitoring (sidetone) loudness, 0-10.

    :param level: Level 0-10
    :type level: int
    """
    self.logger.debug("DBus call set_mic_monitoring_level")

    level = max(0, min(10, int(level)))

    driver_path = self.get_driver_path('mic_monitoring_level')
    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(level))


@endpoint('razer.device.audio', 'getMicMonitoringLevel', out_sig='y')
def get_mic_monitoring_level(self):
    """
    Get the mic monitoring (sidetone) loudness, 0-10.

    :rtype: int
    """
    self.logger.debug("DBus call get_mic_monitoring_level")

    driver_path = self.get_driver_path('mic_monitoring_level')
    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip())


@endpoint('razer.device.misc', 'setBluetoothDnd', in_sig='b')
def set_bluetooth_dnd(self, enabled):
    """
    Toggle Bluetooth "Do Not Disturb" on a dual-mode (2.4GHz + Bluetooth)
    headset: suppress Bluetooth interruptions while in 2.4GHz mode.

    :param enabled: True to enable
    :type enabled: bool
    """
    self.logger.debug("DBus call set_bluetooth_dnd")

    driver_path = self.get_driver_path('bt_dnd')
    with open(driver_path, 'w') as driver_file:
        driver_file.write('1' if enabled else '0')


@endpoint('razer.device.misc', 'getBluetoothDnd', out_sig='b')
def get_bluetooth_dnd(self):
    """
    Get whether Bluetooth "Do Not Disturb" is enabled.

    :rtype: bool
    """
    self.logger.debug("DBus call get_bluetooth_dnd")

    driver_path = self.get_driver_path('bt_dnd')
    with open(driver_path, 'r') as driver_file:
        return bool(int(driver_file.read().strip()))
