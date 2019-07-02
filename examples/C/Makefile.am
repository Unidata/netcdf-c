# This is a automake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.

# This file builds the C examples.

# Ed Hartnett, Ward Fisher, Dennis Heimbigner

# Un comment to use a more verbose test driver
#SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose

LDADD = -lm
AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CPPFLAGS += -I$(top_builddir)/liblib
AM_LDFLAGS = ${top_builddir}/liblib/libnetcdf.la

# These are the netCDF-3 examples.
check_PROGRAMS = simple_xy_wr simple_xy_rd sfc_pres_temp_wr	\
sfc_pres_temp_rd pres_temp_4D_wr pres_temp_4D_rd format		\
sfc_pres_temp_more simple

TESTS = run_examples.sh format sfc_pres_temp_more simple

if LARGE_FILE_TESTS
check_PROGRAMS += quick_large_files quick_small_files
TESTS += quick_large_files quick_small_files
endif # LARGE_FILE_TESTS

# If HDF5 is being used, build the netcdf-4/HDF5 examples.
if USE_HDF5
check_PROGRAMS += simple_nc4_wr simple_nc4_rd simple_xy_nc4_wr	\
simple_xy_nc4_rd
TESTS += run_examples4.sh

if ENABLE_FILTER_TESTING
# filter_example.c should be same as nc_test4/test_filter.c
check_PROGRAMS += filter_example
TESTS += run_filter.sh
endif

endif #USE_HDF5

if TEST_PARALLEL4
if USE_PNETCDF
# This is a pnetcdf example.
check_PROGRAMS += parallel_vara
TESTS += run_par_test.sh
endif #USE_PNETCDF
endif # TEST_PARALLEL4

# These files are created by the tests.
CLEANFILES = *.nc

EXTRA_DIST = CMakeLists.txt run_examples.sh run_examples4.sh	\
run_filter.sh

DISTCLEANFILES = findplugin.sh
