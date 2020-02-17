#!/bin/bash -e

# Checks for duplicate serials in the fake driver cfg files

searchfolder=./pylib/openrazer/_fake_driver

# colors
GREEN='\033[0;32m'
NC='\033[0m' # No Color

serials=$(grep -rh "X000000" $searchfolder | cut -d',' -f3)

duplicates=$(echo "$serials" | sort | uniq -d)

if [ ! -z "$duplicates" ]; then
  echo "Duplicate serial numbers found:"
  while read -r line; do
    echo "================================="
    echo -e "Duplicate: ${GREEN}$line${NC} in files:"
    grep -rl "$line" $searchfolder
  done <<< "$duplicates"
  echo "If you need a new fake serial number, you can use ./scripts/get_next_fake_driver_serial.sh"
  exit 1
fi

echo "No duplicates found."
