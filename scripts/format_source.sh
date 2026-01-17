#!/bin/bash

set -e

missing=0

if ! command -v astyle >/dev/null 2>&1; then
	echo "[!] Missing dependency: astyle" >&2
	echo "    Debian/Ubuntu: sudo apt-get install -y astyle" >&2
	echo "    Fedora/RHEL:   sudo dnf install -y astyle" >&2
	echo "    Arch:          sudo pacman -S astyle" >&2
	missing=1
fi

if ! command -v autopep8 >/dev/null 2>&1; then
	echo "[!] Missing dependency: autopep8" >&2
	echo "    Debian/Ubuntu: sudo apt-get install -y python3-autopep8" >&2
	echo "    Fedora/RHEL:   sudo dnf install -y python3-autopep8" >&2
	echo "    Arch:          sudo pacman -S python-autopep8" >&2
	missing=1
fi

if [ "$missing" -ne 0 ]; then
	exit 1
fi

set -x

# C (astyle)
find driver/ \( -name "*.c" -o -name "*.h" \) -print0 | xargs -0 astyle --style=linux -n

# Python (autopep8)
autopep8 --jobs 0 --recursive --in-place --max-line-length 500 --ignore E402 .
