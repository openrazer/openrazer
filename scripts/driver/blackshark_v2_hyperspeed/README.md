# BlackShark V2 HyperSpeed

The MediaTek vendor protocol differs from the Kraken memory protocol and the
standard Razer 90-byte protocol. `razerblacksharkv2` binds only a HID interface
whose descriptor contains usage `0xFF140001`. Audio playback/capture remains
handled by `snd_usb_audio`; consumer/media and telephony HID reports continue
through the default HID input handling.

USB IDs: `1532:0565` (receiver), `1532:056E` (USB cable). Headset commands use
domain `0x80` through the receiver and domain `0x00` over the cable. Receiver
identity and LED commands always use domain `0x00`. Wired v1.7 firmware replies with
logical headset domain `0x80` even to local-domain requests; correlation permits
this alias only on the wired PID. The receiver's identity is
available while the headset is switched off; headset reads return an error on
timeout instead of inventing a battery reading.

## Supported onboard controls

| Function | API / sysfs | Values |
| --- | --- | --- |
| Battery / charging | Existing `battery_level`, `is_charging` | 0–100%, boolean |
| Device identity | Existing `serial`, `firmware_version` | Receiver identity when wireless |
| Headset identity | `headset_serial`, `headset_firmware_version` | Headset identity over either transport |
| Auto power off | Existing `get_idle_time()`, `set_idle_time(seconds)` | 0–15,300 seconds, multiples of 60; 0 disables |
| Sidetone | `sidetone_enabled`, `sidetone_level` | Boolean, 0–15 |
| EQ stage | `equalizer_enabled` | Boolean |
| EQ preset | `equalizer_preset` | Flat 0, Game 7, Music 8, Movie 9, Custom 255 |
| Custom EQ curve | `equalizer_bands` | Ten integer gains, −9 to +6 dB |
| Onboard DND flag | `do_not_disturb` | Boolean |
| Physical microphone mute | `microphone_muted` | Read only |
| Receiver LED | `dongle_led_mode` | Off 0, Link 1, Battery 2, Warning 3; receiver only |
| Receiver RF link | `wireless_connected` | Read only; receiver only |

Audio-specific methods are exposed on `razer.device.audio.blacksharkv2`; each feature also
has a Python client capability. The factory selects this class only for these two PIDs;
existing headsets retain their existing class and capabilities. Headsets do not receive settings writes during initialization.

EQ frequencies: 31, 63, 125, 250, 500, 1k, 2k, 4k, 8k, 16k Hz. Band readback is
the stored custom curve. Selecting a preset enables the EQ stage. Writing a
custom curve enables EQ, selects Custom, writes gains with the required +5 wire
offset, waits 40 ms, then selects Custom again to latch the new curve. Neither
operation sends any undocumented audio-enhancement command.

The kernel serializes the entire EQ operation and all other transactions. It
validates reply length, payload bounds, checksum, sequence, command, domain and
acknowledgement, device status byte 1, and setter status payload. The ACK flag means receipt; successful
SET replies carry a single zero byte. Unsupported replies return `EOPNOTSUPP`; busy and timeout statuses remain errors.
An unsuccessful SET payload also returns an I/O error.
A query times out after 800 ms. Invalid settings are rejected
before any write. The legacy charge-level ABI remains 0–255; rounding upwards
preserves the headset's exact integer percentage after the daemon's conversion.

These are onboard device controls. Host-side THX processing, firmware flashing,
and Bluetooth configuration/pairing are outside this driver. DND register read/write is verified; its effect on Bluetooth calls or voice prompts
is not claimed without a separate behavioral test.

The reference projects label `0x1D/0x9D` as an audio-enhancement switch, but real
v1.7 hardware rejects both reads and writes: reply byte 1 is `0x05` (unsupported)
rather than `0x02` (success). Both `0x9D [0]` and `0x9D [1]` also return failure
payload `[1]`, while working setters return `[0]`. This was checked over both
transports, including delayed readback. AlexandreFournier's existing fork
independently documents the status-byte interpretation. This patch does not
expose or send these unsupported commands.

## Protocol provenance

See [COMPARISON.md](COMPARISON.md) for the prior-work comparison and
[THIRD_PARTY.md](THIRD_PARTY.md) for reference credits and retained notices.

- MIT-licensed protocol documentation and reference implementation:
  https://github.com/justik13/razer-blackshark-v2-hyperspeed-webhid
- Independent implementation used to cross-check framing and register mapping:
  https://github.com/bbesli/Synapse-for-Linux
- Device support request: https://github.com/openrazer/openrazer/issues/2316
- Standard battery export is separate work:
  https://github.com/openrazer/openrazer/pull/2868

The main device-support branch does not duplicate or import the generic
`power_supply` helper from #2868. An integration branch can register this
headset with that helper when both changes are available. UPower export is not
advertised by this standalone patch.

## Repeatable software tests

From the repository root:

```sh
make driver
cc -Wall -Wextra -Werror -fsanitize=address,undefined \
    scripts/driver/blackshark_v2_hyperspeed/test_protocol.c -o /tmp/blackshark-protocol-test
ASAN_OPTIONS=detect_leaks=0 /tmp/blackshark-protocol-test
PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=pylib:daemon \
    python -m unittest discover -s daemon/tests -p test_blackshark.py -v
PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=pylib:daemon dbus-run-session -- \
    python scripts/driver/blackshark_v2_hyperspeed/test_dbus.py
```

The D-Bus test creates fake wired/wireless devices and a private daemon and bus.
It mocks only the distribution-specific group preflight and never exposes real
devices. It covers discovery, capabilities, signed EQ bands, all writable audio
controls, and power/receiver controls through the public Python client.

## Hardware acceptance

See [VALIDATION.md](VALIDATION.md) for the tested firmware, actual results,
baseline test failure, and remaining limits.

Record read/write/readback and restore results separately for both transports.
Also check headset off/on, receiver reconnect, cable handover, charging changes,
physical microphone mute, concurrent reads, malformed input rejection, and media
keys. Restore the user's original curve, preset, EQ flag, sidetone,
sleep, DND and LED values after any write test. Test audio playback and capture
through the original audio driver after binding and after rollback.
