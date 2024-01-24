# Copyright 2009, UCAR/Unidata
# See the COPYRIGHT file for more information.

# Use automake or CMake for building under nix
# Use CMake for building under windows

# Get AM_CPPFLAGS and AM_LDFLAGS
include $(top_srcdir)/lib_flags.am
#AM_CPPFLAGS += -D_LARGEFILE_SOURCE
#AM_CPPFLAGS += -I$(top_srcdir)/libsrc4 -I$(top_srcdir)/libdap4
#libnczarr_la_CPPFLAGS = ${AM_CPPFLAGS}
AM_CXXFLAGS = 

# This is our output. The ZARR convenience library.
noinst_LTLIBRARIES = libncpoco.la
libncpoco_la_LIBADD =
libncpoco_la_LDFLAGS =

libncpoco_la_SOURCES = ncpoco.c ncpoco.h
if ! ISMSVC 
if ! ISMINGW 
libncpoco_la_SOURCES += cp_unix.c
else
libncpoco_la_SOURCES += cp_win32.c
endif
else
libncpoco_la_SOURCES += cp_win32.c
endif

EXTRA_DIST = CMakeLists.txt README.md COPYRIGHT

##################################################
# Testing
# check_LTLIBRARIES = libcpt.la
# libcpt_la_SOURCES = cptestlib.c
# 
# # Normally check libraries are created only as archives,
# # but we need a shared lib. This appears to do the trick
# libcpt_la_LDFLAGS = -module -avoid-version -shared -export-dynamic -no-undefined -rpath ${abs_builddir}
# 
# AM_LDFLAGS += libcpoco.la
# check_PROGRAMS = cp_test
# TESTS = cp_test
# 
