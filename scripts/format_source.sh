#!/bin/bash

set -ex

# C (astyle)
find driver/ -name "*.c" -o -name "*.h" | xargs astyle --style=linux -n

# Python (autopep8)
autopep8 --jobs 0 --recursive --in-place --max-line-length 500 --ignore E402 .
