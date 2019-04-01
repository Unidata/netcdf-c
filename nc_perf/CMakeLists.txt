# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018, 2019
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

# This is the cmake build file for the nc_perf directory.
# Ed Hartnett

add_sh_test(nc_test4 run_bm_test1)
add_sh_test(nc_test4 run_bm_elena)
add_sh_test(nc_test4 run_bm_test2)
add_sh_test(nc_test4 run_tst_chunks)
add_sh_test(nc_test4 run_knmi_bm)

SET(NC4_TESTS ${NC4_TESTS} tst_create_files bm_file tst_chunks3
tst_ar4 tst_ar4_3d tst_ar4_4d bm_many_objs tst_h_many_atts
bm_many_atts tst_files2 tst_files3 tst_mem tst_knmi bm_netcdf4_recs)

IF(TEST_PARALLEL)
  add_sh_test(nc_test4 run_par_bm_test)
ENDIF()

FOREACH(CTEST ${NC4_TESTS})
  add_bin_test(nc_test4 ${CTEST})
ENDFOREACH()

build_bin_test(bigmeta)
build_bin_test(openbigmeta)
build_bin_test(tst_attsperf)
add_sh_test(nc_test4 perftest)

ADD_EXTRA_DIST(run_par_test.sh.in)
