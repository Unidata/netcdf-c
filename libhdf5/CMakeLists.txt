## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2018-2022, see the COPYRIGHT file for more information.
#
# This builds the HDF5 dispatch layer.
#
# Ed Hartnett

# Build the HDF5 dispatch layer as a library that will be included in
# the netCDF library.
add_library(netcdfhdf5 OBJECT)

# The source files for the HDF5 dispatch layer.
target_sources(netcdfhdf5 PRIVATE 
    nc4hdf.c nc4info.c hdf5file.c hdf5attr.c
    hdf5dim.c hdf5grp.c hdf5type.c hdf5internal.c hdf5create.c hdf5open.c
    hdf5var.c nc4mem.c nc4memcb.c hdf5dispatch.c hdf5filter.c hdf5plugins.c
    hdf5set_format_compatibility.c hdf5debug.c
)

if (NETCDF_ENABLE_DLL)
    target_compile_definitions(netcdfhdf5 PRIVATE DLL_NETCDF DLL_EXPORT)
endif()

if(NETCDF_ENABLE_BYTERANGE)
    target_sources(netcdfhdf5 PRIVATE 
        H5FDhttp.c
    )
endif()


if(STATUS_PARALLEL)
    target_link_libraries(netcdfhdf5 PUBLIC MPI::MPI_C)
endif(STATUS_PARALLEL)

target_link_libraries(netcdfhdf5 PUBLIC HDF5::HDF5)

# Remember to package this file for CMake builds.
add_extra_dist(${libnchdf5_SOURCES} CMakeLists.txt)
