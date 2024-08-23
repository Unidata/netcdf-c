## This is a automake file, part of Unidata's netCDF package.
# Copyright 2019, see the COPYRIGHT file for more information.

# This file builds and runs the unit tests. These tests are not run in
# the CMake build, because we would then have to extern these internal
# functions, to allow Windows to work. Since we have not extern'd
# these functions, they will only be run under the autotools build.

# Ed Hartnett 8/9/19

#SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#sh_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#TESTS_ENVIRONMENT = export SETX=1;

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am
AM_CPPFLAGS += -I${top_srcdir} -I${top_srcdir}/libdispatch

# Find and link to the netcdf-c library.
LDADD = ${top_builddir}/liblib/libnetcdf.la

check_PROGRAMS =
TESTS =

check_PROGRAMS += tst_nclist test_ncuri test_pathcvt

# Performance tests
check_PROGRAMS += tst_exhash tst_xcache
tst_exhash_SOURCES = tst_exhash.c timer_utils.c timer_utils.h 
tst_xcache_SOURCES = tst_xcache.c timer_utils.c timer_utils.h

TESTS += tst_nclist test_ncuri test_pathcvt  tst_exhash tst_xcache

if USE_HDF5
check_PROGRAMS += tst_nc4internal tst_reclaim
TESTS += tst_nc4internal
TESTS += run_reclaim_tests.sh
endif # USE_HDF5

if NETCDF_ENABLE_S3
if NETCDF_ENABLE_S3_TESTALL
check_PROGRAMS += test_s3sdk
TESTS += run_s3sdk.sh
endif
check_PROGRAMS += aws_config
TESTS += run_aws_config.sh
endif

EXTRA_DIST = CMakeLists.txt run_s3sdk.sh run_reclaim_tests.sh
EXTRA_DIST += nctest_netcdf4_classic.nc reclaim_tests.cdl

CLEANFILES = reclaim_tests*.txt reclaim_tests.nc

# If valgrind is present, add valgrind targets.
@VALGRIND_CHECK_RULES@
