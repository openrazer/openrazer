#!/usr/bin/env python3

import os
from setuptools import setup, find_packages

setup(
    name = "razer",
    version = "1.1.3",
    packages=find_packages(exclude=["*.tests", "*.tests.*", "tests.*", "tests"])
)
