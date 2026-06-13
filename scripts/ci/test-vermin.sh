#!/bin/sh

# shellcheck disable=SC2046
vermin \
    -t=3.10- \
    --lint \
    --eval-annotations \
    $(find . -name '*.py')
