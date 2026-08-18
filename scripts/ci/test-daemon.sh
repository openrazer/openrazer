#!/bin/bash -ex

all_devices=$(ls pylib/openrazer/_fake_driver/*.cfg | wc -l)

daemon_devices=$(PYTHONPATH="pylib:daemon" python3 -c "from openrazer.client import DeviceManager; mgr = DeviceManager(); print(len(mgr.devices))")

child_device_count=$(PYTHONPATH="pylib:daemon" python3 -c "
from openrazer_daemon.hardware.device_base import RazerDevice as _RazerDevice
import inspect, importlib, pkgutil
import openrazer_daemon.hardware as hw_pkg

count = 0
for _importer, modname, _ispkg in pkgutil.walk_packages(hw_pkg.__path__, hw_pkg.__name__ + '.'):
    mod = importlib.import_module(modname)
    for _name, obj in inspect.getmembers(mod, inspect.isclass):
        if issubclass(obj, _RazerDevice) and obj is not _RazerDevice:
            if 'get_child_devices' in obj.__dict__:
                count += 1
print(count)
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
