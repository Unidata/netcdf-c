## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2018-2022, see the COPYRIGHT file for more information.
#
# This builds the HDF5 dispatch layer.
#
# Ed Hartnett

# The source files for the HDF5 dispatch layer.
set(libnchdf5_SOURCES nc4hdf.c nc4info.c hdf5file.c hdf5attr.c
hdf5dim.c hdf5grp.c hdf5type.c hdf5internal.c hdf5create.c hdf5open.c
hdf5var.c nc4mem.c nc4memcb.c hdf5dispatch.c hdf5filter.c
hdf5set_format_compatibility.c hdf5debug.c)

if(ENABLE_BYTERANGE)
    set(libnchdf5_SOURCES ${libnchdf5_SOURCES} H5FDhttp.c)
endif()

# Build the HDF5 dispatch layer as a library that will be included in
# the netCDF library.
add_library(netcdfhdf5 OBJECT ${libnchdf5_SOURCES})

if(STATUS_PARALLEL)
    target_link_libraries(netcdfhdf5 PUBLIC MPI::MPI_C)
endif(STATUS_PARALLEL)

# Remember to package this file for CMake builds.
add_extra_dist(${libnchdf5_SOURCES} CMakeLists.txt)
