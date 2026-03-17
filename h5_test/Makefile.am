# This is part of the netCDF package.
# Copyright 2018 University Corporation for Atmospheric Research/Unidata
# See COPYRIGHT file for conditions of use.
#
# This directory contains tests that only use HDF5; these tests don't
# use netCDF at all. These tests will only be run if netcdf-4 is
# enabled.
#
# If one of these tests fails, then netCDF-4 will not work correctly.
# Ed Hartnett, Ward Fisher

# Set AM_CPPFLAGS and AM_LDFLAGS based on user choices.
include $(top_srcdir)/lib_flags.am

# Un comment to use a more verbose test driver
# SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
# sh_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
# LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
# TESTS_ENVIRONMENT = export SETX=1;

TEST_EXTENSIONS = .sh

# These programs test HDF5 to make sure it will do (some of) the stuff
# that netCDF-4 needs.
H5TESTS = tst_h_files tst_h_files2 tst_h_files4 tst_h_atts		\
tst_h_atts3 tst_h_atts4 tst_h_vars tst_h_vars2 tst_h_vars3 tst_h_grps	\
tst_h_compounds tst_h_compounds2 tst_h_wrt_cmp tst_h_vl tst_h_opaques	\
tst_h_strings tst_h_strings1 tst_h_strings2 tst_h_ints			\
tst_h_dimscales tst_h_dimscales1 tst_h_dimscales2 tst_h_dimscales3	\
tst_h_enums tst_h_dimscales4 tst_h_rename

# If benchmarks were turned on, build and run more tests.
if BUILD_BENCHMARKS
H5TESTS += tst_h_mem
endif # BUILD_BENCHMARKS

# List of tests to compile.
check_PROGRAMS = $(H5TESTS)

# List of tests to run.
TESTS = $(H5TESTS)

# tst_h5_open_close measures RSS growth over repeated H5Fopen/H5Fclose cycles
# using pure HDF5 (no netCDF) to isolate whether any open/close memory growth
# originates inside HDF5 itself (issue #2626).  HDF5 versions before 2.0.0
# exhibit RSS growth in H5Fopen/H5Fclose that exceeds the test threshold and
# would cause false failures -- the growth is a known HDF5 issue, not something
# fixable in netCDF-C.  The RSS measurement uses getrusage()/ru_maxrss which
# is only reliable on Linux -- on macOS ru_maxrss is the peak (high-water mark)
# RSS for the entire process lifetime and never decreases, making comparisons
# useless.  Restrict test execution to Linux with HDF5 >= 2.0.0.
# The binary is always built so it can be run manually.
check_PROGRAMS += tst_h5_open_close
if HDF5_VERSION_GE_200
if HOST_IS_LINUX
TESTS += tst_h5_open_close
endif
endif

# If zstd is enabled, run a test for it.
if NETCDF_ENABLE_PLUGINS
if HAVE_ZSTD
check_PROGRAMS += tst_h_zstd
TESTS += run_h5_zstd_tests.sh
endif # HAVE_ZSTD
endif # NETCDF_ENABLE_PLUGINS

# The parallel program is run from a script.
if TEST_PARALLEL4
check_PROGRAMS += tst_h_par tst_h_par_compress
TESTS += run_par_tests.sh
endif

# We must include these files in the distribution.
EXTRA_DIST = run_par_tests.sh.in ref_tst_h_compounds.h5		\
ref_tst_h_compounds2.h5 run_par_tests.sh ref_tst_compounds.nc	\
h5_err_macros.h CMakeLists.txt run_h5_zstd_tests.sh		\
tst_h5_open_close.c

# Clean up test results.
CLEANFILES = tst_h_*.h5

DISTCLEANFILES = run_par_test.sh findplugin.sh

