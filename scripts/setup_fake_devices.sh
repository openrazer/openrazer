#!/bin/bash
#
# A simple script to choose what devices
# you want to fake (for debugging)
#

whereami=$(dirname "$0")

device_cfg_files=$(ls "$whereami/../pylib/razer/_fake_driver/"*.cfg)
config_dir="/tmp/daemon_config/"
test_dir="/tmp/daemon_test"

# Check if x-terminal-emulatr exists (only on Debian & derivatives)
command -v x-terminal-emulator >/dev/null 2>&1
if [ $? == 0 ]; then
    terminal_cmd="x-terminal-emulator -e"
else
    command -v xterm >/dev/null 2>&1
    if [ $? != 0 ]; then
        text="Neither x-terminal-emulator nor xterm was found in \$PATH. Exiting."
        echo $text
        zenity --error --text="$text"
        exit 1
    fi
    terminal_cmd="xterm -e"
fi

devices=""
for cfg in $device_cfg_files; do
    cfg_file=$(basename "$cfg")
    devices="$devices 0 ${cfg_file%.*}"
done

options=$(zenity --list --multiple --checklist \
        --print-column=2 --separator=" " \
        --height=250 --width=400 \
        --title="Create Fake Razer Device" \
        --text="Select the devices to simulate." \
        --column "" --column "Device" \
         $devices)

# Quit if cancelled
if [ ! $? == 0 ]; then
    exit 1
fi

# Prepare fake devices.
mkdir $config_dir/{,data,logs}
mkdir $test_dir
$terminal_cmd "$whereami/create_fake_device.py" --dest "$test_dir" $options &

# Kill razer-service if it is running already.
pkill -e razer-service

# Start the daemon in a new terminal window.
$terminal_cmd razer-service --verbose -F --run-dir "$config_dir/data" --log-dir "$config_dir/logs" --pid-file "$config_dir/daemon.pid" --test-dir "$test_dir"
