#!/bin/bash -ex

apt-get -y install \
    astyle \
    python3-pip

pip3 install autopep8
pip3 install pylint
