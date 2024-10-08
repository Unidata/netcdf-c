# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

SET(H5TESTS tst_h_files tst_h_files2 tst_h_files4 tst_h_atts tst_h_atts3 tst_h_atts4 tst_h_vars tst_h_vars2 tst_h_vars3 tst_h_grps tst_h_compounds tst_h_compounds2 tst_h_wrt_cmp tst_h_vl tst_h_opaques tst_h_strings tst_h_strings1 tst_h_strings2 tst_h_ints tst_h_dimscales tst_h_dimscales1 tst_h_dimscales2 tst_h_dimscales3 tst_h_enums tst_h_dimscales4 tst_h_rename)

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h5 ${CMAKE_CURRENT_SOURCE_DIR}/*.nc)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

FOREACH(CTEST ${H5TESTS})
  ADD_EXECUTABLE(${CTEST} ${CTEST}.c)
  TARGET_LINK_LIBRARIES(${CTEST}
    netcdf
    HDF5::HDF5 hdf5::hdf5_hl
    )
  ADD_TEST(${CTEST} ${EXECUTABLE_OUTPUT_PATH}/${CTEST})
ENDFOREACH()

IF(TEST_PARALLEL4)
  build_bin_test(tst_h_par)
  build_bin_test(tst_h_par_compress)
  add_sh_test(h5_test run_par_tests)
ENDIF()

# If zstd is enabled, run a test for it.
if(NETCDF_ENABLE_PLUGINS)
  if(NETCDF_ENABLE_FILTER_ZSTD)
    build_bin_test(tst_h_zstd)
    add_sh_test(h5_test run_h5_zstd_tests)    
  endif()
endif()

# Copy the shell file to build directory.
file(COPY run_h5_zstd_tests.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)



