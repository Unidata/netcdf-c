# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

set(abs_top_srcdir ${CMAKE_CURRENT_SOURCE_DIR})

include(v3_setup)
v3_setup()

remove_definitions(-DDLL_EXPORT)

file(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.sh
${CMAKE_CURRENT_SOURCE_DIR}/ref*.cdl
${CMAKE_CURRENT_SOURCE_DIR}/ref*.txt
${CMAKE_CURRENT_SOURCE_DIR}/ref*.zmap)

file(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

if(USE_X_GETOPT)
set(XGETOPTSRC "${CMAKE_CURRENT_SOURCE_DIR}/../libdispatch/XGetopt.c")
endif()

if(NETCDF_ENABLE_TESTS)

  add_library(v3_ut_util STATIC ut_util.c ut_test.c ut_includes.h ut_test.h ${XGETOPTSRC})
  if(NETCDF_ENABLE_DLL)
    target_compile_definitions(v3_ut_util PUBLIC -DDLL_EXPORT -DDLL_NETCDF)
  endif(NETCDF_ENABLE_DLL)
  target_include_directories(v3_ut_util PUBLIC ../libnczarr ../plugins ${CMAKE_CURRENT_LIST_DIR})
  target_link_libraries(v3_ut_util PUBLIC netcdf ${ALL_TLL_LIBS})

  add_library(test_utils STATIC test_utils.c test_utils.h ${XGETOPTSRC})
  if(NETCDF_ENABLE_DLL)
    target_compile_definitions(test_utils PUBLIC -DDLL_EXPORT -DDLL_NETCDF)
  endif(NETCDF_ENABLE_DLL)
  target_include_directories(test_utils PUBLIC ../libnczarr ../plugins ${CMAKE_CURRENT_LIST_DIR})
  target_link_libraries(test_utils PUBLIC netcdf ${ALL_TLL_LIBS})

  set_target_properties(ut_util test_utils PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
  )

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

#  add_bin_test(nczarr_test test_endians ${TSTCOMMONSRC})

  # Unlimited Tests
  if(USE_HDF5)
  add_bin_test_with_util_lib(nczarr_test test_unlim_vars test_utils)
  add_bin_test_with_util_lib(nczarr_test test_put_vars_two_unlim_dim test_utils)
  build_bin_test_with_util_lib(test_zchunks ut_util)
  build_bin_test_with_util_lib(test_zchunks2 ut_util)
  build_bin_test_with_util_lib(test_zchunks3 ut_util)
  build_bin_test_with_util_lib(test_unlim_io test_utils)
  build_bin_test_with_util_lib(test_chunking test_utils)
  add_sh_test(nczarr_test run_nccopyz)
  add_sh_test(nczarr_test run_unlim_io)
  add_sh_test(nczarr_test run_nccopy5)
  add_sh_test(nczarr_test run_mud)
  endif()
  
  # Helper programs for testing
  build_bin_test_with_util_lib(zisjson ut_util)
  target_include_directories(zisjson PUBLIC ../libnczarr)
  build_bin_test_with_util_lib(zs3parse ut_util)
  target_include_directories(zs3parse PUBLIC ../libnczarr)
  build_bin_test_with_util_lib(zmapio ut_util)

  if(NETCDF_ENABLE_S3 AND NOT WITH_S3_TESTING STREQUAL "NO")
  # Pure AWS Test
    if(NETCDF_ENABLE_S3_AWS)
      set(TMP_CMAKE_CXX_STANDARD ${MAKE_CXX_STANDARD})
      set(CMAKE_CXX_STANDARD 11)
      add_executable(tst_pure_awssdk tst_pure_awssdk.cpp)
      target_link_libraries(tst_pure_awssdk ${AWSSDK_CORE_LIB_FILE})
      add_test(tst_pure_awssdk ${EXECUTABLE_PUTPUT_PATH}/tst_pure_awssdk)
      set(F tst_pure_awssdk)
      if(MSVC)
        set_property(TEST ${F} PROPERTY FOLDER "tests/")
        set_target_properties(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
          ${CMAKE_CURRENT_BINARY_DIR})
        set_target_properties(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
          ${CMAKE_CURRENT_BINARY_DIR})
        set_target_properties(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
          ${CMAKE_CURRENT_BINARY_DIR})
      endif(MSVC)      

      SET(CMAKE_CXX_STANDARD ${TMP_CMAKE_CXX_STANDARD})
    endif(NETCDF_ENABLE_S3_AWS)
  endif()
  
  if(BUILD_UTILITIES)
    add_sh_test(nczarr_test run_ut_map)
    add_sh_test(nczarr_test run_ut_mapapi)
    add_sh_test(nczarr_test run_ut_misc)
    add_sh_test(nczarr_test run_ncgen4)
    if(LARGE_FILE_TESTS)
      build_bin_test(test_readcaching)
      build_bin_test(test_writecaching)
      build_bin_test(test_chunkcases ${TSTCOMMONSRC})
      add_sh_test(nczarr_test run_cachetest)
      add_sh_test(nczarr_test run_chunkcases)
    endif()

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
#    endif()
    
    if(USE_HDF5)
    add_sh_test(nczarr_test run_fillonlyz)
    endif()

  if(NETCDF_ENABLE_NCZARR_FILTERS)
  if(NETCDF_ENABLE_FILTER_TESTING)
	  build_bin_test(test_nczfilter)
  	  build_bin_test(test_filter_vlen)
	  build_bin_test(testfilter)
	  build_bin_test_with_util_lib(testfilter_misc test_utils)
	  build_bin_test(testfilter_multi)
	  build_bin_test(testfilter_order)
	  build_bin_test(testfilter_repeat)
	  add_sh_test(nczarr_test run_nczfilter)
	  add_sh_test(nczarr_test run_filter)
	  add_sh_test(nczarr_test run_specific_filters)
  	  add_sh_test(nczarr_test run_filter_vlen)
	  if(FALSE)
	    # This test is too dangerous to run in a parallel make environment.
            # It causes race conditions. So suppress and only test by hand.
            add_sh_test(nczarr_test run_unknown)
	  endif(FALSE)
  endif(NETCDF_ENABLE_FILTER_TESTING)
  endif(NETCDF_ENABLE_NCZARR_FILTERS)

  if(NETCDF_ENABLE_NCZARR_ZIP)
      add_sh_test(nczarr_test run_newformat)
      # Test various corrupted files
      add_sh_test(nczarr_test run_corrupt.sh)
  endif()

  if(FALSE) # Obsolete tests
      add_sh_test(nczarr_test run_ut_chunk)
  endif()

  endif(BUILD_UTILITIES)

endif(NETCDF_ENABLE_TESTS)

## Specify files to be distributed by 'make dist'
FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.sh ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
