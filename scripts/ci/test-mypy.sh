#!/bin/sh

# Currently openrazer does not have a lot of typing annotations and
# correctness, so limit checking to known-good paths to avoid regressions.
#
# Long-term we want to enable it fully for both daemon and pylib:
# $ mypy --strict daemon/
# $ mypy --strict pylib/

mypy --strict pylib/openrazer/
