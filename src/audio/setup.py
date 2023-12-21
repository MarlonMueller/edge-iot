#!/usr/bin/env python
# -*- coding: utf-8 -*-

# setup.py
#
# Description: Setup file for C library
# This file provides the setup file for the C preprocess library.
# 
# Usage: In the root directory of the project, run the following command:
#   python3 src/audio/setup.py build

from setuptools import setup, Extension

setup(
    include_dirs = ['include/'], 
    ext_modules = [Extension(
        "preprocess", 
        ["src/audio/preprocess.cpp"],
        extra_compile_args=['-D RUN_PC', '-lm'],
        )
    ],
)