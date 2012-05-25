#!/bin/bash
set -x
./configure --enable-static --disable-doxygen --disable-shared --disable-dap --enable-netcdf-4 --enable-diskless --host=x86_64-w64-mingw32 CFLAGS="-ggdb -O0 -I/c/HDF5189/include" LDFLAGS="-ggdb -O0 -L/c/HDF5189/bin -L/c/HDF5189/lib"



