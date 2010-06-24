## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This automake file generates the Makefile to build netCDF-4. The
# generated makefile is not run unless the user selected to build
# netCDF-4.

# $Id: Makefile.am,v 1.1 2010/05/23 21:05:33 dmh Exp $

# Cause C preprocessor to search current and parent directory.
AM_CPPFLAGS =  -I$(top_srcdir) -I$(top_srcdir)/libsrc

# Linker needs to search current directory when building tests.
AM_LDFLAGS = 

CURL_CFLAGS = @CURL_CFLAGS@
CURL_LIBS = @CURL_LIBS@

# OC Sources
SRC=\
oc.c \
occontent.c \
ocdata.c \
ocdebug.c \
ocdump.c \
oclog.c \
ocnode.c \
ocutil.c \
occompile.c \
dapparse.c \
daplex.c \
dapurl.c \
http.c \
read.c \
curlfunctions.c \
ocinternal.c \
oclist.c \
ocbytes.c \
ocdrno.c \
ocxdr_stdio.c \
occlientparams.c \
rc.c

HDRS=\
oc.h \
constraints.h \
curlfunctions.h \
dapparselex.h \
dapurl.h \
http.h \
ocbytes.h \
occlientparams.h \
occontent.h \
ocdata.h \
ocdatatypes.h \
ocdebug.h \
ocdump.h \
ocinternal.h \
oclist.h \
oclog.h \
ocnode.h \
ocutil.h \
rc.h \
read.h

GRAMFILES=dap.y ce.y
# autoconf persists in removing the bison output unless it is renamed
GRAMC=daptab.c
GRAMH=daptab.h

if BUILD_DAP
noinst_LTLIBRARIES = liboc.la
# Watch out; the gram files must be first
liboc_la_SOURCES = ${GRAMC} ${GRAMH} $(SRC) $(HDRS) 
liboc_la_CPPFLAGS = $(AM_CPPFLAGS) $(CURL_CFLAGS)
liboc_la_LIBADD = $(CURL_LIBS)
endif

EXTRA_DIST=${GRAMFILES}
CLEANFILES =
DISTCLEANFILES =

#Normally not invoked unless after maintainer-clean or deliberate deletion
daptab.c daptab.h: dap.y
	rm -f dap.tab.c dap.tab.h
	bison --debug -d -p dap dap.y
	mv dap.tab.c daptab.c; mv dap.tab.h daptab.h

test: check
