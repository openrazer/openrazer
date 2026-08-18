#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""
Userspace Razer configuration CLI for macOS.

macOS cannot load the OpenRazer Linux kernel module, but the same vendor
protocol is reachable from userspace: the 90-byte razer_report is exchanged
as HID feature reports on the device's control interface, which IOHIDManager
(via hidapi) exposes without any kernel driver or special entitlements.

This tool reuses the protocol implemented in driver/razercommon.c and
driver/razerchromacommon.c. Devices are described in the DEVICES table below;
adding a device is a matter of copying its PID, transaction id and supported
feature set from the Linux driver sources.

Requires the hidapi shared library (`brew install hidapi`) and the `hid`
Python package (`pip install hid`).

Verified on real hardware:
  - Razer Pro Click Mini via HyperSpeed receiver (1532:009A)
"""

import argparse
import ctypes
import os
import sys
import time

# hidapi's Python binding dlopens by bare library name, which misses
# Homebrew's lib dir on Apple Silicon; point it at the dylib explicitly.
_orig_load = ctypes.cdll.LoadLibrary
def _load(name):
    if "hidapi" in name:
        for p in ("/opt/homebrew/lib/libhidapi.dylib", "/usr/local/lib/libhidapi.dylib"):
            if os.path.exists(p):
                return _orig_load(p)
    return _orig_load(name)
ctypes.cdll.LoadLibrary = _load
import hid  # noqa: E402
ctypes.cdll.LoadLibrary = _orig_load

USB_VENDOR_ID_RAZER = 0x1532
REPORT_LEN = 90

# Report status codes (razercommon.h)
STATUS_NEW, STATUS_BUSY, STATUS_OK = 0x00, 0x01, 0x02
STATUS_NAMES = {0x01: "BUSY", 0x02: "OK", 0x03: "FAILURE", 0x04: "TIMEOUT", 0x05: "NOT_SUPPORTED"}

VARSTORE, NOSTORE = 0x01, 0x00
POLL_CODES = {1000: 0x01, 500: 0x02, 125: 0x08}
POLL_RATES = {v: k for k, v in POLL_CODES.items()}


class DeviceSpec:
    def __init__(self, name, pid, transaction_id, dpi_max, features, control_interface=0):
        self.name = name
        self.pid = pid
        self.transaction_id = transaction_id
        self.dpi_max = dpi_max
        self.features = features           # subset of the FEATURES the CLI offers
        self.control_interface = control_interface


# Feature sets mirror the METHODS lists in daemon/openrazer_daemon/hardware/.
# Transaction ids come from the switch tables in driver/razermouse_driver.c.
DEVICES = {
    0x009A: DeviceSpec(
        name="Razer Pro Click Mini (Receiver)",
        pid=0x009A,
        transaction_id=0x1F,
        dpi_max=12000,
        features={"dpi", "dpi_stages", "poll", "battery", "idle"},
    ),
    # Add further devices here — PID, transaction id and features are all in
    # the Linux driver sources. Untested entries should stay commented until
    # verified on hardware.
}


def build_report(tid, cclass, cid, data_size, args=b""):
    r = bytearray(REPORT_LEN)
    r[1] = tid
    r[5] = data_size
    r[6] = cclass
    r[7] = cid
    r[8:8 + len(args)] = args
    crc = 0
    for b in r[2:88]:
        crc ^= b
    r[88] = crc
    return bytes(r)


def find_device():
    for info in hid.enumerate(USB_VENDOR_ID_RAZER, 0):
        spec = DEVICES.get(info["product_id"])
        if spec and info["interface_number"] == spec.control_interface:
            try:
                return spec, hid.Device(path=info["path"])
            except hid.HIDException as e:
                sys.exit(f"Found {spec.name} but could not open it: {e}")
    razer_pids = sorted({i["product_id"] for i in hid.enumerate(USB_VENDOR_ID_RAZER, 0)})
    if razer_pids:
        sys.exit("Razer device(s) found but not in the DEVICES table: "
                 + ", ".join(f"1532:{p:04x}" for p in razer_pids)
                 + "\nAdd an entry (PID/transaction id/features are in the Linux driver sources).")
    sys.exit("No Razer device found.")


def command(spec, dev, cclass, cid, data_size, args=b""):
    """Send one razer_report, poll for the response, return its 80 arg bytes."""
    dev.send_feature_report(b"\x00" + build_report(spec.transaction_id, cclass, cid, data_size, args))
    for _ in range(20):
        # Wireless receivers round-trip the command to the mouse (~50-100 ms).
        time.sleep(0.05)
        resp = bytes(dev.get_feature_report(0x00, REPORT_LEN + 1))
        if len(resp) > REPORT_LEN:
            resp = resp[1:]
        status = resp[0]
        if status in (STATUS_NEW, STATUS_BUSY):
            continue
        if status == STATUS_OK and resp[6] == cclass and resp[7] == cid:
            return resp[8:88]
        raise RuntimeError(f"command {cclass:#04x}/{cid:#04x}: status "
                           f"{STATUS_NAMES.get(status, hex(status))}")
    raise RuntimeError(f"command {cclass:#04x}/{cid:#04x}: no response "
                       "(device asleep or out of range?)")


# ---- protocol commands (builders mirror driver/razerchromacommon.c) ----

def get_serial(s, d):    return command(s, d, 0x00, 0x82, 0x16).split(b"\x00")[0].decode(errors="replace")
def get_firmware(s, d):  a = command(s, d, 0x00, 0x81, 0x02); return f"v{a[0]}.{a[1]}"
def get_battery(s, d):   return round(command(s, d, 0x07, 0x80, 0x02)[1] / 255 * 100)
def get_poll(s, d):      return POLL_RATES.get(command(s, d, 0x00, 0x85, 0x01)[0])
def set_poll(s, d, hz):  command(s, d, 0x00, 0x05, 0x01, bytes([POLL_CODES[hz]]))
def get_idle(s, d):      a = command(s, d, 0x07, 0x83, 0x02); return (a[0] << 8) | a[1]
def set_idle(s, d, sec): command(s, d, 0x07, 0x03, 0x02, bytes([(sec >> 8) & 0xFF, sec & 0xFF]))


def get_dpi(s, d, store=VARSTORE):
    a = command(s, d, 0x04, 0x85, 0x07, bytes([store]))
    return (a[1] << 8) | a[2], (a[3] << 8) | a[4]


def set_dpi(s, d, x, y=None, store=VARSTORE):
    y = y if y is not None else x
    x, y = (max(100, min(s.dpi_max, v)) for v in (x, y))
    command(s, d, 0x04, 0x05, 0x07, bytes([store, x >> 8, x & 0xFF, y >> 8, y & 0xFF, 0, 0]))
    return x, y


def get_dpi_stages(s, d):
    a = command(s, d, 0x04, 0x86, 0x26, bytes([VARSTORE]))
    active, count = a[1], a[2]
    stages, off = [], 3
    for _ in range(count):
        stages.append(((a[off + 1] << 8) | a[off + 2], (a[off + 3] << 8) | a[off + 4]))
        off += 7
    return active, stages


def set_dpi_stages(s, d, stages, active=1):
    args = bytearray([VARSTORE, active, len(stages)])
    for n, (x, y) in enumerate(stages, start=1):
        x, y = (max(100, min(s.dpi_max, v)) for v in (x, y))
        args += bytes([n, x >> 8, x & 0xFF, y >> 8, y & 0xFF, 0, 0])
    command(s, d, 0x04, 0x06, 0x26, bytes(args))


# ---- CLI ----

def parse_dpi(v):
    parts = v.lower().split("x")
    return (int(parts[0]), int(parts[1])) if len(parts) == 2 else (int(parts[0]), None)


def main():
    p = argparse.ArgumentParser(description="Configure Razer devices on macOS (userspace, no kernel driver).")
    sub = p.add_subparsers(dest="cmd")
    sub.add_parser("status", help="show device info and current settings (default)")
    sp = sub.add_parser("dpi", help="get/set DPI, e.g. `dpi 1600` or `dpi 800x1600`")
    sp.add_argument("value", nargs="?", type=parse_dpi)
    sp = sub.add_parser("poll", help="get/set polling rate (125/500/1000)")
    sp.add_argument("hz", nargs="?", type=int, choices=sorted(POLL_CODES))
    sp = sub.add_parser("stages", help="get/set DPI stages, e.g. `stages 400,800,1600 --active 2`")
    sp.add_argument("list", nargs="?")
    sp.add_argument("--active", type=int, default=1)
    sp = sub.add_parser("idle", help="get/set sleep timeout in seconds")
    sp.add_argument("seconds", nargs="?", type=int)
    args = p.parse_args()

    spec, dev = find_device()
    try:
        cmd = args.cmd or "status"
        if cmd == "status":
            print(f"device    : {spec.name}")
            def show(label, feature, fn):
                if feature and feature not in spec.features:
                    return
                try:
                    print(f"{label:<10}: {fn()}")
                except RuntimeError as e:
                    print(f"{label:<10}: n/a ({e})")
            show("serial", None, lambda: get_serial(spec, dev))
            show("firmware", None, lambda: get_firmware(spec, dev))
            show("battery", "battery", lambda: f"{get_battery(spec, dev)}%")
            show("dpi", "dpi", lambda: "%d x %d" % get_dpi(spec, dev))
            show("poll rate", "poll", lambda: f"{get_poll(spec, dev)} Hz")
            def stages_str():
                active, stages = get_dpi_stages(spec, dev)
                return f"{', '.join('%d' % x for x, _ in stages)} (active: #{active})"
            show("stages", "dpi_stages", stages_str)
            show("idle time", "idle", lambda: f"{get_idle(spec, dev)} s")
        elif cmd == "dpi":
            if args.value:
                print("dpi set to %d x %d" % set_dpi(spec, dev, args.value[0], args.value[1]))
            else:
                print("%d x %d" % get_dpi(spec, dev))
        elif cmd == "poll":
            if args.hz:
                set_poll(spec, dev, args.hz)
            print(f"{get_poll(spec, dev)} Hz")
        elif cmd == "stages":
            if args.list:
                vals = [int(v) for v in args.list.split(",")][:5]
                set_dpi_stages(spec, dev, [(v, v) for v in vals], active=args.active)
            active, stages = get_dpi_stages(spec, dev)
            print(f"stages: {', '.join('%d' % x for x, _ in stages)} (active: #{active})")
        elif cmd == "idle":
            if args.seconds:
                set_idle(spec, dev, max(60, min(900, args.seconds)))
            print(f"idle time: {get_idle(spec, dev)} s")
    finally:
        dev.close()


if __name__ == "__main__":
    main()
