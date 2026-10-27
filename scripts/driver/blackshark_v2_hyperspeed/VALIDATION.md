# Validation record

Tested on 2026-10-27, Arch Linux, kernel `7.2.11-arch1-1`, physical
BlackShark V2 HyperSpeed headset and receiver, both reporting firmware
`v1.7.0.0`. Base: OpenRazer master
`477f4d6d3ab7e9deb6310d5bd3742560c5cbe80d`; the branch was explicitly rebased
onto that commit after fetching current master.

## Software checks

Passed:

- All five modules built with `make driver KCFLAGS='-Wall -Werror' W=1`.
  Kbuild reports the existing shared `compat.o` multiple-module warning; there
  were no compiler warnings or errors.
- Upstream `check-astyle-formatting.sh`, `check-autopep8-formatting.sh`,
  `check-pylint.sh`, `test-mypy.sh`, `test-vermin.sh`,
  `test-hex-casing.sh`, and `test-auto-generate.sh`.
- Upstream `test-daemon.sh` with all 276 fake devices on a private D-Bus.
- Five BlackShark daemon/client unit tests, including signed EQ values,
  invalid input, wired capabilities, and actual `dbus.Byte` conversion.
- The shared protocol-helper test with AddressSanitizer and UndefinedBehaviorSanitizer.
- Private D-Bus round trips through the source daemon and public Python client
  for both generated BlackShark devices.

The complete daemon unit suite ran 29 tests: 28 passed, and
`test_effect_sync.EffectSyncTest.test_notify_run_effect_edge_case_3` errored
because `effect_call` was `None`. The identical failure was reproduced on
pristine master `477f4d6d` (23 of 24 tests passed). This patch does not modify
that test or effect synchronization.

The fake-device harness bypassed only the distribution-specific `plugdev`
membership preflight. Live source-daemon tests used a private bus and disabled
startup persistence restoration, effect synchronization and battery notifications.
The reconnect test used real udev discovery, not a pinned fake-device path;
its private daemon loaded only the two BlackShark device classes.

## Physical hardware results

"Read/write/readback" below means values were accepted and read back through
the actual kernel driver, source daemon and public client. It does not imply an
acoustic measurement of an audio effect. All saved settings were restored and
compared with the originals after each write test.

| Check | Wireless `1532:0565` | Wired `1532:056E` |
| --- | --- | --- |
| Battery, charging | Passed; 91–96% observed across the session, not charging without cable | Passed; charging with cable |
| Receiver/headset identities and firmware | Passed; distinct real receiver and headset serials | Passed; real headset identity |
| Sidetone enable and levels | Read/write/readback; levels 0 and 15 | Same |
| EQ enable and all five presets | Read/write/readback | Same |
| Ten-band custom EQ and latch | Read/write/readback, including −9 and +6 dB | Same |
| Onboard DND bit | Read/write/readback | Same |
| Idle timer | Read/write/readback for 0, 60 and 15,300 seconds | Same |
| Microphone mute state | Read; physical switch transitions separately checked | Read |
| Receiver link and all four LED modes | Read/write/readback | Not advertised by wired client |
| Concurrent reads | 72 reads, four threads | Same |
| Invalid writes | Rejected out-of-range level/preset/bands/timer/LED, overflow and embedded NUL | Same, without receiver LED |
| Audio USB bindings | Interfaces 0–2 retained `snd_usb_audio` | Same |

Additional acceptance checks:

- Cable handover in both directions passed, including repeating the checks on
  the final V2 module. With the cable attached, receiver-local identity remains
  readable; an inactive radio battery query fails explicitly with `ETIMEDOUT`
  (about 0.84 seconds), rather than reporting a fabricated value.
- Physical receiver unplug/replug to the same port passed. The live daemon
  removed the old object, a stale client failed explicitly, and the same daemon
  discovered a new HID node with the same stable identity. Battery, firmware
  and all saved settings remained readable after reconnect.
  The receiver briefly enumerated as `0e8d:0003` before returning as
  `1532:0565`; this observation is included rather than treating a successful
  reconnect as proof against every possible enumeration fault. No kernel
  WARN/BUG or USB error/reset was observed during this test.
- Headset idle sleep occurred after 60.3 seconds with a temporary one-minute
  timer. Receiver-local identity remained readable while asleep, and a battery
  query returned a D-Bus timeout error. After the user woke the headset, the
  same client recovered battery, charging, firmware and mute reads. The original
  1,800-second timer and every other saved setting were verified restored.
  The desktop's continuously linked playback/capture streams initially
  prevented sleep. For the successful test, only this headset's PipeWire audio
  profile was temporarily turned off with `save=false`; the original profile
  and all five original headset routes were compared and restored afterward.
  The PC was not suspended.
- The physical microphone mute **switch** was observed changing `1 → 0 → 1`
  through the driver, returning to its initial state. Microphone audio was not
  recorded for this check.
- The user confirmed that short-pressing the power button controls play/pause
  and that normal playback works with the temporary driver. This is a user
  confirmation of behavior; no specific Linux key code is claimed as captured.

## Limits

Not established by these tests: PC suspend/resume, long-duration soak or repeated
reconnect stress, other firmware revisions, Bluetooth call/voice-prompt behavior
of the DND bit, measured EQ/sidetone audio response, or firmware flashing.
Host-side THX processing and Bluetooth pairing are outside this patch. Battery
`power_supply`/UPower export is separate work in #2868 and is not claimed here.

The experimental module was loaded temporarily, without persistent installation.
The production driver/daemon configuration is a separate deployment decision.
