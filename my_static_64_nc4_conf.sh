#!/bin/bash
set -x
./configure --enable-static --disable-doxygen --disable-shared --disable-dap --enable-netcdf-4 --host=x86_64-w64-mingw32 CFLAGS="-ggdb -O0 -I/c/Users/wfisher/Desktop/hdf5-1.8.9/src" LDFLAGS="-ggdb -O0 -L/c/Users/wfisher/Desktop/szip-2.1/build_win64/bin -L/c/Users/wfisher/Desktop/hdf5-1.8.9/build_win64/bin -L/c/Users/wfisher/Desktop/zlib125dll/staticx64"

#./configure --enable-static --disable-doxygen --disable-shared --disable-dap --enable-netcdf-4 --host=x86_64-w64-mingw32 CFLAGS="-ggdb -O0 -I/c/Users/wfisher/Desktop/hdf5-1.8.9/src" LDFLAGS="-ggdb -O0 -L/c/Users/wfisher/Desktop/szip-2.1/build_win64/bin -L/c/Users/wfisher/Desktop/hdf5-1.8.9/build_win64_static/bin -L/c/Users/wfisher/Desktop/zlib125dll/staticx64"



