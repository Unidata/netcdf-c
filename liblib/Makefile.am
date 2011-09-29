## This is a automake file, part of Unidata's netCDF package.
# Copyright 2010, see the COPYRIGHT file for more information.

# This Makefile assembles the correct libnetcdf based on various
# configure flags. It is assumed that all the relevant convenience
# libraries have been built (e.g. libsrc, libsrc4, libncdap3, libcdmr,
# libncdap4, fortran).

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# This is our output, the netcdf library the user will install.
lib_LTLIBRARIES = libnetcdf.la

# These linker flags specify libtool version info.
libnetcdf_la_LDFLAGS = -version-number 7:1:1 
libnetcdf_la_CPPFLAGS = ${AM_CPPFLAGS}
libnetcdf_la_LIBADD = 
CLEANFILES = 

# Turn on some extra stuff when building a DLL for windows.
if BUILD_DLL
libnetcdf_la_LDFLAGS += -no-undefined -Wl,--output-def,netcdfdll.def
libnetcdf_la_CPPFLAGS += -DDLL_EXPORT
endif # BUILD_DLL

if BUILD_F77
# These are the source files for the fortran library.
FORTRAN_SRC = fort-attio.c fort-control.c fort-dim.c		\
fort-genatt.c fort-geninq.c fort-genvar.c fort-lib.c fort-misc.c	\
fort-v2compat.c fort-vario.c fort-var1io.c fort-varaio.c fort-varmio.c	\
fort-varsio.c cfortran.h fort-lib.h ncfortran.h netcdf.inc
BUILT_SOURCES = fort-attio.c
noinst_LTLIBRARIES = libnetcdff.la
libnetcdff_la_SOURCES = ${FORTRAN_SRC}
libnetcdf_la_LIBADD += libnetcdff.la

fort-attio.c:
	F77_DIR=@F77_DIR@ ./get_fortran_src.sh

CLEANFILES += ${FORTRAN_SRC}

endif

# The v2 API...
if BUILD_V2
libnetcdf_la_LIBADD += ${top_builddir}/libdispatch/libnetcdf2.la
endif # BUILD_V2

# The output library will always include netcdf3 and dispatch
# libraries
libnetcdf_la_LIBADD += ${top_builddir}/libdispatch/libdispatch.la	\
${top_builddir}/libsrc/libnetcdf3.la					

# netcdf3 + dap
if USE_DAP
AM_CPPFLAGS += -I${top_srcdir}/libdap2 -I${top_srcdir}/oc
libnetcdf_la_LIBADD += ${top_builddir}/libdap2/libdap2.la
libnetcdf_la_LIBADD += ${top_builddir}/oc/liboc.la
endif # USE_DAP

# NetCDF-4 ...
if USE_NETCDF4

AM_CPPFLAGS += -I${top_srcdir}/libsrc4

libnetcdf_la_LIBADD += ${top_builddir}/libsrc4/libnetcdf4.la

libnetcdf_la_LIBADD += ${top_builddir}/libdiskless/libdiskless.la

# netcdf4+cdmremote
if BUILD_CDMREMOTE
libnetcdf_la_LIBADD += ${top_builddir}/libcdmr/libnccr.la
endif # BUILD_CDMREMOTE

endif #USE_NETCDF4

# We need at least one source file
libnetcdf_la_SOURCES = stub.c 
