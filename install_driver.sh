#!/bin/bash

#install dbus dependency (daemon)
sudo apt-get install libdbus-1-dev
#install jq dependency (test_daemon.sh)
sudo apt-get install jq
#install libsdl dependency
sudo apt-get install libsdl2-dev
sudo apt-get install libsdl2-image-dev

make -s all
sudo make -s install
