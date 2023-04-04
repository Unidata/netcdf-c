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

IF(BUILD_UTILITIES)
add_sh_test(nc_perf run_bm_test1)
add_sh_test(nc_perf run_bm_test2)

# This will run a parallel I/O benchmark for parallel builds.
IF(TEST_PARALLEL4)
add_bin_test(nc_perf tst_compress_par tst_utils.c)
add_sh_test(nc_perf run_par_bm_test)
ENDIF(TEST_PARALLEL4)
ENDIF(BUILD_UTILITIES)

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

