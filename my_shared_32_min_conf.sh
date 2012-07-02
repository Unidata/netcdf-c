#!/bin/bash
set -x
./configure --disable-dap --disable-doxygen --enable-shared --disable-static --enable-dll --disable-netcdf-4 CFLAGS="-ggdb -O0" LDFLAGS="-ggdb -O0"

#./configure --disable-dap --disable-doxygen --enable-shared --disable-static --enable-dll --disable-netcdf-4 --disable-utilities --host=x86_64-w64-mingw32 CFLAGS="-ggdb -O0" LDFLAGS="-ggdb -O0" 