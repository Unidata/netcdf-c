# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

SET(abs_top_srcdir ${CMAKE_CURRENT_SOURCE_DIR})

remove_definitions(-DDLL_EXPORT)

FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/../nc_test4/tst_quantize.c QSOURCE)
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test_quantize.c "#define TESTNCZARR\n")
FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/test_quantize.c "${QSOURCE}")

FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/../nc_test4/tst_filter_avail.c ASOURCE)
STRING(PREPEND ASOURCE "#define TESTNCZARR\n")
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/test_filter_avail.c "${ASOURCE}")

FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/../nc_test4/tst_specific_filters.sh SPSOURCE)
STRING(PREPEND SPSOURCE "#!/bin/bash\n")
STRING(PREPEND SPSOURCE "TESTNCZARR=1\n")
# Replace with FILE(CONFIGURE) when cmake 3.18 is in common use
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/run_specific_filters.1 "${SPSOURCE}")
CONFIGURE_FILE(${CMAKE_CURRENT_BINARY_DIR}/run_specific_filters.1 ${CMAKE_CURRENT_BINARY_DIR}/run_specific_filters.sh FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE @ONLY NEWLINE_STYLE LF)
FILE(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/run_specific_filters.1)
	
FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/../nc_test4/tst_unknown.sh SPSOURCE)
STRING(PREPEND SPSOURCE "#!/bin/bash\n")
STRING(PREPEND SPSOURCE "TESTNCZARR=1\n")
# Replace with FILE(CONFIGURE) when cmake 3.18 is in common use
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/run_unknown.1 "${SPSOURCE}")
CONFIGURE_FILE(${CMAKE_CURRENT_BINARY_DIR}/run_unknown.1 ${CMAKE_CURRENT_BINARY_DIR}/run_unknown.sh FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE @ONLY NEWLINE_STYLE LF)
FILE(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/run_unknown.1)
	
FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.sh
${CMAKE_CURRENT_SOURCE_DIR}/ref*.cdl
${CMAKE_CURRENT_SOURCE_DIR}/ref*.txt
${CMAKE_CURRENT_SOURCE_DIR}/ref*.zmap)

FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

IF(ENABLE_TESTS)

  SET(COMMONSRC ut_util.c ut_test.c)
  IF(USE_X_GETOPT)
    SET(COMMONSRC ${COMMONSRC} XGetopt.c)
  ENDIF()
  
  SET(TSTCOMMONSRC tst_utils.c)
  IF(USE_X_GETOPT)
    SET(TSTCOMMONSRC ${TSTCOMMONSRC} XGetopt.c)
  ENDIF()

  # Base tests
  # The tests are set up as a combination of shell scripts and executables that
  # must be run in a particular order. It is painful but will use macros to help
  # keep it from being too bad.

  BUILD_BIN_TEST(ut_map ${COMMONSRC})
  BUILD_BIN_TEST(ut_mapapi ${COMMONSRC})
  BUILD_BIN_TEST(ut_json ${COMMONSRC})
  BUILD_BIN_TEST(ut_projections ${COMMONSRC})
  BUILD_BIN_TEST(ut_chunking ${COMMONSRC})
  BUILD_BIN_TEST(tst_zchunks ${COMMONSRC})
  BUILD_BIN_TEST(tst_zchunks2 ${COMMONSRC})
  BUILD_BIN_TEST(tst_zchunks3 ${COMMONSRC})
  BUILD_BIN_TEST(tst_fillonlyz ${TSTCOMMONSRC})

  TARGET_INCLUDE_DIRECTORIES(ut_map PUBLIC ../libnczarr)
  TARGET_INCLUDE_DIRECTORIES(ut_mapapi PUBLIC ../libnczarr)
  TARGET_INCLUDE_DIRECTORIES(ut_json PUBLIC ../libnczarr)
  TARGET_INCLUDE_DIRECTORIES(ut_projections PUBLIC ../libnczarr)
  TARGET_INCLUDE_DIRECTORIES(ut_chunking PUBLIC ../libnczarr)
  TARGET_INCLUDE_DIRECTORIES(tst_zchunks PUBLIC ../libnczarr)
  TARGET_INCLUDE_DIRECTORIES(tst_zchunks2 PUBLIC ../libnczarr)
  TARGET_INCLUDE_DIRECTORIES(tst_zchunks3 PUBLIC ../libnczarr)
  TARGET_INCLUDE_DIRECTORIES(tst_fillonlyz PUBLIC ../libnczarr)

  # Helper programs for testing
  BUILD_BIN_TEST(zmapio ${COMMONSRC})
  TARGET_INCLUDE_DIRECTORIES(zmapio PUBLIC ../libnczarr)
  BUILD_BIN_TEST(zhex)
  BUILD_BIN_TEST(zisjson ${COMMONSRC})
  TARGET_INCLUDE_DIRECTORIES(zisjson PUBLIC ../libnczarr)
  BUILD_BIN_TEST(zs3parse ${COMMONSRC})
  TARGET_INCLUDE_DIRECTORIES(zs3parse PUBLIC ../libnczarr)
  if(ENABLE_S3)
      BUILD_BIN_TEST(s3util ${COMMONSRC})
      TARGET_INCLUDE_DIRECTORIES(s3util PUBLIC ../libnczarr)
  endif()
  
  SET(ncdumpchunks_SOURCE ncdumpchunks.c)
  IF(USE_X_GETOPT)
    SET(ncdumpchunks_SOURCE ${ncdumpchunks_SOURCE} XGetopt.c)
  ENDIF()
  BUILD_BIN_TEST(ncdumpchunks ${ncdumpchunks_SOURCE})
  TARGET_INCLUDE_DIRECTORIES(ncdumpchunks PUBLIC ../libnczarr)

  IF(BUILD_UTILITIES)
    add_sh_test(nczarr_test run_ut_map)
    add_sh_test(nczarr_test run_ut_mapapi)
    add_sh_test(nczarr_test run_ut_misc)
    add_sh_test(nczarr_test run_ut_chunk)
    IF(USE_HDF5)
#    add_sh_test(nczarr_test run_nccopyz)
    add_sh_test(nczarr_test run_fillonlyz)
    ENDIF()
    add_sh_test(nczarr_test run_ncgen4)

    BUILD_BIN_TEST(tst_chunkcases ${TSTCOMMONSRC})
    TARGET_INCLUDE_DIRECTORIES(tst_chunkcases PUBLIC ../libnczarr)
    add_sh_test(nczarr_test run_chunkcases)

    add_sh_test(nczarr_test run_purezarr)
    add_sh_test(nczarr_test run_interop)
    add_sh_test(nczarr_test run_misc)
    add_sh_test(nczarr_test run_nczarr_fill)
    add_sh_test(nczarr_test run_jsonconvention)
    add_sh_test(nczarr_test run_strings)
    add_sh_test(nczarr_test run_scalar)
    add_sh_test(nczarr_test run_nulls)

    BUILD_BIN_TEST(test_quantize ${TSTCOMMONSRC})
    add_sh_test(nczarr_test run_quantize)

    if(ENABLE_S3)
	add_sh_test(nczarr_test run_s3_cleanup)
    ENDIF()

    IF(ENABLE_FILTER_TESTING)
	build_bin_test(tst_nczfilter)
	build_bin_test(testfilter)
	build_bin_test(testfilter_misc)
	build_bin_test(testfilter_multi)
	build_bin_test(testfilter_order)
	build_bin_test(testfilter_repeat)
	build_bin_test(test_filter_avail)
	ADD_SH_TEST(nczarr_test run_nczfilter)
	ADD_SH_TEST(nczarr_test run_filter)
	ADD_SH_TEST(nczarr_test run_unknown)
	ADD_SH_TEST(nczarr_test run_specific_filters)
    ENDIF(ENABLE_FILTER_TESTING)
    if(ENABLE_NCZARR_ZIP)
        add_sh_test(nczarr_test run_newformat)
    endif()

  ENDIF(BUILD_UTILITIES)

ENDIF(ENABLE_TESTS)

## Specify files to be distributed by 'make dist'
FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.sh ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am zmapio.c zhex.c ncdumpchunks.c)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
