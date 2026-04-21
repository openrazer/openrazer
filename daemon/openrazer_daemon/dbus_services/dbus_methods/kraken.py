# SPDX-License-Identifier: GPL-2.0-or-later

"""
Module for kraken methods
"""
from openrazer_daemon.dbus_services import endpoint


@endpoint('razer.device.lighting.kraken', 'setCustom', in_sig='ai')
def set_custom_kraken(self, rgbi):
    """
    Set custom colour on kraken

    :return: List of args used for breathing effect
    :rtype: int
    """
    self.logger.debug("DBus call set custom")

    driver_path = self.get_driver_path('matrix_effect_custom')

    if len(rgbi) not in (3, 4):
        raise ValueError("List must be of 3 or 4 bytes")

    # DodgyCoding
    rgbi_list = list(rgbi)
    for index, item in enumerate(list(rgbi)):
        item = int(item)

        if item < 0:
            rgbi_list[index] = 0
        elif item > 255:
            rgbi_list[index] = 255
        else:
            rgbi_list[index] = item

    with open(driver_path, 'wb') as driver_file:
        driver_file.write(bytes(rgbi_list))


@endpoint('razer.device.audio.microphone', 'getVolume', out_sig='d')
def get_mic_volume(self):
    self.logger.debug("DBus call get_mic_volume")
    driver_path = self.get_driver_path('mic_volume')
    with open(driver_path, 'r') as f:
        return float(f.read().strip())


@endpoint('razer.device.audio.microphone', 'setVolume', in_sig='d')
def set_mic_volume(self, volume):
    self.logger.debug("DBus call set_mic_volume")
    driver_path = self.get_driver_path('mic_volume')
    vol = max(0, min(100, int(volume)))
    with open(driver_path, 'w') as f:
        f.write(str(vol))


@endpoint('razer.device.audio.headphone', 'getSidetone', out_sig='d')
def get_sidetone(self):
    self.logger.debug("DBus call get_sidetone")
    driver_path = self.get_driver_path('sidetone')
    with open(driver_path, 'r') as f:
        return float(f.read().strip())


@endpoint('razer.device.audio.headphone', 'setSidetone', in_sig='d')
def set_sidetone(self, level):
    self.logger.debug("DBus call set_sidetone")
    driver_path = self.get_driver_path('sidetone')
    val = max(0, min(30, int(level)))
    with open(driver_path, 'w') as f:
        f.write(str(val))


@endpoint('razer.device.audio.effects', 'getThxSpatialAudio', out_sig='b')
def get_thx_spatial_audio(self):
    self.logger.debug("DBus call get_thx_spatial_audio")
    driver_path = self.get_driver_path('thx_spatial_audio')
    with open(driver_path, 'r') as f:
        return bool(int(f.read().strip()))


@endpoint('razer.device.audio.effects', 'setThxSpatialAudio', in_sig='b')
def set_thx_spatial_audio(self, enabled):
    self.logger.debug("DBus call set_thx_spatial_audio")
    driver_path = self.get_driver_path('thx_spatial_audio')
    with open(driver_path, 'w') as f:
        f.write('1' if enabled else '0')
