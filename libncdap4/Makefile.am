## This is a automake file, part of Unidata's netCDF package.
# Copyright 2010, see the COPYRIGHT file for more information.

# This builds the netcdf-4 version of the opendap client.

# $Id: Makefile.am,v 1.18 2010/05/29 18:45:47 dmh Exp $

DISTCLEANFILES =
CLEANFILES =
LDADD=
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS = 

#CURL_CFLAGS = @CURL_CFLAGS@
#AM_CPPFLAGS = ${CURL_CFLAGS}
#AM_LDFLAGS = $(CURL_LIBS)

# DRNO Sources
SRC=\
cdf4.c \
constraints4.c \
ncd4dispatch.c ncdap4.c getvara4.c

HDRS=\
constraints4.h \
ncd4dispatch.h ncdap4.h

if USE_DAP

if USE_NETCDF4

noinst_LTLIBRARIES = libncdap4.la

# Build convenience library
libncdap4_la_SOURCES = $(SRC) $(HDRS)

AM_CPPFLAGS += -I$(top_srcdir)/libncdap3 \
	      -I$(top_srcdir)/libsrc4 \
	      -I$(top_srcdir)/libsrc

if INTERNAL_OCLIB
  AM_CPPFLAGS += -I${top_srcdir}/oc
  libncdap4_la_LIBADD = ${top_builddir}/oc/liboc.la
endif # INTERNAL_OCLIB

AM_CPPFLAGS += -I${top_srcdir}/libdispatch @EXTERN_CPPFLAGS@
AM_LDFLAGS += @EXTERN_LDFLAGS@
AM_LDFLAGS += ${top_builddir}/libncdap3/libncdap3.la \
         ${top_builddir}/libsrc4/libnetcdf4.la \
         ${top_builddir}/libsrc/libnetcdf3.la \
         ${top_builddir}/libdispatch/libdispatch.la

if BUILD_V2
AM_LDFLAGS += ${top_builddir}/libdispatch/libnetcdf2.la 
endif # BUILD_V2

AM_LDFLAGS += @EXTERN_LDFLAGS@

LDADD += ${top_builddir}/libncdap4/libncdap4.la ${AM_LDFLAGS}

# Add a trivial test case to check for undefined references
check_PROGRAMS = t_dap
TESTS = t_dap
t_dap_SOURCES = t_dap.c stubdap4.c
CLEANFILES += t_dap

endif # USE_NETCDF4


# One last thing
BUILT_SOURCES = .dodsrc
.dodsrc:
	echo "#DODSRC" >.dodsrc
DISTCLEANFILES += .dodsrc

endif # BUILD_DAP

test: check

.PHONY: visualstudio
visualstudio:
	perl -w ${abs_top_srcdir}/win32/NET/vs add netcdf ${abs_srcdir} ${top_srcdir} "${libncdap4_la_SOURCES}"
