#!/usr/bin/python3

from setuptools import setup, find_packages

setup(
    name="openrazer",
    version="3.0.0",
    packages=find_packages(exclude=["*.tests", "*.tests.*", "tests.*", "tests"])
)
