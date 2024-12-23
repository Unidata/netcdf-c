## This is a automake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

AM_TESTS_ENVIRONMENT += export NCZARRFORMAT=3;
TESTS_ENVIRONMENT =
TEST_EXTENSIONS = .sh
#SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#sh_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#TESTS_ENVIRONMENT += export SETX=1;

AM_CFLAGS += -I${top_srcdir}
AM_CPPFLAGS += -I${top_srcdir}
AM_CPPFLAGS += -I${top_srcdir}/libnczarr
AM_CPPFLAGS += -I${top_srcdir}/plugins
LDADD = ${top_builddir}/liblib/libnetcdf.la
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la
AM_CXXFLAGS = 

BUILT_SOURCES =
EXTRA_DIST =
DISTCLEANFILES =
TESTS =
check_PROGRAMS =
NCZARRFILES =

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

if NETCDF_ENABLE_NCZARR_ZIP
	TESTS += run_newformat.sh
endif

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

UTILSRC = bm_utils.c timer_utils.c test_utils.c

bm_chunks3_SOURCES = bm_chunks3.c ${UTILSRC}

check_PROGRAMS += bm_chunks3

# The perf tests need modernization
if AX_DISABLE
TESTS += run_perf_chunks1.sh
endif

endif # NETCDF_BUILD_BENCHMARKS

if NETCDF_ENABLE_NCZARR_FILTERS
if NETCDF_ENABLE_FILTER_TESTING

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
#noinst_PROGRAMS = zhex
#zhex_SOURCES = zhex.c
#noinst_PROGRAMS += zisjson
#zisjson_SOURCES = zisjson.c
#noinst_PROGRAMS += zmapio
#zmapio_SOURCES = zmapio.c
#noinst_PROGRAMS += zs3parse
#zs3parse_SOURCES = zs3parse.c

if NETCDF_ENABLE_S3
#noinst_PROGRAMS += s3util 
#s3util_SOURCES = s3util.c
if NETCDF_ENABLE_S3_TESTALL
if NETCDF_ENABLE_S3_AWS
#check_PROGRAMS += tst_pure_awssdk
#tst_pure_awssdk_SOURCES = tst_pure_awssdk.cpp
#AM_CXXFLAGS += -std=c++11
endif 	
endif
endif

if USE_HDF5
# Given a netcdf4|NCZarr file, dump the actual chunk contents.
# Used to validate nczarr chunking code.
#AM_CPPFLAGS += -I$(top_srcdir)/libnczarr
#noinst_PROGRAMS += ncdumpchunks
#ncdumpchunks_SOURCES = ncdumpchunks.c
endif

CLEANFILES = ut_*.txt ut*.cdl tmp*.nc tmp*.cdl tmp*.txt tmp*.dmp tmp*.zip tmp*.nc tmp*.dump tmp*.tmp tmp*.zmap tmp_ngc.c ref_zarr_test_data.cdl tst_*.nc.zip ref_quotes.zip ref_power_901_constants.zip

EXTRA_DIST += CMakeLists.txt v3manifest.am

include v3manifest.am

EXTRA_DIST += ${V3FILES} ${V3DATA} CMakeLists.txt

DISTCLEANFILES += findplugin.sh
DISTCLEANFILES += ${TESTFILES_NCZARR_C} ${TESTDATA_NCZARR}

# Note: v3_nczarr_test is initialized with the files specified by the
# list ${V3FILES} and ${V3DATA} in v3manifest.am. The actual
# files are kept in the file ${top_srcdir}/v3init.tgz and they
# are copied into v3_nczarr_test in configure.ac.

BUILT_SOURCES = ${TESTFILES_NCZARR_SH} ${TESTFILES_NCZARR_C} ${TESTDATA_NCZARR}

dist-hook: setup
all-local: setup

SRC = ${abs_top_srcdir}/nczarr_test
DST = ${abs_top_srcdir}/v3_nczarr_test
#DST = .

.PHONY: setup
setup ${TESTFILES_NCZARR_SH} ${TESTFILES_NCZARR_C} ${TESTDATA_NCZARR}:
	set -x ; for u in ${TESTFILES_NCZARR_SH}; do \
	cp -f ${SRC}/$$u ${DST}/$$u ; \
	chmod a+x ${DST}/$$u ; \
	done
	set -x ; for u in ${TESTFILES_NCZARR_C} ; do \
	cp -f ${SRC}/$$u ${DST}/$$u ; \
	done
	set -x; for u in ${TESTDATA_NCZARR}; do \
	cp -f ${SRC}/$$u ${DST}/$$u ; \
	done

# Remove directories
clean-local:
	rm -fr tmp_*.nc tmp_*.zarr tst_quantize*.zarr tmp*.file results.file results.s3 results.zip
	rm -fr rcmiscdir ref_power_901_constants.file 
	rm -fr v3_*.log v3_*.trs
if NETCDF_ENABLE_S3_TESTALL
	bash ${abs_top_builddir}/s3cleanup.sh
endif

# If valgrind is present, add valgrind targets.
@VALGRIND_CHECK_RULES@

##################################################
