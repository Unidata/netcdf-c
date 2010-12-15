## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This file builds and runs the nc_test program, which tests the
# netCDF-3 API for all formats.

# $Id: Makefile.am,v 1.36 2010/05/29 00:40:23 dmh Exp $

AM_CPPFLAGS = -I$(top_srcdir)/include
LDADD = -lm

# Link to the netCDF-4 and the HDF5 libraries.

if BUILD_DAP

LDADD += ${top_builddir}/liblib/libnetcdf.la @EXTERN_LDFLAGS@
AM_CPPFLAGS += -I$(top_builddir)/liblib

# Set up the tests; do the .sh first, then .c
check_PROGRAMS = 
TESTS = 
TESTS_ENVIRONMENT=TOPSRCDIR=${abs_top_srcdir}

TESTS += tst_ncdap3.sh

if USE_NETCDF4
TESTS += tst_ncdap4.sh
endif # USE_NETCDF4

# remote tests are optional
# because the server may be down
if ENABLE_DAP_REMOTE_TESTS
TESTS += tst_remote3.sh
if ENABLE_DAP_LONG_TESTS
TESTS += tst_longremote3.sh
endif

if USE_NETCDF4
TESTS += tst_remote4.sh
if ENABLE_DAP_LONG_TESTS
TESTS += tst_longremote4.sh
endif
endif

endif #REMOTE_TESTS

# Do the programmatic tests last; all are part of the remote test set
if ENABLE_DAP_REMOTE_TESTS

# Conversion tests

if USE_NETCDF4
test_cvt4_SOURCES = test_cvt.c
check_PROGRAMS += test_cvt4
TESTS += test_cvt4
else
test_cvt3_SOURCES = test_cvt.c
test_varm3_SOURCES = test_varm3.c
check_PROGRAMS += test_cvt3 test_varm3
TESTS += test_cvt3 test_varm3
endif

endif ENABLE_DAP_REMOTE_TESTS

# Need to add subdirs
SUBDIRS = testdata3 expected3 expected4 expectremote3 expectremote4

EXTRA_DIST = tst_ncdap3.sh tst_ncdap4.sh \
             tst_remote3.sh tst_remote4.sh \
             tst_longremote3.sh tst_longremote4.sh \
             tst_ncdap.sh tst_ncdap_shared.sh tst_remote.sh

CLEANFILES = test_varm3 test_cvt3 results/*.dmp results/*.das results/*.dds datadds*

if USE_NETCDF4
CLEANFILES += test_cvt4
endif

endif # BUILD_DAP

