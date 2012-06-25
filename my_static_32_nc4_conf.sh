#!/bin/bash
set -x
./configure --enable-static --disable-doxygen --disable-shared --disable-dap --enable-netcdf-4 CFLAGS="-ggdb -O0 -I/c/Users/wfisher/Desktop/hdf5-1.8.9/src -I/c/Users/wfisher/Desktop/hdf5-1.8.9/hl/src -I/c/Users/wfisher/Desktop/netcdf-4.2/include/" LDFLAGS="-static -ggdb -O0 -L/c/Users/wfisher/Desktop/hdf5-1.8.9/build_win32_static/bin -L/c/Users/wfisher/Desktop/szip-2.1/build_win32/bin -L/c/Users/wfisher/Desktop/zlib125dll/static32"


