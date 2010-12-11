## This is a automake file, part of Unidata's netCDF package.
# Copyright 2010, see the COPYRIGHT file for more information.

# This builds the netcdf-4 version of the opendap client.

# $Id: Makefile.am,v 1.18 2010/05/29 18:45:47 dmh Exp $

DISTCLEANFILES =
CLEANFILES =
LDADD=
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS = 

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

AM_CPPFLAGS += -I${top_srcdir}/libdispatch \
	       -I$(top_srcdir)/libncdap3 \
	       -I$(top_srcdir)/libsrc4 \
	       -I$(top_srcdir)/libsrc

AM_CPPFLAGS += -I${top_srcdir}/oc

AM_CPPFLAGS += @EXTERN_CPPFLAGS@

# Define the load libraries for stub
LDADD += ${top_builddir}/libncdap4/libncdap4.la \
         ${top_builddir}/libncdap3/libncdap3.la \
         ${top_builddir}/libsrc4/libnetcdf4.la \
         ${top_builddir}/libsrc/libnetcdf3.la \
         ${top_builddir}/libdispatch/libdispatch.la

if BUILD_V2
LDADD += ${top_builddir}/libdispatch/libnetcdf2.la 
endif # BUILD_V2

LDADD += ${top_builddir}/oc/liboc.la

LDADD += @EXTERN_LDFLAGS@

# Add a trivial test case to check for undefined references
check_PROGRAMS = t_dap
TESTS_ENVIRONMENT=TOPSRCDIR=${abs_top_srcdir}
TESTS = t_dap
t_dap_SOURCES = ../libncdap3/t_dap.c stubdap4.c
CLEANFILES += t_dap

endif # USE_NETCDF4


# One last thing
BUILT_SOURCES = .dodsrc
.dodsrc:
	echo "#DODSRC" >.dodsrc
DISTCLEANFILES += .dodsrc

endif # BUILD_DAP

test: check


