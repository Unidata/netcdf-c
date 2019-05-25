# This is part of the netCDF package. Copyright 2005-2019 University
# Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
# conditions of use.
#
# This directory holds performance tests for netCDF-4. It is only
# built for netCDF-4 builds that use the --enable-benchmarks configure
# option.
#
# Ed Hartnett 3/18/19

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

TEST_EXTENSIONS = .sh

# Link to our assembled library.
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la
LDADD = ${top_builddir}/liblib/libnetcdf.la

check_PROGRAMS = tst_create_files bm_file tst_chunks3 tst_ar4		\
tst_ar4_3d tst_ar4_4d bm_many_objs tst_h_many_atts bm_many_atts		\
tst_files2 tst_files3 tst_mem tst_knmi bm_netcdf4_recs tst_wrf_reads	\
tst_attsperf bigmeta openbigmeta

bm_file_SOURCES = bm_file.c tst_utils.c
bm_netcdf4_recs_SOURCES = bm_netcdf4_recs.c tst_utils.c
bm_many_atts_SOURCES = bm_many_atts.c tst_utils.c
bm_many_objs_SOURCES = bm_many_objs.c tst_utils.c
tst_ar4_3d_SOURCES = tst_ar4_3d.c tst_utils.c
tst_ar4_4d_SOURCES = tst_ar4_4d.c tst_utils.c
tst_files2_SOURCES = tst_files2.c tst_utils.c
tst_ar4_SOURCES = tst_ar4.c tst_utils.c
tst_h_many_atts_SOURCES = tst_h_many_atts.c tst_utils.c
tst_knmi_SOURCES = tst_knmi.c tst_utils.c
tst_wrf_reads_SOURCES = tst_wrf_reads.c tst_utils.c

TESTS = tst_ar4_3d tst_create_files tst_files3 tst_mem run_knmi_bm.sh	\
tst_wrf_reads tst_attsperf perftest.sh run_tst_chunks.sh		\
run_bm_elena.sh

run_bm_elena.log: tst_create_files.log

if BUILD_UTILITIES
TESTS += run_bm_test1.sh run_bm_test2.sh

# tst_create_files creates files for other tests.
run_bm_test1.log: tst_create_files.log
run_bm_test2.log: tst_create_files.log

# This will run a parallel I/O benchmark for parallel builds.
if TEST_PARALLEL4
TESTS += run_par_bm_test.sh
run_par_bm_test.log: tst_create_files.log
endif # TEST_PARALLEL4
endif # BUILD_UTILITIES

EXTRA_DIST = run_par_bm_test.sh.in run_knmi_bm.sh	\
perftest.sh run_bm_test1.sh run_bm_test2.sh \
CMakeLists.txt

CLEANFILES = tst_*.nc bigmeta.nc bigvars.nc floats*.nc	\
floats*.cdl shorts*.nc shorts*.cdl ints*.nc ints*.cdl tst_*.cdl

DISTCLEANFILES = run_par_bm_test.sh MSGCPP_CWP_NC*.nc

# If valgrind is present, add valgrind targets.
@VALGRIND_CHECK_RULES@
