# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

SET(abs_top_srcdir ${CMAKE_CURRENT_SOURCE_DIR})

remove_definitions(-DDLL_EXPORT)

# Create nczarr tests from nc_test4 tests
macro(NCZARR_SH_TEST basename src)
FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/../${src}/tst_${basename}.sh SHSOURCE)
# Make sure the order of prepended lines is correct
STRING(PREPEND SHSOURCE "TESTNCZARR=1\n")
STRING(PREPEND SHSOURCE "#!/bin/bash\n")
# Replace with FILE(CONFIGURE) when cmake 3.18 is in common use
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/run_${basename}.1 "${SHSOURCE}")
CONFIGURE_FILE(${CMAKE_CURRENT_BINARY_DIR}/run_${basename}.1 ${CMAKE_CURRENT_BINARY_DIR}/run_${basename}.sh FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE @ONLY NEWLINE_STYLE LF)
FILE(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/run_${basename}.1)
endmacro(NCZARR_SH_TEST)

macro(NCZARR_C_TEST basename newname src)
FILE(READ ${CMAKE_CURRENT_SOURCE_DIR}/../${src}/${basename}.c CSOURCE)
STRING(PREPEND CSOURCE "#define TESTNCZARR\n")
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${newname}.c "${CSOURCE}")
endmacro(NCZARR_C_TEST)

NCZARR_C_TEST(tst_quantize test_quantize nc_test4)
NCZARR_C_TEST(tst_filter_vlen test_filter_vlen nc_test4)
NCZARR_C_TEST(tst_unlim_vars test_unlim_vars nc_test4)
NCZARR_C_TEST(tst_h5_endians test_endians nc_test4)
NCZARR_C_TEST(tst_put_vars_two_unlim_dim test_put_vars_two_unlim_dim nc_test4)
NCZARR_C_TEST(tst_chunking test_chunking ncdump)

NCZARR_SH_TEST(specific_filters nc_test4)
NCZARR_SH_TEST(unknown nc_test4)
NCZARR_SH_TEST(filter_vlen nc_test4)

# Unlimited tests
NCZARR_SH_TEST(mud ncdump)
NCZARR_SH_TEST(nccopy5 ncdump)

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.sh
${CMAKE_CURRENT_SOURCE_DIR}/ref*.cdl
${CMAKE_CURRENT_SOURCE_DIR}/ref*.txt
${CMAKE_CURRENT_SOURCE_DIR}/ref*.zmap)

FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

IF(USE_X_GETOPT)
SET(XGETOPTSRC "${CMAKE_CURRENT_SOURCE_DIR}/../libdispatch/XGetopt.c")
ENDIF()

IF(ENABLE_TESTS)

  SET(COMMONSRC ut_util.c ut_test.c ${XGETOPTSRC})
  
  SET(TSTCOMMONSRC test_utils.c test_utils.h ${XGETOPTSRC})

  INCLUDE_DIRECTORIES(../libnczarr ../plugins ${CMAKE_SOURCE_DIR}/nczarr_test)

  # Base tests
  # The tests are set up as a combination of shell scripts and executables that
  # must be run in a particular order. It is painful but will use macros to help
  # keep it from being too bad.

  BUILD_BIN_TEST(ut_map ${COMMONSRC})
  BUILD_BIN_TEST(ut_mapapi ${COMMONSRC})
  BUILD_BIN_TEST(ut_json ${COMMONSRC})

  BUILD_BIN_TEST(test_fillonlyz ${TSTCOMMONSRC})
  BUILD_BIN_TEST(test_quantize ${TSTCOMMONSRC})
  BUILD_BIN_TEST(test_notzarr ${TSTCOMMONSRC})

#  ADD_BIN_TEST(nczarr_test test_endians ${TSTCOMMONSRC})

  # Unlimited Tests
  IF(USE_HDF5)
  ADD_BIN_TEST(nczarr_test test_unlim_vars ${TSTCOMMONSRC})
  ADD_BIN_TEST(nczarr_test test_put_vars_two_unlim_dim ${TSTCOMMONSRC})
  BUILD_BIN_TEST(test_zchunks ${COMMONSRC})
  BUILD_BIN_TEST(test_zchunks2 ${COMMONSRC})
  BUILD_BIN_TEST(test_zchunks3 ${COMMONSRC})
  BUILD_BIN_TEST(test_unlim_io ${TSTCOMMONSRC})
  BUILD_BIN_TEST(test_chunking ${TSTCOMMONSRC})
  ADD_SH_TEST(nczarr_test run_nccopyz)
  ADD_SH_TEST(nczarr_test run_unlim_io)
  ADD_SH_TEST(nczarr_test run_nccopy5)
  ADD_SH_TEST(nczarr_test run_mud)
  ENDIF()
  
  IF(FALSE) # Obsolete tests
      BUILD_BIN_TEST(ut_projections ${COMMONSRC})
      BUILD_BIN_TEST(ut_chunking ${COMMONSRC})
      BUILD_BIN_TEST(ut_walk ${COMMONSRC})
  ENDIF()

  # Helper programs for testing
  BUILD_BIN_TEST(zhex)
  BUILD_BIN_TEST(zisjson ${COMMONSRC})
  TARGET_INCLUDE_DIRECTORIES(zisjson PUBLIC ../libnczarr)
  BUILD_BIN_TEST(zs3parse ${COMMONSRC})
  TARGET_INCLUDE_DIRECTORIES(zs3parse PUBLIC ../libnczarr)
  BUILD_BIN_TEST(zmapio ${COMMONSRC})

  IF(ENABLE_S3 AND NOT WITH_S3_TESTING STREQUAL "NO")
  # Helper programs for testing
  BUILD_BIN_TEST(s3util ${COMMONSRC})
  endif()
  
  SET(ncdumpchunks_SOURCE ncdumpchunks.c)
  SET(ncdumpchunks_SOURCE ${ncdumpchunks_SOURCE} ${XGETOPTSRC})
  BUILD_BIN_TEST(ncdumpchunks ${ncdumpchunks_SOURCE})

  IF(BUILD_UTILITIES)
    add_sh_test(nczarr_test run_ut_map)
    add_sh_test(nczarr_test run_ut_mapapi)
    add_sh_test(nczarr_test run_ut_misc)
    add_sh_test(nczarr_test run_ncgen4)
    if(LARGE_FILE_TESTS)
      BUILD_BIN_TEST(test_readcaching})
      BUILD_BIN_TEST(test_writecaching})
      BUILD_BIN_TEST(test_chunkcases ${TSTCOMMONSRC})
      add_sh_test(nczarr_test run_cachetest)
      add_sh_test(nczarr_test run_chunkcases)
    ENDIF()

    add_sh_test(nczarr_test run_purezarr)
    add_sh_test(nczarr_test run_misc)
    add_sh_test(nczarr_test run_nczarr_fill)
    add_sh_test(nczarr_test run_jsonconvention)
    add_sh_test(nczarr_test run_strings)
    add_sh_test(nczarr_test run_scalar)
    add_sh_test(nczarr_test run_nulls)
    add_sh_test(nczarr_test run_external)
    add_sh_test(nczarr_test run_quantize)
    add_sh_test(nczarr_test run_notzarr)

    # This has timeout under CMake
#    if(NOT ISCMAKE)
      add_sh_test(nczarr_test run_interop)
#    ENDIF()
    
    IF(USE_HDF5)
    add_sh_test(nczarr_test run_fillonlyz)
    ENDIF()

  IF(ENABLE_NCZARR_FILTERS)
  IF(ENABLE_FILTER_TESTING)
	  build_bin_test(test_nczfilter)
  	  build_bin_test(test_filter_vlen)
	  build_bin_test(testfilter)
	  build_bin_test(testfilter_misc)
	  build_bin_test(testfilter_multi)
	  build_bin_test(testfilter_order)
	  build_bin_test(testfilter_repeat)
	  ADD_SH_TEST(nczarr_test run_nczfilter)
	  ADD_SH_TEST(nczarr_test run_filter)
	  ADD_SH_TEST(nczarr_test run_specific_filters)
  	  ADD_SH_TEST(nczarr_test run_filter_vlen)
	  IF(FALSE)
	    # This test is too dangerous to run in a parallel make environment.
            # It causes race conditions. So suppress and only test by hand.
            ADD_SH_TEST(nczarr_test run_unknown)
	  ENDIF(FALSE)
  ENDIF(ENABLE_FILTER_TESTING)
  ENDIF(ENABLE_NCZARR_FILTERS)

  if(ENABLE_NCZARR_ZIP)
      add_sh_test(nczarr_test run_newformat)
  endif()

  IF(FALSE) # Obsolete tests
      add_sh_test(nczarr_test run_ut_chunk)
  ENDIF()

  ENDIF(BUILD_UTILITIES)

ENDIF(ENABLE_TESTS)

## Specify files to be distributed by 'make dist'
FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.sh ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am zmapio.c zhex.c ncdumpchunks.c)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
