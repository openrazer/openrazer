"""
Read and change the audio settings of a BlackShark headset.

Shows the equalizer, EQ presets, sidetone and do-not-disturb controls exposed
by the razerblackshark driver, plus the standard battery / idle-time API.

Reporting only by default. Pass --apply to also demonstrate the setters; that
overwrites the headset's stored custom EQ curve, so it is opt-in.
"""

import sys

from openrazer.client import DeviceManager

apply_changes = '--apply' in sys.argv[1:]

device_manager = DeviceManager()

headsets = [device for device in device_manager.devices
            if device.type == 'headset' and device.has('equalizer')]

if not headsets:
    print("No BlackShark headset found.")
    raise SystemExit(1)

for headset in headsets:
    print("{} ({})".format(headset.name, headset.serial))

    # The product id the headset reports for itself. Over a 2.4GHz dongle this
    # identifies the paired headset, so it differs from the dongle's USB id.
    if headset.has('hardware_model'):
        print("  hardware model: {}".format(headset.hardware_model))

    # Battery is the standard power API, shared with mice and keyboards.
    if headset.has('battery'):
        print("  battery       : {}%{}".format(
            round(headset.battery_level),
            " (charging)" if headset.is_charging else ""))

    # Auto power-off, in seconds. 0 disables it.
    if headset.has('get_idle_time'):
        print("  auto power-off: {} min".format(headset.get_idle_time() // 60))

    print("  eq preset     : {}".format(headset.equalizer_preset))
    print("  eq curve      : {}".format(headset.equalizer))

    if headset.has('sidetone'):
        print("  sidetone      : {}%".format(headset.sidetone))

    # State of the hardware mic-mute button. Read-only: only the button
    # itself changes it.
    if headset.has('mic_mute'):
        print("  mic muted     : {}".format("yes" if headset.mic_mute else "no"))

    if headset.has('dnd'):
        print("  do not disturb: {}".format("on" if headset.dnd else "off"))

    if not apply_changes:
        print("  (re-run with --apply to demonstrate the setters)")
        continue

    # Switch to a device-stored preset...
    headset.equalizer_preset = 'music'

    # ...or push a custom 10-band curve (31Hz, 63, 125, 250, 500, 1k, 2k, 4k,
    # 8k, 16k) in dB. Setting a curve implicitly selects the custom preset and
    # replaces whatever curve was stored there.
    headset.equalizer = [4, 3, 2, 0, 0, 0, 1, 2, 3, 4]

    # Sidetone is a percentage; 0 turns it off.
    if headset.has('sidetone'):
        headset.sidetone = 50

    print("  -> applied a custom V-shaped curve")
