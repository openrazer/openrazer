#!/bin/sh

set -e

. /usr/share/razer_bcd/bash_keyboard_functions.sh

if [ "$1" = "bind" ]; then
	bind_all
else
	unbind_all
fi
