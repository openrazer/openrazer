#!/bin/bash -ex

apt-get -y install \
    debhelper \
    linux-headers-$(uname -r) \
    python3 \
    python3-setuptools

apt-get -y install \
    devscripts

apt-get -y install \
    build-essential:native

