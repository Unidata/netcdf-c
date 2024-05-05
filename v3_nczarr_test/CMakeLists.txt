# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

set(abs_top_srcdir ${CMAKE_CURRENT_SOURCE_DIR})

# This set of tests is almost exactly the same as those of nczarr_test.
# So provide a Makefile target to update the files here with copies
# of those in nczarr test and elsewhere.

foreach(TF ${TESTFILES_NCZARR})
  file(COPY "${netCDF_SOURCE_DIR}/nczarr_test/${TF}"
      DESTINATION "${netCDF_BINARY_DIR}/v3_nczarr_test/${TF}"
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endforeach()

foreach(TD ${TESTDATA_NCZARR})
  file(COPY "${netCDF_SOURCE_DIR}/nczarr_test/${TD}"
      DESTINATION "${netCDF_BINARY_DIR}/v3_nczarr_test/${TD}"
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endforeach()

remove_definitions(-DDLL_EXPORT)

file(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.sh
${CMAKE_CURRENT_SOURCE_DIR}/ref*.cdl
${CMAKE_CURRENT_SOURCE_DIR}/ref*.txt
${CMAKE_CURRENT_SOURCE_DIR}/ref*.zmap)

file(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

if(USE_X_GETOPT)
set(XGETOPTSRC "${CMAKE_CURRENT_SOURCE_DIR}/../libdispatch/XGetopt.c")
endif()

# Built Sources that are copies from nczarr_test
set(NCZARRFILES run_corrupt.sh run_external.sh run_interop.sh run_chunkcases.sh run_fillonlyz.sh run_filter.sh run_nccopyz.sh run_nczarr_fill.sh run_nczfilter.sh run_newformat.sh run_perf_chunks1.sh run_unlim_io.sh run_ut_map.sh run_ut_mapapi.sh run_ncgen4.sh run_quantize.sh run_purezarr.sh run_misc.sh run_jsonconvention.sh run_strings.sh run_scalar.sh run_nulls.sh run_notzarr.sh)

set(NCZARRFILES ${NCZARRFILES} ref_noshape.file.zip ref_power_901_constants.cdl ref_power_901_constants_orig.zip ref_avail1.cdl ref_avail1.dmp ref_avail1.txt ref_byte.cdl ref_byte_fill_value_null.cdl ref_byte_fill_value_null.zarr.zip ref_byte.zarr.zip ref_fillonly.cdl ref_groups.h5 ref_misc1.cdl ref_misc1.dmp ref_ndims.cdl ref_ndims.dmp ref_newformatpure.cdl ref_notzarr.tar.gz ref_nulls.cdl ref_nulls_nczarr.baseline ref_oldformat.cdl ref_oldformat.zip ref_perdimspecs.cdl ref_purezarr_base.cdl ref_quotes.cdl ref_quotes_orig.zip ref_rem.cdl ref_rem.dmp ref_scalar.cdl ref_skip.cdl ref_skip.txt ref_skipw.cdl ref_string.cdl ref_t_meta_dim1.cdl ref_t_meta_var1.cdl ref_ut_json_build.txt ref_ut_json_parse.txt ref_ut_mapapi_create.cdl ref_ut_mapapi_data.cdl ref_ut_mapapi_meta.cdl ref_ut_mapapi_search.txt ref_ut_map_create.cdl ref_ut_map_readmeta2.txt ref_ut_map_readmeta.txt ref_ut_map_search.txt ref_ut_map_writedata.cdl ref_ut_map_writemeta2.cdl ref_ut_map_writemeta.cdl ref_ut_proj.txt ref_ut_testmap_create.cdl ref_whole.cdl ref_whole.txt ref_zarr_test_data.cdl.gz)

foreach(NZ ${NCZARRFILES})
  file(COPY "${netCDF_SOURCE_DIR}/nczarr_test/${NZ}"
      DESTINATION "${netCDF_BINARY_DIR}/v3_nczarr_test/${NZ}"
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endforeach()

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

# Sources that are specific to Zarr V3 testing
set(V3FILES run_nan.sh run_mud.sh)
set(V3DATA ref_nczarr2zarr.cdl ref_purezarr.cdl ref_xarray.cdl ref_misc2.cdl ref_jsonconvention.cdl ref_jsonconvention.zmap ref_nulls_zarr.baseline ref_string_zarr.baseline ref_string_nczarr.baseline ref_zarr_test_data_2d.cdl.gz ref_groups_regular.cdl ref_filtered.cdl ref_any.cdl ref_multi.cdl ref_tst_nans.dmp ref_bzip2.cdl ref_tst_mud4-bc.cdl ref_tst_mud4.cdl ref_tst_mud4_chars.cdl)

# Sources that are copies of same files from nczarr_test
set(TESTFILES_NCZARR test_nczarr.sh test_chunking.c test_filter_vlen.c test_h5_endians.c test_put_vars_two_unlim_dim.c test_quantize.c test_unlim_vars.c tst_pure_awssdk.cpp)

set(TESTFILES_NCZARR ${TESTFILES_NCZARR} run_chunkcases.sh run_corrupt.sh run_external.sh run_fillonlyz.sh run_filter.sh run_filterinstall.sh run_filter_misc.sh run_filter_vlen.sh run_interop.sh run_jsonconvention.sh run_misc.sh run_mud.sh run_nccopy5.sh run_nccopyz.sh run_ncgen4.sh run_nczarr_fill.sh run_nczfilter.sh run_newformat.sh run_notzarr.sh run_nulls.sh run_perf_chunks1.sh run_purezarr.sh run_quantize.sh run_scalar.sh run_specific_filters.sh run_strings.sh run_unknown.sh run_unlim_io.sh run_ut_map.sh run_ut_mapapi.sh run_ut_misc.sh)

set(TESTDATA_NCZARR ref_nulls_nczarr.baseline ref_zarr_test_data.cdl.gz ref_avail1.cdl ref_byte.cdl ref_byte_fill_value_null.cdl ref_fillonly.cdl ref_misc1.cdl ref_ndims.cdl ref_newformatpure.cdl ref_nulls.cdl ref_oldformat.cdl ref_perdimspecs.cdl ref_power_901_constants.cdl ref_purezarr_base.cdl ref_quotes.cdl ref_rem.cdl ref_scalar.cdl ref_skip.cdl ref_skipw.cdl ref_string.cdl ref_t_meta_dim1.cdl ref_t_meta_var1.cdl ref_ut_mapapi_create.cdl ref_ut_mapapi_data.cdl ref_ut_mapapi_meta.cdl ref_ut_map_create.cdl ref_ut_map_writedata.cdl ref_ut_map_writemeta.cdl ref_ut_map_writemeta2.cdl ref_ut_testmap_create.cdl ref_whole.cdl ref_avail1.dmp ref_misc1.dmp ref_ndims.dmp ref_rem.dmp ref_noshape.file.zip ref_groups.h5 ref_notzarr.tar.gz ref_avail1.txt ref_skip.txt ref_ut_json_build.txt ref_ut_json_parse.txt ref_ut_mapapi_search.txt ref_ut_map_readmeta.txt ref_ut_map_readmeta2.txt ref_ut_map_search.txt ref_ut_proj.txt ref_whole.txt ref_byte.zarr.zip ref_byte_fill_value_null.zarr.zip ref_oldformat.zip ref_power_901_constants_orig.zip ref_quotes_orig.zip)

# Add command to update the test source from nczarr_test
add_custom_target(update_testfiles DEPENDS testfiles_nczarr testdata_nczarr)

add_custom_target(testfiles_nczarr
  COMMAND
    foreach(U ${TESTFILES_NCZARR})
      set(SRCPATH "${netCDF_SOURCE_DIR}/nczarr_test/${U}")
      set(DSTPATH "${netCDF_SOURCE_DIR}/v3_nczarr_test/${U}")
      file(REMOVE ${DSTPATH})
      file(COPY ${SRCPATH} DESTINATION ${DSTPATH}
           FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    endforeach()
)

add_custom_target(testdata_nczarr
  COMMAND
    foreach(U ${TESTDATA_NCZARR})
      set(SRCPATH "${netCDF_SOURCE_DIR}/nczarr_test/${U}")
      set(DSTPATH "${netCDF_SOURCE_DIR}/v3_nczarr_test/${U}")
      file(REMOVE ${DSTPATH})
      file(COPY ${SRCPATH} DESTINATION ${DSTPATH})
    endforeach()
)
