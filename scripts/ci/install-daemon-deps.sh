#!/bin/bash

# Ubuntus python3-daemonize version is outdated. Grab and install it manually.
curl -L -O https://launchpad.net/~openrazer/+archive/ubuntu/daily/+files/python3-daemonize_2.4.7-xenial_all.deb
dpkg -i python3-daemonize_2.4.7-xenial_all.deb
curl -L -O https://launchpad.net/~z3ntu/+archive/ubuntu/experiments/+files/python3-evdev_0.7.0-4_amd64.deb
dpkg -i python3-evdev_0.7.0-4_amd64.deb
apt-get -f -y install

apt-get -y install \
    dbus-x11 \
    gir1.2-gtk-3.0 \
    python3 \
    python3-daemonize \
    python3-dbus \
    python3-gi \
    python3-notify2 \
    python3-numpy \
    python3-pyudev \
    python3-setproctitle \
    xautomation \
    xdotool
