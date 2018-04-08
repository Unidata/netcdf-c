## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.
#
# This builds the HDF5 dispatch layer.
#
# Ed Hartnett

# The source files for the HDF5 dispatch layer.
# SET(libhdf5_SOURCES hdf5file.c hdf5dispatch.c hdf5var.c
#   hdf5internal.c hdf5type.c hdf5func.c hdf5attr.c nc4hdf.c)
SET(libnchdf5_SOURCES hdf5file.c nc4hdf.c hdf5attr.c hdf5dispatch.c
  hdf5var.c hdf5type.c hdf5internal.c)

# Build the HDF4 dispatch layer as a library that will be included in
# the netCDF library.
add_library(netcdfhdf5 OBJECT ${libnchdf5_SOURCES})

# Remember to package this file for CMake builds.
ADD_EXTRA_DIST(${libnchdf5_SOURCES} CMakeLists.txt)
