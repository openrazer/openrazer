#!/usr/bin/env python3

from setuptools import setup, find_packages

setup(
    name = "razer",
    version = "1.1.8",
    packages=find_packages(exclude=["*.tests", "*.tests.*", "tests.*", "tests"])
)
