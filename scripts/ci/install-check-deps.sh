#!/bin/bash -ex

apt-get -y install \
    astyle \
    git \
    python3-pip

pip3 install autopep8
pip3 install pylint

# pylint errors with 1.8+ on pylib/tests/integration_tests/test_device_manager.py about the missing 'coverage' module
pip3 install coverage
