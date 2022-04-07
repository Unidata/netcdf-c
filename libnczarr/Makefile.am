# This is part of Unidata's netCDF package. Copyright 2018, see the
# COPYRIGHT file for more information.

# Build the ZARR dispatch layer.

# Dennis Heimbigner

# Get AM_CPPFLAGS and AM_LDFLAGS
include $(top_srcdir)/lib_flags.am

AM_CPPFLAGS += -D_LARGEFILE_SOURCE
AM_CPPFLAGS += -I$(top_srcdir)/libsrc4 -I$(top_srcdir)/libncpoco
libnczarr_la_CPPFLAGS = ${AM_CPPFLAGS}
AM_CXXFLAGS = 
libnczarr_la_LIBADD =
libnczarr_la_LDFLAGS =

# Remember ENABLE_NCZARR implies USE_NETCDF4

# We may have to add to these later.
DISTCLEANFILES =
MAINTAINERCLEANFILES =
CLEANFILES =
EXTRA_DIST = 
LDADD=

# This is our output. The ZARR convenience library.
noinst_LTLIBRARIES = libnczarr.la

# The source files.
libnczarr_la_SOURCES = \
zarr.c \
zattr.c \
zxcache.c \
zchunking.c \
zclose.c \
zcreate.c \
zcvt.c \
zdim.c \
zdispatch.c \
zfile.c \
zgrp.c \
zinternal.c \
zmap.c \
zmap_file.c \
zodom.c \
zopen.c \
zprov.c \
zsync.c \
ztype.c \
zutil.c \
zvar.c \
zwalk.c \
zdebug.c \
zarr.h \
zcache.h \
zchunking.h \
zdispatch.h \
zincludes.h \
zinternal.h \
zmap.h \
zodom.h \
zprovenance.h \
zfilter.h \
zdebug.h

if ENABLE_NCZARR_ZIP
libnczarr_la_SOURCES += zmap_zip.c 
endif

if ENABLE_NCZARR_FILTERS
libnczarr_la_SOURCES += zfilter.c
endif

if ENABLE_S3_SDK
libnczarr_la_SOURCES += zmap_s3sdk.c
AM_CXXFLAGS += -std=c++11
endif

# For now, ignore these
IGNORED=ztype.c 

# Package this for cmake build.
EXTRA_DIST += CMakeLists.txt
