#!/bin/bash
rm -fr ./build
mkdir ./build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX="t:\\cygwin\\usr\\local"
cmake --build .

