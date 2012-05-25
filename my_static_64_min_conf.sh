#!/bin/bash
set -x
./configure --enable-static --disable-dap --disable-doxygen --disable-shared --disable-netcdf-4 --host=x86_64-w64-mingw32 CFLAGS="-ggdb -static -O0" LDFLAGS="-static -ggdb -O0"