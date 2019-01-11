## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This automake file generates the Makefile to libdap2. Whatever that
# is!

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# Remember ENABLE_ZARR implies USE_NETCDF4

# We may have to add to these later.
DISTCLEANFILES =
MAINTAINERCLEANFILES =
CLEANFILES =
EXTRA_DIST = CMakeLists.txt

LDADD=

SRC= \
zdispatch.c \
znc4.c \
zfile.c \
zjson.c \
zmap.c \
zmap_file.c

HDRS= \
nczdispatch.h \
ncjson.h \
nczmap.h \
ncztypes.h \
zincludes.h

if ENABLE_ZARR

AM_CPPFLAGS += -I$(top_srcdir)/libsrc4

# Build convenience library
noinst_LTLIBRARIES = libzarr.la
libzarr_la_SOURCES = $(SRC) $(HDRS)
libzarr_la_CPPFLAGS = $(AM_CPPFLAGS)
libzarr_la_LIBADD =

endif # ENABLE_ZARR
