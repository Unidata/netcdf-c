## This is a automake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

AM_TESTS_ENVIRONMENT += export NCZARRFORMAT=2;export NCNOZMETADATA=1;
TESTS_ENVIRONMENT =
TEST_EXTENSIONS = .sh
SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
sh_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
TESTS_ENVIRONMENT += export SETX=1;

AM_CFLAGS += -I${top_srcdir}
AM_CPPFLAGS += -I${top_srcdir}
AM_CPPFLAGS += -I${top_srcdir}/libnczarr
AM_CPPFLAGS += -I${top_srcdir}/plugins
LDADD = ${top_builddir}/liblib/libnetcdf.la
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la
AM_CXXFLAGS = 

EXTRA_DIST =
DISTCLEANFILES =
TESTS =
check_PROGRAMS =

if NETCDF_BUILD_UTILITIES

# Test fill issues
TESTS += run_nczarr_fill.sh

# Test various corrupted files
TESTS += run_corrupt.sh

if NETCDF_ENABLE_EXTERNAL_SERVER_TESTS
TESTS += run_external.sh
endif

endif #BUILD_UTILITIES

testcommonsrc = test_utils.c test_utils.h
commonsrc = ut_util.c ut_test.c ut_includes.h ut_test.h ut_util.h test_nczarr_utils.h

check_PROGRAMS += ut_map ut_mapapi ut_json
ut_map_SOURCES = ut_map.c ${commonsrc}
ut_mapapi_SOURCES = ut_mapapi.c ${commonsrc}
ut_json_SOURCES = ut_json.c ${commonsrc}

test_fillonlyz_SOURCES = test_fillonlyz.c ${testcommonsrc}

check_PROGRAMS += test_fillonlyz test_quantize test_notzarr

# Unlimited Dimension tests
if USE_HDF5
test_put_vars_two_unlim_dim_SOURCES = test_put_vars_two_unlim_dim.c ${testcommonsrc}
check_PROGRAMS += test_zchunks test_zchunks2 test_zchunks3 test_unlim_vars test_put_vars_two_unlim_dim
check_PROGRAMS += test_unlim_io
test_unlim_io_SOURCES = test_unlim_io.c ${testcommonsrc}
TESTS += test_put_vars_two_unlim_dim
endif

if NETCDF_BUILD_UTILITIES

TESTS += run_ut_misc.sh
TESTS += run_ut_map.sh
TESTS += run_ut_mapapi.sh

if AX_DISABLE # Obsolete tests
TESTS += run_ut_chunk.sh
endif

TESTS += run_ncgen4.sh run_nan.sh

TESTS += run_quantize.sh
TESTS += run_purezarr.sh
TESTS += run_misc.sh
TESTS += run_jsonconvention.sh
TESTS += run_strings.sh
TESTS += run_scalar.sh
TESTS += run_nulls.sh
TESTS += run_notzarr.sh
TESTS += run_consolidated_zarr.sh

if NETCDF_ENABLE_EXTERNAL_SERVER_TESTS
TESTS += run_external.sh
endif

# Unlimited Dimension tests (at least in part)
TESTS += run_mud.sh
TESTS += test_unlim_vars

if USE_HDF5
# Unlimited Dimension tests
check_PROGRAMS += test_chunking
TESTS += run_nccopy5.sh
endif

if USE_HDF5
TESTS += run_unlim_io.sh run_nccopyz.sh
endif

if LARGE_FILE_TESTS
check_PROGRAMS += test_writecaching test_readcaching
TESTS += run_cachetest.sh
endif

endif #NETCDF_BUILD_UTILITIES

if NETCDF_BUILD_UTILITIES 

if LARGE_FILE_TESTS
check_PROGRAMS += test_chunkcases
test_chunkcases_SOURCES = test_chunkcases.c ${testcommonsrc}
TESTS += run_chunkcases.sh
endif

check_PROGRAMS += test_h5_endians
TESTS += test_h5_endians

if USE_HDF5
TESTS += run_fillonlyz.sh
endif

if BUILD_BENCHMARKS

UTILSRC = bm_utils.c timer_utils.c test_utils.c bm_utils.h bm_timer.h

bm_chunks3_SOURCES = bm_chunks3.c ${UTILSRC}

check_PROGRAMS += bm_chunks3

# The perf tests need modernization
if AX_DISABLE
TESTS += run_perf_chunks1.sh
endif

endif # BUILD_BENCHMARKS

if NETCDF_ENABLE_NCZARR_FILTERS
if NETCDF_ENABLE_FILTER_TESTING

# Test interoperability of V2 files
TESTS += run_interop.sh

check_PROGRAMS += test_filter_avail test_filter_vlen
TESTS += run_filter_vlen.sh run_filter_misc.sh

# Do simple filter functionality tests for nczarr
check_PROGRAMS += test_nczfilter
TESTS += run_nczfilter.sh

# Echo filter tests from nc_test4
check_PROGRAMS += testfilter testfilter_misc testfilter_order testfilter_repeat testfilter_multi
TESTS += run_filter.sh 
TESTS += run_specific_filters.sh

if AX_DISABLE
# This test is too dangerous to run in a parallel make environment.
# It causes race conditions. So suppress and only test by hand.
TESTS += run_unknown.sh
endif

if ISMINGW
XFAIL_TESTS = run_filter.sh run_specific_filters.sh run_nczfilter.sh
endif  # ISMINGW

endif #NETCDF_ENABLE_FILTER_TESTING
endif #NETCDF_ENABLE_NCZARR_FILTERS

endif #NETCDF_BUILD_UTILITIES

# These programs are used by the test cases
noinst_PROGRAMS = zhex
zhex_SOURCES = zhex.c
noinst_PROGRAMS += zisjson
zisjson_SOURCES = zisjson.c
noinst_PROGRAMS += zmapio
zmapio_SOURCES = zmapio.c
noinst_PROGRAMS += zs3parse
zs3parse_SOURCES = zs3parse.c

if NETCDF_ENABLE_S3
noinst_PROGRAMS += s3util 
s3util_SOURCES = s3util.c
if NETCDF_ENABLE_S3_TESTALL
if NETCDF_ENABLE_S3_AWS
check_PROGRAMS += tst_pure_awssdk
tst_pure_awssdk_SOURCES = tst_pure_awssdk.cpp
AM_CXXFLAGS += -std=c++11
endif 	
endif
endif

if USE_HDF5
# Given a netcdf4|NCZarr file, dump the actual chunk contents.
# Used to validate nczarr chunking code.
AM_CPPFLAGS += -I$(top_srcdir)/libnczarr
noinst_PROGRAMS += ncdumpchunks
ncdumpchunks_SOURCES = ncdumpchunks.c
endif

CLEANFILES = ut_*.txt ut*.cdl tmp*.nc tmp*.cdl tmp*.txt tmp*.dmp tmp*.zip tmp*.nc tmp*.dump tmp*.tmp tmp*.zmap tmp_ngc.c ref_zarr_test_data.cdl tst_*.nc.zip ref_quotes.zip ref_power_901_constants.zip \
tmp_*.nc tmp_*.zarr tst_quantize*.zarr rcmiscdir ref_power_901_constants.file

# *** WARNING *** Most of the files in nczarr_test have been duplicated into v3_nczarr_test.
# If any file here is changed, then it may need to be propagated to the other copy.

EXTRA_DIST += CMakeLists.txt test_nczarr.sh

EXTRA_DIST += run_chunkcases.sh run_corrupt.sh run_external.sh run_fillonlyz.sh run_filterinstall.sh run_filter_misc.sh run_filter.sh run_filter_vlen.sh run_interop.sh run_jsonconvention.sh run_misc.sh run_mud.sh run_nan.sh run_nccopy5.sh run_nccopyz.sh run_ncgen4.sh run_nczarr_fill.sh run_nczarr_fill.sh run_nczfilter.sh run_notzarr.sh run_nulls.sh run_perf_chunks1.sh run_purezarr.sh run_quantize.sh run_scalar.sh run_specific_filters.sh run_strings.sh run_unknown.sh run_unlim_io.sh run_ut_mapapi.sh run_ut_map.sh run_ut_misc.sh run_consolidated_zarr.sh

EXTRA_DIST += bm_utils.c test_h5_endians.c test_put_vars_two_unlim_dim.c test_put_vars_two_unlim_dim.c test_unlim_vars.c test_utils.c timer_utils.c bm_timer.h bm_utils.h timer_utils.h

EXTRA_DIST += ref_noshape.file.zip ref_power_901_constants.cdl ref_power_901_constants_orig.zip ref_avail1.cdl ref_avail1.dmp ref_avail1.txt ref_byte.cdl ref_byte_fill_value_null.cdl ref_byte_fill_value_null.zarr.zip ref_byte.zarr.zip ref_fillonly.cdl ref_groups.h5 ref_misc1.cdl ref_misc1.dmp ref_ndims.cdl ref_ndims.dmp ref_notzarr.tar.gz ref_nulls.cdl ref_perdimspecs.cdl ref_purezarr_base.cdl ref_quotes.cdl ref_quotes_orig.zip ref_rem.cdl ref_rem.dmp ref_scalar.cdl ref_scalar_nczarr.cdl ref_skip.cdl ref_skip.txt ref_skipw.cdl ref_string.cdl ref_t_meta_dim1.cdl ref_t_meta_var1.cdl ref_ut_json_build.txt ref_ut_json_parse.txt ref_ut_mapapi_create.cdl ref_ut_mapapi_data.cdl ref_ut_mapapi_meta.cdl ref_ut_mapapi_search.txt ref_ut_map_create.cdl ref_ut_map_readmeta2.txt ref_ut_map_readmeta.txt ref_ut_map_search.txt ref_ut_map_writedata.cdl ref_ut_map_writemeta2.cdl ref_ut_map_writemeta.cdl ref_ut_proj.txt ref_ut_testmap_create.cdl ref_whole.cdl ref_whole.txt ref_zarr_test_data.cdl.gz ref_bzip2.cdl ref_filtered.cdl ref_purezarr.cdl ref_jsonconvention.cdl ref_groups_regular.cdl ref_zarr_test_data_2d.cdl.gz ref_misc2.cdl ref_string_zarr.baseline ref_string_nczarr.baseline ref_nulls_zarr.baseline ref_nulls_nczarr.baseline ref_any.cdl ref_xarray.cdl ref_jsonconvention.zmap ref_nczarr2zarr.cdl ref_multi.cdl ref_tst_mud4.cdl ref_tst_mud4-bc.cdl ref_tst_mud4_chars.cdl ref_tst_nans.dmp ref_cmip6.zmap ref_consolidated_zarr.cdl ref_consolidated_zarr_base.cdl ref_consolidated_zarr_2.18.2_python.zarr.tgz

OLDFORMAT = ref_oldformat.cdl ref_oldformat.zip ref_oldformat_only_consolidated.zip run_newformat.sh ref_newformatpure.cdl 

DISTCLEANFILES += findplugin.sh

clean-local:
	rm -fr tmp*.file
if NETCDF_ENABLE_S3_TESTALL
	rm -fr testdir_* testset_*
	rm -fr tmp_*.nc tmp_*.zarr tst_quantize*.zarr tmp*.file results.file results.s3 results.zip
	rm -fr rcmiscdir ref_power_901_constants.file 
endif

if NETCDF_ENABLE_S3_TESTALL
check-local:
	bash ${abs_top_builddir}/s3cleanup.sh
endif

# If valgrind is present, add valgrind targets.
@VALGRIND_CHECK_RULES@
