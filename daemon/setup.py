#!/usr/bin/python3

from setuptools import setup, find_packages

setup(
    name="openrazer_daemon",
    version="2.8.0",
    packages=find_packages(exclude=["*.tests", "*.tests.*", "tests.*", "tests"])
)
