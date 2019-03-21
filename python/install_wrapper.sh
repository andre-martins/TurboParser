#!/bin/bash

# Root folder where TurboParser is located.
root_folder="`cd $(dirname $0);cd ..;pwd`"

# Lib folder.
lib_folder=${root_folder}/libturboparser

# Python folder.
python_folder=${root_folder}/python

# Make a static lib.
cd $lib_folder
make clean
make

# Now use cython to build a Python wrapper.
cd $python_folder
python setup.py build_ext --force
