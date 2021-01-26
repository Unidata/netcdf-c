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

check_PROGRAMS += tst_zchunks tst_zchunks2 tst_zchunks3 tst_fillonlyz

TESTS += run_ut_map.sh
TESTS += run_ut_mapapi.sh
TESTS += run_ut_misc.sh
TESTS += run_ut_chunk.sh

if BUILD_UTILITIES

if USE_HDF5
TESTS += run_nccopyz.sh
TESTS += run_fillonlyz.sh
endif

TESTS += run_ncgen4.sh

check_PROGRAMS += tst_chunkcases
tst_chunkcases_SOURCES = tst_chunkcases.c ${tstcommonsrc}
TESTS += run_chunkcases.sh

endif

if BUILD_BENCHMARKS
if BUILD_UTILITIES 

UTILSRC = bm_utils.c timer_utils.c

bm_chunks3_SOURCES = bm_chunks3.c ${UTILSRC}

check_PROGRAMS += bm_chunks3

TESTS += run_perf_chunks1.sh

endif # BUILD_BENCHMARKS

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

# Given a netcdf4|NCZarr file, dump the actual chunk contents.
# Used to validate nczarr chunking code.
AM_CPPFLAGS += -I$(top_srcdir)/libnczarr
noinst_PROGRAMS += ncdumpchunks
ncdumpchunks_SOURCES = ncdumpchunks.c

EXTRA_DIST = CMakeLists.txt \
run_ut_map.sh run_ut_mapapi.sh run_ut_misc.sh run_ut_chunk.sh run_ncgen4.sh \
run_nccopyz.sh run_fillonlyz.sh run_chunkcases.sh test_nczarr.sh run_perf_chunks1.sh \
ref_ut_map_readmeta_nzf.txt \
ref_ut_map_writedata_nzf.cdl \
ref_ut_map_writemeta_nzf.cdl \
ref_ut_map_writemeta_nz4.cdl \
ref_ut_map_writedata_nz4.cdl \
ref_ut_map_write2meta_nzf.cdl \
ref_ut_map_write2meta_nz4.cdl \
ref_ut_map_search_nzf.txt \
ref_ut_map_search_nz4.txt \
ref_ut_map_readmeta_nz4.txt \
ref_ut_map_readdata_nz4.txt \
ref_ut_map_create_nz4.cdl \
ref_ut_json_parse.txt \
ref_ut_json_build.txt \
ref_t_meta_var1.cdl \
ref_t_meta_dim1.cdl \
ref_ut_testmap_create_nz4.cdl \
ref_ut_proj.txt \
ref_ut_map_create_nzf.cdl \
ref_ut_mapapi_create_nz4.cdl \
ref_ut_mapapi_create_nzf.cdl \
ref_ut_mapapi_create_s3.cdl \
ref_ut_mapapi_data_nz4.cdl \
ref_ut_mapapi_data_nzf.cdl \
ref_ut_mapapi_data_s3.cdl \
ref_ut_mapapi_meta_nz4.cdl \
ref_ut_mapapi_meta_nzf.cdl \
ref_ut_mapapi_meta_s3.cdl \
ref_ut_mapapi_search_nz4.txt \
ref_ut_mapapi_search_nzf.txt \
ref_ut_mapapi_search_s3.txt \
ref_perdimspecs.cdl ref_fillonly.cdl \
ref_whole.cdl ref_whole.txt \
ref_skip.cdl ref_skip.txt ref_skipw.cdl \
ref_rem.cdl ref_rem.dmp ref_ndims.cdl  ref_ndims.dmp \
ref_misc1.cdl ref_misc1.dmp \
ref_avail1.cdl ref_avail1.dmp ref_avail1.txt

CLEANFILES = test.nzf test.nz4 test.nz4 \
testmap.nzf testmap.nz4 testmapapi.nzf testmapapi.nz4 \
tst_chunks.nz4 tst_chunks2.nc.nz4 \
ut_*.txt ut*.cdl tmp*.nc tmp*.cdl tmp*.txt tmp*.dmp test.nzf \
tst_chunks3.nc

# Remove directories
clean-local:
	rm -fr tst_chunks.nzf tst_chunks2.nzf tst_perdimspecs.nzf testmap.nzf test.nzf tmp*.nzf \
	test_mapapi.nzf test_mapapi.nz4 tmp*.nz4
