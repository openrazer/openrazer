# Experimental Basilisk V3 Pro Bluetooth backend

This opt-in userspace backend controls the Basilisk V3 Pro using its BlueZ vendor GATT service. It needs the optional `dbus-fast` dependency (`openrazer_daemon[bluetooth]`). Pair and connect the mouse through the desktop Bluetooth settings first. The backend does not scan, pair, connect, disconnect the mouse, or require a Bluetooth kernel driver.

In the daemon configuration:

```ini
[General]
bluetooth_enabled = True
# Optional: skip USB discovery and its plugdev requirement.
bluetooth_only = False
```

The defaults preserve existing USB operation. Fake-driver tests do not initialize BlueZ. Discovery checks paired, connected, services-resolved state, vendor service UUID and Bluetooth PnP VID/PID `068e:00ac`. Only this model is supported. The stable D-Bus identifier is `BT_` plus the Bluetooth address; it is not a hardware serial number.

## Features

Existing OpenRazer interfaces expose:

- Battery percentage and idle timeout (60..900 seconds).
- Current X/Y DPI (100..30000), one to five editable stages and active-stage selection.
- Independent body/underglow, logo and wheel brightness and effects: off, static, spectrum, single/dual/random breathing and wave in either direction.
- Tactile/free-spin mode, Smart-Reel and scroll acceleration through `razer.device.scroll`.

RazerGenie discovers standard lighting and DPI-stage controls through introspection. Its body effect changes only underglow; logo and wheel are independent. RazerGenie has no scroll-mechanics controls, but `openrazer.client` exposes `scroll_mode`, `scroll_smart_reel` and `scroll_acceleration`.

Not implemented: charging status, firmware version (reported as `unknown`), polling rate, low-battery threshold, reactive lighting, adjustable effect speed, per-LED frames, onboard profile management, remapping/macros, screensaver lighting and daemon persistence restoration. A separate `charging_status` client capability avoids calling an unavailable method. Current RazerGenie may still display “Not Charging” as a fallback when its charging query fails.

## Transport and restrictions

Vendor exchanges are serialized on an asyncio worker so notifications keep flowing while GLib dispatches synchronous requests. Frames are fragmented into at most 20-byte writes. Exchanges time out and are never automatically retried. Timeout, cancellation or transport failure poisons the session until the mouse reconnects. Firmware notification IDs 0/1 are excluded from request rollover. Complete device rejections are reported without losing synchronization.

Cached BlueZ objects are reconciled every three seconds; already registered devices receive no discovery vendor queries. Disconnection removes the D-Bus device and reconnection recreates it with standard signals. Calls remain synchronous to the daemon and discovery/commands may block GLib until their bounded timeout. BlueZ service restart recovery is not separately validated; restart the OpenRazer daemon if needed.

DPI, brightness, lighting and scroll writes are restricted to the active base profile, target 1. No profile is created, deleted or selected. Setters verify readback and surface differences without retrying or guessing a rollback. DPI edits preserve visible stage IDs and reserved bytes; expanding a shortened table is restricted to the validated sequential IDs 1..5. The mouse omits final reserved bytes in some readbacks, and setters send the validated complete 38-byte DPI command.

Lighting writes verify target 1 and the live target-0 mirror, with a distinct live write only if needed. Startup, reconnect and shutdown do not apply cached settings. Only incoming static/brightness synchronization is recognized; effect synchronization with other devices is incomplete.

## Hardware validation

Tested on one Linux/BlueZ Basilisk V3 Pro, Bluetooth PnP `068e:00ac`, through the normal daemon, standard Python client and RazerGenie:

- DPI edits, independent X/Y values, selection, and counts 4, 2, 1, 3, 5; original presets and selected stage restored.
- Brightness and idle-time changes with verified restoration.
- Independent zone RGB and every advertised effect, with visual confirmation of animations and wave reversal.
- Static, spectrum, reversed wave and lighting off retained their stored/live states after physical power cycles. Breathing variants were visually confirmed and read back, but not separately power-cycled.
- Software tactile/free-spin switching and Smart-Reel automatic switching physically confirmed. Acceleration toggles/readback verified; its effect on scroll distance was tentatively confirmed by the user.
- Free-spin, Smart-Reel off and acceleration on all retained after a physical off/on cycle, with smooth wheel feel and standard-client readback.
- Legacy image API supplied for RazerGenie startup compatibility; normal reconnect rediscovery verified.

Other model variants, USB/BT coexistence on physical hardware, BlueZ restart, system suspend and natural idle-sleep retention are not separately validated.

## Tests

```sh
PYTHONPATH=daemon python -m unittest discover -s daemon/tests -p test_bluetooth.py -v
```

Tests cover framing, fragmentation, timeout poisoning, request rollover, malformed DPI, preserved reserved data, profile guards, count changes, independent zone controls, effect readback, scroll settings, lifecycle signals and RazerGenie metadata. The complete daemon suite also has a pre-existing `test_effect_sync.test_notify_run_effect_edge_case_3` failure reproduced on the unmodified upstream snapshot under Python 3.14.
