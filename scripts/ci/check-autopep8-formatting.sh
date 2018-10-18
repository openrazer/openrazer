#!/bin/bash

OPTIONS="--max-line-length 500 --ignore E402"

RETURN=0
AUTOPEP8=$(which autopep8)
if [ $? -ne 0 ]; then
        echo "[!] autopep8 not installed. Unable to check source file format policy." >&2
        exit 1
fi

FILES=`find . -name "*.py"`
for FILE in $FILES; do
        $AUTOPEP8 $OPTIONS $FILE | cmp -s $FILE -
        if [ $? -ne 0 ]; then
                echo "[!] $FILE does not respect the agreed coding style." >&2
                RETURN=1
        fi
done

if [ $RETURN -eq 1 ]; then
        echo "" >&2
        echo "Make sure you have run autopep8 with the following options:" >&2
        echo $OPTIONS >&2
fi

exit $RETURN
