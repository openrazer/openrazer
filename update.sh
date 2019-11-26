#!/bin/bash
set -e
pushd /mnt/home/madrat/Documents/programming/github/openrazer-beranat/
rmmod razermouse || true
make driver_install
sign-module /lib/modules/$(uname -r)/extra/razer*.ko
modprobe razermouse
popd
