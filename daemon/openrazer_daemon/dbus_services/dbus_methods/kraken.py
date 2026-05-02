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


def _path_or_helper(self, *names):
    """Return the first attr path that exists for this device — V3 vs V3 Pro
    use different sysfs names for the same logical feature."""
    import os
    for name in names:
        try:
            p = self.get_driver_path(name)
            if os.path.exists(p):
                return p
        except Exception:
            continue
    return self.get_driver_path(names[0])  # fallback, will error on open


@endpoint('razer.device.audio.headphone', 'getWirelessPowerSave', out_sig='i')
def get_wireless_power_save(self):
    self.logger.debug("DBus call get_wireless_power_save")
    p = _path_or_helper(self, 'wireless_power_save', 'v3pro_power_save')
    with open(p, 'r') as f:
        return int(f.read().strip())


@endpoint('razer.device.audio.headphone', 'setWirelessPowerSave', in_sig='i')
def set_wireless_power_save(self, minutes):
    self.logger.debug("DBus call set_wireless_power_save")
    p = _path_or_helper(self, 'wireless_power_save', 'v3pro_power_save')
    with open(p, 'w') as f:
        f.write(str(int(minutes)))


@endpoint('razer.device.audio.headphone', 'getUltraLowLatency', out_sig='b')
def get_ultra_low_latency(self):
    self.logger.debug("DBus call get_ultra_low_latency")
    p = _path_or_helper(self, 'ultra_low_latency', 'v3pro_ultra_low_latency')
    with open(p, 'r') as f:
        return bool(int(f.read().strip()))


@endpoint('razer.device.audio.headphone', 'setUltraLowLatency', in_sig='b')
def set_ultra_low_latency(self, enabled):
    self.logger.debug("DBus call set_ultra_low_latency")
    p = _path_or_helper(self, 'ultra_low_latency', 'v3pro_ultra_low_latency')
    with open(p, 'w') as f:
        f.write('1' if enabled else '0')


@endpoint('razer.device.audio.equalizer', 'getHeadphoneEQ', out_sig='ai')
def get_headphone_eq(self):
    self.logger.debug("DBus call get_headphone_eq")
    p = _path_or_helper(self, 'headphone_eq', 'v3pro_headphone_eq')
    with open(p, 'r') as f:
        return [int(v) for v in f.read().strip().split()]


@endpoint('razer.device.audio.equalizer', 'setHeadphoneEQ', in_sig='ai')
def set_headphone_eq(self, bands):
    self.logger.debug("DBus call set_headphone_eq")
    # V3: full 11-int form is "profile b0..b9". V3 Pro driver only accepts a
    # single byte — the active firmware slot index — so for V3 Pro we pass
    # just the first int.
    import os
    v3pro_path = self.get_driver_path('v3pro_headphone_eq')
    is_v3pro = os.path.exists(v3pro_path)
    p = v3pro_path if is_v3pro else self.get_driver_path('headphone_eq')

    if is_v3pro:
        slot = int(bands[0]) if len(bands) else 0
        with open(p, 'w') as f:
            f.write(str(max(0, min(8, slot))))
        return

    # V3: two accepted shapes — 10 ints (legacy → profile 1 default) or
    # 11 ints (preferred: [profile, b0..b9]). The 11-value form keeps the
    # firmware's "current profile" pointer in sync with what the UI thinks
    # is selected, so the onboard EQ button cycles correctly.
    if len(bands) >= 11:
        profile = max(0, min(4, int(bands[0])))
        vals = [max(-6, min(6, int(b))) for b in bands[1:11]]
        payload = str(profile) + ' ' + ' '.join(str(v) for v in vals)
    else:
        vals = [max(-6, min(6, int(b))) for b in bands[:10]]
        payload = ' '.join(str(v) for v in vals)
    with open(p, 'w') as f:
        f.write(payload)


@endpoint('razer.device.audio.effects', 'getThxSpatialAudio', out_sig='b')
def get_thx_spatial_audio(self):
    self.logger.debug("DBus call get_thx_spatial_audio")
    p = _path_or_helper(self, 'thx_spatial_audio', 'v3pro_thx_spatial_audio')
    with open(p, 'r') as f:
        return bool(int(f.read().strip()))


@endpoint('razer.device.audio.effects', 'setThxSpatialAudio', in_sig='b')
def set_thx_spatial_audio(self, enabled):
    self.logger.debug("DBus call set_thx_spatial_audio")
    p = _path_or_helper(self, 'thx_spatial_audio', 'v3pro_thx_spatial_audio')
    with open(p, 'w') as f:
        f.write('1' if enabled else '0')


# V3 sysfs attrs are bare names ("sidetone", "headphone_eq", ...). V3 Pro uses
# v3pro_* prefixed paths for the headset-side ones (THX, ANC, ULL, sidetone,
# power-save, headphone_eq, battery, charging) but shares bare names for
# game_chat_balance / in_call_audio_mix / audio_prompts / mic_eq / mic_eq_preset.
# Each endpoint below resolves whichever attr exists via _path_or_helper().

# ── sidetone (V3 + V3 Pro) ───────────────────────────────────────────────────
@endpoint('razer.device.audio.headphone', 'getSidetone', out_sig='i')
def get_sidetone(self):
    self.logger.debug("DBus call get_sidetone")
    p = _path_or_helper(self, 'sidetone', 'v3pro_sidetone')
    with open(p, 'r') as f:
        return int(f.read().strip())


@endpoint('razer.device.audio.headphone', 'setSidetone', in_sig='i')
def set_sidetone(self, level):
    self.logger.debug("DBus call set_sidetone")
    p = _path_or_helper(self, 'sidetone', 'v3pro_sidetone')
    with open(p, 'w') as f:
        f.write(str(max(0, min(15, int(level)))))


# ── mic EQ (V3 + V3 Pro share these attr names) ──────────────────────────────
@endpoint('razer.device.audio.equalizer', 'getMicEQ', out_sig='ai')
def get_mic_eq(self):
    self.logger.debug("DBus call get_mic_eq")
    p = self.get_driver_path('mic_eq')
    with open(p, 'r') as f:
        return [int(v) for v in f.read().strip().split()]


@endpoint('razer.device.audio.equalizer', 'setMicEQ', in_sig='ai')
def set_mic_eq(self, bands):
    self.logger.debug("DBus call set_mic_eq")
    p = self.get_driver_path('mic_eq')
    vals = [max(-6, min(6, int(b))) for b in bands[:10]]
    with open(p, 'w') as f:
        f.write(' '.join(str(v) for v in vals))


@endpoint('razer.device.audio.equalizer', 'getMicEQPreset', out_sig='i')
def get_mic_eq_preset(self):
    self.logger.debug("DBus call get_mic_eq_preset")
    p = self.get_driver_path('mic_eq_preset')
    with open(p, 'r') as f:
        return int(f.read().strip())


@endpoint('razer.device.audio.equalizer', 'setMicEQPreset', in_sig='i')
def set_mic_eq_preset(self, preset_idx):
    self.logger.debug("DBus call set_mic_eq_preset")
    p = self.get_driver_path('mic_eq_preset')
    with open(p, 'w') as f:
        f.write(str(max(0, min(3, int(preset_idx)))))


# ── audio function button (V3 only — V3 Pro lacks this hw button) ────────────
@endpoint('razer.device.audio.headphone', 'getAudioFunctionButton', out_sig='i')
def get_audio_function_button(self):
    self.logger.debug("DBus call get_audio_function_button")
    p = self.get_driver_path('audio_function_button')
    with open(p, 'r') as f:
        return int(f.read().strip())


@endpoint('razer.device.audio.headphone', 'setAudioFunctionButton', in_sig='i')
def set_audio_function_button(self, mode):
    self.logger.debug("DBus call set_audio_function_button")
    p = self.get_driver_path('audio_function_button')
    with open(p, 'w') as f:
        f.write(str(max(1, min(2, int(mode)))))


# ── game/chat balance (V3 + V3 Pro) ──────────────────────────────────────────
@endpoint('razer.device.audio.headphone', 'getGameChatBalance', out_sig='i')
def get_game_chat_balance(self):
    self.logger.debug("DBus call get_game_chat_balance")
    p = self.get_driver_path('game_chat_balance')
    with open(p, 'r') as f:
        return int(f.read().strip())


@endpoint('razer.device.audio.headphone', 'setGameChatBalance', in_sig='i')
def set_game_chat_balance(self, balance):
    """0 = full game · 10 = center · 20 = full chat."""
    self.logger.debug("DBus call set_game_chat_balance")
    p = self.get_driver_path('game_chat_balance')
    with open(p, 'w') as f:
        f.write(str(max(0, min(20, int(balance)))))


# ── in-call audio mix (V3 + V3 Pro) ──────────────────────────────────────────
@endpoint('razer.device.audio.headphone', 'getInCallAudioMix', out_sig='i')
def get_in_call_audio_mix(self):
    self.logger.debug("DBus call get_in_call_audio_mix")
    p = self.get_driver_path('in_call_audio_mix')
    with open(p, 'r') as f:
        return int(f.read().strip())


@endpoint('razer.device.audio.headphone', 'setInCallAudioMix', in_sig='i')
def set_in_call_audio_mix(self, mode):
    """0=combine, 1=lower 2.4 GHz, 2=mute 2.4 GHz when BT call is active."""
    self.logger.debug("DBus call set_in_call_audio_mix")
    p = self.get_driver_path('in_call_audio_mix')
    with open(p, 'w') as f:
        f.write(str(max(0, min(2, int(mode)))))


# ── audio prompts (V3 + V3 Pro) ──────────────────────────────────────────────
@endpoint('razer.device.audio.headphone', 'getAudioPrompts', out_sig='b')
def get_audio_prompts(self):
    self.logger.debug("DBus call get_audio_prompts")
    p = self.get_driver_path('audio_prompts')
    with open(p, 'r') as f:
        return bool(int(f.read().strip()))


@endpoint('razer.device.audio.headphone', 'setAudioPrompts', in_sig='b')
def set_audio_prompts(self, enabled):
    self.logger.debug("DBus call set_audio_prompts")
    p = self.get_driver_path('audio_prompts')
    with open(p, 'w') as f:
        f.write('1' if enabled else '0')


# ── ANC / Ambient (V3 Pro only) ──────────────────────────────────────────────
@endpoint('razer.device.audio.effects', 'getAnc', out_sig='ii')
def get_anc(self):
    """Returns (mode, level). mode: 0=off / 1=ANC / 2=ambient. level: 1..4 (ANC only)."""
    self.logger.debug("DBus call get_anc")
    p = self.get_driver_path('v3pro_anc')
    with open(p, 'r') as f:
        parts = f.read().strip().split()
        return (int(parts[0]) if parts else 0, int(parts[1]) if len(parts) > 1 else 1)


@endpoint('razer.device.audio.effects', 'setAnc', in_sig='ii')
def set_anc(self, mode, level):
    self.logger.debug("DBus call set_anc")
    p = self.get_driver_path('v3pro_anc')
    mode = max(0, min(2, int(mode)))
    level = max(1, min(4, int(level)))
    with open(p, 'w') as f:
        f.write(f"{mode} {level}")


# ── battery (V3 Pro only) ────────────────────────────────────────────────────
# Distinct function names from mamba's get_battery / is_charging — those are
# imported into dbus_methods/__init__.py via `from mamba import *` and would
# be shadowed if we redefined them here. Polychromatic / pylib map both onto
# razer.device.power.{getBattery,isCharging} via interface routing.
@endpoint('razer.device.power', 'getBattery', out_sig='d')
def get_battery_v3pro(self):
    """Battery percent 0..100, or -1 if unknown (no GET response yet)."""
    self.logger.debug("DBus call get_battery_v3pro")
    p = self.get_driver_path('v3pro_battery_level')
    with open(p, 'r') as f:
        return float(f.read().strip())


@endpoint('razer.device.power', 'isCharging', out_sig='b')
def is_charging_v3pro(self):
    self.logger.debug("DBus call is_charging_v3pro")
    p = self.get_driver_path('v3pro_charging')
    with open(p, 'r') as f:
        return int(f.read().strip()) == 1
