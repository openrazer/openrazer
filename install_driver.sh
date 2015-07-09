#!/bin/bash

#install dbus dependency (daemon)
sudo apt-get install libdbus-1-dev
#install jq dependency (test_daemon.sh)
sudo apt-get install jq
make -s all
sudo make -s install
