# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
#IF(BUILD_SHARED_LIBS AND WIN32)
#  remove_definitions(-DDLL_EXPORT)
#  remove_definitions(-DDLL_NETCDF)
#ENDIF()

SET(RCMERGE OFF)

SET(ncdump_FILES ncdump.c vardata.c dumplib.c indent.c nctime0.c utils.c nciter.c)
SET(nccopy_FILES nccopy.c nciter.c chunkspec.c utils.c dimmap.c list.c)
SET(ocprint_FILES ocprint.c)
SET(ncvalidator_FILES ncvalidator.c)
SET(printfqn_FILES printfqn.c)
SET(ncpathcvt_FILES ncpathcvt.c)
SET(ncfilteravail_FILES ncfilteravail.c)
SET(nchdf5version_FILES nchdf5version.c)

IF(USE_X_GETOPT)
  SET(ncdump_FILES ${ncdump_FILES} XGetopt.c)
  SET(nccopy_FILES ${nccopy_FILES} XGetopt.c)
  SET(ocprint_FILES ${ocprint_FILES} XGetopt.c)
  SET(ncvalidator_FILES ${ncvalidator_FILES} XGetopt.c)
  SET(printfqn_FILES ${printfqn_FILES} XGetopt.c)
  SET(ncpathcvt_FILES ${ncpathcvt_FILES} XGetopt.c)
  SET(ncfilteravail_FILES ${ncfilteravail_FILES} XGetopt.c)
ENDIF(USE_X_GETOPT)

ADD_EXECUTABLE(ncdump ${ncdump_FILES})
ADD_EXECUTABLE(nccopy ${nccopy_FILES})
ADD_EXECUTABLE(ncvalidator ${ncvalidator_FILES})
ADD_EXECUTABLE(ncpathcvt ${ncpathcvt_FILES})
ADD_EXECUTABLE(ncfilteravail ${ncfilteravail_FILES})

IF(USE_HDF5)
  ADD_EXECUTABLE(nc4print nc4print.c nc4printer.c)
  ADD_EXECUTABLE(printfqn  ${printfqn_FILES})
  ADD_EXECUTABLE(nchdf5version ${nchdf5version_FILES})
ENDIF(USE_HDF5)

IF(ENABLE_DAP)
  ADD_EXECUTABLE(ocprint ${ocprint_FILES})
ENDIF(ENABLE_DAP)

TARGET_LINK_LIBRARIES(ncdump netcdf ${ALL_TLL_LIBS})
TARGET_LINK_LIBRARIES(nccopy netcdf ${ALL_TLL_LIBS})
TARGET_LINK_LIBRARIES(ncvalidator netcdf ${ALL_TLL_LIBS})
TARGET_LINK_LIBRARIES(ncpathcvt netcdf ${ALL_TLL_LIBS})
TARGET_LINK_LIBRARIES(ncfilteravail netcdf ${ALL_TLL_LIBS})

IF(USE_HDF5)
  TARGET_LINK_LIBRARIES(nc4print netcdf ${ALL_TLL_LIBS})
  TARGET_LINK_LIBRARIES(printfqn netcdf ${ALL_TLL_LIBS})
  TARGET_LINK_LIBRARIES(nchdf5version netcdf ${ALL_TLL_LIBS})
ENDIF(USE_HDF5)

IF(ENABLE_DAP)
  TARGET_LINK_LIBRARIES(ocprint netcdf ${ALL_TLL_LIBS})
ENDIF(ENABLE_DAP)

####
# We have to do a little tweaking
# to remove the Release/ and Debug/ directories
# in MSVC builds.  This is required to get
# test scripts to work.
####

IF(MSVC)
  macro(setbinprops name)
    SET_TARGET_PROPERTIES(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
      ${CMAKE_CURRENT_BINARY_DIR})
  endmacro()

  setbinprops(ncdump)
  setbinprops(nccopy)
  setbinprops(ncvalidator)
  setbinprops(ncpathcvt)
  setbinprops(ncfilteravail)

  IF(USE_HDF5)
    setbinprops(printfqn)
    setbinprops(nchdf5version)
  ENDIF(USE_HDF5)

  IF(ENABLE_DAP)
    setbinprops(ocprint)
  ENDIF(ENABLE_DAP)


ENDIF()

IF(ENABLE_TESTS)

  ADD_EXECUTABLE(rewrite-scalar rewrite-scalar.c)
  ADD_EXECUTABLE(bom bom.c)
  ADD_EXECUTABLE(tst_dimsizes tst_dimsizes.c)
  ADD_EXECUTABLE(nctrunc nctrunc.c)
if(RCMERGE)
  ADD_EXECUTABLE(tst_rcmerge tst_rcmerge.c)
  ADD_EXECUTABLE(tst_rcmerge tst_rcapi.c)
endif()
  TARGET_LINK_LIBRARIES(rewrite-scalar netcdf)
  TARGET_LINK_LIBRARIES(bom netcdf)
  TARGET_LINK_LIBRARIES(tst_dimsizes netcdf)
  TARGET_LINK_LIBRARIES(nctrunc netcdf)
if(RCMERGE)
  TARGET_LINK_LIBRARIES(tst_rcmerge netcdf)
  TARGET_LINK_LIBRARIES(tst_rcapi netcdf)
endif()

  IF(USE_HDF5)
    ADD_EXECUTABLE(tst_fileinfo tst_fileinfo.c)
    TARGET_LINK_LIBRARIES(tst_fileinfo netcdf)
  ENDIF()

  IF(MSVC)
    SET_TARGET_PROPERTIES(rewrite-scalar PROPERTIES RUNTIME_OUTPUT_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(rewrite-scalar PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(rewrite-scalar PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
      ${CMAKE_CURRENT_BINARY_DIR})

    SET_TARGET_PROPERTIES(bom PROPERTIES RUNTIME_OUTPUT_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(bom PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(bom PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
      ${CMAKE_CURRENT_BINARY_DIR})

    SET_TARGET_PROPERTIES(tst_dimsizes PROPERTIES RUNTIME_OUTPUT_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(tst_dimsizes PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(tst_dimsizes PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
      ${CMAKE_CURRENT_BINARY_DIR})

    SET_TARGET_PROPERTIES(nctrunc PROPERTIES RUNTIME_OUTPUT_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(nctrunc PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(nctrunc PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
      ${CMAKE_CURRENT_BINARY_DIR})

    IF(RCMERGE)
        SET_TARGET_PROPERTIES(tst_rcmerge PROPERTIES RUNTIME_OUTPUT_DIRECTORY
          ${CMAKE_CURRENT_BINARY_DIR})
        SET_TARGET_PROPERTIES(tst_rcmerge PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
          ${CMAKE_CURRENT_BINARY_DIR})
        SET_TARGET_PROPERTIES(tst_rcmerge PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
        SET_TARGET_PROPERTIES(tst_rcapi PROPERTIES RUNTIME_OUTPUT_DIRECTORY
          ${CMAKE_CURRENT_BINARY_DIR})
        SET_TARGET_PROPERTIES(tst_rcapi PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
          ${CMAKE_CURRENT_BINARY_DIR})
        SET_TARGET_PROPERTIES(tst_rcapi PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
    endif()

    IF(USE_HDF5)
      SET_TARGET_PROPERTIES(tst_fileinfo PROPERTIES RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_CURRENT_BINARY_DIR})
      SET_TARGET_PROPERTIES(tst_fileinfo PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
        ${CMAKE_CURRENT_BINARY_DIR})
      SET_TARGET_PROPERTIES(tst_fileinfo PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
        ${CMAKE_CURRENT_BINARY_DIR})

    ENDIF(USE_HDF5)
  ENDIF(MSVC)

  # Build support programs
  build_bin_test_no_prefix(tst_utf8)
  build_bin_test_no_prefix(tst_fillbug)
  IF(USE_HDF5)
    build_bin_test_no_prefix(tst_h_rdc0)
    build_bin_test_no_prefix(tst_unicode)
    build_bin_test_no_prefix(tst_vlen_data)
    add_bin_test_no_prefix(tst_create_files)
    add_bin_test_no_prefix(tst_opaque_data)
    add_bin_test_no_prefix(tst_string_data)
    add_bin_test_no_prefix(tst_comp2)
    add_bin_test_no_prefix(tst_nans)
    add_bin_test_no_prefix(tst_h_scalar)
    add_bin_test_no_prefix(tst_compress)
    add_bin_test_no_prefix(tst_chunking)
    add_bin_test_no_prefix(tst_group_data)
    add_bin_test_no_prefix(tst_enum_data)
    add_bin_test_no_prefix(tst_enum_undef)
    add_bin_test_no_prefix(tst_comp)
    # Add this test by hand, as it is also called from a script.
    # Editing the script would break autotools compatibility.
    add_bin_test_no_prefix(tst_special_atts)
  ENDIF(USE_HDF5)

  # Base tests
  # The tests are set up as a combination of shell scripts and executables that
  # must be run in a particular order. It is painful but will use macros to help
  # keep it from being too bad.

  IF(HAVE_BASH)

  ## Start adding tests in the appropriate order
  add_bin_test_no_prefix(ref_ctest)
  add_bin_test_no_prefix(ref_ctest64)

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
  IF(USE_STRICT_NULL_BYTE_HEADER_PADDING)
    SET_TESTS_PROPERTIES(ncdump_tst_null_byte_padding PROPERTIES WILL_FAIL TRUE)
  ENDIF(USE_STRICT_NULL_BYTE_HEADER_PADDING)

  IF(NOT MSVC AND NOT MINGW)
    add_sh_test(ncdump tst_output)
    add_sh_test(ncdump tst_nccopy3)
    # Known failure on MSVC; the number of 0's padding
    # is different, but the result is actually correct.
    if(USE_HDF5)
      add_sh_test(ncdump tst_netcdf4)
    endif()

    SET_TESTS_PROPERTIES(ncdump_tst_nccopy3 PROPERTIES DEPENDS
        "ncdump_tst_calendars;ncdump_run_utf8_tests;ncdump_tst_output;ncdump_tst_64bit;ncdump_run_tests;ncdump_tst_lengths")

  ENDIF()

  IF(USE_HDF5)
    add_sh_test(ncdump tst_formatx4)
    add_sh_test(ncdump_sh tst_fillbug)
    add_sh_test(ncdump_shell tst_h_scalar)
    add_sh_test(ncdump tst_mud)
    add_sh_test(ncdump tst_grp_spec)
    add_sh_test(ncdump tst_nccopy5)
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
    add_sh_test(ncdump tst_calendars_nc4)

    SET_TESTS_PROPERTIES(ncdump_tst_nccopy4 PROPERTIES DEPENDS "ncdump_run_ncgen_tests;ncdump_tst_output;ncdump_tst_ncgen4;ncdump_sh_tst_fillbug;ncdump_tst_netcdf4_4;ncdump_tst_h_scalar;tst_comp;tst_comp2;tst_nans;tst_opaque_data;tst_create_files;tst_special_atts")
    SET_TESTS_PROPERTIES(ncdump_tst_nccopy5 PROPERTIES DEPENDS "ncdump_tst_nccopy4")

  ENDIF(USE_HDF5)

  # The following test script invokes
  # gcc directly.
  IF(CMAKE_COMPILER_IS_GNUCC OR APPLE)
    IF(ENABLE_LARGE_FILE_TESTS)
      add_sh_test(ncdump tst_iter)
    ENDIF(ENABLE_LARGE_FILE_TESTS)
  ENDIF(CMAKE_COMPILER_IS_GNUCC OR APPLE)

  ###
  # This test fails on Visual Studio builds with bash.
  # It passes, but technically fails because the scientific
  # formatting omits a 0.
  ###
  IF(EXTRA_TESTS)
    IF(USE_HDF5)
        IF(NOT MSVC AND NOT MINGW)
          add_sh_test(ncdump run_back_comp_tests)
        ENDIF()
      ENDIF()
  ENDIF(EXTRA_TESTS)

  # The unicode tests
  if(NOT ISMINGW)
    add_sh_test(ncdump test_unicode_directory)
    add_sh_test(ncdump test_unicode_path)
  ENDIF()

  IF(USE_CDF5)
    add_sh_test(ncdump test_keywords)
  ENDIF()

  IF(USE_HDF5)
    add_sh_test(ncdump test_scope)
  ENDIF()

  if(RCMERGE)
    add_sh_test(ncdump test_rcmerge)
  endif()

ENDIF(HAVE_BASH)

ENDIF(ENABLE_TESTS)

#IF(MSVC)
#  SET_TARGET_PROPERTIES(ncdump
#    PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
#    )
#  SET_TARGET_PROPERTIES(nccopy
#    PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
#    )
#  SET_TARGET_PROPERTIES(ncvalidator
#    PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
#    )
#  IF(ENABLE_DAP)
#    SET_TARGET_PROPERTIES(ocprint
#      PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
#      )
#  ENDIF(ENABLE_DAP)
#ENDIF()

INSTALL(TARGETS ncdump RUNTIME DESTINATION bin COMPONENT utilities)
INSTALL(TARGETS nccopy RUNTIME DESTINATION bin COMPONENT utilities)

SET(MAN_FILES nccopy.1 ncdump.1)

# Note, the L512.bin file is file containing exactly 512 bytes each of value 0.
# It is used for creating hdf5 files with varying offsets for testing.

FILE(GLOB COPY_FILES ${CMAKE_BINARY_DIR}/ncgen/*.nc ${CMAKE_BINARY_DIR}/nc_test4/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.ncml ${CMAKE_CURRENT_SOURCE_DIR}/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl ${CMAKE_CURRENT_SOURCE_DIR}/*.sh ${CMAKE_CURRENT_SOURCE_DIR}/*.1 ${CMAKE_CURRENT_SOURCE_DIR}/L512.bin ${CMAKE_CURRENT_SOURCE_DIR}/ref_ctest*.c )
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

ADD_SUBDIRECTORY(cdl)
ADD_SUBDIRECTORY(expected)

SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEANFILES}")

IF(NOT MSVC)
  INSTALL(FILES ${MAN_FILES} DESTINATION "share/man/man1" COMPONENT documentation)
ENDIF()
