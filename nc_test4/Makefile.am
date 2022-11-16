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
#sh_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#TESTS_ENVIRONMENT = export SETX=1;

TEST_EXTENSIONS = .sh

# Link to our assembled library.
LDADD = ${top_builddir}/liblib/libnetcdf.la

# These are netCDF-4 C test programs which are built and run.
NC4_TESTS = tst_dims tst_dims2 tst_dims3 tst_files tst_files4		\
tst_vars tst_varms tst_unlim_vars tst_converts tst_converts2 tst_grps	\
tst_grps2 tst_compounds tst_compounds2 tst_compounds3 tst_opaques	\
tst_strings tst_strings2 tst_interops tst_interops4 tst_interops5	\
tst_interops6 tst_interops_dims tst_enums tst_coords tst_coords2	\
tst_coords3 tst_vars3 tst_vars4 tst_chunks tst_chunks2 tst_utf8		\
tst_fills tst_fills2 tst_fillbug tst_xplatform tst_xplatform2		\
tst_endian_fill tst_atts t_type cdm_sea_soundings tst_camrun tst_vl	\
tst_atts1 tst_atts2 tst_vars2 tst_files5 tst_files6 tst_sync		\
tst_h_scalar tst_rename tst_rename2 tst_rename3 tst_h5_endians		\
tst_atts_string_rewrite tst_hdf5_file_compat tst_fill_attr_vanish	\
tst_rehash tst_filterparser tst_bug324 tst_types tst_atts3		\
tst_put_vars tst_elatefill tst_udf tst_put_vars_two_unlim_dim		\
tst_bug1442 tst_quantize

if HAS_PAR_FILTERS
NC4_TESTS += tst_alignment
endif

NC4_TESTS += tst_h_strbug tst_h_refs

# Build test programs plus programs used in test scripts.
check_PROGRAMS = $(NC4_TESTS) tst_empty_vlen_unlim tst_charvlenbug tst_vlenstr
TESTS = $(NC4_TESTS) run_empty_vlen_test.sh
XFAIL_TESTS =

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

check_PROGRAMS += tst_fillonly
TESTS += test_fillonly.sh

# H5 and nczarr Fixed string support
TESTS += tst_fixedstring.sh

# Szip Tests (requires ncdump)
if HAVE_H5Z_SZIP
check_PROGRAMS += test_szip h5testszip
TESTS += tst_szip.sh
endif

# Filter Tests (requires ncdump and ncgen and HDF5)
if USE_HDF5
if ENABLE_FILTER_TESTING
extradir =
check_PROGRAMS += test_filter test_filter_misc test_filter_order test_filter_repeat test_filter_vlen
check_PROGRAMS += tst_multifilter tst_filter_avail
TESTS += tst_filter.sh
TESTS += tst_specific_filters.sh
TESTS += tst_bloscfail.sh
if ISMINGW
XFAIL_TESTS += tst_filter.sh
endif # ISMINGW
# This test is too dangerous to run in a parallel make environment.
# It causes race conditions. So suppress and only test by hand.
#TESTS += tst_unknown.sh
endif # ENABLE_FILTER_TESTING
endif # USE_HDF5
endif # BUILD_UTILITIES

# This are extra tests that will only be run if netcdf-4 is configured
# with --enable-parallel-tests.
if TEST_PARALLEL4
check_PROGRAMS += tst_mpi_parallel tst_parallel tst_parallel3		\
tst_parallel4 tst_parallel5 tst_nc4perf tst_mode tst_simplerw_coll_r	\
tst_mode tst_parallel_zlib tst_parallel_compress tst_quantize_par	\
tst_parallel6
TESTS += run_par_test.sh
endif # TEST_PARALLEL4

EXTRA_DIST = run_par_test.sh.in ref_tst_compounds.nc			\
ref_tst_xplatform2_1.nc ref_tst_xplatform2_2.nc ref_tst_dims.nc		\
ref_tst_interops4.nc CMakeLists.txt run_grp_rename.sh			\
run_empty_vlen_test.sh ref_hdf5_compat1.nc ref_hdf5_compat2.nc		\
ref_hdf5_compat3.nc tst_misc.sh tdset.h5 tst_szip.sh ref_szip.h5	\
ref_szip.cdl tst_filter.sh bzip2.cdl noop.cdl ref_filtered.cdl		\
ref_unfiltered.cdl ref_bzip2.c findplugin.in ref_unfilteredvv.cdl	\
ref_filteredvv.cdl ref_multi.cdl                                        \
ref_ncgenF.cdl ref_nccopyF.cdl                                          \
ref_filter_repeat.txt ref_fillonly.cdl test_fillonly.sh                 \
ref_filter_order_create.txt ref_filter_order_read.txt			\
ref_any.cdl tst_specific_filters.sh tst_unknown.sh                      \
tst_virtual_datasets.c noop1.cdl unknown.cdl  \
tst_broken_files.c ref_bloscx.cdl tst_bloscfail.sh			\
tst_fixedstring.sh ref_fixedstring.h5 ref_fixedstring.cdl

# The tst_filterinstall test can only be run after an install
# occurred with --with-plugin-dir enabled. So there is no point
#in running it via make check. It is kept here so it can be
# manually invoked if desired
EXTRA_DIST += tst_filterinstall.sh

CLEANFILES = tst_mpi_parallel.bin cdm_sea_soundings.nc bm_chunking.nc	\
tst_floats_1D.cdl floats_1D_3.nc floats_1D.cdl tst_*.nc tmp_*.txt       \
tst_floats2_*.cdl tst_ints2_*.cdl tst_shorts2_*.cdl tst_elena_*.cdl	\
tst_simple*.cdl tst_chunks.cdl pr_A1.* tauu_A1.* usi_01.* thetau_01.*	\
tst_*.h5 tst_grp_rename.cdl tst_grp_rename.dmp ref_grp_rename.cdl	\
foo1.nc tst_*.h4 test.nc testszip.nc test.h5 szip_dump.cdl		\
perftest.txt bigmeta.nc bigvars.nc *.gz MSGCPP_*.nc	                \
floats*.nc floats*.cdl shorts*.nc shorts*.cdl ints*.nc ints*.cdl        \
testfilter_reg.nc filterrepeat.txt tmp_fillonly.nc                      \
testfilter_order.nc crfilterorder.txt rdfilterorder.txt 1               \
tmp_*.txt tmp_*.nc tmp*.dump tmp*.cdl tmp*.txt tmp*.tmp                 \
tmp_bzip2.c bzip2.nc noop.nc tmp_*.dmp tmp_*.cdl

DISTCLEANFILES = findplugin.sh run_par_test.sh

# If valgrind is present, add valgrind targets.
@VALGRIND_CHECK_RULES@

# The (otherwise unused) program build_fixedstring.c
# is used to generate the test file ref_fixedstring.h5.
# That test file is build and included as part of the distribution,
# so the build_fixedstring.c program generally does not need to
# be executed unless the test file needs to be modified..

check_PROGRAMS += build_fixedstring
ref_fixedstring.h5: build_fixedstring.c
	${srcdir}/buildfixedstring
