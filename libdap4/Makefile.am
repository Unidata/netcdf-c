## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This automake file generates the Makefile to libdap2. Whatever that
# is!

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# We may have to add to these later.
DISTCLEANFILES =
MAINTAINERCLEANFILES =
CLEANFILES =
EXTRA_DIST = CMakeLists.txt

LDADD=

X=t_dmrdata.c \
t_dmrmeta.c \
t_dmrparse.c

SRC= \
d4fix.c \
d4data.c \
d4parser.c \
d4meta.c \
d4dump.c \
d4swap.c \
d4chunk.c \
d4printer.c \
d4read.c \
d4http.c \
d4util.c \
d4debug.c \
d4bytes.c \
ncd4dispatch.c \
d4file.c \
ezxml.c

HDRS= \
ncd4dispatch.h \
ncd4types.h \
d4http.h \
d4read.h \
d4curlfunctions.h \
d4util.h \
d4debug.h \
d4bytes.h \
d4includes.h

if ENABLE_DAP4
if USE_NETCDF4
AM_CPPFLAGS += -I$(top_srcdir)/libsrc4
endif

# Build convenience library
noinst_LTLIBRARIES = libdap4.la
libdap4_la_SOURCES = $(SRC) $(HDRS)
libdap4_la_CPPFLAGS = $(AM_CPPFLAGS)
libdap4_la_LIBADD =

endif # ENABLE_DAP4

# Show what is needed to insert a new version of ezxml
# primary fix: The original ezxml.[ch] uses '//' comments;
# unpack and replace with '/*..*/'
EZXML=ezxml-0.8.6.tar.gz
ezxml::
	rm -fr ./ezxml ./ezxml.[ch] ./license.txt
	tar -zxf ./${EZXML}
	sed -e 's|//\(.*\)|/*\1*/|' <ezxml/ezxml.c >./ezxml.c
	sed -e 's|//\(.*\)|/*\1*/|' <ezxml/ezxml.h >./ezxml.h
	cp ezxml/license.txt .
