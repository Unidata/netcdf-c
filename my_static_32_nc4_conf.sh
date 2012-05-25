#!/bin/bash
set -x
./configure --enable-static --disable-doxygen --disable-shared --disable-dap --enable-netcdf-4 --enable-diskless CFLAGS="-ggdb -O0 -I/c/Users/wfisher/Desktop/hdf5-1.8.9/src -I/c/Users/wfisher/Desktop/hdf5-1.8.9/hl/src -I/c/Users/wfisher/Desktop/netcdf-4.2/include/ -I/c/Users/wfisher/Desktop/curl-7.25.0/include/" LDFLAGS="-static -ggdb -O0 -L/c/Users/wfisher/Desktop/hdf5-1.8.9/build_win/bin"

#./configure --enable-static --disable-doxygen --disable-shared --disable-dap --enable-netcdf-4 --enable-diskless CFLAGS="-ggdb -O0 -I/c/Users/wfisher/Desktop/hdf5-1.8.8/src -I/c/Users/wfisher/Desktop/hdf5-1.8.8/hl/src -I/c/Users/wfisher/Desktop/netcdf-4.2/include/ -I/c/Users/wfisher/Desktop/curl-7.25.0/include/" LDFLAGS="-static -ggdb -O0 -L/c/Users/wfisher/Desktop/curl-7.25.0/lib/.libs -L/c/GnuWin32/lib/libz.a -L/c/Users/wfisher/Desktop/hdf5-1.8.8/build_win/bin"

