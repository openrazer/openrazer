#!/usr/bin/env bash
# Safe OpenRazer installer with automatic rollback.
#
# When openrazer is installed via the package manager it uses DKMS, which
# places modules in updates/dkms/ — a directory with HIGHER priority than
# kernel/drivers/hid/.  This script updates the DKMS source tree and rebuilds
# through DKMS so that modprobe always loads the correct module.
#
# On failure the previous DKMS-installed modules are restored.
# Also installs the udev rules and the daemon keyboards.py.

set -euo pipefail

MODULES=(razerkbd razermouse razerkraken razeraccessory)
BACKUP_DIR="/tmp/openrazer-module-backup-$(date +%Y%m%d-%H%M%S)"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Resolve DKMS name/version from the top-level Makefile
DKMS_NAME=$(grep -m1 '^DKMS_NAME' "${SCRIPT_DIR}/Makefile" | awk -F= '{print $2}' | tr -d ' ')
DKMS_VER=$(grep -m1 '^DKMS_VER'  "${SCRIPT_DIR}/Makefile" | awk -F= '{print $2}' | tr -d ' ')
DKMS_SRC="/usr/src/${DKMS_NAME}-${DKMS_VER}"

# DKMS module directory (higher priority than kernel/drivers/hid)
DKMS_MODDIR="/lib/modules/$(uname -r)/updates/dkms"

# Daemon keyboards.py: repo source vs installed destination
KEYBOARDS_SRC="${SCRIPT_DIR}/daemon/openrazer_daemon/hardware/keyboards.py"
KEYBOARDS_DST="$(python3 -c "
import sys, os
for p in sys.path:
    candidate = os.path.join(p, 'openrazer_daemon/hardware/keyboards.py')
    if os.path.isfile(candidate):
        print(candidate)
        break
" 2>/dev/null)"

if [[ $EUID -ne 0 ]]; then
    echo "Error: this script must be run as root (use sudo)." >&2
    exit 1
fi

if [[ ! -d "${DKMS_SRC}" ]]; then
    echo "Error: DKMS source directory not found: ${DKMS_SRC}" >&2
    echo "       Is openrazer installed via the package manager?" >&2
    exit 1
fi

echo ":: DKMS source: ${DKMS_SRC}"
echo ":: DKMS module: ${DKMS_NAME}/${DKMS_VER}"

# ── 1. Backup existing DKMS modules ─────────────────────────────────────────
echo ":: Backing up existing modules to ${BACKUP_DIR}"
mkdir -p "${BACKUP_DIR}"
for mod in "${MODULES[@]}"; do
    for suffix in "" ".xz" ".zst" ".gz"; do
        f="${DKMS_MODDIR}/${mod}.ko${suffix}"
        if [[ -f "$f" ]]; then
            cp -v "$f" "${BACKUP_DIR}/"
            break
        fi
    done
done

# ── 2. Record which modules are currently loaded ─────────────────────────────
loaded_before=()
for mod in "${MODULES[@]}"; do
    lsmod | grep -q "^${mod} " && loaded_before+=("${mod}") || true
done

# ── 3. Stop daemon, unbind devices, and unload current modules ───────────────
echo ":: Stopping openrazer-daemon (if running)"
systemctl stop openrazer-daemon 2>/dev/null || true

echo ":: Unbinding HID devices from razer modules"
for mod in "${MODULES[@]}"; do
    driver_path="/sys/bus/hid/drivers/${mod}"
    [[ -d "${driver_path}" ]] || continue
    for dev in "${driver_path}"/????:????:????.????; do
        [[ -e "${dev}" ]] || continue
        devid=$(basename "${dev}")
        echo "  unbinding ${devid} from ${mod}"
        echo "${devid}" > "${driver_path}/unbind" 2>/dev/null || true
    done
done

echo ":: Unloading current modules"
for mod in "${loaded_before[@]}"; do
    if ! modprobe -r "${mod}"; then
        echo "Error: failed to unload ${mod} — cannot replace in-memory module." >&2
        exit 1
    fi
done

# ── Helper: restore backup and reload ───────────────────────────────────────
rollback() {
    echo ":: ROLLBACK – restoring backed-up modules" >&2
    for f in "${BACKUP_DIR}"/*.ko*; do
        [[ -e "$f" ]] && cp -v "$f" "${DKMS_MODDIR}/"
    done
    depmod
    for mod in "${loaded_before[@]}"; do
        modprobe "${mod}" 2>/dev/null && echo "  reloaded ${mod}" || true
    done
    systemctl start openrazer-daemon 2>/dev/null || true
    echo ":: Rollback complete. Backup kept at ${BACKUP_DIR}" >&2
}

# ── 4. Copy updated source into DKMS tree ───────────────────────────────────
echo ":: Updating DKMS source tree"
cp -v "${SCRIPT_DIR}/driver/razerkbd_driver.c"  "${DKMS_SRC}/driver/"
cp -v "${SCRIPT_DIR}/driver/razerkbd_driver.h"  "${DKMS_SRC}/driver/"
cp -v "${SCRIPT_DIR}/driver/razermouse_driver.c" "${DKMS_SRC}/driver/" 2>/dev/null || true
cp -v "${SCRIPT_DIR}/driver/razeraccessory_driver.c" "${DKMS_SRC}/driver/" 2>/dev/null || true
cp -v "${SCRIPT_DIR}/driver/razercommon.c"       "${DKMS_SRC}/driver/" 2>/dev/null || true
cp -v "${SCRIPT_DIR}/driver/razerchromacommon.c" "${DKMS_SRC}/driver/" 2>/dev/null || true

# ── 5. Build via DKMS ────────────────────────────────────────────────────────
echo ":: Building via DKMS"
if ! dkms build "${DKMS_NAME}/${DKMS_VER}" --force; then
    echo "Error: dkms build failed." >&2
    rollback
    exit 1
fi

# ── 6. Install via DKMS ──────────────────────────────────────────────────────
echo ":: Installing via DKMS"
if ! dkms install "${DKMS_NAME}/${DKMS_VER}" --force; then
    echo "Error: dkms install failed." >&2
    rollback
    exit 1
fi

# ── 7. Load and verify ───────────────────────────────────────────────────────
echo ":: Loading razerkbd"
if ! modprobe razerkbd; then
    echo "Error: modprobe razerkbd failed." >&2
    rollback
    exit 1
fi

# Verify the RUNNING module matches the newly installed DKMS module by
# comparing srcversions.  modinfo reads from disk; /sys/module reads from
# the in-memory module — they must match or the old module is still loaded.
DKMS_KO=$(ls "${DKMS_MODDIR}/razerkbd.ko"* 2>/dev/null | head -1)
DISK_SRCVER=$(modinfo "${DKMS_KO}" 2>/dev/null | awk '/^srcversion:/{print $2}')
RUN_SRCVER=$(cat /sys/module/razerkbd/srcversion 2>/dev/null || echo "")
if [[ -z "${RUN_SRCVER}" || "${DISK_SRCVER}" != "${RUN_SRCVER}" ]]; then
    echo "Error: running razerkbd (srcversion ${RUN_SRCVER}) does not match" >&2
    echo "       DKMS module (srcversion ${DISK_SRCVER})." >&2
    echo "       The stale in-memory module could not be replaced." >&2
    rollback
    exit 1
fi
echo ":: razerkbd module verified (srcversion: ${RUN_SRCVER})"

for mod in "${loaded_before[@]}"; do
    [[ "${mod}" == "razerkbd" ]] && continue
    modprobe "${mod}" 2>/dev/null || echo "Warning: could not reload ${mod}"
done

# ── 8. Install udev rules ────────────────────────────────────────────────────
echo ":: Installing udev rules"
make -C "${SCRIPT_DIR}" udev_install
udevadm control --reload-rules
# Trigger 'add' on HID subsystem so razer_mount is called for already-connected
# devices.  The udev rule skips everything except ACTION==add.
udevadm trigger --action=add --subsystem-match=hid

# ── 9. Update installed daemon keyboards.py ──────────────────────────────────
if [[ -n "${KEYBOARDS_DST}" ]]; then
    echo ":: Updating daemon keyboards.py → ${KEYBOARDS_DST}"
    cp -v "${KEYBOARDS_SRC}" "${KEYBOARDS_DST}"
else
    echo "Warning: could not locate installed openrazer_daemon keyboards.py – skipping."
fi

# ── 10. Restart daemon ───────────────────────────────────────────────────────
echo ":: Starting openrazer-daemon"
REAL_USER="${SUDO_USER:-}"
if [[ -n "${REAL_USER}" ]]; then
    sudo -u "${REAL_USER}" systemctl --user restart openrazer-daemon 2>/dev/null || true
else
    systemctl restart openrazer-daemon 2>/dev/null || true
fi

echo ""
echo ":: Success. Backup kept at ${BACKUP_DIR}"
echo "   To clean it up: rm -rf ${BACKUP_DIR}"
