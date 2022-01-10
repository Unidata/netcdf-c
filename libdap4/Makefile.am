## This is a automake file, part of Unidata's netCDF package.
# Copyright 2008, see the COPYRIGHT file for more information.

# This automake file generates the Makefile to libdap2. Whatever that
# is!

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am
AM_CPPFLAGS += -I$(top_srcdir)/libncxml
libdap4_la_CPPFLAGS = ${AM_CPPFLAGS}

# We may have to add to these later.
DISTCLEANFILES =
MAINTAINERCLEANFILES =
CLEANFILES =
EXTRA_DIST = CMakeLists.txt

LDADD=

#d4curlflags.c

SRC= \
d4curlfunctions.c \
d4fix.c \
d4data.c \
d4file.c \
d4parser.c \
d4meta.c \
d4varx.c \
d4dump.c \
d4swap.c \
d4chunk.c \
d4printer.c \
d4read.c \
d4http.c \
d4util.c \
d4odom.c \
d4cvt.c \
d4debug.c \
ncd4dispatch.c

HDRS= \
ncd4dispatch.h \
ncd4types.h \
ncd4.h \
d4chunk.h \
d4http.h \
d4read.h \
d4curlfunctions.h \
d4util.h \
d4debug.h \
d4odom.h \
d4bytes.h \
d4includes.h

if ENABLE_DAP4
if USE_NETCDF4
AM_CPPFLAGS += -I$(top_srcdir)/libsrc4
endif

# Build convenience library
noinst_LTLIBRARIES = libdap4.la
libdap4_la_SOURCES = $(SRC) $(HDRS)
libdap4_la_CPPFLAGS += $(AM_CPPFLAGS)
libdap4_la_LIBADD =

endif # ENABLE_DAP4
