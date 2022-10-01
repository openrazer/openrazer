#!/usr/bin/python3
# SPDX-License-Identifier: GPL-2.0-or-later

from setuptools import setup, find_packages

setup(
    name="openrazer_daemon",
    version="3.4.0",
    packages=find_packages(exclude=["*.tests", "*.tests.*", "tests.*", "tests"])
)
