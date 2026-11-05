#!/usr/bin/python3
# SPDX-License-Identifier: GPL-2.0-or-later

"""Initialize Basilisk Mobile driver mode once, retrying on actual mouse input."""
import errno
import os
from pathlib import Path
import re
import select
import struct
import sys
import time

EVENT = struct.Struct('@llHHi')
HID_NAME = re.compile(r'0003:1532:00D[34]\.[0-9A-Fa-f]+')


def target(name):
    if HID_NAME.fullmatch(name) is None:
        raise ValueError('Unexpected mouse HID identifier')
    hid = Path('/sys/bus/hid/drivers/razermouse') / name
    if not hid.exists():
        return None
    if (hid.resolve().parent / 'bInterfaceNumber').read_text().strip() != '00':
        raise ValueError('Only the primary mouse interface may initialize mode')
    return hid


def attempt(hid):
    """Always write, even if already in mode 3, to establish kernel intent."""
    try:
        mode = hid / 'device_mode'
        if mode.write_bytes(b'\x03\x00') != 2:
            return False
        return mode.read_bytes() == b'\x03\x00'
    except OSError:
        return False


def burst(hid):
    for retry in range(3):
        if not hid.exists():
            return False
        if attempt(hid):
            print('Basilisk Mobile driver mode initialized and verified.', flush=True)
            return True
        if retry != 2:
            time.sleep(1)
    return False


def open_inputs(hid):
    fds = []
    for node in hid.glob('input/input*/event*'):
        try:
            fds.append(os.open('/dev/input/' + node.name, os.O_RDONLY | os.O_NONBLOCK | os.O_CLOEXEC))
        except OSError:
            pass
    return fds


def activity(data):
    # Interpret only event types/codes; never retain or log input data.
    for offset in range(0, len(data) - EVENT.size + 1, EVENT.size):
        _, _, kind, code, value = EVENT.unpack_from(data, offset)
        if kind == 2 and value != 0:
            return True
        if kind == 1 and 0x110 <= code <= 0x11f and value in (0, 1):
            return True
    return False


def initialize(hid):
    if burst(hid):
        return 0
    print('Mouse not ready; waiting for mouse activity before retrying.', flush=True)
    fds = []
    try:
        while hid.exists():
            if not fds:
                fds = open_inputs(hid)
                if not fds:
                    time.sleep(1)
                    continue
            ready, _, _ = select.select(fds, [], [], 1)
            for fd in ready:
                try:
                    data = os.read(fd, EVENT.size * 64)
                except OSError as error:
                    if error.errno in (errno.ENODEV, errno.ENXIO):
                        return 0
                    if error.errno == errno.EAGAIN:
                        continue
                    raise
                if not data:
                    return 0
                if activity(data) and burst(hid):
                    return 0
        return 0
    finally:
        for fd in fds:
            os.close(fd)


def main():
    if len(sys.argv) != 2 or os.geteuid() != 0:
        raise SystemExit('Run as root with one mouse HID identifier')
    hid = target(sys.argv[1])
    if hid is None:
        return 0
    # Input nodes appear after the driver's HID context has been installed.
    while hid.exists() and not any(hid.glob('input/input*/event*')):
        time.sleep(0.1)
    return initialize(hid) if hid.exists() else 0


if __name__ == '__main__':
    raise SystemExit(main())
