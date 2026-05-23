#!/usr/bin/env bash
# Dev-install OpenRazer daemon + pylib from source, without touching /usr/bin.
# Also installs kernel modules and udev support so everything works after reboot.
# Run with sudo: sudo bash scripts/dev_install.sh

set -euo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
PYTHON=python3.14
SITE=$($PYTHON -c "import site; print(site.getsitepackages()[0])")

echo "==> Repo:    $REPO"
echo "==> Site:    $SITE"

# 1. System dependencies
echo "==> Installing Python dependencies..."
apt-get install -y python3-daemonize python3-pyudev python3-notify2 python3-xlib

# 2. Source paths via .pth so python3.14 finds openrazer_daemon and openrazer.client
echo "==> Adding .pth files to $SITE..."
echo "$REPO/daemon"  > "$SITE/openrazer-daemon-dev.pth"
echo "$REPO/pylib"   > "$SITE/openrazer-pylib-dev.pth"

# 3. Kernel modules → /lib/modules so modprobe and udev work
KVER=$(uname -r)
HID_DIR="/lib/modules/$KVER/kernel/drivers/hid"
echo "==> Installing kernel modules to $HID_DIR..."
install -m 644 -D "$REPO/driver/razerlaptop.ko" "$HID_DIR/razerlaptop.ko"
install -m 644 -D "$REPO/driver/razerkbd.ko"    "$HID_DIR/razerkbd.ko"
install -m 644 -D "$REPO/driver/razermouse.ko"  "$HID_DIR/razermouse.ko"
install -m 644 -D "$REPO/driver/razerkraken.ko" "$HID_DIR/razerkraken.ko"
install -m 644    "$REPO/driver/razeraccessory.ko" "$HID_DIR/razeraccessory.ko"
depmod -a

# 4. udev rules + razer_mount helper
echo "==> Installing udev rules and razer_mount..."
install -m 644 "$REPO/install_files/udev/99-razer.rules" /etc/udev/rules.d/99-razer.rules
install -m 755 "$REPO/install_files/udev/razer_mount"    /lib/udev/razer_mount
udevadm control --reload-rules
udevadm trigger --action=add --subsystem-match=hid

# 5. razer.conf example (daemon hardcodes this path)
echo "==> Installing razer.conf.example..."
install -m 644 -D "$REPO/daemon/resources/razer.conf" /usr/share/openrazer/razer.conf.example

# 6. D-Bus session service (enables dbus-daemon to auto-activate org.razer)
echo "==> Installing D-Bus service file..."
install -m 644 -D /dev/stdin /usr/share/dbus-1/services/org.razer.service <<EOF
[D-BUS Service]
Name=org.razer
Exec=$REPO/daemon/run_openrazer_daemon.py -F
SystemdService=openrazer-daemon.service
EOF

# 7. Systemd user service
UNIT_DIR="/usr/lib/systemd/user"
echo "==> Installing systemd user service to $UNIT_DIR..."
install -m 644 -D /dev/stdin "$UNIT_DIR/openrazer-daemon.service" <<EOF
[Unit]
Description=OpenRazer daemon (dev, source at $REPO)
Documentation=man:openrazer-daemon(8)

[Service]
Type=dbus
BusName=org.razer
ExecStart=$REPO/daemon/run_openrazer_daemon.py -F

[Install]
WantedBy=default.target
EOF

echo "==> Reloading systemd user daemon..."
# Run as the actual user (sudo loses \$USER but keeps SUDO_USER)
REAL_USER="${SUDO_USER:-$USER}"
sudo -u "$REAL_USER" systemctl --user daemon-reload

# 8. Ensure user is in plugdev and input groups
echo "==> Checking group membership for $REAL_USER..."
for GRP in plugdev input; do
    if ! id -nG "$REAL_USER" | grep -qw "$GRP"; then
        echo "    Adding $REAL_USER to $GRP (re-login required)"
        usermod -aG "$GRP" "$REAL_USER"
    else
        echo "    $REAL_USER already in $GRP"
    fi
done

echo ""
echo "Done. To start the daemon:"
echo "  systemctl --user enable --now openrazer-daemon"
echo "  journalctl --user -u openrazer-daemon -f"
echo ""
echo "NOTE: If you were just added to plugdev or input, log out and back in first."
