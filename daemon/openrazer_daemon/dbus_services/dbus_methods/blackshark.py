# SPDX-License-Identifier: GPL-2.0-or-later

"""
DBus methods for BlackShark V3 X audio and power controls,
backed by the razerblackshark kernel driver's sysfs attributes.
"""
from openrazer_daemon.dbus_services import endpoint


def _read_str(self, filename):
    """
    Read an attribute, falling back to the last known value.

    The dongle cannot answer while the headset is unlinked, so follow the
    same approach as the DPI methods and return what is held locally
    rather than letting the error reach the DBus client.
    """
    driver_path = self.get_driver_path(filename)
    try:
        with open(driver_path, 'r') as driver_file:
            value = driver_file.read().strip()
    except OSError:
        self.logger.debug("Device did not answer for %s, using last known value", filename)
        return str(self.headset_audio[filename])

    self.headset_audio[filename] = value
    return value


def _read_int(self, filename):
    return int(_read_str(self, filename))


def _write_str(self, filename, value):
    driver_path = self.get_driver_path(filename)
    with open(driver_path, 'w') as driver_file:
        driver_file.write(value)

    self.headset_audio[filename] = value


def _write_int(self, filename, value):
    # NB: str(dbus.Byte(5)) is '\x05', so the value must go via int() first
    _write_str(self, filename, str(int(value)))


@endpoint('razer.device.audio.headset', 'getSidetone', out_sig='y')
def get_sidetone(self):
    """
    Get mic monitoring (sidetone) level, 0..15
    """
    self.logger.debug("DBus call get_sidetone")

    return _read_int(self, 'sidetone')


@endpoint('razer.device.audio.headset', 'setSidetone', in_sig='y')
def set_sidetone(self, level):
    """
    Set mic monitoring (sidetone) level, 0..15

    :param level: Sidetone level
    :type level: int
    """
    self.logger.debug("DBus call set_sidetone")

    if level > 15:
        raise ValueError("Sidetone level must be 0..15")

    _write_int(self, 'sidetone', level)


@endpoint('razer.device.audio.headset', 'getPowerSaving', out_sig='y')
def get_power_saving(self):
    """
    Get wireless power saving state, 0 or 1
    """
    self.logger.debug("DBus call get_power_saving")

    return _read_int(self, 'power_saving')


@endpoint('razer.device.audio.headset', 'setPowerSaving', in_sig='y')
def set_power_saving(self, enabled):
    """
    Enable or disable wireless power saving

    :param enabled: 1 to enable, 0 to disable
    :type enabled: int
    """
    self.logger.debug("DBus call set_power_saving")

    if enabled > 1:
        raise ValueError("Power saving must be 0 or 1")

    _write_int(self, 'power_saving', enabled)


@endpoint('razer.device.audio.headset', 'getEqualizerPreset', out_sig='y')
def get_equalizer_preset(self):
    """
    Get active EQ preset: 0 Default, 1 Game, 2 Music, 3 Movie
    """
    self.logger.debug("DBus call get_equalizer_preset")

    return _read_int(self, 'equalizer_preset')


@endpoint('razer.device.audio.headset', 'setEqualizerPreset', in_sig='y')
def set_equalizer_preset(self, preset):
    """
    Set active EQ preset

    :param preset: 0 Default, 1 Game, 2 Music, 3 Movie
    :type preset: int
    """
    self.logger.debug("DBus call set_equalizer_preset")

    if preset > 3:
        raise ValueError("Preset must be 0..3")

    _write_int(self, 'equalizer_preset', preset)


@endpoint('razer.device.audio.headset', 'getCustomEqualizer', out_sig='s')
def get_custom_equalizer(self):
    """
    Get the active preset's EQ band values in dB (-6..+6), space
    separated, ordered 31 Hz, 63, 125, 250, 500 Hz, 1, 2, 4, 8, 16 kHz
    """
    self.logger.debug("DBus call get_custom_equalizer")

    return _read_str(self, 'equalizer')


@endpoint('razer.device.audio.headset', 'setCustomEqualizer', in_sig='s')
def set_custom_equalizer(self, values):
    """
    Set the active preset's EQ band values in dB (-6..+6), space
    separated, ordered 31 Hz, 63, 125, 250, 500 Hz, 1, 2, 4, 8, 16 kHz

    The device stores one band set per preset, so this overwrites the
    bands of whichever preset is currently selected.

    :param values: Example '-1 -2 -1 -2 0 1 2 1 0 2'
    :type values: str
    """
    self.logger.debug("DBus call set_custom_equalizer")

    try:
        bands = [int(v) for v in values.split()]
    except ValueError:
        raise ValueError("Bands must be integers")
    if len(bands) != 10:
        raise ValueError("Exactly 10 band values are required")
    for band in bands:
        if not -6 <= band <= 6:
            raise ValueError("Band values must be within -6..+6 dB")

    _write_str(self, 'equalizer', ' '.join(str(b) for b in bands))


@endpoint('razer.device.audio.headset', 'getMicNoiseCancel', out_sig='y')
def get_mic_noise_cancel(self):
    """
    Get microphone noise cancellation state, 0 or 1
    """
    self.logger.debug("DBus call get_mic_noise_cancel")

    return _read_int(self, 'mic_noise_cancel')


@endpoint('razer.device.audio.headset', 'setMicNoiseCancel', in_sig='y')
def set_mic_noise_cancel(self, enabled):
    """
    Enable or disable microphone noise cancellation

    :param enabled: 1 to enable, 0 to disable
    :type enabled: int
    """
    self.logger.debug("DBus call set_mic_noise_cancel")

    if enabled > 1:
        raise ValueError("Mic noise cancellation must be 0 or 1")

    _write_int(self, 'mic_noise_cancel', enabled)
