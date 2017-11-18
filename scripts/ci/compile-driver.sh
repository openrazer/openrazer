#!/bin/bash -ex

if [ -z $(grep ccflags-y driver/Makefile) ]; then
    echo "ccflags-y := -Wall -Werror" >> driver/Makefile
fi

make
