#!/bin/bash

set -e

if ! command -v pylint >/dev/null 2>&1; then
	echo "[!] pylint not installed. Unable to run pylint checks." >&2
	echo "    Debian/Ubuntu: sudo apt-get install -y pylint python3-pylint" >&2
	echo "    Fedora/RHEL:   sudo dnf install -y pylint python3-pylint" >&2
	echo "    Arch:          sudo pacman -S python-pylint" >&2
	echo "    pip (user):    python3 -m pip install --user pylint" >&2
	exit 1
fi

find . -name "*.py" \
	-not -path "./test/*" \
	-not -path "./.git/*" \
	-not -path "./.venv/*" \
	-not -path "./venv/*" \
	-not -path "./build/*" \
	-not -path "./dist/*" \
	-not -path "./*/__pycache__/*" \
	-print0 | xargs -0 pylint --errors-only
