# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
#IF(BUILD_SHARED_LIBS AND WIN32)
#  remove_definitions(-DDLL_EXPORT)
#  remove_definitions(-DDLL_NETCDF)
#ENDIF()

set(RCMERGE OFF)

if(USE_X_GETOPT)
  set(XGETOPTSRC "${CMAKE_CURRENT_SOURCE_DIR}/../libdispatch/XGetopt.c")
endif()

set(ncdump_FILES ncdump.c vardata.c dumplib.c indent.c nctime0.c utils.c nciter.c ${XGETOPTSRC})
set(nccopy_FILES nccopy.c nciter.c chunkspec.c utils.c dimmap.c list.c ${XGETOPTSRC})
set(ocprint_FILES ocprint.c ${XGETOPTSRC})
set(ncvalidator_FILES ncvalidator.c ${XGETOPTSRC})
set(printfqn_FILES printfqn.c ${XGETOPTSRC})
set(ncpathcvt_FILES ncpathcvt.c ${XGETOPTSRC})
set(ncfilteravail_FILES ncfilteravail.c ${XGETOPTSRC})
set(nchdf5version_FILES nchdf5version.c)
set(echon_FILES echon.c ${XGETOPTSRC})

## 
# Turn off inclusion of particular files when using the cmake-native
# option to turn on Unity Builds.  
#
# For more information, see:
#  * https://github.com/Unidata/netcdf-c/pull/2839/
#  * https://cmake.org/cmake/help/latest/prop_tgt/UNITY_BUILD.html
#  * https://cmake.org/cmake/help/latest/prop_tgt/UNITY_BUILD_MODE.html#prop_tgt:UNITY_BUILD_MODE
##
set_property(SOURCE dumplib.c
  PROPERTY
    SKIP_UNITY_BUILD_INCLUSION ON)

add_executable(ncdump ${ncdump_FILES})
add_executable(nccopy ${nccopy_FILES})
add_executable(ncvalidator ${ncvalidator_FILES})
add_executable(ncpathcvt ${ncpathcvt_FILES})
add_executable(ncfilteravail ${ncfilteravail_FILES})
add_executable(echon ${echon_FILES})

if(USE_HDF5)
  add_executable(nc4print nc4print.c nc4printer.c)
  target_link_libraries(nc4print netcdf ${ALL_TLL_LIBS})

  add_executable(printfqn  ${printfqn_FILES})
  target_link_libraries(printfqn netcdf ${ALL_TLL_LIBS})

  add_executable(nchdf5version ${nchdf5version_FILES})
  target_link_libraries(nchdf5version netcdf ${ALL_TLL_LIBS})
endif(USE_HDF5)

if(NETCDF_ENABLE_DAP)
  add_executable(ocprint ${ocprint_FILES})
endif(NETCDF_ENABLE_DAP)

target_link_libraries(ncdump netcdf ${ALL_TLL_LIBS})
target_link_libraries(nccopy netcdf ${ALL_TLL_LIBS})
target_link_libraries(ncvalidator netcdf ${ALL_TLL_LIBS})
target_link_libraries(ncpathcvt netcdf ${ALL_TLL_LIBS})
target_link_libraries(ncfilteravail netcdf ${ALL_TLL_LIBS})
target_link_libraries(echon netcdf ${ALL_TLL_LIBS})

if(NETCDF_ENABLE_DAP)
  target_link_libraries(ocprint netcdf ${ALL_TLL_LIBS})
endif(NETCDF_ENABLE_DAP)

####
# We have to do a little tweaking
# to remove the Release/ and Debug/ directories
# in MSVC builds.  This is required to get
# test scripts to work.
####

if(MSVC)
  macro(setbinprops name)
    set_target_properties(${name} 
      PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
    )
  endmacro()

  setbinprops(ncdump)
  setbinprops(nccopy)
  setbinprops(ncvalidator)
  setbinprops(ncpathcvt)
  setbinprops(ncfilteravail)
  setbinprops(echon)

  if(USE_HDF5)
    setbinprops(printfqn)
    setbinprops(nchdf5version)
  endif(USE_HDF5)

  if(NETCDF_ENABLE_DAP)
    setbinprops(ocprint)
  endif(NETCDF_ENABLE_DAP)


endif()

if(NETCDF_ENABLE_TESTS)

  add_executable(rewrite-scalar rewrite-scalar.c)
  add_executable(bom bom.c)
  add_executable(tst_dimsizes tst_dimsizes.c)
  add_executable(nctrunc nctrunc.c)
if(RCMERGE)
  add_executable(tst_rcmerge tst_rcmerge.c)
  add_executable(tst_rcmerge tst_rcapi.c)
endif()
  target_link_libraries(rewrite-scalar netcdf)
  target_link_libraries(bom netcdf)
  target_link_libraries(tst_dimsizes netcdf)
  target_link_libraries(nctrunc netcdf)
if(RCMERGE)
  target_link_libraries(tst_rcmerge netcdf)
  target_link_libraries(tst_rcapi netcdf)
endif()

  IF(USE_HDF5)
    add_executable(tst_fileinfo tst_fileinfo.c)
    target_link_libraries(tst_fileinfo netcdf ${ALL_TLL_LIBS})
  ENDIF()

  IF(MSVC)
    set_target_properties(rewrite-scalar 
      PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
    )

    set_target_properties(bom 
      PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
    )

    set_target_properties(tst_dimsizes 
      PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
    )

    set_target_properties(nctrunc 
      PROPERTIES 
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
    )

    if(RCMERGE)
        set_target_properties(tst_rcmerge 
          PROPERTIES 
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
        )

        set_target_properties(tst_rcapi 
          PROPERTIES 
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
        )
    endif()

    if(USE_HDF5)
      set_target_properties(tst_fileinfo 
        PROPERTIES 
          RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
          RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
      )
    endif(USE_HDF5)
  endif(MSVC)

  # Build support programs
  build_bin_test_no_prefix(tst_utf8)
  build_bin_test_no_prefix(tst_fillbug)
  if(USE_HDF5)
    build_bin_test_no_prefix(tst_h_rdc0)
    build_bin_test_no_prefix(tst_unicode)
    build_bin_test_no_prefix(tst_vlen_data)
    build_bin_test_no_prefix(tst_create_files)
    build_bin_test_no_prefix(tst_opaque_data)
    build_bin_test_no_prefix(tst_string_data)
    build_bin_test_no_prefix(tst_comp2)
    build_bin_test_no_prefix(tst_nans)
    build_bin_test_no_prefix(tst_h_scalar)
    build_bin_test_no_prefix(tst_compress)
    build_bin_test_no_prefix(tst_chunking)
    build_bin_test_no_prefix(tst_group_data)
    build_bin_test_no_prefix(tst_enum_data)
    build_bin_test_no_prefix(tst_enum_undef)
    build_bin_test_no_prefix(tst_comp)
    # Add this test by hand, as it is also called from a script.
    # Editing the script would break autotools compatibility.
    build_bin_test_no_prefix(tst_special_atts)
  endif(USE_HDF5)

  # Base tests
  # The tests are set up as a combination of shell scripts and executables that
  # must be run in a particular order. It is painful but will use macros to help
  # keep it from being too bad.

  if(HAVE_BASH)

  # Support programs
  add_bin_test_no_prefix(ref_ctest)
  add_bin_test_no_prefix(ref_ctest64)

  ## Start adding tests in the appropriate order
  add_sh_test(ncdump run_tests)
  add_sh_test(ncdump tst_64bit)
  add_sh_test(ncdump tst_lengths)
  add_sh_test(ncdump tst_calendars)
  add_sh_test(ncdump run_utf8_tests)

  add_sh_test(ncdump tst_nccopy3_subset)
  add_sh_test(ncdump tst_charfill)
  add_sh_test(ncdump tst_formatx3)
  add_sh_test(ncdump tst_bom)
  add_sh_test(ncdump tst_dimsizes)
  add_sh_test(ncdump tst_inmemory_nc3)
  add_sh_test(ncdump tst_nccopy_w3)
  add_sh_test(ncdump run_ncgen_tests)
  add_sh_test(ncdump tst_inttags)
  add_sh_test(ncdump test_radix)
  add_sh_test(ncdump tst_ctests)

  add_sh_test(ncdump tst_null_byte_padding)
  if(USE_STRICT_NULL_BYTE_HEADER_PADDING)
    SET_TESTS_PROPERTIES(ncdump_tst_null_byte_padding PROPERTIES WILL_FAIL TRUE)
  endif(USE_STRICT_NULL_BYTE_HEADER_PADDING)

  if(NOT MSVC AND NOT MINGW)
    add_sh_test(ncdump tst_output)
    add_sh_test(ncdump tst_nccopy3)
    # Known failure on MSVC; the number of 0's padding
    # is different, but the result is actually correct.
    if(USE_HDF5)
      add_sh_test(ncdump tst_netcdf4)
    endif()

    set_tests_properties(ncdump_tst_nccopy3 PROPERTIES DEPENDS
      "ncdump_tst_calendars;ncdump_run_utf8_tests;ncdump_tst_output;ncdump_tst_64bit;ncdump_run_tests;ncdump_tst_lengths")
    set_tests_properties(ncdump_tst_nccopy3 PROPERTIES RUN_SERIAL TRUE)

  endif()

  if(USE_HDF5)
    add_sh_test(ncdump tst_formatx4)
    add_sh_test(ncdump_sh tst_fillbug)
    add_sh_test(ncdump_shell tst_h_scalar)
    add_sh_test(ncdump tst_mud)
    add_sh_test(ncdump tst_grp_spec)
    add_sh_test(ncdump tst_inttags4)
    add_sh_test(ncdump run_utf8_nc4_tests)
    add_sh_test(ncdump tst_fileinfo)
    add_sh_test(ncdump tst_hdf5_offset)
    add_sh_test(ncdump tst_inmemory_nc4)
    add_sh_test(ncdump tst_nccopy_w4)
    add_sh_test(ncdump run_ncgen_nc4_tests)
    add_sh_test(ncdump tst_ncgen4)
    add_sh_test(ncdump tst_netcdf4_4)
    add_sh_test(ncdump tst_nccopy4)
    set_tests_properties(ncdump_tst_nccopy4 PROPERTIES DEPENDS
      "ncdump_ncgen_tests;ncdump_tst_ncgen4;ncdump_tst_fillbug;ncdump_tst_netcdf4_4;ncdump_tst_h_scalar;ncdump_tst_netcdf4;ncdump_run_ncgen_tests;ncdump_tst_output;ncdump_tst_ncgen4;ncdump_sh_tst_fillbug;tst_comp;tst_comp2;tst_nans;tst_opaque_data;tst_create_files;tst_special_atts;ncdump_tst_nccopy3")
    set_tests_properties(ncdump_tst_nccopy4 PROPERTIES RUN_SERIAL TRUE)
    add_sh_test(ncdump tst_calendars_nc4)
    add_sh_test(ncdump tst_nccopy5)
    set_tests_properties(ncdump_tst_nccopy5 PROPERTIES DEPENDS "ncdump_tst_nccopy4")

  endif(USE_HDF5)

  # The following test script invokes
  # gcc directly.
  if(CMAKE_COMPILER_IS_GNUCC OR APPLE)
    if(NETCDF_ENABLE_LARGE_FILE_TESTS)
      add_sh_test(ncdump tst_iter)
    endif(NETCDF_ENABLE_LARGE_FILE_TESTS)
  endif(CMAKE_COMPILER_IS_GNUCC OR APPLE)

  ###
  # This test fails on Visual Studio builds with bash.
  # It passes, but technically fails because the scientific
  # formatting omits a 0.
  ###
  if(EXTRA_TESTS)
    if(USE_HDF5)
        if(NOT MSVC AND NOT MINGW)
          add_sh_test(ncdump run_back_comp_tests)
        endif()
      endif()
  endif(EXTRA_TESTS)

  # The unicode tests
  if(NOT ISMINGW)
    add_sh_test(ncdump test_unicode_directory)
    add_sh_test(ncdump test_unicode_path)
  ENDIF()

  if(USE_CDF5)
    add_sh_test(ncdump test_keywords)
  ENDIF()

  if(USE_HDF5)
    add_sh_test(ncdump test_scope)
  ENDIF()

  if(RCMERGE)
    add_sh_test(ncdump test_rcmerge)
  endif()

endif(HAVE_BASH)

endif(NETCDF_ENABLE_TESTS)

#IF(MSVC)
#  set_target_properties(ncdump
#    PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
#    )
#  set_target_properties(nccopy
#    PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
#    )
#  set_target_properties(ncvalidator
#    PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
#    )
#  IF(NETCDF_ENABLE_DAP)
#    set_target_properties(ocprint
#      PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
#      )
#  ENDIF(NETCDF_ENABLE_DAP)
#ENDIF()

install(TARGETS ncdump RUNTIME DESTINATION bin COMPONENT utilities)
install(TARGETS nccopy RUNTIME DESTINATION bin COMPONENT utilities)

set(MAN_FILES nccopy.1 ncdump.1)

# Note, the L512.bin file is file containing exactly 512 bytes each of value 0.
# It is used for creating hdf5 files with varying offsets for testing.

file(GLOB COPY_FILES ${CMAKE_BINARY_DIR}/ncgen/*.nc ${CMAKE_BINARY_DIR}/nc_test4/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.ncml ${CMAKE_CURRENT_SOURCE_DIR}/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl ${CMAKE_CURRENT_SOURCE_DIR}/*.sh ${CMAKE_CURRENT_SOURCE_DIR}/*.1 ${CMAKE_CURRENT_SOURCE_DIR}/L512.bin ${CMAKE_CURRENT_SOURCE_DIR}/ref_ctest*.c )
file(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

add_subdirectory(cdl)
add_subdirectory(expected)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEANFILES}")

if(NOT MSVC)
  install(FILES ${MAN_FILES} DESTINATION "${CMAKE_INSTALL_MANDIR}/man1" COMPONENT documentation)
endif()
