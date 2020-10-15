#!/bin/bash -ex
# -e = exit on error
# -x = print all commands

dir=$(dirname $0)

# Update the package index
./$dir/apt-update.sh

# Install dependencies
./$dir/install-check-deps.sh
./$dir/install-driver-deps.sh
./$dir/install-daemon-deps.sh

# Check the formatting of the driver
./$dir/check-astyle-formatting.sh
./$dir/check-autopep8-formatting.sh
# Check with pylint for errors
./$dir/check-pylint.sh

# Check for duplicate fake driver serials
./$dir/test-duplicate-fake-driver-serials.sh

# Check for hex casing issues
./$dir/test-hex-casing.sh

# Check for auto-generated files
./$dir/test-auto-generate.sh

# Launch dbus
eval $(dbus-launch --sh-syntax)

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
