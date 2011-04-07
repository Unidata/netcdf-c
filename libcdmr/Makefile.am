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
ncStreamx.c curlwrap.c ncaux.c \
ast_byteio.c ast_curl.c ast_internal.c \
ast_runtime.c ast_util.c ast_debug.c \
crce.c

HDRS=nccrdispatch.h nccr.h mccrmeta.h nccrproto.h \
crdebug.h crutil.h \
nccrdump.h \
ncStreamx.h curlwrap.h ncaux.h \
ast.h ast_byteio.h ast_curl.h ast_internal.h \
ast_runtime.h ast_util.h ast_debug.h \
crce.h

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
EXTRA_DIST=ncStream.proto ncStream.h ncStreamc

ASTDIR=./ast/src/compiler

ASTJAR=${ASTDIR}/ast.jar

ncStreamx.c ncStreamx.h: ncStreamx.proto ncStream.proto ${ASTJAR}
	java -jar ${ASTJAR} -I. ncStreamx.proto

.PHONEY: jar

${ASTJAR}: 
	cd ${ASTDIR}; make -f Make0 jar


endif # BUILD_CDMREMOTE
