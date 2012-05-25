#!/bin/bash
set -x

./configure --disable-dap --disable-doxygen --enable-shared --disable-static --enable-dll --enable-netcdf-4 --enable-utilities --host=x86_64-w64-mingw32 CFLAGS="-ggdb -gdwarf-2 -O0 -I/c/Users/wfisher/Desktop/hdf5-1.8.9/build_win64_dll -I/c/Users/wfisher/Desktop/hdf5-1.8.9/src" LDFLAGS="-ggdb -gdwarf-2 -O0 -L/c/Users/wfisher/Desktop/hdf5-1.8.9/build_win64_dll/bin -L/c/Users/wfisher/Desktop/zlib125dll/dllx64 -L/c/Users/wfisher/Desktop/szip-2.1/build_win64_dll/bin"

#./configure --disable-dap --disable-doxygen --enable-shared --disable-static --enable-dll --enable-netcdf-4 --enable-utilities --host=x86_64-w64-mingw32 CFLAGS="-ggdb -gdwarf-2 -O0 -I/c/HDF5189/include" LDFLAGS="-ggdb -gdwarf-2 -O0 -L/c/HDF5189/bin"

#./configure --disable-dap --disable-doxygen --enable-shared --disable-static --enable-dll --enable-netcdf-4 --disable-utilities --host=x86_64-w64-mingw32 CFLAGS="-ggdb -O0 -I/c/HDF5189/include" LDFLAGS="-ggdb -O0 -L/home/wfisher/mypath/x86_64-w64-mingw32/lib -L/home/wfisher/zlib-1.2.5 -L/c/Users/wfisher/Desktop/szip-2.1/build_win64/bin -L/c/HDF5189/bin"
