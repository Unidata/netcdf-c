## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.
#
# This builds the HDF5 dispatch layer.
#
# Ed Hartnett

# The source files for the HDF5 dispatch layer.
SET(libnchdf5_SOURCES nc4hdf.c nc4info.c hdf5file.c hdf5attr.c
hdf5dim.c hdfd5grp.c)

# Build the HDF4 dispatch layer as a library that will be included in
# the netCDF library.
add_library(netcdfhdf5 OBJECT ${libnchdf5_SOURCES})

# Remember to package this file for CMake builds.
ADD_EXTRA_DIST(${libnchdf5_SOURCES} CMakeLists.txt)
