## This is an automake file, part of Unidata's netCDF package.
# Copyright 2017, see the COPYRIGHT file for more information.

# This automake file is in charge of building the libpio directory,
# which contains the code which calls the PIO parallel IO library.

include $(top_srcdir)/lib_flags.am

libnetcdfpio_la_CPPFLAGS = ${AM_CPPFLAGS}

# This is the code for a dispatch table for PIO

# Turn on a pre-processor flag when building a DLL for windows.
if BUILD_DLL
libnetcdfpio_la_CPPFLAGS += -DDLL_EXPORT
endif # BUILD_DLL

# These files comprise the PIO dispatch library code.
libnetcdfpio_la_SOURCES = piodispatch.c

noinst_LTLIBRARIES = libnetcdfpio.la

EXTRA_DIST = CMakeLists.txt 
