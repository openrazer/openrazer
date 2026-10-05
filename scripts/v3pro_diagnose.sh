#!/usr/bin/env bash
# V3 Pro diagnostic — figures out why charge_level returns -1.
# Usage:
#   curl -sL <url> | bash
# or:
#   bash v3pro_diagnose.sh

set -u

echo "=== 0. Pulling latest from fork ==="
if [ -d ~/openrazer-blackshark/.git ]; then
    cd ~/openrazer-blackshark
    git fetch fork blackshark-v3-pro 2>&1 || git fetch origin blackshark-v3-pro 2>&1 || true
    BEFORE=$(git rev-parse HEAD)
    git pull --ff-only 2>&1 | tail -3
    AFTER=$(git rev-parse HEAD)
    if [ "$BEFORE" != "$AFTER" ]; then
        echo "  pulled new commits — rebuilding driver via DKMS..."
        sudo make setup_dkms 2>&1 | tail -3
        sudo dkms install openrazer-driver/3.12.1 --force 2>&1 | tail -5
        sudo rmmod razerkraken 2>/dev/null || true
        sudo modprobe razerkraken
        sleep 1
    else
        echo "  already up to date"
    fi

    # Manually rebind any Razer HID device that's currently in /sys/bus/hid/devices
    # but missing from /sys/bus/hid/drivers/razerkraken/. Replicates a hot-replug
    # so the new module attaches without the user touching the dongle.
    echo "  rebinding any Razer HID devices..."
    for hid_dev in /sys/bus/hid/devices/0003:1532:*; do
        [ -e "$hid_dev" ] || continue
        hid_name=$(basename "$hid_dev")
        # if already bound to razerkraken, skip
        if [ -e "/sys/bus/hid/drivers/razerkraken/$hid_name" ]; then
            continue
        fi
        # unbind from whatever driver currently owns it (if any)
        cur_drv=$(readlink "$hid_dev/driver" 2>/dev/null)
        if [ -n "$cur_drv" ]; then
            echo "$hid_name" | sudo tee "$hid_dev/driver/unbind" >/dev/null 2>&1 || true
        fi
        # bind to razerkraken
        echo "$hid_name" | sudo tee /sys/bus/hid/drivers/razerkraken/bind >/dev/null 2>&1 \
            && echo "    rebound $hid_name to razerkraken" \
            || echo "    razerkraken refused $hid_name (probably not a razerkraken-supported PID)"
    done
    sleep 1
else
    echo "  ~/openrazer-blackshark not found — clone it first:"
    echo "    cd ~ && git clone https://github.com/mehmetbayoglu/openrazer.git openrazer-blackshark"
    echo "    cd openrazer-blackshark && git checkout blackshark-v3-pro"
    exit 1
fi

echo ""
echo "=== 1. Razer device on USB bus ==="
lsusb | grep -i razer || echo "  no Razer device found via lsusb"

echo ""
echo "=== 2. Loaded razer kernel modules ==="
lsmod | grep -i raze || echo "  no razer modules loaded"

echo ""
echo "=== 3. razerkraken module info ==="
modinfo razerkraken 2>&1 | grep -E "filename|srcversion|version" | head -5

echo ""
echo "=== 4. Bound device path ==="
DEV=$(ls /sys/bus/hid/drivers/razerkraken/ 2>/dev/null | grep -iE '057[67]' | head -1)
if [ -z "$DEV" ]; then
    echo "  no V3 Pro bound to razerkraken (not connected, or driver not attached)"
    echo "  contents of /sys/bus/hid/drivers/razerkraken/:"
    ls /sys/bus/hid/drivers/razerkraken/ 2>/dev/null
    exit 1
fi
echo "  $DEV"
SYSFS="/sys/bus/hid/drivers/razerkraken/$DEV"

echo ""
echo "=== 5. Permissions on charge_level ==="
ls -la "$SYSFS/charge_level"

echo ""
echo "=== 6. Read with regular user ==="
RESULT_USER=$(cat "$SYSFS/charge_level" 2>&1)
echo "  result: $RESULT_USER"

echo ""
echo "=== 7. Read with sudo ==="
RESULT_ROOT=$(sudo cat "$SYSFS/charge_level" 2>&1)
echo "  result: $RESULT_ROOT"

echo ""
echo "=== 8. udev rules files ==="
for f in /etc/udev/rules.d/*razer* /usr/lib/udev/rules.d/*razer* /run/udev/rules.d/*razer*; do
    [ -f "$f" ] || continue
    echo "--- $f ---"
    cat "$f"
    echo ""
done

echo ""
echo "=== 9. Recent kernel messages from razerkraken ==="
sudo dmesg | grep -i razer | tail -20

echo ""
echo "=== 10. git status of the fork ==="
if [ -d ~/openrazer-blackshark/.git ]; then
    cd ~/openrazer-blackshark
    git log -3 --oneline
    echo "  branch: $(git branch --show-current)"
fi

echo ""
echo "=== DIAGNOSIS ==="
if [ "$RESULT_USER" = "-1" ] && [ "$RESULT_ROOT" = "-1" ]; then
    echo "  Both reads return -1 → driver code issue, NOT permissions"
    echo "  Likely causes:"
    echo "    - V3 Pro headset is OFF (turn it on, dongle queries headset over 2.4GHz)"
    echo "    - Stale source in /usr/src — re-pull and re-install:"
    echo "        cd ~/openrazer-blackshark && git pull && sudo make setup_dkms"
    echo "        sudo dkms install openrazer-driver/3.12.1 --force"
    echo "        sudo rmmod razerkraken && sudo modprobe razerkraken"
elif [ "$RESULT_USER" = "-1" ] && [ "$RESULT_ROOT" != "-1" ]; then
    echo "  User read fails, sudo works → udev permission issue"
    echo "  Check the udev rules files above for conflicting ownership/perms"
    echo "  Try:  sudo udevadm control --reload-rules && sudo udevadm trigger"
elif [ "$RESULT_USER" != "-1" ]; then
    echo "  charge_level returns $RESULT_USER (real battery × 2.55 — divide by 2.55 for %)"
    echo "  Driver is working. Run the capture script next:"
    echo "    ~/Documents/v3-research-2026-05-03/v3pro_linux_capture.sh"
fi
