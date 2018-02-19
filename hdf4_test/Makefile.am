# This is part of the netCDF package.
# Copyright 2018 University Corporation for Atmospheric Research/Unidata
# See COPYRIGHT file for conditions of use.
#
# This directory has tests for the HDF4 code. This directory will be
# skipped if HDF4 is not enabled.
#
# Ed Hartnett

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# Note which tests depend on other tests. necessary for make -j check
TEST_EXTENSIONS = .sh

# Link to our assembled library.
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la

# These are the tests for HDF4.
check_PROGRAMS = tst_chunk_hdf4 tst_h4_lendian
TESTS = tst_chunk_hdf4 tst_h4_lendian

if BUILD_UTILITIES
# This test script depends on ncdump.
check_PROGRAMS += tst_interops2 
TESTS += tst_formatx_hdf4.sh
endif # BUILD_UTILITIES

if USE_HDF4_FILE_TESTS
check_PROGRAMS += tst_interops3
TESTS += run_get_hdf4_files.sh
endif # USE_HDF4_FILE_TESTS

EXTRA_DIST = CMakeLists.txt ref_contiguous.hdf4 ref_chunked.hdf4	\
run_get_hdf4_files.sh tst_formatx_hdf4.sh

CLEANFILES = *.h4 *.hdf

