## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.
#
# This builds the libzarr dispatch layer.
#
# Dennis Heimbigner

# The source files for the HDF5 dispatch layer.
SET(libnczarr_SOURCES
zformat.c
zformat3.c
zformat2.c
zsync.c
zinfer.c
zfill.c
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
ztype.c
zutil.c
zvar.c
zwalk.c
znc4.c
zfilter.c
zplugins.c
zdebug.c
zformat.h
zarr.h
zfill.h
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
znc4.h
zdebug.h
)

if(NETCDF_ENABLE_NCZARR_ZIP)
    SET(libnczarr_SOURCES ${libnczarr_SOURCES} zmap_zip.c)
endif()

if(NETCDF_ENABLE_S3) 
    SET(libnczarr_SOURCES ${libnczarr_SOURCES} zmap_s3sdk.c)
endif()

if(NETCDF_ENABLE_NCZARR_FILTERS)
    SET(libnczarr_SOURCES ${libnczarr_SOURCES} zfilter.c)
endif()

set_property(SOURCE zdispatch.c
  PROPERTY
    SKIP_UNITY_BUILD_INCLUSION ON)

# Build the Zarr dispatch layer as a library that will be included in
# the netCDF library.

add_library(nczarr OBJECT ${libnczarr_SOURCES})

target_include_directories(nczarr PUBLIC ../libncpoco)

if (USE_HDF5)
    target_link_libraries(nczarr PRIVATE HDF5::HDF5)
endif(USE_HDF5)

if(NETCDF_ENABLE_NCZARR_ZIP)
   target_include_directories(nczarr PRIVATE ${Zip_INCLUDE_DIRS})
endif(NETCDF_ENABLE_NCZARR_ZIP)

if(STATUS_PARALLEL)
    target_link_libraries(nczarr PUBLIC MPI::MPI_C)
endif(STATUS_PARALLEL)

if (NETCDF_ENABLE_DLL)
    target_compile_definitions(nczarr PRIVATE DLL_NETCDF DLL_EXPORT)
endif()

target_link_libraries(nczarr PUBLIC ${CURL_LIBRARIES})
target_include_directories(nczarr PUBLIC ${CURL_INCLUDE_DIRS})
target_compile_options(nczarr
  PRIVATE
    -DCURL_STATICLIB=1
)

if (NETCDF_ENABLE_DLL)
  target_compile_definitions(nczarr PRIVATE DLL_NETCDF DLL_EXPORT)
endif()

if(STATUS_PARALLEL)
  target_link_libraries(nczarr PUBLIC MPI::MPI_C)
endif(STATUS_PARALLEL)

# Remember to package this file for CMake builds.
add_extra_dist(${libnczarr_SOURCES} CMakeLists.txt)
