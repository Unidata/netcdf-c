#!/bin/bash
set -x
./configure --disable-dap --disable-doxygen --enable-shared --enable-static --enable-dll --enable-netcdf-4 --disable-utilities --host=x86_64-w64-mingw32 CFLAGS="-ggdb -O0 -I/c/HDF5189/include" LDFLAGS="-ggdb -O0 -L/c/HDF5189/bin -L/c/HDF5189/lib"

#./configure --disable-dap --disable-doxygen --enable-shared --disable-static --enable-dll --enable-netcdf-4 --disable-utilities --host=x86_64-w64-mingw32 CFLAGS="-ggdb -O0 -I/c/HDF5189/include" LDFLAGS="-ggdb -O0 -L/home/wfisher/mypath/x86_64-w64-mingw32/lib -L/home/wfisher/zlib-1.2.5 -L/c/Users/wfisher/Desktop/szip-2.1/build_win64/bin -L/c/HDF5189/bin"
