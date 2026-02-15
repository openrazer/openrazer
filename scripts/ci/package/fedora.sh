#!/bin/bash -xe
#
# Script that sets up the environment for building packages inside a Podman container.
# This is for Fedora-based distributions.
#
###############################################
# For CI use only! DO NOT RUN ON YOUR SYSTEM! #
###############################################

dnf install -y rpm-build rpmdevtools rpmspectool git curl make systemd-rpm-macros

# Download the spec file
rpmdev-setuptree
cd ~/rpmbuild/SPECS
curl -fLO https://raw.githubusercontent.com/openrazer/OBS-packaging/master/openrazer.spec

# Build from the checked out sources instead of a downloaded tarball
git config --global --add safe.directory /build
commit=$(git -C /build rev-parse HEAD)
git -C /build archive --format=tar.gz --prefix="openrazer-$commit/" -o ~/rpmbuild/SOURCES/openrazer-$commit.tar.gz HEAD

# The spec's dkms_version must match the version the Makefile installs
dkms_version=$(sed -n 's/^PACKAGE_VERSION="\(.*\)"/\1/p' /build/install_files/dkms/dkms.conf)
sed -i \
    -e "s|^#define gitcommit.*|%define gitcommit $commit|" \
    -e "s|^%define dkms_version .*|%define dkms_version $dkms_version|" \
    -e "s|^Version:.*|Version: $dkms_version|" \
    -e "s|^Release:.*|Release: 0.git.${commit:0:8}%{?dist}|" \
    -e "s|^Source0:.*openrazer/archive.*|Source0: openrazer-%{gitcommit}.tar.gz|" \
    openrazer.spec

dnf -y builddep openrazer.spec
rpmbuild -bb openrazer.spec

# Hand the packages back to the workspace
mkdir -p /build/fedora-rpms
mv ~/rpmbuild/RPMS/noarch/*.rpm /build/fedora-rpms/
rm -f /build/fedora-rpms/openrazer-meta-*
