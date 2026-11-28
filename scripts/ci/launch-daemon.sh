#!/bin/bash -ex

env PYTHONPATH="pylib:daemon" \
python3 ./daemon/run_openrazer_daemon.py \
    --run-dir /tmp/daemon_stuff/data \
    --log-dir /tmp/daemon_stuff/logs \
    --test-dir /tmp/daemon_test \
    --config=$(pwd)/daemon/resources/razer.conf \
    --as-root &
