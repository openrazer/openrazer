#!/usr/bin/python3
# vim: tabstop=2 shiftwidth=2 softtabstop=2 expandtab colorcolumn=76 :
import string
from datetime import datetime
from dateutil import parser
from dateutil import tz
import sys
import re

items = {
    'header': re.compile(r'^\w+\W*\(([0-9]+[^)]+)\)'),
    'tail': re.compile(r'--\W+(.+<.+>)\W+(.+)'),
    'item': re.compile(r'\W+\*\W+(.*)$')
}

version = None
author = None
date = None
changes = []


def apply():
    global version
    global author
    global date
    global changes
    if ((date is not None) and (author is not None) and (version is not None)):
        if (0 >= len(changes)):
            changes.append('Changelog available at site')

        dt = date.strftime('%a %b %d %Y')
        print(f'* {dt} {author} - {version}')
        for item in changes:
            print(f'- {item}')
        print('')

    version = None
    author = None
    timestamp = None
    changes.clear()


for line in sys.stdin:
    line = line.rstrip()
    if (not line):
        continue

    match = None
    typ = None

    for t, r in items.items():
        match = r.search(line)
        if match:
            typ = t
            break

    if (typ == 'header'):
        apply()
        version = match[1]
    elif (typ == 'item'):
        changes.append(match[1])
    elif (typ == 'tail'):
        date = parser.parse(match[2])
        author = match[1]
apply()
