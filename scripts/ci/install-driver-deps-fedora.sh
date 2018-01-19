#!/bin/bash

dnf -y install \
    linux-headers-$(uname -r) \
    python3 \
    python3-setuptools \
    dh-autoreconf-12-2.fc25.noarch

dnf -y install \
    devscripts

dnf -y install \
    groupinstall "Development Tools and Libraries"

