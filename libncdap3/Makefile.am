## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This automake file generates the Makefile to build netCDF-4. The
# generated makefile is not run unless the user selected to build
# netCDF-4.

# $Id: Makefile.am,v 1.41 2010/05/29 18:45:47 dmh Exp $

# We may have to add to these later.
DISTCLEANFILES =
MAINTAINERCLEANFILES =
CLEANFILES =
LDADD=

AM_CPPFLAGS = -I$(top_srcdir)/include

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
cache.c \
dapdump.c \
dapdebug.c \
dapattr3.c \
cetab.c \
ceparse.c \
celex.c \
ncd3dispatch.c ncdap3.c ncdap3a.c getvara3.c

HDRS=\
nccommon.h \
ncconstraints.h \
constraints3.h \
ncd3dispatch.h \
ncdap3.h \
dapalign.h \
dapodom.h \
getvara.h \
dapnc.h \
daputil.h \
dapdebug.h \
dapdump.h \
ceparselex.h \
cetab.h

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

AM_CPPFLAGS += -I${top_srcdir}/oc

AM_CPPFLAGS += @EXTERN_CPPFLAGS@

LDADD +=  ${top_builddir}/libncdap3/libncdap3.la \
	  ${top_builddir}/oc/liboc.la \
	  ${top_builddir}/libdispatch/libdispatch.la

LDADD += @EXTERN_LDFLAGS@

# Add a trivial test case to check for undefined references
check_PROGRAMS = t_dap3a t_dap3c
CLEANFILES += t_dap3a t_dap3c
TESTS_ENVIRONMENT=TOPSRCDIR=${abs_top_srcdir}
TESTS = t_dap3a t_dap3c
t_dap3a_SOURCES = t_dap3a.c stubdap3.c
t_dap3c_SOURCES = t_dap3c.c stubdap3.c
# Set ldflags
t_dap3a_LDFLAGS = ${top_builddir}/libsrc/libnetcdf3.la ${top_builddir}/libdispatch/libdispatch.la
t_dap3c_LDFLAGS = ${t_dap3a_LDFLAGS}
EXTRA_DIST = ce.y t_dap.c

endif # BUILD_DAP


# This rule are used if someone wants to rebuild t_dap3a.c
# Otherwise never invoked, but records how to do it.
t_dap3a.c: ${srcdir}/t_dap.c
	echo "#define NETCDF3ONLY" > t_dap3a.c
	cat t_dap.c >> t_dap3a.c

# One last thing
BUILT_SOURCES = .dodsrc

.dodsrc:
	echo "#DODSRC" >.dodsrc

DISTCLEANFILES += .dodsrc


