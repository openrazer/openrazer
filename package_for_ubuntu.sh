#!/bin/bash

# Make deployment directory
directory=$(mktemp -d)

# Copy DEBIAN directory
mkdir -p ${directory}
cp -r install_files/DEBIAN_ubuntu ${directory}/DEBIAN
chmod 755 ${directory}/DEBIAN
chmod 755 ${directory}/DEBIAN/{pre,post}*


# Create file structure
DESTDIR=${directory} make ubuntu_install
echo ''

rm -f $TMPDIR/razer-chroma*.deb
dpkg-deb --build ${directory}
dpkg-name ${directory}.deb

rm -rf ${directory}








