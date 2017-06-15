#!/bin/bash

find driver/ -name "*.c" -o -name "*.h" | xargs astyle --style=linux -n
