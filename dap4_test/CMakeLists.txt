# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

SET(abs_top_srcdir ${CMAKE_CURRENT_SOURCE_DIR})

remove_definitions(-DDLL_EXPORT)

ADD_SUBDIRECTORY(baseline)
ADD_SUBDIRECTORY(baselineraw)
ADD_SUBDIRECTORY(baselineremote)
ADD_SUBDIRECTORY(baselinehyrax)
ADD_SUBDIRECTORY(baselinethredds)
ADD_SUBDIRECTORY(cdltestfiles)
ADD_SUBDIRECTORY(rawtestfiles)

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)

FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

IF(NETCDF_ENABLE_DAP_REMOTE_TESTS)
    # Change name (add '4') to avoid cmake
    # complaint about duplicate targets.
    BUILD_BIN_TEST(findtestserver4)
    BUILD_BIN_TEST(pingurl4)
ENDIF()

IF(NETCDF_ENABLE_TESTS)

  # Base tests
  # The tests are set up as a combination of shell scripts and executables that
  # must be run in a particular order. It is painful but will use macros to help
  # keep it from being too bad.

  IF(NETCDF_BUILD_UTILITIES)
      INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/libdap4)
      build_bin_test(test_parse)
      build_bin_test(test_meta)
IF(USE_X_GETOPT)
      build_bin_test(test_data ${CMAKE_SOURCE_DIR}/libdispatch/XGetopt.c)
ELSE()
      build_bin_test(test_data)
ENDIF()
      add_sh_test(dap4_test test_parse)
      add_sh_test(dap4_test test_raw)
      add_sh_test(dap4_test test_meta)
      add_sh_test(dap4_test test_data)
  ENDIF(NETCDF_BUILD_UTILITIES)

  IF(NETCDF_ENABLE_DAP_REMOTE_TESTS)
      add_sh_test(dap4_test test_remote)
      add_sh_test(dap4_test test_hyrax)
      add_sh_test(dap4_test test_dap4url)
    IF(RUN_MANUAL_TESTS)
        # The following test can only be run by hand.
        # It tests earthdata authorization.
        # Before running it, one needs to do the following:
        #     export USERPWD="<username>:<password>"
        # where the username and password are registered
        # with urs.earthdata.nasa.gov.
      add_sh_test(dap4_test test_earthdata)
    ENDIF()
    IF(RUN_IGNORED_TESTS)
      add_sh_test(dap4_test test_thredds)
    ENDIF()
  ENDIF(NETCDF_ENABLE_DAP_REMOTE_TESTS)

ENDIF(NETCDF_ENABLE_TESTS)

#FILE(COPY ./baseline DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
#FILE(COPY ./baselineraw DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
#FILE(COPY ./baselineremote DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
#FILE(COPY ./baselinethredds DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
#FILE(COPY ./baselinehyrax DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
#FILE(COPY ./cdltestfiles DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
#FILE(COPY ./rawtestfiles DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})


