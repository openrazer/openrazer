#!/usr/bin/python3
"""
Checks other files in the repository are consistent.
- Ensure PID is documented in README for end users.
- Ensure PID is added for udev rules so permissions work.
"""
import os
import openrazer.client

repo_root = os.path.realpath(os.path.join(os.path.dirname(__file__), "..", ".."))
devmgr = openrazer.client.DeviceManager()
passed = True

# Files containing PIDs
with open(f"{repo_root}/README.md") as f:
    README = f.readlines()

with open(f"{repo_root}/install_files/udev/99-razer.rules") as f:
    UDEV = f.readlines()

for device in devmgr.devices:
    pid_upper = str(hex(device._pid))[2:].upper().rjust(4, '0')
    pid_lower = pid_upper.lower()
    found_readme = False
    found_udev = False

    for line in README:
        if pid_upper != pid_lower and line.find(pid_lower) > 0:
            print(f"Warning: Found lowercase pid {pid_lower} in README.md, should be uppercase!")
        if line.find(pid_upper) > 0:
            found_readme = True
            break

    for line in UDEV:
        if pid_upper != pid_lower and line.find(pid_upper) > 0:
            print(f"Warning: Found uppercase pid {pid_upper} in 99-razer.rules, should be lowercase!")
        if line.find(pid_lower) > 0:
            found_udev = True
            break

    if not found_readme:
        print(f"\nMissing from README: {device.name} ({pid_upper})")
        passed = False

    if not found_udev:
        print(f"\nMissing from 99-razer.rules: {device.name} ({pid_lower})")
        passed = False

if not passed:
    exit(1)
