## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.
#
# This builds the libzarr dispatch layer.
#
# Dennis Heimbigner

#add_compile_options("/showincludes")

# The source files for the HDF5 dispatch layer.
SET(libnczarr_SOURCES
zarr.c
zattr.c
zcache.c
zchunking.c
zclose.c
zcreate.c
zcvt.c
zdim.c
zdispatch.c
zfile.c
zfilter.c
zgrp.c
zinternal.c
zjson.c
zmap.c
zmap_nz4.c
zmap_nzf.c
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
zjson.h
zmap.h
zodom.h
zprovenance.h
zdebug.h
)

IF(ENABLE_S3_SDK) 
SET(libnczarr_SOURCES ${libnczarr_SOURCES} zs3sdk.cpp zmap_s3sdk.c awsincludes.h zs3sdk.h)
ENDIF()

# Build the Zarr dispatch layer as a library that will be included in
# the netCDF library.
add_library(nczarr OBJECT ${libnczarr_SOURCES})

IF(ENABLE_S3_SDK) 
IF(NOT MSVC)
target_compile_features(nczarr PUBLIC cxx_std_11)
ENDIF()
target_include_directories(nczarr PRIVATE ${AWSSDK_INCLUDE_DIR})
ENDIF()

# Remember to package this file for CMake builds.
ADD_EXTRA_DIST(${libnczarr_SOURCES} CMakeLists.txt)

