#!/bin/bash
set -x
CTMP=""
if [ $# -gt 0 ]; then
    CTMP="-fno-stack-check -fno-stack-protector -mno-stack-arg-probe"
    echo "Building with additional flags: $CTMP"
fi

./configure --enable-static --disable-doxygen --disable-shared --enable-dap --enable-netcdf-4 --host=x86_64-w64-mingw32 CFLAGS="$CTMP -ggdb -O0 -I/c/Users/wfisher/Desktop/hdf5-1.8.9/src -I/c/Users/wfisher/Desktop/hdf5-1.8.9/build_win64_static/ -I/c/Users/wfisher/Desktop/curl-7.25.0/build_win64/include -DCURL_STATICLIB" LDFLAGS="-ggdb -O0 -L/c/Users/wfisher/Desktop/szip-2.1/build_win64/bin -L/c/Users/wfisher/Desktop/hdf5-1.8.9/build_win64_static/bin -L/c/Users/wfisher/Desktop/zlib125dll/staticx64 -L/c/Users/wfisher/Desktop/curl-7.25.0/build_win64/lib/.libs"
