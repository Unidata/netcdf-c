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

IF(NETCDF_ENABLE_TESTS)

  add_library(ut_util STATIC ut_util.c ut_test.c ut_includes.h ut_test.h ${XGETOPTSRC})
  IF(NETCDF_ENABLE_DLL)
    target_compile_definitions(ut_util PUBLIC -DDLL_EXPORT -DDLL_NETCDF)
  ENDIF(NETCDF_ENABLE_DLL)
  target_include_directories(ut_util PUBLIC ../libnczarr ../plugins ${CMAKE_CURRENT_LIST_DIR})
  target_link_libraries(ut_util PUBLIC netcdf ${ALL_TLL_LIBS})

  add_library(test_utils STATIC test_utils.c test_utils.h ${XGETOPTSRC})
  IF(NETCDF_ENABLE_DLL)
    target_compile_definitions(test_utils PUBLIC -DDLL_EXPORT -DDLL_NETCDF)
  ENDIF(NETCDF_ENABLE_DLL)
  target_include_directories(test_utils PUBLIC ../libnczarr ../plugins ${CMAKE_CURRENT_LIST_DIR})
  target_link_libraries(test_utils PUBLIC netcdf ${ALL_TLL_LIBS})

  set_target_properties(ut_util test_utils PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
  )

  macro(build_bin_test_with_util_lib F UTIL_LIB)
    build_bin_test(${F})
    IF(NETCDF_ENABLE_DLL)
      target_compile_definitions(${F} PUBLIC -DDLL_NETCDF)
    ENDIF(NETCDF_ENABLE_DLL)
    target_link_libraries(${F} ${UTIL_LIB} ${ALL_TLL_LIBS})
  endmacro()

  macro(add_bin_test_with_util_lib PREFIX F UTIL_LIB)
    add_bin_test(${PREFIX} ${F})
    IF(NETCDF_ENABLE_DLL)
      target_compile_definitions(${PREFIX}_${F} PUBLIC -DDLL_NETCDF)
    ENDIF(NETCDF_ENABLE_DLL)
    target_link_libraries(${PREFIX}_${F} ${UTIL_LIB} ${ALL_TLL_LIBS})

  endmacro()

  # Base tests
  # The tests are set up as a combination of shell scripts and executables that
  # must be run in a particular order. It is painful but will use macros to help
  # keep it from being too bad.

  build_bin_test_with_util_lib(ut_map ut_util)
  build_bin_test_with_util_lib(ut_mapapi ut_util)
  build_bin_test_with_util_lib(ut_json ut_util)

  build_bin_test_with_util_lib(test_fillonlyz test_utils)
  build_bin_test_with_util_lib(test_quantize test_utils)
  build_bin_test_with_util_lib(test_notzarr test_utils)

#  ADD_BIN_TEST(nczarr_test test_endians ${TSTCOMMONSRC})

  # Unlimited Tests
  IF(USE_HDF5)
  add_bin_test_with_util_lib(nczarr_test test_unlim_vars test_utils)
  add_bin_test_with_util_lib(nczarr_test test_put_vars_two_unlim_dim test_utils)
  build_bin_test_with_util_lib(test_zchunks ut_util)
  build_bin_test_with_util_lib(test_zchunks2 ut_util)
  build_bin_test_with_util_lib(test_zchunks3 ut_util)
  build_bin_test_with_util_lib(test_unlim_io test_utils)
  build_bin_test_with_util_lib(test_chunking test_utils)
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
  build_bin_test_with_util_lib(zisjson ut_util)
  TARGET_INCLUDE_DIRECTORIES(zisjson PUBLIC ../libnczarr)
  build_bin_test_with_util_lib(zs3parse ut_util)
  TARGET_INCLUDE_DIRECTORIES(zs3parse PUBLIC ../libnczarr)
  build_bin_test_with_util_lib(zmapio ut_util)

  IF(NETCDF_ENABLE_S3 AND NOT WITH_S3_TESTING STREQUAL "NO")
  # Helper programs for testing
    BUILD_BIN_TEST(s3util ${COMMONSRC})
  # Pure AWS Test
    IF(NETCDF_ENABLE_S3_AWS)
      SET(TMP_CMAKE_CXX_STANDARD ${MAKE_CXX_STANDARD})
      SET(CMAKE_CXX_STANDARD 11)
      ADD_EXECUTABLE(tst_pure_awssdk tst_pure_awssdk.cpp)
      TARGET_LINK_LIBRARIES(tst_pure_awssdk ${AWSSDK_CORE_LIB_FILE})
      ADD_TEST(tst_pure_awssdk ${EXECUTABLE_PUTPUT_PATH}/tst_pure_awssdk)
      SET(F tst_pure_awssdk)
      IF(WIN32)
        SET_PROPERTY(TEST ${F} PROPERTY FOLDER "tests/")
        SET_TARGET_PROPERTIES(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
          ${CMAKE_CURRENT_BINARY_DIR})
        SET_TARGET_PROPERTIES(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
          ${CMAKE_CURRENT_BINARY_DIR})
        SET_TARGET_PROPERTIES(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
          ${CMAKE_CURRENT_BINARY_DIR})
      ENDIF(WIN32)      

      SET(CMAKE_CXX_STANDARD ${TMP_CMAKE_CXX_STANDARD})
    ENDIF(NETCDF_ENABLE_S3_AWS)
    TARGET_INCLUDE_DIRECTORIES(s3util PUBLIC ../libnczarr)
  endif()
  
  if(ENABLE_NCDUMPCHUNKS)
    build_bin_test_with_util_lib(ncdumpchunks ut_util)
  endif()

  IF(NETCDF_BUILD_UTILITIES)
    add_sh_test(nczarr_test run_ut_map)
    add_sh_test(nczarr_test run_ut_mapapi)
    add_sh_test(nczarr_test run_ut_misc)
    add_sh_test(nczarr_test run_ncgen4)
    if(LARGE_FILE_TESTS)
      BUILD_BIN_TEST(test_readcaching)
      BUILD_BIN_TEST(test_writecaching)
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

  IF(NETCDF_ENABLE_NCZARR_FILTERS)
  IF(NETCDF_ENABLE_FILTER_TESTING)
	  build_bin_test(test_nczfilter)
  	  build_bin_test(test_filter_vlen)
	  build_bin_test(testfilter)
	  build_bin_test_with_util_lib(testfilter_misc test_utils)
	  build_bin_test(testfilter_multi)
	  build_bin_test(testfilter_order)
	  build_bin_test(testfilter_repeat)
	  ADD_SH_TEST(nczarr_test run_nczfilter)
    if(NOT MINGW)
	    ADD_SH_TEST(nczarr_test run_filter)
    endif()
	  ADD_SH_TEST(nczarr_test run_specific_filters)
  	  ADD_SH_TEST(nczarr_test run_filter_vlen)
	  IF(FALSE)
	    # This test is too dangerous to run in a parallel make environment.
            # It causes race conditions. So suppress and only test by hand.
            ADD_SH_TEST(nczarr_test run_unknown)
	  ENDIF(FALSE)
  ENDIF(NETCDF_ENABLE_FILTER_TESTING)
  ENDIF(NETCDF_ENABLE_NCZARR_FILTERS)

  if(NETCDF_ENABLE_NCZARR_ZIP)
      add_sh_test(nczarr_test run_newformat)
      # Test various corrupted files
      ADD_SH_TEST(nczarr_test run_corrupt.sh)
  endif()

  IF(FALSE) # Obsolete tests
      add_sh_test(nczarr_test run_ut_chunk)
  ENDIF()

  ENDIF(NETCDF_BUILD_UTILITIES)

ENDIF(NETCDF_ENABLE_TESTS)

## Specify files to be distributed by 'make dist'
FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.sh ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am zmapio.c zhex.c ncdumpchunks.c)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
