## This is a automake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.

# This file builds and runs DAP4 tests.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

#TEST_EXTENSIONS = .sh

#SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#sh_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#TESTS_ENVIRONMENT = export SETX=1;

AM_CFLAGS += -I${top_srcdir} -I${top_srcdir}/libnczarr
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la
LDADD = ${top_builddir}/liblib/libnetcdf.la

# Set up the tests; do the .sh first, then .c
check_PROGRAMS =
TESTS =

# Unit Tests
check_PROGRAMS += ut_map ut_mapapi ut_json ut_projections ut_chunking

commonsrc = ut_util.c ut_test.c ut_includes.h ut_test.h ut_util.h test_nczarr_utils.h

ut_map_SOURCES = ut_map.c ${commonsrc}
ut_mapapi_SOURCES = ut_mapapi.c ${commonsrc}
ut_json_SOURCES = ut_json.c ${commonsrc}
ut_projections_SOURCES = ut_projections.c ${commonsrc}
ut_chunking_SOURCES = ut_chunking.c ${commonsrc}

# Integration Tests
#check_PROGRAMS += it_meta

check_PROGRAMS += tst_chunks tst_chunks2

TESTS += run_ut_map.sh
TESTS += run_ut_mapapi.sh
TESTS += run_ut_misc.sh
TESTS += run_ut_chunk.sh

TESTS += run_it_test1.sh run_it_test2.sh

ignorec = ut_allslices.c ut_transfer.c ut_vars.c ut_walk.c
ignoresh = run_meta_tests.sh run_unittests.sh
ignoreh = ut_varstest.h

noinst_PROGRAMS = zmapio
zmapio_SOURCES = zmapio.c

EXTRA_DIST = \
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
ref_ut_mapapi_meta_s3.cdl

#CLEANFILES = \
#	ut_map*.cdl ut_map*.txt \
#        ut_mapapi*.cdl ut_mapapi*.txt
