## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.
#
# This builds the libzarr dispatch layer.
#
# Dennis Heimbigner

# The source files for the HDF5 dispatch layer.
SET(libnczarr_SOURCES zattr.c zcache.c zcreate.c zdim.c zdispatch.c zfile.c zgrp.c zinternal.c zopen.c zprov.c zvar.c ztype.c zodom.c zsliceindices.c zarr.c zcvt.c zsync.c zallchunks.c zchunkio.c zclose.c zjson.c zutil.c zmap.c zmap_nc4.c zdebug.c)

# Build the Zarr dispatch layer as a library that will be included in
# the netCDF library.
add_library(nczarr OBJECT ${libnczarr_SOURCES})

# Remember to package this file for CMake builds.
ADD_EXTRA_DIST(${libnczarr_SOURCES} CMakeLists.txt)

