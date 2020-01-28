# This is part of Unidata's netCDF package. Copyright 2018, see the
# COPYRIGHT file for more information.

# Build the ZARR dispatch layer.

# Dennis Heimbigner

# Get AM_CPPFLAGS.
include $(top_srcdir)/lib_flags.am
AM_CPPFLAGS += -I$(top_srcdir)/libsrc4
libnczarr_la_CPPFLAGS = ${AM_CPPFLAGS}
libnczarr_la_LIBADD =

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
zattr.c \
zcache.c \
zcreate.c \
zdim.c \
zdispatch.c \
zfile.c \
zgrp.c \
zinternal.c \
zopen.c \
zprov.c \
zvar.c \
ztype.c \
zodom.c \
zsliceindices.c \
 \
zarr.c \
zcvt.c \
zsync.c \
zallchunks.c \
zchunkio.c \
zclose.c \
zjson.c \
zutil.c \
zmap.c \
zmap_nc4.c \
zdebug.c

HDRS= \
zdispatch.h \
zinternal.h \
zchunking.h \
zjson.h \
zincludes.h \
zmap.h \
zprovenance.h \
zdebug.h

# For now, ignore these
IGNORED=ztype.c 

# Package this for cmake build.
EXTRA_DIST += CMakeLists.txt






