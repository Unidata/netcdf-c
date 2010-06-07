## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This Makefile controls the building of the dispatch layer of the
# netCDF library. The dispatch layer decides whether to call the
# netcdf-classic code, netcdf-4 code, nc3 dap code, or nc4 dap code.

# $Id: Makefile.am,v 1.10 2010/06/01 15:34:50 ed Exp $

AM_LDFLAGS = 

# This is our output
noinst_LTLIBRARIES = libdispatch.la

# The source files.
libdispatch_la_SOURCES = netcdf.m4 dispatch.h parallel.c copy.c
if USE_NETCDF4
libdispatch_la_SOURCES += nc4.c
endif

# Include this so that user does not have to have m4.
EXTRA_DIST = netcdf.c

# Set up include directories.
AM_CPPFLAGS = -I${top_srcdir} -I${top_srcdir}/liblib -I${top_srcdir}/libsrc
if USE_DAP
AM_CPPFLAGS += -I${top_srcdir}/oc
endif

# Clean up.
MAINTAINERCLEANFILES = netcdf.c

# This tells make how to turn .m4 files into .c files.
.m4.c:
	if test -f $@ ; then rm -f $@.bak ; cp $@ $@.bak ; fi
	m4 $(AM_M4FLAGS) $(M4FLAGS) $< >$@



