# SPDX-License-Identifier: GPL-2.0-or-later

"""
DBus methods for BlackShark V3 X audio and power controls,
backed by the razerblackshark kernel driver's sysfs attributes.
"""
from openrazer_daemon.dbus_services import endpoint


def _read_int(self, filename):
    driver_path = self.get_driver_path(filename)
    with open(driver_path, 'r') as driver_file:
        return int(driver_file.read().strip())


def _write_value(self, filename, value):
    driver_path = self.get_driver_path(filename)
    with open(driver_path, 'w') as driver_file:
        driver_file.write(str(value))


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

    _write_value(self, 'sidetone', level)


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

    _write_value(self, 'power_saving', enabled)


@endpoint('razer.device.audio.headset', 'getEqualizerPreset', out_sig='y')
def get_equalizer_preset(self):
    """
    Get active EQ preset: 0 Default, 1 Game, 2 Movie, 3 Music
    """
    self.logger.debug("DBus call get_equalizer_preset")

    return _read_int(self, 'equalizer_preset')


@endpoint('razer.device.audio.headset', 'setEqualizerPreset', in_sig='y')
def set_equalizer_preset(self, preset):
    """
    Set active EQ preset

    :param preset: 0 Default, 1 Game, 2 Movie, 3 Music
    :type preset: int
    """
    self.logger.debug("DBus call set_equalizer_preset")

    if preset > 3:
        raise ValueError("Preset must be 0..3")

    _write_value(self, 'equalizer_preset', preset)


@endpoint('razer.device.audio.headset', 'getCustomEqualizer', out_sig='s')
def get_custom_equalizer(self):
    """
    Get custom EQ band values in dB (-6..+6), space separated,
    ordered 31 Hz, 63, 125, 250, 500 Hz, 1, 2, 4, 8, 16 kHz
    """
    self.logger.debug("DBus call get_custom_equalizer")

    driver_path = self.get_driver_path('equalizer')
    with open(driver_path, 'r') as driver_file:
        return driver_file.read().strip()


@endpoint('razer.device.audio.headset', 'setCustomEqualizer', in_sig='s')
def set_custom_equalizer(self, values):
    """
    Set custom EQ band values in dB (-6..+6), space separated,
    ordered 31 Hz, 63, 125, 250, 500 Hz, 1, 2, 4, 8, 16 kHz

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

    _write_value(self, 'equalizer', ' '.join(str(b) for b in bands))


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

    _write_value(self, 'mic_noise_cancel', enabled)
