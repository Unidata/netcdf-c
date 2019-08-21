# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018, 2019
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

# This is the cmake build file for the nc_test4 directory.
# Ward Fisher, Ed Hartnett

# Some extra tests
SET(NC4_TESTS tst_dims tst_dims2 tst_dims3 tst_files tst_files4
  tst_vars tst_varms tst_unlim_vars tst_converts tst_converts2
  tst_grps tst_grps2 tst_compounds tst_compounds2 tst_compounds3
  tst_opaques tst_strings tst_strings2 tst_interops tst_interops4
  tst_interops6 tst_enums tst_coords tst_coords2 tst_coords3 tst_vars3
  tst_vars4 tst_chunks tst_chunks2 tst_utf8 tst_fills tst_fills2
  tst_fillbug tst_xplatform2 tst_endian_fill tst_atts t_type
  cdm_sea_soundings tst_vl tst_atts1 tst_atts2 tst_vars2 tst_files5
  tst_files6 tst_sync tst_h_strbug tst_h_refs tst_h_scalar tst_rename
  tst_rename2 tst_rename3 tst_h5_endians tst_atts_string_rewrite tst_put_vars_two_unlim_dim
  tst_hdf5_file_compat tst_fill_attr_vanish tst_rehash tst_types tst_bug324
  tst_atts3 tst_put_vars tst_elatefill tst_udf tst_bug1442)

# Note, renamegroup needs to be compiled before run_grp_rename

IF(BUILD_UTILITIES)
  SET(NC4_TESTS ${NC4_TESTS} tst_xplatform)
  build_bin_test(renamegroup)
  add_sh_test(nc_test4 run_grp_rename)
  ADD_SH_TEST(nc_test4 tst_misc)
IF(ENABLE_FILTER_TESTING)
  build_bin_test(test_filter)
  build_bin_test(test_filter_misc)
  build_bin_test(test_filter_reg)
  ADD_SH_TEST(nc_test4 tst_filter)
  SET(NC4_TESTS ${NC4_TESTS} tst_filterparser test_filter_reg)
ENDIF(ENABLE_FILTER_TESTING)

ENDIF(BUILD_UTILITIES)

##
# The shell script, run_empty_vlen_test.sh,
# depends on the 'tst_empty_vlen_unlim' binary.
##
BUILD_BIN_TEST(tst_empty_vlen_unlim)
ADD_SH_TEST(nc_test4 run_empty_vlen_test)

IF(NOT MSVC)
  SET(NC4_TESTS ${NC4_TESTS} tst_interops5 tst_camrun)
ENDIF()

# If the v2 API was built, add the test program.
IF(ENABLE_V2_API)
  build_bin_test(tst_v2)
ENDIF()

IF(LARGE_FILE_TESTS)
  SET(NC4_TESTS ${NC4_TESTS} tst_large tst_large2)
ENDIF()


IF(USE_SZIP)
  BUILD_BIN_TEST(test_szip)
  BUILD_BIN_TEST(h5testszip)
  IF(BUILD_UTILITIES)
    add_sh_test(nc_test4 tst_szip)
  ENDIF()
ENDIF()

# Copy some test files from current source dir to out-of-tree build dir.
FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/ref_bzip2.c ${CMAKE_CURRENT_SOURCE_DIR}/*.sh ${CMAKE_CURRENT_SOURCE_DIR}/*.h5 ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
IF(MSVC)
  FILE(COPY ${COPY_FILES} DESTINATION ${RUNTIME_OUTPUT_DIRECTORY}/)
ENDIF()

FOREACH(CTEST ${NC4_TESTS})
  add_bin_test(nc_test4 ${CTEST})
ENDFOREACH()

IF(TEST_PARALLEL4)
  build_bin_test(tst_mpi_parallel)
  build_bin_test(tst_parallel)
  build_bin_test(tst_parallel3)
  build_bin_test(tst_parallel4)
  build_bin_test(tst_parallel5)
  build_bin_test(tst_nc4perf)
  build_bin_test(tst_mode)
  build_bin_test(tst_simplerw_coll_r)
  add_sh_test(nc_test4 run_par_test)
ENDIF()
