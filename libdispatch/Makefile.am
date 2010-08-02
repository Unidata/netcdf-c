## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This Makefile controls the building of the dispatch layer of the
# netCDF library. The dispatch layer decides whether to call the
# netcdf-classic code, netcdf-4 code, nc3 dap code, or nc4 dap
# code. It also contains code that sit above the dispatch layer, like
# the v2 API.

# $Id: Makefile.am,v 1.10 2010/06/01 15:34:50 ed Exp $

# Set up include directories.
AM_CPPFLAGS = -I${top_srcdir}/include -I${top_srcdir}/liblib -I${top_srcdir}/libsrc

# This is our output, the dispatch convenience library.
noinst_LTLIBRARIES = libdispatch.la

# The source files.
libdispatch_la_SOURCES = dispatch.h parallel.c copy.c file.c dim.c	\
att.c error.c var.c dispatch.c

# Add functions only found in netCDF-4.
if USE_NETCDF4
libdispatch_la_SOURCES += nc4.c
endif # USE_NETCDF4

# Add functions needed by opendap client.
if USE_DAP
libdispatch_la_SOURCES += dap.c
AM_CPPFLAGS += -I${top_srcdir}/oc
endif # USE_DAP

# Add V2 API convenience library if needed.
if BUILD_V2
noinst_LTLIBRARIES += libnetcdf2.la 
libnetcdf2_la_SOURCES = v2i.c
endif # BUILD_V2

.PHONY: visualstudio
visualstudio:
	perl -w ${abs_top_srcdir}/win32/NET/vs add netcdf ${abs_srcdir} ${top_srcdir} "${libdispatch_la_SOURCES}"

