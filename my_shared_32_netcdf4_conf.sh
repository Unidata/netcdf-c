#!/bin/bash
set -x
./configure --disable-static --disable-doxygen --enable-shared --disable-dap --enable-netcdf-4 --enable-utilities --enable-dll CFLAGS="-ggdb -O0 -I/c/Users/wfisher/Desktop/hdf5-1.8.9/src" LDFLAGS="-ggdb -O0 -L/c/Users/wfisher/Desktop/hdf5-1.8.9/build_win32_dll/bin -L/c/Users/wfisher/Desktop/szip-2.1/build_win/bin"



