#!/bin/sh

# shellcheck disable=SC2046
vermin \
    -t=3.9- \
    --backport argparse \
    --backport configparser \
    --backport typing \
    --lint \
    --eval-annotations \
    $(find . -name '*.py')
