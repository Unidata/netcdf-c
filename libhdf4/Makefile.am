# This is part of Unidata's netCDF package.  Copyright 2005-2011, see
# the COPYRIGHT file for more information.

# This automake file generates the Makefile to build netCDF-4.
# Ed Hartnett

include $(top_srcdir)/lib_flags.am

libnetcdf4_la_CPPFLAGS = ${AM_CPPFLAGS}

# This is our output. The netCDF-4 convenience library.
noinst_LTLIBRARIES = libnchdf4.la
libnchdf4_la_SOURCES = hdf4dispatch.c hdf4attr.c hdf4dim.c hdf4file.c	\
hdf4grp.c hdf4type.c hdf4var.c hdf4func.c

EXTRA_DIST = CMakeLists.txt





