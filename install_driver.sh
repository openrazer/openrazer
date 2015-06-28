#!/bin/bash

#install dbus dependency
sudo apt-get install libdbus-1-dev

make -s all
sudo make -s install
