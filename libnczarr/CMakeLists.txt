## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.
#
# This builds the libzarr dispatch layer.
#
# Dennis Heimbigner

#add_compile_options("/showincludes")

# The source files for the HDF5 dispatch layer.
set(libnczarr_SOURCES
zarr.c
zattr.c
zxcache.c
zchunking.c
zclose.c
zcreate.c
zcvt.c
zdim.c
zdispatch.c
zfile.c
zgrp.c
zinternal.c
zmap.c
zmap_file.c
zodom.c
zopen.c
zprov.c
zsync.c
ztype.c
zutil.c
zvar.c
zwalk.c
zdebug.c
zarr.h
zcache.h
zchunking.h
zdispatch.h
zincludes.h
zinternal.h
zmap.h
zodom.h
zprovenance.h
zplugins.h
zfilter.h
zdebug.h
)

if(NETCDF_ENABLE_NCZARR_ZIP)
    set(libnczarr_SOURCES ${libnczarr_SOURCES} zmap_zip.c)
endif()

if(NETCDF_ENABLE_NCZARR_FILTERS)
set(libnczarr_SOURCES ${libnczarr_SOURCES} zfilter.c zplugins.c)
endif()

if(NETCDF_ENABLE_S3) 
    set(libnczarr_SOURCES ${libnczarr_SOURCES} zmap_s3sdk.c)
endif()

# Build the Zarr dispatch layer as a library that will be included in
# the netCDF library.

add_library(nczarr OBJECT ${libnczarr_SOURCES})

target_include_directories(nczarr PUBLIC ../libncpoco)

if (USE_HDF5)
    target_link_libraries(nczarr PRIVATE HDF5::HDF5)
endif(USE_HDF5)

if(STATUS_PARALLEL)
    target_link_libraries(nczarr PUBLIC MPI::MPI_C)
endif(STATUS_PARALLEL)

if (NETCDF_ENABLE_DLL)
    target_compile_definitions(nczarr PRIVATE DLL_NETCDF DLL_EXPORT)
endif()

# Remember to package this file for CMake builds.
ADD_EXTRA_DIST(${libnczarr_SOURCES} CMakeLists.txt)
