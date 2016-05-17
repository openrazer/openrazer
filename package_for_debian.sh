#!/bin/bash

# Make deployment directory
directory=$(mktemp -d)

# Copy DEBIAN directory
mkdir -p ${directory}
cp -r install_files/DEBIAN_debian ${directory}/DEBIAN
chmod 755 ${directory}/DEBIAN
chmod 755 ${directory}/DEBIAN/{pre,post}*


# Create file structure
mkdir -p ${directory}/etc/{init.d,udev/rules.d,dbus-1/system.d,xdg/autostart}
mkdir -p ${directory}/usr/{bin,lib/python3/dist-packages/,sbin,share/razer_bcd/fx,src/razer_chroma_driver-1.0.0/driver}
mkdir -p ${directory}/lib/systemd/system


# Copy over startup script
cp install_files/systemd/razer_bcd.service ${directory}/lib/systemd/system/razer_bcd.service
cp install_files/init.d/razer_bcd_debian ${directory}/etc/init.d/razer_bcd

# Copy over udev rule
cp install_files/udev/95-razerkbd.rules ${directory}/etc/udev/rules.d/95-razerkbd.rules

# Copy over dbus rule
cp install_files/dbus/org.voyagerproject.razer.daemon.conf ${directory}/etc/dbus-1/system.d/org.voyagerproject.razer.daemon.conf

# Copy over bash helper
cp install_files/share/bash_keyboard_functions.sh ${directory}/usr/share/razer_bcd/bash_keyboard_functions.sh
cp install_files/share/systemd_helpers.sh ${directory}/usr/share/razer_bcd/systemd_helpers.sh

# Copy over libchroma and daemon
cp lib/librazer_chroma.so ${directory}/usr/lib/librazer_chroma.so
cp lib/librazer_chroma_controller.so ${directory}/usr/lib/librazer_chroma_controller.so
cp daemon/razer_bcd ${directory}/usr/sbin/razer_bcd
cp daemon/fx/pez2001_collection.so ${directory}/usr/share/razer_bcd/fx
cp daemon/fx/pez2001_mixer.so ${directory}/usr/share/razer_bcd/fx
cp daemon/fx/pez2001_light_blast.so ${directory}/usr/share/razer_bcd/fx
cp daemon/fx/pez2001_progress_bar.so ${directory}/usr/share/razer_bcd/fx

# Copy daemon controller
cp daemon_controller/razer_bcd_controller ${directory}/usr/bin/razer_bcd_controller

# Copy Python3 libraries into the python path
cp -r pylib ${directory}/usr/lib/python3/dist-packages/razer

# Copy razer kernel driver to src
cp Makefile ${directory}/usr/src/razer_chroma_driver-1.0.0/Makefile
cp install_files/dkms/dkms.conf ${directory}/usr/src/razer_chroma_driver-1.0.0/dkms.conf
#cp driver/{Makefile,razerkbd_driver.c,razerkbd_driver.h,razermouse_driver.c,razermouse_driver.h,razerfirefly_driver.c,razerfirefly_driver.h} ${directory}/usr/src/razer_chroma_driver-1.0.0/driver
cp driver/Makefile ${directory}/usr/src/razer_chroma_driver-1.0.0/driver
cp driver/*.h ${directory}/usr/src/razer_chroma_driver-1.0.0/driver
cp driver/*.c ${directory}/usr/src/razer_chroma_driver-1.0.0/driver
# Remove unwanted sources
rm ${directory}/usr/src/razer_chroma_driver-1.0.0/driver/*.mod*

rm -f $TMPDIR/razer-chroma*.deb
dpkg-deb --build ${directory}
dpkg-name ${directory}.deb

rm -rf ${directory}








