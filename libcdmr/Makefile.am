## This is a automake file, part of Unidata's netCDF package.
# Copyright 2010, see the COPYRIGHT file for more information.

# This builds the netcdf-4 version of the opendap client.

# $Id: Makefile.am,v 1.18 2010/05/29 18:45:47 dmh Exp $

DISTCLEANFILES =
CLEANFILES =
LDADD=
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS = 

# CDMREMORE Sources
SRC=curlwrap.c nccr.c nccrdispatch.c nccrgetvara.c nccrmeta.c \
crdebug.c crutil.c

HDRS=curlwrap.h nccrconstraints.h nccrdispatch.h nccr.h crdebug.h

if BUILD_CDMREMOTE

noinst_LTLIBRARIES = libnccr.la

# Build convenience library
libnccr_la_SOURCES = $(SRC) $(HDRS)

AM_CPPFLAGS += -I$(top_srcdir)/libsrc4 \
	      -I$(top_srcdir)/libsrc \
	      -I$(top_srcdir)/libdispatch

AM_CPPFLAGS += -I${top_srcdir}/libdispatch @EXTERN_CPPFLAGS@

# Define the load libraries for stub3
LDADD += ${top_builddir}/libncdap4/libnccr.la \
	 ${top_builddir}/libsrc4/libnetcdf4.la \
         ${top_builddir}/libsrc/libnetcdf3.la \
         ${top_builddir}/libdispatch/libdispatch.la

if BUILD_V2
LDADD += ${top_builddir}/libdispatch/libnetcdf2.la 
endif # BUILD_V2

LDADD += @EXTERN_LDFLAGS@

# Add a trivial test case to check for undefined references
check_PROGRAMS = t_cdmr
TESTS = t_cdmr
TESTS_ENVIRONMENT=TOPSRCDIR=${abs_top_srcdir}
t_cdmr_SOURCES = t_cdmr.c nccrstub.c
CLEANFILES += t_cdmr

endif # BUILD_CDMREMOTE


