#!/bin/bash
PREFIX="/usr/local"
#PREFIX="t:\\cygwin\\usr\\local"
rm -fr ./build
mkdir ./build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX="${PREFIX}"
#cmake --build .
