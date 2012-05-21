#!/bin/bash
set -x
./configure --enable-static --disable-dap --disable-doxygen --disable-shared --disable-netcdf-4 --enable-diskless CFLAGS="-ggdb -static -O0" LDFLAGS="-static -ggdb -O0"