## This is a automake file, part of Unidata's netCDF package.
# Copyright 2019, see the COPYRIGHT file for more information.

# This file builds and runs the unit tests. These tests are not run in
# the CMake build, because we would then have to extern these internal
# functions, to allow Windows to work. Since we have not extern'd
# these functions, they will only be run under the autotools build.

# Ed Hartnett 8/9/19

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

#SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#sh_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#TESTS_ENVIRONMENT = export SETX=1;

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

if USE_NETCDF4
check_PROGRAMS += tst_nc4internal
TESTS += tst_nc4internal
endif # USE_NETCDF4

if ENABLE_NCZARR_S3_TESTS
check_PROGRAMS += test_aws
TESTS += run_aws.sh
endif

EXTRA_DIST = CMakeLists.txt run_aws.sh
EXTRA_DIST += nctest_netcdf4_classic.nc

# If valgrind is present, add valgrind targets.
@VALGRIND_CHECK_RULES@
