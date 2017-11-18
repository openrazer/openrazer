#!/bin/bash -ex

mkdir /tmp/daemon_stuff/{,data,logs}
mkdir /tmp/daemon_test
./scripts/create_fake_device.py \
    --dest /tmp/daemon_test \
    --create-only \
    razernaga2014
