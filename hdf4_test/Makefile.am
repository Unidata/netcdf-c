# This is part of the netCDF package.
# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.
# See netcdf-c/COPYRIGHT file for more info.
#
# This directory has tests for the HDF4 code. This directory will be
# skipped if HDF4 is not enabled.
#
# Ed Hartnett

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# Link to our assembled library.
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la

# These are the C tests for HDF4.
check_PROGRAMS = tst_chunk_hdf4 tst_h4_lendian tst_hdf4_extra
TESTS = tst_chunk_hdf4 tst_h4_lendian tst_hdf4_extra

# This test script depends on ncdump and tst_interops2.c.
if NETCDF_BUILD_UTILITIES
check_PROGRAMS += tst_interops2
TESTS += run_formatx_hdf4.sh
endif # NETCDF_BUILD_UTILITIES

# This test script fetches HDF4 files from an FTP server and uses
# program tst_interops3.c to read them.
if USE_HDF4_FILE_TESTS
check_PROGRAMS += tst_interops3
TESTS += run_get_hdf4_files.sh
endif # USE_HDF4_FILE_TESTS

EXTRA_DIST = CMakeLists.txt ref_contiguous.hdf4 ref_chunked.hdf4	\
run_get_hdf4_files.sh run_formatx_hdf4.sh

CLEANFILES = *.h4 *.hdf

# If valgrind is present, add valgrind targets.
@VALGRIND_CHECK_RULES@
