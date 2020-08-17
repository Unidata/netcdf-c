# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

SET(abs_top_srcdir ${CMAKE_CURRENT_SOURCE_DIR})

remove_definitions(-DDLL_EXPORT)

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.sh
${CMAKE_CURRENT_SOURCE_DIR}/ref*.cdl
${CMAKE_CURRENT_SOURCE_DIR}/ref*.txt)

FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

IF(ENABLE_TESTS)

  SET(COMMONSRC ut_util.c ut_test.c)

  # Base tests
  # The tests are set up as a combination of shell scripts and executables that
  # must be run in a particular order. It is painful but will use macros to help
  # keep it from being too bad.

  BUILD_BIN_TEST_SOURCED(ut_map ${COMMONSRC})
  BUILD_BIN_TEST_SOURCED(ut_mapapi ${COMMONSRC})
  BUILD_BIN_TEST_SOURCED(ut_json ${COMMONSRC})
  BUILD_BIN_TEST_SOURCED(ut_projections ${COMMONSRC})
  BUILD_BIN_TEST_SOURCED(ut_chunking ${COMMONSRC})

  TARGET_INCLUDE_DIRECTORIES(ut_map PUBLIC ../libnczarr)
  TARGET_INCLUDE_DIRECTORIES(ut_mapapi PUBLIC ../libnczarr)
  TARGET_INCLUDE_DIRECTORIES(ut_json PUBLIC ../libnczarr)
  TARGET_INCLUDE_DIRECTORIES(ut_projections PUBLIC ../libnczarr)
  TARGET_INCLUDE_DIRECTORIES(ut_chunking PUBLIC ../libnczarr)

  BUILD_BIN_TEST_SOURCED(zmapio ${COMMONSRC})
  TARGET_INCLUDE_DIRECTORIES(zmapio PUBLIC ../libnczarr)

# Integration tests
#  BUILD_BIN_TEST(it_meta)
#  TARGET_INCLUDE_DIRECTORIES(it_meta PUBLIC ../libnczarr)

  IF(BUILD_UTILITIES)
    add_sh_test(nczarr_test run_ut_map)
    add_sh_test(nczarr_test run_ut_mapapi)
    add_sh_test(nczarr_test run_ut_misc)
    add_sh_test(nczarr_test run_ut_chunk)
  ENDIF(BUILD_UTILITIES)

ENDIF(ENABLE_TESTS)

## Specify files to be distributed by 'make dist'
FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.sh ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am zmapio.c)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
