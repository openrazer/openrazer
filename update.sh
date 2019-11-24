#!/bin/bash
set -e
rmmod razermouse || true
make driver_install
sign-module /lib/modules/$(uname -r)/extra/razer*.ko
modprobe razermouse
