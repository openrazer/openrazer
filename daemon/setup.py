#!/usr/bin/env python3

from setuptools import setup, find_packages

setup(
    name = "razer_daemon",
    version = "1.1.7",
    packages=find_packages(exclude=["*.tests", "*.tests.*", "tests.*", "tests"])
)
