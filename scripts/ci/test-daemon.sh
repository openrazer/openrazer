#!/bin/bash -ex

all_devices=$(ls pylib/openrazer/_fake_driver/*.cfg | wc -l)

daemon_devices=$(PYTHONPATH="pylib:daemon" python3 -c "from openrazer.client import DeviceManager; mgr = DeviceManager(); print(len(mgr.devices))")

if [ "$all_devices" != "$daemon_devices" ]; then
    echo
    echo "Unexpected discrepancy between devices in fake driver and devices from daemon:"
    echo "$all_devices != $daemon_devices"
    echo
    exit 1
fi

PYTHONPATH="pylib:daemon" python3 scripts/ci/test-daemon.py
