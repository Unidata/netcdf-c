## This is an automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This automake file is in charge of building the libsrc directory,
# which contains the classic library code.

# $Id: Makefile.am,v 2.98 2010/06/01 15:34:50 ed Exp $

# We may have to add to these later.
AM_CPPFLAGS = -I$(top_srcdir)/include -I${top_srcdir}/liblib		\
-I${top_srcdir}/libsrc -I${top_srcdir}/libdispatch -I${top_srcdir}/oc	\
@EXTERN_CFLAGS@
AM_LDFLAGS = @EXTERN_LDFLAGS@

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
libnetcdf3_la_SOURCES = nc.h error3.c libvers.c string.c v1hpg.c fbits.h ncio.h	\
onstack.h rnd.h utf8proc.c utf8proc.h utf8proc_data.h nclistmgr.c	\
putget.m4 attr.m4 nc3dispatch.c nc3dispatch.h nc.c var.c dim.c ncx.m4	\
ncx.h lookup3.c

# Does the user want to use ffio or posixio?
if USE_FFIO
libnetcdf3_la_SOURCES += ffio.c
else
libnetcdf3_la_SOURCES += posixio.c
endif
noinst_LTLIBRARIES = libnetcdf3.la

# Define library for local test programs to use
LDADD = ${top_builddir}/libsrc/libnetcdf3.la stub3.lo	\
${top_builddir}/libdispatch/libdispatch.la

if USE_DAP
LDADD += ${top_builddir}/oc/liboc.la
endif

# These files are cleaned on developer workstations (and then rebuilt
# with m4), but they are included in the distribution so that the user
# does not have to have m4.
MAINTAINERCLEANFILES = attr.c ncx.c putget.c $(man_MANS) attrx.c putgetx.c
EXTRA_DIST = attr.c ncx.c putget.c $(man_MANS) stub3.c 

# This tells make how to turn .m4 files into .c files.
.m4.c:
	m4 $(AM_M4FLAGS) $(M4FLAGS) $< >$@


