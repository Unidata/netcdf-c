## This is a automake file, part of Unidata's netCDF package.
# Copyright 2010, see the COPYRIGHT file for more information.

# This Makefile assembles the correct libnetcdf based on various
# configure flags. It is assumed that all the relevant convenience
# libraries have been built (e.g. libsrc, libsrc4, libncdap3,
# libncdap4, fortran).

AM_LDFLAGS = @EXTERN_LDFLAGS@
AM_CPPFLAGS = -I$(top_srcdir)/include -I${top_srcdir}/libsrc	\
-I${top_srcdir}/libdispatch

# This is our output, the netcdf library the user will install.
lib_LTLIBRARIES = libnetcdf.la

libnetcdf_la_LIBADD = 

# The v2 API...
if BUILD_V2
libnetcdf_la_LIBADD += ${top_builddir}/libdispatch/libnetcdf2.la
endif # BUILD_V2

# The output library will always include netcdf3 and dispatch libraries.
libnetcdf_la_LIBADD += ${top_builddir}/libdispatch/libdispatch.la	\
${top_builddir}/libsrc/libnetcdf3.la

# netcdf3 + dap
if USE_DAP
AM_CPPFLAGS += -I${top_srcdir}/libncdap3 -I${top_srcdir}/oc
libnetcdf_la_LIBADD += ${top_builddir}/libncdap3/libncdap3.la
if INTERNAL_OCLIB
libnetcdf_la_LIBADD += ${top_builddir}/oc/liboc.la
endif # INTERNAL_OCLIB
endif # USE_DAP

# NetCDF-4 ...
if USE_NETCDF4
AM_CPPFLAGS += -I${top_srcdir}/libsrc4
libnetcdf_la_LIBADD += ${top_builddir}/libsrc4/libnetcdf4.la

# netcdf4+dap
if USE_DAP
libnetcdf_la_LIBADD += ${top_builddir}/libncdap4/libncdap4.la
endif # USE_DAP
endif #USE_NETCDF4

if BUILD_F77
if ! BUILD_SEPARATE_FORTRAN
libnetcdf_la_LIBADD += ${top_builddir}/fortran/libnetcdff.la
endif #!BUILD_SEPARATE_FORTRAN
endif #BUILD_F77

# We need at least one source file
libnetcdf_la_SOURCES = stub.c 

.PHONY: visualstudio
visualstudio:
	sh ${abs_top_srcdir}/visualbuild add ${abs_srcdir} "${libnetcdf_la_SOURCES}"

