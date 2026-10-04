# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

build_bin_test(bm_file tst_utils.c)
build_bin_test(tst_chunks3 tst_utils.c)
build_bin_test(tst_ar4 tst_utils.c)
build_bin_test(tst_ar4_4d tst_utils.c)
build_bin_test(bm_many_objs tst_utils.c)
build_bin_test(tst_h_many_atts tst_utils.c)
build_bin_test(bm_many_atts tst_utils.c)
build_bin_test(tst_files2 tst_utils.c)
build_bin_test(tst_knmi tst_utils.c)
build_bin_test(bm_netcdf4_recs tst_utils.c)
build_bin_test(bigmeta tst_utils.c)
build_bin_test(openbigmeta tst_utils.c)

add_bin_test(nc_perf tst_ar4_3d tst_utils.c)
add_bin_test(nc_perf tst_create_files tst_utils.c)
add_bin_test(nc_perf tst_files3 tst_utils.c)
add_bin_test(nc_perf tst_mem tst_utils.c)
add_bin_test(nc_perf tst_wrf_reads tst_utils.c)
add_bin_test(nc_perf tst_attsperf tst_utils.c)
add_bin_test(nc_perf tst_bm_rando tst_utils.c)
add_bin_test(nc_perf tst_compress tst_utils.c)

#add_sh_test(nc_perf run_knmi_bm)
add_sh_test(nc_perf perftest)
add_sh_test(nc_perf run_tst_chunks)
add_sh_test(nc_perf run_bm_elena)

IF(NETCDF_BUILD_UTILITIES)
add_sh_test(nc_perf run_bm_test1)
add_sh_test(nc_perf run_bm_test2)

# This will run a parallel I/O benchmark for parallel builds.
IF(TEST_PARALLEL4)
add_bin_test(nc_perf tst_compress_par tst_utils.c)
add_sh_test(nc_perf run_par_bm_test)
ENDIF(TEST_PARALLEL4)
ENDIF(NETCDF_BUILD_UTILITIES)

# tst_create_files writes the files several of the shell tests read:
# run_bm_elena reads tst_elena_int_3D.nc, run_bm_test1 reads tst_floats_1D.nc
# and tst_<type>2_<n>D.nc, run_bm_test2 reads tst_simple.nc, and
# run_par_bm_test reads what run_bm_test1 leaves behind. Makefile.am states
# these orderings explicitly (`run_bm_test1.log: tst_create_files.log` and
# friends); without the same statement here, `ctest -j` is free to start a
# reader while tst_create_files is still writing.
#
# add_sh_test expands to nothing when bash is absent, and run_par_bm_test only
# exists for parallel builds, so guard on the test rather than on the
# conditions that created it.
FOREACH(READER run_bm_elena run_bm_test1 run_bm_test2)
  IF(TEST nc_perf_${READER})
    SET_TESTS_PROPERTIES(nc_perf_${READER} PROPERTIES
                         DEPENDS nc_perf_tst_create_files)
  ENDIF()
ENDFOREACH()
IF(TEST nc_perf_run_par_bm_test)
  SET_TESTS_PROPERTIES(nc_perf_run_par_bm_test PROPERTIES
                       DEPENDS "nc_perf_tst_create_files;nc_perf_run_bm_test1")
ENDIF()

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

