#!/bin/bash

# Make deployment directory
directory=$(mktemp -d)

# Copy DEBIAN directort
mkdir -p ${directory}
cp -r install_files/DEBIAN ${directory}/DEBIAN



# Create file structure
mkdir -p ${directory}/etc/{init,udev/rules.d,dbus-1/system.d}
mkdir -p ${directory}/usr/{bin,lib,sbin,share/razer_bcd/fx,src/razer_chroma_driver-1.0.0/driver}


# Copy over upstart script
cp install_files/init/razer_bcd.conf ${directory}/etc/init/razer_bcd.conf

# Copy over udev rule
cp install_files/udev/95-razerkbd.rules ${directory}/etc/udev/rules.d/95-razerkbd.rules

# Copy over dbus rule
cp install_files/dbus/org.voyagerproject.razer.daemon.conf ${directory}/etc/dbus-1/system.d/org.voyagerproject.razer.daemon.conf

# Copy over bash helper
cp install_files/share/bash_keyboard_functions.sh ${directory}/usr/share/razer_bcd/bash_keyboard_functions.sh

# Copy over libchroma and daemon
cp daemon/librazer_chroma.so ${directory}/usr/lib/librazer_chroma.so
cp daemon/razer_bcd ${directory}/usr/sbin/razer_bcd
cp daemon/fx/pez2001_collection.so ${directory}/usr/share/razer_bcd/fx
cp daemon/fx/pez2001_mixer.so ${directory}/usr/share/razer_bcd/fx
cp daemon/fx/pez2001_light_blast.so ${directory}/usr/share/razer_bcd/fx
cp daemon/fx/pez2001_progress_bar.so ${directory}/usr/share/razer_bcd/fx

# Copy daemon controller
cp daemon_controller/razer_bcd_controller ${directory}/usr/bin/razer_bcd_controller

# Copy razer kernel driver to src
cp Makefile ${directory}/usr/src/razer_chroma_driver-1.0.0/Makefile
cp install_files/dkms/dkms.conf ${directory}/usr/src/razer_chroma_driver-1.0.0/dkms.conf
cp driver/{Makefile,razerkbd.c,razerkbd.h} ${directory}/usr/src/razer_chroma_driver-1.0.0/driver

rm $TMPDIR/razer-chroma*.deb
dpkg-deb --build ${directory}
dpkg-name ${directory}.deb

rm -rf ${directory}








