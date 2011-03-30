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
SRC=nccr.c nccrdispatch.c nccrgetvara.c nccrmeta.c nccrproto.c \
crdebug.c crutil.c \
nccrdump.c \
ncstreamx.c curlwrap.c ncaux.c \
ast_byteio.c ast_curl.c ast_internal.c \
ast_runtime.c ast_util.c ast_debug.c

HDRS=nccrconstraints.h nccrdispatch.h nccr.h mccrmeta.h nccrproto.h \
crdebug.h crutil.h \
nccrdump.h \
ncstreamx.h curlwrap.h ncaux.h \
ast.h ast_byteio.h ast_curl.h ast_internal.h \
ast_runtime.h ast_util.h ast_debug.h

if BUILD_CDMREMOTE

noinst_LTLIBRARIES = libnccr.la

# Build convenience library
libnccr_la_SOURCES = $(SRC) $(HDRS)

AM_CPPFLAGS += -I$(top_srcdir)/libsrc4 \
	      -I$(top_srcdir)/libsrc \
	      -I$(top_srcdir)/libdispatch

AM_CPPFLAGS += -I${top_srcdir}/libdispatch @EXTERN_CPPFLAGS@

# Define the load libraries for stub3
LDADD += ${top_builddir}/libcdmr/libnccr.la \
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

##################################################
# Generate the augmented ncstream structures
EXTRA_DIST=ncStream.proto ncstream.h ncstreamc augmenth.sed augmentc.sed

ncstreamx.h: ncstream.h augmenth.sed
	rm -f ncstreamx.h
	sed -f augmenth.sed <ncstream.h >ncstreamx.h

ncstreamx.c: ncstream.c augmentc.sed
	rm -f ncstreamx.c
	sed -f augmentc.sed <ncstream.c >ncstreamx.c

ncstream.c ncstream.h: ncStream.proto
	java -jar ast.jar -I. ncStream.proto

ncStream.proto ast.jar:
	cp f:/IdeaProjects/ast/src/compiler/ncStream.proto .
	cp f:/IdeaProjects/ast/src/compiler/ast.jar .

endif # BUILD_CDMREMOTE


