#!/bin/bash
#
# git-bisect script for ncf-345. Note you need to put it outside
# the netcdf directory or it will disappear when git does its thing.
#

mkdir -p build
cd build
cmake .. -DENABLE_TESTS=OFF -DBUILD_SHARED_LIBS=OFF
make -j 4
ncgen/ncgen -b -o ~/Desktop/in.nc ~/Desktop/in.cdl
RES=$?
rm -f ~/Desktop/in.nc
cd ..
echo "Exiting: $RES"
exit $RES
