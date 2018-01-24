#!/bin/bash

#env PYTHONPATH="pylib:daemon" \
#python3 ./daemon/run_openrazer_daemon.py \
#    --verbose \
#    --foreground \
#    --config=$(pwd)/daemon/resources/razer.conf &

env PYTHONPATH="pylib:daemon" \
python3 ./../../daemon/run_openrazer_daemon.py \
    --foreground \
    --verbose \
    --run-dir /tmp/daemon_stuff/data \
    --log-dir /tmp/daemon_stuff/logs \
    --test-dir /tmp/daemon_test \
    --config=/home/rabreu/workspace-intellij-idea/openrazer/daemon/resources/razer.conf &
