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


@endpoint('razer.device.audio.headphone', 'getWirelessPowerSave', out_sig='i')
def get_wireless_power_save(self):
    self.logger.debug("DBus call get_wireless_power_save")
    driver_path = self.get_driver_path('wireless_power_save')
    with open(driver_path, 'r') as f:
        return int(f.read().strip())


@endpoint('razer.device.audio.headphone', 'setWirelessPowerSave', in_sig='i')
def set_wireless_power_save(self, minutes):
    self.logger.debug("DBus call set_wireless_power_save")
    driver_path = self.get_driver_path('wireless_power_save')
    with open(driver_path, 'w') as f:
        f.write(str(int(minutes)))


@endpoint('razer.device.audio.headphone', 'getUltraLowLatency', out_sig='b')
def get_ultra_low_latency(self):
    self.logger.debug("DBus call get_ultra_low_latency")
    driver_path = self.get_driver_path('ultra_low_latency')
    with open(driver_path, 'r') as f:
        return bool(int(f.read().strip()))


@endpoint('razer.device.audio.headphone', 'setUltraLowLatency', in_sig='b')
def set_ultra_low_latency(self, enabled):
    self.logger.debug("DBus call set_ultra_low_latency")
    driver_path = self.get_driver_path('ultra_low_latency')
    with open(driver_path, 'w') as f:
        f.write('1' if enabled else '0')


@endpoint('razer.device.audio.equalizer', 'getHeadphoneEQ', out_sig='ai')
def get_headphone_eq(self):
    self.logger.debug("DBus call get_headphone_eq")
    driver_path = self.get_driver_path('headphone_eq')
    with open(driver_path, 'r') as f:
        return [int(v) for v in f.read().strip().split()]


@endpoint('razer.device.audio.equalizer', 'setHeadphoneEQ', in_sig='ai')
def set_headphone_eq(self, bands):
    self.logger.debug("DBus call set_headphone_eq")
    driver_path = self.get_driver_path('headphone_eq')
    # Two accepted shapes:
    #   10 ints: just bands  → driver writes to profile 1 (Game) by default
    #   11 ints: [profile, b0..b9] → driver writes to the given profile slot
    # The 11-value form keeps the firmware's "current profile" pointer in sync
    # with what the UI thinks is selected, so the headset's profile-switch
    # button cycles correctly relative to the GUI.
    if len(bands) >= 11:
        profile = max(0, min(4, int(bands[0])))
        vals = [max(-6, min(6, int(b))) for b in bands[1:11]]
        payload = str(profile) + ' ' + ' '.join(str(v) for v in vals)
    else:
        vals = [max(-6, min(6, int(b))) for b in bands[:10]]
        payload = ' '.join(str(v) for v in vals)
    with open(driver_path, 'w') as f:
        f.write(payload)


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
