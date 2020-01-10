#!/bin/bash

# Checks for correct casing of hex values in the source

out=$(grep -rh "USB_PID = " daemon/openrazer_daemon/hardware/ | cut -d '=' -f2 | grep -v None | sort -f | grep -E '[a-f]')

if [ ! -z "$out" ]; then
  echo "ERROR: wrong casing of hex number!"
  echo "Please convert the following hex numbers in daemon/openrazer_daemon/hardware/ to uppercase:"
  echo "$out"
  exit 1
fi

echo "No issues found."
