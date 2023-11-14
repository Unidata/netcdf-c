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
zxcache.c
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
zfilter.h
zdebug.h
)

IF(ENABLE_NCZARR_ZIP)
SET(libnczarr_SOURCES ${libnczarr_SOURCES} zmap_zip.c)
ENDIF()

IF(ENABLE_S3) 
SET(libnczarr_SOURCES ${libnczarr_SOURCES} zmap_s3sdk.c)
ENDIF()

# Build the Zarr dispatch layer as a library that will be included in
# the netCDF library.

add_library(nczarr OBJECT ${libnczarr_SOURCES})

TARGET_INCLUDE_DIRECTORIES(nczarr PUBLIC ../libncpoco)

IF(STATUS_PARALLEL)
    target_link_libraries(nczarr PUBLIC MPI::MPI_C)
ENDIF(STATUS_PARALLEL)

# Remember to package this file for CMake builds.
ADD_EXTRA_DIST(${libnczarr_SOURCES} CMakeLists.txt)
