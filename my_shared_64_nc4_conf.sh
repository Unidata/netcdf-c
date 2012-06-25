#!/bin/bash
set -x

./configure --disable-dap --disable-doxygen --enable-shared --disable-static --enable-dll --enable-netcdf-4 --host=x86_64-w64-mingw32 CFLAGS="-ggdb -gdwarf-2 -O0 -I/c/Users/wfisher/Desktop/hdf5-1.8.9/src" LDFLAGS="-ggdb -gdwarf-2 -O0 -L/c/Users/wfisher/Desktop/hdf5-1.8.9/build_win64_dll/bin -L/c/Users/wfisher/Desktop/zlib125dll/dllx64 -L/c/Users/wfisher/Desktop/szip-2.1/build_win64/bin"

