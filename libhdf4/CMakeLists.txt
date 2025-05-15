## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.
#
# This builds the HDF4 dispatch layer.
#
# Ed Hartnett


# Build the HDF4 dispatch layer as a library that will be included in
# the netCDF library.
add_library(netcdfhdf4 OBJECT ${libhdf4_SOURCES})
target_include_directories(netcdfhdf4 PUBLIC ${MFHDF_H_INCLUDE_DIR})

target_sources(netcdfhdf4
    PRIVATE
        hdf4dispatch.c
        hdf4file.c
        hdf4func.c
        hdf4var.c
)

if (NETCDF_ENABLE_DLL)
    target_compile_definitions(netcdfhdf4 PRIVATE DLL_NETCDF DLL_EXPORT)
endif()

target_link_libraries(netcdfhdf4 PUBLIC ${HDF4_LIBRARIES})

# Remember to package this file for CMake builds.
ADD_EXTRA_DIST(${libhdf4_SOURCES} CMakeLists.txt)
