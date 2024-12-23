# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018, 2019
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

# This is the cmake build file for unit_test/ directory.
# @author Ward Fisher

IF(USE_X_GETOPT)
  SET(XGETOPTSRC "${CMAKE_CURRENT_SOURCE_DIR}/../libdispatch/XGetopt.c")
ENDIF()

# Currently (August 21, 2019): Some tests will not work with
# Visual Studio. The unit tests are for internal netCDF functions,
# so we don't want to make them external, which would be required to
# run on Windows.

SET(UNIT_TESTS test_ncuri)
add_bin_test(unit_test test_ncuri)

IF(NETCDF_ENABLE_HDF5)
  IF(NOT WIN32)
    add_bin_test(unit_test tst_nc4internal)
  ENDIF(NOT WIN32)
  build_bin_test(tst_reclaim ${XGETOPTSRC})
  add_sh_test(unit_test run_reclaim_tests)
  if(NETCDF_ENABLE_NCZARR_FILTERS)
    build_bin_test(tst_pluginpaths ${XGETOPTSRC})
    add_sh_test(unit_test run_pluginpaths)
    add_sh_test(unit_test run_dfaltpluginpath)
    build_bin_test(ncpluginpath ${XGETOPTSRC})
    add_sh_test(unit_test run_dfaltpluginpath)
  endif()
ENDIF(NETCDF_ENABLE_HDF5)

#This needs repair
# Path convert test(s)
#add_bin_test(unit_test test_pathcvt)

IF(NETCDF_BUILD_UTILITIES)
  IF(NETCDF_ENABLE_S3 AND WITH_S3_TESTING)
  # SDK Test
    build_bin_test(test_s3sdk ${XGETOPTSRC})
    add_sh_test(unit_test run_s3sdk)
  #AWS Configuration test
    build_bin_test(aws_config)
    add_sh_test(unit_test run_aws_config)
  ENDIF()
ENDIF()

# Performance tests
if(BUILD_BENCHMARKS)
add_bin_test(unit_test tst_exhash timer_utils.c)
add_bin_test(unit_test tst_xcache timer_utils.c)
endif()

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)
