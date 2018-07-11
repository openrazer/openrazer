#!/usr/bin/python3

from setuptools import setup, find_packages

setup(
    name="openrazer",
    version="2.3.1",
    packages=find_packages(exclude=["*.tests", "*.tests.*", "tests.*", "tests"])
)
