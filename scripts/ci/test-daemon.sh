#!/bin/bash -ex

PYTHONPATH="pylib:daemon" python3 -c "from openrazer.client import DeviceManager; a = DeviceManager(); print(a.devices)"
