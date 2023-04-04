## This is a automake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.

# This file builds and runs DAP4 tests.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

TESTS_ENVIRONMENT =

#TEST_EXTENSIONS = .sh

#SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#sh_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#TESTS_ENVIRONMENT += export SETX=1;

AM_CPPFLAGS += -I${top_srcdir} -I${top_srcdir}/libnczarr
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la
LDADD = ${top_builddir}/liblib/libnetcdf.la

# Set up the tests; do the .sh first, then .c
check_PROGRAMS =
TESTS =

check_PROGRAMS += ut_map ut_mapapi ut_json ut_projections ut_chunking

commonsrc = ut_util.c ut_test.c ut_includes.h ut_test.h ut_util.h test_nczarr_utils.h
tstcommonsrc = tst_utils.c tst_utils.h

ut_map_SOURCES = ut_map.c ${commonsrc}
ut_mapapi_SOURCES = ut_mapapi.c ${commonsrc}
ut_json_SOURCES = ut_json.c ${commonsrc}
ut_projections_SOURCES = ut_projections.c ${commonsrc}
ut_chunking_SOURCES = ut_chunking.c ${commonsrc}
tst_fillonlyz_SOURCES = tst_fillonlyz.c ${tstcommonsrc}

check_PROGRAMS += tst_zchunks tst_zchunks2 tst_zchunks3 tst_fillonlyz test_quantize 

TESTS += run_ut_chunk.sh

if BUILD_UTILITIES

TESTS += run_ut_map.sh
TESTS += run_ut_mapapi.sh
TESTS += run_ut_misc.sh
TESTS += run_ncgen4.sh

if USE_HDF5
#TESTS += run_nccopyz.sh
TESTS += run_fillonlyz.sh
endif

check_PROGRAMS += tst_chunkcases
tst_chunkcases_SOURCES = tst_chunkcases.c ${tstcommonsrc}
TESTS += run_chunkcases.sh

TESTS += run_quantize.sh
TESTS += run_purezarr.sh
TESTS += run_interop.sh
TESTS += run_misc.sh
TESTS += run_nczarr_fill.sh
TESTS += run_jsonconvention.sh
TESTS += run_strings.sh
TESTS += run_scalar.sh
TESTS += run_nulls.sh
endif

if BUILD_UTILITIES 

if ENABLE_NCZARR_ZIP
TESTS += run_newformat.sh
endif

if BUILD_BENCHMARKS

UTILSRC = bm_utils.c timer_utils.c tst_utils.c bm_utils.h bm_timer.h

bm_chunks3_SOURCES = bm_chunks3.c ${UTILSRC}

check_PROGRAMS += bm_chunks3

# The perf tests need modernization
if AX_IGNORE
TESTS += run_perf_chunks1.sh
endif

endif # BUILD_BENCHMARKS

if ENABLE_FILTER_TESTING

# Do simple filter functionality tests for nczarr
check_PROGRAMS += tst_nczfilter
TESTS += run_nczfilter.sh

# Echo filter tests from nc_test4
check_PROGRAMS += testfilter testfilter_misc testfilter_order testfilter_repeat testfilter_multi test_filter_avail
TESTS += run_filter.sh 
TESTS += run_unknown.sh
TESTS += run_specific_filters.sh

if ISMINGW
XFAIL_TESTS = run_filter.sh run_unknown.sh run_specific_filters.sh
endif  # ISMINGW

endif #ENABLE_FILTER_TESTING

endif #BUILD_UTILITIES

# These programs are used by the test cases
noinst_PROGRAMS = zmapio
zmapio_SOURCES = zmapio.c
noinst_PROGRAMS += zhex
zhex_SOURCES = zhex.c
noinst_PROGRAMS += zisjson
zisjson_SOURCES = zisjson.c
noinst_PROGRAMS += zs3parse
zs3parse_SOURCES = zs3parse.c

if ENABLE_S3
noinst_PROGRAMS += s3util
s3util_SOURCES = s3util.c
TESTS += run_s3_cleanup.sh
if ISCYGWIN
XFAIL_TESTS = run_s3_cleanup.sh
endif # ISCYGWIN
endif

# Given a netcdf4|NCZarr file, dump the actual chunk contents.
# Used to validate nczarr chunking code.
AM_CPPFLAGS += -I$(top_srcdir)/libnczarr
noinst_PROGRAMS += ncdumpchunks
ncdumpchunks_SOURCES = ncdumpchunks.c

EXTRA_DIST = CMakeLists.txt \
run_ut_map.sh run_ut_mapapi.sh run_ut_misc.sh run_ut_chunk.sh run_ncgen4.sh \
run_nccopyz.sh run_fillonlyz.sh run_chunkcases.sh test_nczarr.sh run_perf_chunks1.sh run_s3_cleanup.sh \
run_purezarr.sh run_interop.sh run_misc.sh \
run_filter.sh \
run_newformat.sh run_nczarr_fill.sh run_quantize.sh \
run_jsonconvention.sh run_nczfilter.sh run_unknown.sh \
run_scalar.sh run_strings.sh run_nulls.sh

EXTRA_DIST += \
ref_ut_map_create.cdl ref_ut_map_writedata.cdl ref_ut_map_writemeta2.cdl ref_ut_map_writemeta.cdl \
ref_ut_map_readmeta.txt ref_ut_map_readmeta2.txt ref_ut_map_search.txt \
ref_ut_mapapi_create.cdl ref_ut_mapapi_data.cdl ref_ut_mapapi_meta.cdl ref_ut_mapapi_search.txt \
ref_t_meta_dim1.cdl ref_t_meta_var1.cdl \
ref_ut_json_build.txt ref_ut_json_parse.txt \
ref_ut_proj.txt ref_ut_testmap_create.cdl \
ref_perdimspecs.cdl ref_fillonly.cdl \
ref_whole.cdl ref_whole.txt \
ref_skip.cdl ref_skip.txt ref_skipw.cdl \
ref_rem.cdl ref_rem.dmp ref_ndims.cdl  ref_ndims.dmp \
ref_misc1.cdl ref_misc1.dmp ref_misc2.cdl \
ref_avail1.cdl ref_avail1.dmp ref_avail1.txt \
ref_xarray.cdl ref_purezarr.cdl ref_purezarr_base.cdl ref_nczarr2zarr.cdl \
ref_bzip2.cdl ref_filtered.cdl ref_multi.cdl \
ref_any.cdl ref_oldformat.cdl ref_oldformat.zip ref_newformatpure.cdl \
ref_groups.h5 ref_byte.zarr.zip ref_byte_fill_value_null.zarr.zip \
ref_groups_regular.cdl ref_byte.cdl ref_byte_fill_value_null.cdl \
ref_jsonconvention.cdl ref_jsonconvention.zmap \
ref_string.cdl ref_string_nczarr.baseline ref_string_zarr.baseline ref_scalar.cdl \
ref_nulls_nczarr.baseline ref_nulls_zarr.baseline ref_nulls.cdl

# Interoperability files
EXTRA_DIST += ref_power_901_constants_orig.zip ref_power_901_constants.cdl ref_quotes_orig.zip ref_quotes.cdl \
ref_zarr_test_data.cdl.gz

CLEANFILES = ut_*.txt ut*.cdl tmp*.nc tmp*.cdl tmp*.txt tmp*.dmp tmp*.zip tmp*.nc tmp*.dump tmp*.tmp tmp_ngc.c ref_zarr_test_data.cdl tst_*.nc.zip ref_quotes.zip ref_power_901_constants.zip

BUILT_SOURCES = test_quantize.c test_filter_avail.c run_specific_filters.sh run_filterinstall.sh run_unknown.sh
test_quantize.c: $(top_srcdir)/nc_test4/tst_quantize.c
	rm -f $@
	echo "#define TESTNCZARR" > $@
	cat $(top_srcdir)/nc_test4/tst_quantize.c >> $@

test_filter_avail.c: $(top_srcdir)/nc_test4/tst_filter_avail.c
	rm -f $@
	echo "#define TESTNCZARR" > $@
	cat $(top_srcdir)/nc_test4/tst_filter_avail.c >> $@

run_unknown.sh: $(top_srcdir)/nc_test4/tst_unknown.sh
	rm -f $@ run_unknown.tmp
	echo "#!/bin/bash" > run_unknown.tmp
	echo "TESTNCZARR=1" >> run_unknown.tmp
	cat $(top_srcdir)/nc_test4/tst_unknown.sh >> run_unknown.tmp
	tr -d '\r' < run_unknown.tmp > $@
	chmod a+x $@
	rm -f run_unknown.tmp

run_specific_filters.sh: $(top_srcdir)/nc_test4/tst_specific_filters.sh
	rm -f $@ run_specific_filters.tmp
	echo "#!/bin/bash" > run_specific_filters.tmp
	echo "TESTNCZARR=1" >> run_specific_filters.tmp
	cat $(top_srcdir)/nc_test4/tst_specific_filters.sh >> run_specific_filters.tmp
	tr -d '\r' < run_specific_filters.tmp > $@
	chmod a+x $@
	rm -f run_specific_filters.tmp

run_filterinstall.sh: $(top_srcdir)/nc_test4/tst_filterinstall.sh
	rm -f $@ run_filterinstall.tmp
	echo "#!/bin/bash" > run_filterinstall.tmp
	echo "TESTNCZARR=1" >> run_filterinstall.tmp
	cat $(top_srcdir)/nc_test4/tst_filterinstall.sh >> run_filterinstall.tmp
	tr -d '\r' < run_filterinstall.tmp > $@
	chmod a+x $@
	rm -f run_filterinstall.tmp

# Remove directories
clean-local:
	rm -fr tmp_*.nc tmp_*.zarr tst_quantize*.zarr tmp*.file results.file results.s3 results.zip
	rm -fr rcmiscdir ref_power_901_constants.file 


DISTCLEANFILES = findplugin.sh test_quantize.c run_specific_filters.sh run_filterinstall.sh run_unknown.sh test_filter_avail.c

# If valgrind is present, add valgrind targets.
@VALGRIND_CHECK_RULES@
