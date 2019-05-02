# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

message(STATUS "CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}")

add_definitions(-D"TOPSRCDIR=${CMAKE_SOURCE_DIR}")
add_definitions(-D"TOPBINDIR=${CMAKE_BINARY_DIR}")

INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)

SET (nc_test_m4_SOURCES test_get test_put test_read test_write)
FOREACH (F ${nc_test_m4_SOURCES})
  GEN_m4(${F})
ENDFOREACH()

SET (nc_test_SRC
  nc_test.c
  error.c
  test_get.c
  test_put.c
  test_read.c
  test_write.c
  util.c
  )

ADD_EXECUTABLE(nc_test ${nc_test_SRC})
TARGET_LINK_LIBRARIES(nc_test
  netcdf
  ${HAVE_LIBM}
  )

# Some extra stand-alone tests
SET(TESTS t_nc tst_small tst_misc tst_norm tst_names tst_nofill tst_nofill2 tst_nofill3 tst_meta tst_inq_type tst_utf8_validate tst_utf8_phrases tst_global_fillval tst_max_var_dims tst_formats tst_def_var_fill tst_err_enddef tst_default_format)

IF(NOT HAVE_BASH)
  SET(TESTS ${TESTS} tst_atts3)
ENDIF()

IF(USE_PNETCDF)
  build_bin_test_no_prefix(tst_pnetcdf)
  build_bin_test_no_prefix(tst_parallel2)
  build_bin_test_no_prefix(tst_addvar)
  add_sh_test(nc_test run_pnetcdf_test)
  add_bin_test(nc_test tst_formatx_pnetcdf)
  add_bin_test(nc_test tst_default_format_pnetcdf)
ENDIF()

IF(LARGE_FILE_TESTS)
  SET(TESTS ${TESTS} quick_large_files tst_big_var6 tst_big_var2 tst_big_rvar tst_big_var tst_large tst_large_cdf5)
  IF(NOT MSVC)
    SET(TESTS ${TESTS} large_files)
  ENDIF()

ENDIF()

SET(TESTFILES ${TESTFILES} tst_diskless tst_diskless3 tst_diskless4 tst_diskless5 tst_inmemory tst_open_mem)
IF(USE_NETCDF4)
  SET(TESTFILES ${TESTFILES} tst_diskless2)
  SET(TESTS ${TESTS} tst_diskless6)
ENDIF()

# Build executables required for the shell scripts.
FOREACH(BINFILE ${TESTFILES})
  build_bin_test(${BINFILE})
ENDFOREACH()

# Process single-file test files.
FOREACH(CTEST ${TESTS})
  add_bin_test(nc_test ${CTEST})
ENDFOREACH()

ADD_TEST(nc_test ${EXECUTABLE_OUTPUT_PATH}/nc_test)

IF(BUILD_UTILITIES)

    add_sh_test(nc_test run_diskless)
    IF(BUILD_MMAP)
      add_sh_test(nc_test run_mmap)
      SET_TESTS_PROPERTIES(nc_test_run_mmap PROPERTIES RUN_SERIAL TRUE)
    ENDIF(BUILD_MMAP)
    add_sh_test(nc_test run_diskless5)
    add_sh_test(nc_test run_inmemory)
    IF(LARGE_FILE_TESTS)
      add_sh_test(nc_test run_diskless2)
    ENDIF()

  IF(ENABLE_BYTERANGE)
    build_bin_test_no_prefix(tst_byterange)
    add_sh_test(nc_test test_byterange)
  ENDIF()

  IF(BUILD_MMAP)
    add_sh_test(nc_test run_mmap)
  ENDIF()

ENDIF(BUILD_UTILITIES)

# Copy some test files from current source dir to out-of-tree build dir.
FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
IF(MSVC)
#MESSAGE(STATUS "XXX")
#MESSAGE(STATUS "${COPY_FILES}")
#MESSAGE(STATUS "${RUNTIME_OUTPUT_DIRECTORY}")
  FILE(COPY ${COPY_FILES} DESTINATION ${RUNTIME_OUTPUT_DIRECTORY}/)
ENDIF()


## Specify files to be distributed by 'make dist'
FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} test_get.m4 test_put.m4 test_read.m4 test_write.m4 ref_tst_diskless2.cdl tst_diskless5.cdl
ref_tst_diskless3_create.cdl ref_tst_diskless3_open.cdl
ref_tst_http_nc3.cdl ref_tst_http_nc4.cdl)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
