#!/bin/bash -e

./scripts/generate_appstream_file.sh
if [ -n "$(git diff)" ]; then
    echo
    echo "ERROR: Appstream file needs to be regenerated!"
    echo
    git diff
    echo
    echo "Please run './scripts/generate_appstream_file.sh'"
    echo
    exit 1
fi

./scripts/generate_all_fake_drivers.sh -f
git add --intent-to-add ./pylib/openrazer/_fake_driver/
if [ -n "$(git diff)" ]; then
    echo
    echo "ERROR: Fake driver files needs to be regenerated!"
    echo
    git diff
    echo
    echo "Please run './scripts/generate_all_fake_drivers.sh -f'"
    echo "Note that this will delete all existing fake driver files."
    echo
    exit 1
fi
