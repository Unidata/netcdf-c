## This is an automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# THis automake file is in charge of building the libsrc directory.

# $Id: Makefile.am,v 2.98 2010/06/01 15:34:50 ed Exp $

# We may have to add to these later.
CLEANFILES =
LDADD=
libnetcdf_la_LIBADD = 
noinst_LTLIBRARIES = 
AM_CPPFLAGS =
AM_LDFLAGS =
libnetcdf_la_LDFLAGS =

# The C API man page.
man_MANS = netcdf.3 

# This rule generates the C manpage.
ARGS_MANPAGE = -DAPI=C
if USE_NETCDF4
ARGS_MANPAGE += -DNETCDF4=TRUE
endif
if BUILD_DAP
ARGS_MANPAGE += -DDAP=TRUE
endif
if BUILD_PARALLEL
ARGS_MANPAGE += -DPARALLEL_IO=TRUE
endif

netcdf.3: $(top_srcdir)/man4/netcdf.m4 
	m4 $(M4FLAGS) $(ARGS_MANPAGE) $? >$@  || rm $@

# These files are part of the netCDF-3 library.
NC3_SOURCES = nc.h error3.c libvers.c \
string.c v1hpg.c fbits.h ncio.h \
onstack.h rnd.h utf8proc.c utf8proc.h utf8proc_data.h \
nclistmgr.c


if USE_DISPATCH
NC3_SOURCES += putget.m4 attr.m4 nc3dispatch.c nc3dispatch.h \
	       nc.c var.c dim.c ncx.m4 ncx.h
#else
#NC3_SOURCES += old_ncx.m4 old_putget.m4 old_attr.m4 netcdf3.h \
#	       old_nc.c old_var.c old_dim.c rename.h old_ncx.h \
#	       netcdf3l.h nctonc3.h nctolnc3.h
endif

# Does the user want to use ffio or posixio?
if USE_FFIO
NC3_SOURCES += ffio.c
else
NC3_SOURCES += posixio.c
endif


# Does  the user want the V2 API?
if BUILD_V2

# This will create a convenience library for all the netcdf-2
# functions.
noinst_LTLIBRARIES += libnetcdf2.la 

# This is the v2 API source code.
libnetcdf2_la_SOURCES = v2i.c

endif # BUILD_V2

# Define what is being built
# If the user enabled netcdf-4 in the configure, then the netcdf-3
# library will only be built as a non-installing library.
# The noinst libraries are convenience for later DAP and/or netcdf4 builds
# If using dispatch, then build libncf3.la as a noinstall library

if USE_DISPATCH

AM_CPPFLAGS += -I${top_srcdir} -I${top_srcdir}/liblib

AM_CPPFLAGS +=  -I${top_srcdir}/libsrc -I${top_srcdir}/libdispatch -I${top_srcdir}/oc

libnetcdf3_la_SOURCES = $(NC3_SOURCES)
noinst_LTLIBRARIES += libnetcdf3.la
# Define library for local test programs to use
LDADD += ${top_builddir}/libsrc/libnetcdf3.la stub3.lo \
         ${top_builddir}/libdispatch/libdispatch.la

if USE_DAP
LDADD += ${top_builddir}/oc/liboc.la
endif

else # !USE_DISPATCH

if USE_NETCDF4
  libnetcdf3_la_SOURCES = $(NC3_SOURCES)
# Define library for local test programs to use
  LDADD += ${top_builddir}/libsrc/libnetcdf3.la
  noinst_LTLIBRARIES += libnetcdf3.la
else # ! USE_NETCDF4
  libnetcdf_la_SOURCES = $(NC3_SOURCES)
# Define library for local test programs to use
  LDADD += ${top_builddir}/libsrc/libnetcdf.la
  lib_LTLIBRARIES = libnetcdf.la
endif # USE_CDF4

if BUILD_DAP
if USE_NETCDF4
libnetcdf3_la_LIBADD = ${top_builddir}/libncdap3/libncdap3.la
else
libnetcdf_la_LIBADD += ${top_builddir}/libncdap3/libncdap3.la
endif
AM_CPPFLAGS += @CURL_CFLAGS@
AM_LDFLAGS += @CURL_LIBS@
endif

endif #USE_DISPATCH

if USE_NETCDF4

# If the user specified a root for HDF5, use it.
if USE_HDF5_DIR
AM_CPPFLAGS += -I@HDF5DIR@/include
AM_LDFLAGS += -L@HDF5DIR@/lib
endif

# If the user specified a root location for ZLIB, use it.
if USE_ZLIB_DIR
AM_CPPFLAGS += -I@ZLIBDIR@/include
AM_LDFLAGS += -L@ZLIBDIR@/lib
endif

# If the user specified a root location for SZLIB, use it.
if USE_SZLIB_DIR
AM_CPPFLAGS += -I@SZLIBDIR@/include
AM_LDFLAGS += -L@SZLIBDIR@/lib
endif

else # not USE_NETCDF4

# This will cause the netcdf-3 header file to be installed.
#include_HEADERS = netcdf.h

# Include netcdf-2 convenience library.
if BUILD_V2
if ! USE_DISPATCH
libnetcdf_la_LIBADD += libnetcdf2.la
endif
endif

# If we are not building separate fortran libraries, then include
# their functions in the C library.
if !BUILD_SEPARATE_FORTRAN

# If F77 is built, include its convenience library. If F90 is built,
# it will already be part of the F77 convenience library.
if BUILD_F77
if ! USE_DISPATCH
libnetcdf_la_LIBADD += ${top_builddir}/fortran/libnetcdff.la
endif
endif

endif # !BUILD_SEPARATE_FORTRAN

# Verson for the netcdf-3 library. Don't mess with this number
# lightly!! This is not the same as the netCDF version, it is a
# completely different versioning system for shared library binary
# files, which is used in a specific way by OSs which are using shared
# libraries. Do don't dork around with it or you will break netCDF for
# shared library users, and everyone will be very confused. We are
# starting with 4 because some package distributors (and other users)
# have already build netCDF with shared libraries, and we need to have
# a larger number than any they used.
libnetcdf_la_LDFLAGS += $(AM_LDFLAGS) -version-info 4:0:0

# If building the DLL on mingw, do some extra stuff. For now, this
# works with netCDF-3 only. (Ed 12/3/6)
if BUILD_DLL

# We need extra arguments to produce the def file, which is needed by
# MS tools to create the VC++ import library. Add -avoid-version to
# avoid the version number in the .dll file name. 
libnetcdf_la_LDFLAGS += -Wl,--output-def,.libs/libnetcdf.def  -no-undefined
AM_CPPFLAGS += -DNC_DLL_EXPORT
endif # BUILD_DLL

endif # USE_NETCDF4

# These files are cleaned on developer workstations (and then rebuilt
# with m4), but they are included in the distribution so that the user
# does not have to have m4.
MAINTAINERCLEANFILES = attr.c ncx.c putget.c $(man_MANS) attrx.c putgetx.c
EXTRA_DIST = attr.c ncx.c putget.c $(man_MANS) stub3.c netcdf_par.h

# This tells make how to turn .m4 files into .c files.
.m4.c:
	m4 $(AM_M4FLAGS) $(M4FLAGS) $< >$@

test: check

