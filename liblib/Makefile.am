## This is a automake file, part of Unidata's netCDF package.
# Copyright 2010, see the COPYRIGHT file for more information.

# This Makefile assembles the correct libnetcdf based on various
# configure flags. It is assumed that all the relevant dispatch
# libraries have been built. (e.g libsrc, libsrc4, libncdap3,
# libncdap4, fortran).

# $Id: Makefile.am,v 1.13 2010/06/01 15:34:50 ed Exp $

AM_LDFLAGS = @EXTERN_LDFLAGS@
AM_CPPFLAGS = -I$(top_srcdir)/include

# This is our output, the netcdf library the user will install.
lib_LTLIBRARIES = libnetcdf.la

# The output library will always include netcdf3 and dispatch libraries.
libnetcdf_la_LIBADD = ${top_builddir}/libdispatch/libdispatch.la	\
${top_builddir}/libsrc/libnetcdf3.la

# We need at least one source file
libnetcdf_la_SOURCES = stub.c 

if USE_NETCDF4
AM_CPPFLAGS += -I${top_srcdir}/libsrc4
endif

if USE_DAP
AM_CPPFLAGS += -I${top_srcdir}/libncdap3 -I${top_srcdir}/oc
endif

AM_CPPFLAGS += -I${top_srcdir}/libsrc -I${top_srcdir}/libdispatch

if USE_DAP
# netcdf3 + dap
libnetcdf_la_LIBADD += ${top_builddir}/libncdap3/libncdap3.la
endif

# The v2 API...
if BUILD_V2
libnetcdf_la_LIBADD += ${top_builddir}/libdispatch/libnetcdf2.la
endif

if USE_NETCDF4
libnetcdf_la_LIBADD += ${top_builddir}/libsrc4/libnetcdf4.la
if USE_DAP
# netcdf4+dap
libnetcdf_la_LIBADD += ${top_builddir}/libncdap4/libncdap4.la
endif
endif #USE_NETCDF4

if BUILD_F77
if ! BUILD_SEPARATE_FORTRAN
libnetcdf_la_LIBADD += ${top_builddir}/fortran/libnetcdff.la
endif #!BUILD_SEPARATE_FORTRAN
endif #BUILD_F77




