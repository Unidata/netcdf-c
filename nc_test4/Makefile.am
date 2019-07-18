# This is part of the netCDF package. Copyright 2005-2019 University
# Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
# conditions of use.
#
# This directory holds tests for netCDF-4. It is skipped if netCDF-4
# is not enabled.
#
# Ed Hartnett, Ward Fisher

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# Un comment to use a more verbose test driver
#SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose

TEST_EXTENSIONS = .sh

# Link to our assembled library.
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la
LDADD = ${top_builddir}/liblib/libnetcdf.la

# These are netCDF-4 C test programs which are built and run.
NC4_TESTS = tst_dims tst_dims2 tst_dims3 tst_files tst_files4		\
tst_vars tst_varms tst_unlim_vars tst_converts tst_converts2 tst_grps	\
tst_grps2 tst_compounds tst_compounds2 tst_compounds3 tst_opaques	\
tst_strings tst_strings2 tst_interops tst_interops4 tst_interops5	\
tst_interops6 tst_enums tst_coords tst_coords2 tst_coords3 tst_vars3	\
tst_vars4 tst_chunks tst_chunks2 tst_utf8 tst_fills tst_fills2		\
tst_fillbug tst_xplatform tst_xplatform2 tst_endian_fill tst_atts	\
t_type cdm_sea_soundings tst_camrun tst_vl tst_atts1 tst_atts2		\
tst_vars2 tst_files5 tst_files6 tst_sync tst_h_scalar tst_rename	\
tst_rename2 tst_rename3 tst_h5_endians tst_atts_string_rewrite		\
tst_hdf5_file_compat tst_fill_attr_vanish tst_rehash tst_filterparser	\
tst_bug324 tst_types tst_atts3 tst_put_vars tst_elatefill tst_udf	\
tst_put_vars_two_unlim_dim tst_bug1442

# Temporary I hoped, but hoped in vain.
if !ISCYGWIN
NC4_TESTS += tst_h_strbug tst_h_refs
endif

# Build test programs plus programs used in test scripts.
check_PROGRAMS = $(NC4_TESTS) tst_empty_vlen_unlim
TESTS = $(NC4_TESTS) run_empty_vlen_test.sh

# Add these if large file tests are turned on.
if LARGE_FILE_TESTS
check_PROGRAMS += tst_large tst_large2
TESTS += tst_large tst_large2
endif

# If the v2 API was built, add its test program.
if BUILD_V2
check_PROGRAMS += tst_v2
TESTS += tst_v2
endif # BUILD_V2

# The rest of the tests require ncgen/ncdump.
if BUILD_UTILITIES
check_PROGRAMS += renamegroup
TESTS += run_grp_rename.sh tst_misc.sh

# Szip Tests (requires ncdump)
if USE_SZIP
check_PROGRAMS += test_szip h5testszip
TESTS += tst_szip.sh
endif

# Filter Tests (requires ncdump and ncgen)
if ENABLE_FILTER_TESTING
extradir =
extra_PROGRAMS = test_filter test_filter_misc
check_PROGRAMS += test_filter_reg
TESTS += tst_filter.sh test_filter_reg
endif
endif # BUILD_UTILITIES

# This are extra tests that will only be run if netcdf-4 is configured
# with --enable-parallel-tests.
if TEST_PARALLEL4
check_PROGRAMS += tst_mpi_parallel tst_parallel tst_parallel3		\
tst_parallel4 tst_parallel5 tst_nc4perf tst_mode tst_simplerw_coll_r	\
tst_mode
TESTS += run_par_test.sh
endif

EXTRA_DIST = run_par_test.sh.in ref_tst_compounds.nc			\
ref_tst_xplatform2_1.nc ref_tst_xplatform2_2.nc ref_tst_dims.nc		\
ref_tst_interops4.nc CMakeLists.txt run_grp_rename.sh			\
run_empty_vlen_test.sh ref_hdf5_compat1.nc ref_hdf5_compat2.nc		\
ref_hdf5_compat3.nc tst_misc.sh tdset.h5 tst_szip.sh ref_szip.h5	\
ref_szip.cdl tst_filter.sh bzip2.cdl ref_filtered.cdl			\
ref_unfiltered.cdl ref_bzip2.c findplugin.in ref_unfilteredvv.cdl	\
ref_filteredvv.cdl

CLEANFILES = tst_mpi_parallel.bin cdm_sea_soundings.nc bm_chunking.nc	\
tst_floats_1D.cdl floats_1D_3.nc floats_1D.cdl tst_*.nc			\
tst_floats2_*.cdl tst_ints2_*.cdl tst_shorts2_*.cdl tst_elena_*.cdl	\
tst_simple*.cdl tst_chunks.cdl pr_A1.* tauu_A1.* usi_01.* thetau_01.*	\
tst_*.h5 tst_grp_rename.cdl tst_grp_rename.dmp ref_grp_rename.cdl	\
foo1.nc tst_*.h4 test.nc testszip.nc test.h5 szip_dump.cdl		\
perftest.txt bigmeta.nc bigvars.nc *.gz MSGCPP_*.nc	                \
floats*.nc floats*.cdl shorts*.nc shorts*.cdl ints*.nc ints*.cdl        \
testfilter_reg.nc

DISTCLEANFILES = findplugin.sh run_par_test.sh

# If valgrind is present, add valgrind targets.
@VALGRIND_CHECK_RULES@
