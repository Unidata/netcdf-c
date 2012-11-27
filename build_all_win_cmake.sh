#!/bin/bash

#####
# 32-bit
#####

RDIR=/c/share/w32/static

###
# Static
###

SHARED=OFF

#
# Minimum
#

BDIR="build_32_min_static"
NAME="4.2.x-No_DAP-No_HDF-static-snapshot"
echo "Building $NAME"
mkdir $BDIR
cd $BDIR
CMAKE_PREFIX_PATH=$RDIR cmake .. -DENABLE_NETCDF_4=OFF -DENABLE_DAP=OFF -DCPACK_PACKAGE_FILE_NAME=NetCDF-$NAME -DENABLE_TESTS=OFF -DBUILD_SHARED_LIBS=$SHARED
cmake --build .
cmake --build . --target package
echo "Finished building $NAME"

#
# NetCDF-4
#

#
# NetCDF-4, DAP
# 