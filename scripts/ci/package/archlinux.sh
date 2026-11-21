#!/bin/bash -xe
#
# Script that sets up the environment for building packages inside a Podman container.
# This is for distributions based on Arch Linux.
#
###############################################
# For CI use only! DO NOT RUN ON YOUR SYSTEM! #
###############################################

# Set up repositories and build environment
echo -e "[multilib]\nInclude = /etc/pacman.d/mirrorlist" >> /etc/pacman.conf
echo 'MAKEFLAGS="-j$(nproc)"' >> /etc/makepkg.conf

# Install dependencies
pacman -Syu sudo git python python-setuptools --noconfirm

# Download AUR packaging files
cd /build
curl https://aur.archlinux.org/cgit/aur.git/snapshot/openrazer-git.tar.gz | tar -xvz
cd /build/openrazer-git
sed -i "s|source=(.*)|source=(\"openrazer::git+file:///build\")|" PKGBUILD

# Build the package as a non-root user (makepkg cannot run as root)
useradd -m -G wheel -s /bin/bash builder
sudo chown -R builder:builder /build
echo "builder ALL=(ALL:ALL) ALL" >> /etc/sudoers
echo "builder:123456" | chpasswd
su -s /bin/bash builder -c "PATH=/usr/bin:$PATH makepkg -sC"
