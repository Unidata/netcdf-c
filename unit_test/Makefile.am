## This is a automake file, part of Unidata's netCDF package.
# Copyright 2019, see the COPYRIGHT file for more information.

# This file builds and runs the unit tests.

# Ed Hartnett 8/9/19

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la

check_PROGRAMS = tst_nclist
TESTS = tst_nclist

# If valgrind is present, add valgrind targets.
@VALGRIND_CHECK_RULES@
