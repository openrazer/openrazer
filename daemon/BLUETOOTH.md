# Experimental Basilisk V3 Pro Bluetooth backend

This branch adds a userspace BlueZ GATT transport to the existing OpenRazer daemon. It requires the optional `dbus-fast` dependency (`openrazer_daemon[bluetooth]`) and a mouse already paired and connected using the desktop's Bluetooth settings. No kernel module, udev rule, scanning, pairing, or device disconnect is required for this transport.

Enable `bluetooth_enabled = True` in `[General]` of the daemon configuration. The default is disabled. `bluetooth_only = True` disables USB discovery and its plugdev requirement; leave that option false if USB devices are needed. Fake-driver tests do not start BlueZ.

Discovery uses cached BlueZ objects, connected/paired/resolved state, the vendor GATT service and a PnP identity matching VID `068e`, PID `00ac`. The D-Bus serial is a stable `BT_` prefix plus the paired Bluetooth address, rather than a manufactured hardware serial. Disconnect removes the device object; reconnect recreates it and emits the standard device signals. Polling runs every three seconds and sends no vendor commands for already registered devices. Commands are serialized on a worker event loop so notification reception does not depend on the daemon's GLib callback returning. Vendor exchanges time out, do not automatically retry, and fault the session after a timeout; recovery requires reconnecting the mouse.

Validated features exposed through existing interfaces:

- battery charge level (0..255 scaled to percent)
- current X/Y DPI (100..30000), changing only the active entry of the existing five-stage table
- a read-only DPI-stage query (existing API uses one-based active stage)
- per-zone brightness: underglow/backlight, logo and scroll wheel
- whole-device and per-zone static RGB colors
- sleep timeout (60..900 seconds)

Every settings setter checks device readback. Startup, reconnect and shutdown do not apply saved settings or change hardware state. Hardware retains ownership of settings; persistence across sleep/reconnect is not guaranteed by this backend. A multi-zone static setter can fail after earlier zones have already changed; errors are surfaced without retry or guessing at a rollback.

Not exposed: charging status (unknown over Bluetooth), firmware version (reported as `unknown`), polling rate, DPI stage-table editing, low battery threshold, unvalidated lighting modes, per-LED frames, scroll controls, profiles, remapping, macros, screensaver lighting or daemon persistence restoration. Only validated incoming static/brightness synchronization is recognized. RazerGenie can use the existing discovery, DPI, power and lighting APIs; its current power widget may log a warning and display “Not Charging” when a charging query is unavailable. The Python client now checks the separate `charging_status` capability before querying it.

## Validation

Run `PYTHONPATH=daemon python -m unittest discover -s daemon/tests -p test_bluetooth.py -v` for protocol, settings, input validation, serialized read/modify/write, failed-readback/no-retry, timeout and device reconnect lifecycle coverage. No hardware is required for those tests.

The real Bluetooth mouse was exercised through an isolated full daemon and the standard `openrazer.client.DeviceManager`: discovery, DPI write/readback, brightness write/readback, idle timeout write/readback, and static red write/readback. DPI, brightness and idle timeout were restored and verified; static red was left applied at the user's request. Earlier raw tests covered all three lighting zones. A pre-existing `test_effect_sync.test_notify_run_effect_edge_case_3` failure reproduces on the unmodified upstream snapshot with Python 3.14.
