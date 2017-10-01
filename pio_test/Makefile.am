# This is part of the netCDF package.
# Copyright 2017 University Corporation for Atmospheric Research/Unidata
# See COPYRIGHT file for conditions of use.
#
# This file builds the tests for the PIO parallel I/O features. This
# entire directory will be skipped if PIO is not enabled.
#

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# Link to our assembled library.
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la

# These are PIO test programs.
PIO_TESTS = test_iosystem2

check_PROGRAMS = $(PIO_TESTS)

TESTS = $(PIO_TESTS)
test_iosystem2_SOURCES = test_iosystem2.c
