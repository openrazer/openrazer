#!/bin/bash -ex

#env PYTHONPATH="pylib:daemon" \
#python3 ./daemon/run_openrazer_daemon.py \
#    --verbose \
#    --foreground \
#    --config=$(pwd)/daemon/resources/razer.conf \
#    --as-root &

env PYTHONPATH="pylib:daemon" \
python3 ./daemon/run_openrazer_daemon.py \
    --foreground \
    --run-dir /tmp/daemon_stuff/data \
    --log-dir /tmp/daemon_stuff/logs \
    --test-dir /tmp/daemon_test \
    --config=$(pwd)/daemon/resources/razer.conf \
    --as-root &
