# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018, 2019
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

# This is the cmake build file for unit_test/ directory.
# @author Ward Fisher

# Currently (August 21, 2019): Some tests will not work with
# Visual Studio. The unit tests are for internal netCDF functions,
# so we don't want to make them external, which would be required to
# run on Windows.

SET(UNIT_TESTS test_ncuri test_pathcvt)

IF(NOT MSVC)
  IF(ENABLE_NETCDF_4)
    SET(UNIT_TESTS ${UNIT_TESTS} tst_nclist tst_nc4internal)
  ENDIF(ENABLE_NETCDF_4)
ENDIF(NOT MSVC)

FOREACH(CTEST ${UNIT_TESTS})
  add_bin_test(unit_test ${CTEST})
ENDFOREACH()

# Performance tests
add_bin_test(unit_test tst_exhash timer_utils.c)
add_bin_test(unit_test tst_xcache timer_utils.c)

