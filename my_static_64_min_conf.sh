#!/bin/bash
CTMP=""
if [ $# -gt 0 ]; then
    CTMP="-fno-stack-check -fno-stack-protector -mno-stack-arg-probe"
    echo "Building with additional flags: $CTMP"
fi

set -x
./configure --enable-static --disable-dap --disable-doxygen --disable-shared --disable-netcdf-4 --host=x86_64-w64-mingw32 CFLAGS="$CTMP -ggdb -static -O0" LDFLAGS="-static -ggdb -O0"