#!/bin/bash
set -x
./configure --enable-static --disable-dap --disable-doxygen --disable-shared --disable-netcdf-4 CFLAGS="-ggdb -static -O0 -fno-stack-check -fno-stack-protector -mno-stack-arg-probe" LDFLAGS="-ggdb -static -O0"