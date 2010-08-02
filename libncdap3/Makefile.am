## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This automake file generates the Makefile to build netCDF-4. The
# generated makefile is not run unless the user selected to build
# netCDF-4.

# $Id: Makefile.am,v 1.41 2010/05/29 18:45:47 dmh Exp $

# We may have to add to these later.
DISTCLEANFILES =
CLEANFILES =
LDADD=

#CURL_CFLAGS = @CURL_CFLAGS@
#AM_CPPFLAGS = ${CURL_CFLAGS}
#AM_LDFLAGS = $(CURL_LIBS)
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS = 

# DRNO Sources
SRC=\
constraints3.c \
common34.c \
dapcvt.c \
dapalign.c \
dapodom.c \
daputil.c \
ncdaperr.c \
cdf3.c \
dapdump.c \
dapdebug.c \
dapattr3.c \
cetab.c \
ceparse.c \
celex.c \
ncd3dispatch.c ncdap3.c getvara3.c \
nchashmap.c nclist.c ncbytes.c

HDRS=\
constraints3.h \
ncdap.h \
ncd3dispatch.h \
ncdap3.h \
dapalign.h \
dapodom.h \
getvara.h \
dapnc.h \
daputil.h \
dapdebug.h \
dapdump.h \
netcdf3l.h \
ceparselex.h \
cetab.h \
nchashmap.h nclist.h ncbytes.h

if BUILD_DAP

noinst_LTLIBRARIES = libncdap3.la

# Build convenience library
libncdap3_la_SOURCES = $(SRC) $(HDRS)
libncdap3_la_CPPFLAGS = $(AM_CPPFLAGS)
libncdap3_la_LIBADD =

AM_CPPFLAGS += -I${top_srcdir}/libdispatch

if USE_NETCDF4
AM_CPPFLAGS += -I$(top_srcdir)/libsrc4 -I$(top_srcdir)/libsrc
else
AM_CPPFLAGS += -I$(top_srcdir)/libsrc
endif

if INTERNAL_OCLIB
  AM_CPPFLAGS += -I${top_srcdir}/oc
  libncdap3_la_LIBADD += ${top_builddir}/oc/liboc.la
endif # INTERNAL_OCLIB

AM_CPPFLAGS += @EXTERN_CPPFLAGS@
AM_LDFLAGS += @EXTERN_LDFLAGS@

LDADD +=  ${top_builddir}/libncdap3/libncdap3.la ${AM_LDFLAGS}

# Add a trivial test case to check for undefined references
check_PROGRAMS = t_dap
TESTS = t_dap
t_dap_SOURCES = t_dap.c stubdap3.c
t_dap_LDFLAGS = ${top_builddir}/libsrc/libnetcdf3.la	\
${top_builddir}/libdispatch/libdispatch.la


CLEANFILES += t_dap

EXTRA_DIST = ce.y

endif # BUILD_DAP

# These rule are used if someone wants to rebuild the grammar files.
# Otherwise never invoked, but records how to do it.
# BTW: note that renaming is essential because otherwise
# autoconf will forcibly delete files of the name *.tab.*
makece::
	bison -d -t -p ce ce.y
	rm -f cetab.c cetab.h
	mv ce.tab.c cetab.c
	mv ce.tab.h cetab.h

# One last thing
BUILT_SOURCES = .dodsrc
.dodsrc:
	echo "#DODSRC" >.dodsrc
DISTCLEANFILES += .dodsrc

test: check

.PHONY: visualstudio
visualstudio:
	perl -w ${abs_top_srcdir}/win32/NET/vs add netcdf ${top_srcdir} "${libncdap3_la_SOURCES}"

