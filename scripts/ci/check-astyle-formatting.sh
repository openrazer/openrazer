#!/bin/bash
# Installation:
#   cd my_gitproject
#   wget -O pre-commit.sh http://tinyurl.com/mkovs45
#   ln -s ../../pre-commit.sh .git/hooks/pre-commit
#   chmod +x pre-commit.sh

# Modified for the OpenRazer CI.

OPTIONS="--style=linux"

RETURN=0
ASTYLE=$(which astyle)
if [ $? -ne 0 ]; then
        echo "[!] astyle not installed. Unable to check source file format policy." >&2
        exit 1
fi

FILES=`find driver/ -name "*.c" -o -name "*.h"`
for FILE in $FILES; do
        $ASTYLE $OPTIONS < $FILE | cmp -s $FILE -
        if [ $? -ne 0 ]; then
                echo "[!] $FILE does not respect the agreed coding style." >&2
                RETURN=1
        fi
done

if [ $RETURN -eq 1 ]; then
        echo "" >&2
        echo "You can run ./scripts/format_source.sh to automatically format those files." >&2
fi

exit $RETURN
