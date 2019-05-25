# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

build_bin_test(bm_file)
build_bin_test(tst_chunks3)
build_bin_test(tst_ar4)
build_bin_test(tst_ar4_4d)
build_bin_test(bm_many_objs)
build_bin_test(tst_h_many_atts)
build_bin_test(bm_many_atts)
build_bin_test(tst_files2)
build_bin_test(tst_knmi)
build_bin_test(bm_netcdf4_recs)
build_bin_test(bigmeta)
build_bin_test(openbigmeta)

add_bin_test(tst_ar4_3d)
add_bin_test(tst_create_files)
add_bin_test(tst_files3)
add_bin_test(tst_mem)
add_bin_test(tst_wrf_reads)
add_bin_test(tst_attsperf)

add_sh_test(run_knmi_bm.sh)
add_sh_test(perftest.sh)
add_sh_test(run_tst_chunks.sh)
add_sh_test(run_bm_elena.sh))

IF(BUILD_UTILITIES)
add_sh_test(run_bm_test1.sh)
add_sh_test(run_bm_test2.sh)

# This will run a parallel I/O benchmark for parallel builds.
IF(TEST_PARALLEL4)
add_sh_test(run_par_bm_test.sh)
ENDIF()
ENDIF()

ADD_EXTRA_DIST(run_par_bm_test.sh.in run_knmi_bm.sh CMakeLists.txt
perftest.sh run_bm_test1.sh run_bm_test2.sh)
