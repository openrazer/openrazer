#!/bin/bash -ex

mkdir /tmp/daemon_stuff/{,data,logs}
mkdir /tmp/daemon_test
./scripts/create_fake_device.py \
    --dest /tmp/daemon_test \
    --non-interactive \
    --all &

# wait a bit for the previous script to settle
sleep 0.5
