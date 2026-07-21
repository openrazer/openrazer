# SPDX-License-Identifier: GPL-2.0-or-later

"""
DBus methods for headset audio settings (equalizer, presets, sidetone) and
related device toggles.

Used by the BlackShark headsets (razerblackshark module), whose DSP applies a
10-band equalizer on the device itself. The protocol is documented in
driver/razerblackshark_driver.h.
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

#: Fallback for devices that do not declare their own sidetone range.
DEFAULT_SIDETONE_MAX = 10


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


@endpoint('razer.device.audio', 'setSidetone', in_sig='y')
def set_sidetone(self, level):
    """
    Set the sidetone (mic monitoring) level: hear your own mic in the headset.

    The value is a percentage so the API is the same across headsets whose
    hardware ranges differ; it is scaled to the device's own range on the way
    to the driver.

    :param level: 0-100, where 0 disables sidetone
    :type level: int
    """
    self.logger.debug("DBus call set_sidetone")

    percent = max(0, min(100, int(level)))
    device_max = getattr(self, 'SIDETONE_MAX', DEFAULT_SIDETONE_MAX)
    value = round(percent * device_max / 100)

    driver_path = self.get_driver_path('sidetone')
    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(value))


@endpoint('razer.device.audio', 'getSidetone', out_sig='y')
def get_sidetone(self):
    """
    Get the sidetone (mic monitoring) level as a percentage.

    :return: 0-100, where 0 means sidetone is off
    :rtype: int
    """
    self.logger.debug("DBus call get_sidetone")

    driver_path = self.get_driver_path('sidetone')
    with open(driver_path, 'r') as driver_file:
        value = int(driver_file.read().strip())

    device_max = getattr(self, 'SIDETONE_MAX', DEFAULT_SIDETONE_MAX)
    if device_max <= 0:
        return 0
    return round(value * 100 / device_max)


@endpoint('razer.device.audio', 'getMicMute', out_sig='b')
def get_mic_mute(self):
    """
    Get the state of the headset's hardware mic-mute button.

    Read-only: the button is the only thing that changes it.

    :return: True if the microphone is muted
    :rtype: bool
    """
    self.logger.debug("DBus call get_mic_mute")

    driver_path = self.get_driver_path('mic_mute')
    with open(driver_path, 'r') as driver_file:
        return bool(int(driver_file.read().strip()))


@endpoint('razer.device.misc', 'getHardwareModel', out_sig='s')
def get_hardware_model(self):
    """
    Get the product id the headset reports for itself, as a 4-digit hex string.

    Over a 2.4GHz dongle this identifies the paired headset rather than the
    dongle, so it can differ from the USB product id.

    :return: product id (e.g. "0556")
    :rtype: str
    """
    self.logger.debug("DBus call get_hardware_model")

    driver_path = self.get_driver_path('hw_model')
    with open(driver_path, 'r') as driver_file:
        return driver_file.read().strip()


@endpoint('razer.device.misc', 'setDnd', in_sig='b')
def set_dnd(self, enabled):
    """
    Toggle the headset's "Do Not Disturb" mode.

    :param enabled: True to enable
    :type enabled: bool
    """
    self.logger.debug("DBus call set_dnd")

    driver_path = self.get_driver_path('dnd')
    with open(driver_path, 'w') as driver_file:
        driver_file.write('1' if enabled else '0')


@endpoint('razer.device.misc', 'getDnd', out_sig='b')
def get_dnd(self):
    """
    Get whether "Do Not Disturb" is enabled.

    :rtype: bool
    """
    self.logger.debug("DBus call get_dnd")

    driver_path = self.get_driver_path('dnd')
    with open(driver_path, 'r') as driver_file:
        return bool(int(driver_file.read().strip()))
