#!/bin/bash -ex

all_devices=$(ls pylib/openrazer/_fake_driver/*.cfg | wc -l)

daemon_devices=$(PYTHONPATH="pylib:daemon" python3 -c "from openrazer.client import DeviceManager; mgr = DeviceManager(); print(len(mgr.devices))")

child_device_count=$(PYTHONPATH="pylib:daemon" python3 -c "
from openrazer_daemon.hardware import get_device_classes
print(sum(1 for c in get_device_classes() if 'get_child_devices' in c.__dict__))
")

expected_devices=$((all_devices + child_device_count))

if [ "$expected_devices" != "$daemon_devices" ]; then
    echo
    echo "Unexpected discrepancy between devices in fake driver and devices from daemon:"
    echo "$expected_devices (cfg: $all_devices + children: $child_device_count) != $daemon_devices"
    echo
    exit 1
fi

PYTHONPATH="pylib:daemon" python3 scripts/ci/test-daemon.py
PYTHONPATH="pylib:daemon" python3 scripts/ci/test-pids.py
