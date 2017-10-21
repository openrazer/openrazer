#!/bin/bash

# -x = print all commands
# -e = exit on error
set -x -e

dir=$(dirname $0)

# Update the package index
./$dir/apt-update.sh

# Install dependencies
./$dir/install-astyle.sh
./$dir/install-driver-deps.sh
./$dir/install-daemon-deps.sh

# Check the formatting of the driver
./$dir/check-formatting.sh

# Launch dbus
eval `dbus-launch --sh-syntax`

# Compile the kernel driver
./$dir/compile-driver.sh

# Setup the fake driver
./$dir/setup-fakedriver.sh

# Launch the daemon
./$dir/launch-daemon.sh

# Wait for the daemon to be ready
sleep 10

# Run a simple check to see if the daemon is alive
./$dir/test-daemon.sh
