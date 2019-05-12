# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

SET(exam_C_tests simple_xy_wr simple_xy_rd sfc_pres_temp_wr sfc_pres_temp_rd pres_temp_4D_wr pres_temp_4D_rd format)
SET(exam_C_tests_source "")
FOREACH(F ${exam_C_tests})
  set(exam_C_tests_source ${exam_C_test_source} ${F}.c)
ENDFOREACH()

FOREACH(F ${exam_C_tests})
  add_bin_test(C_tests ${F})
ENDFOREACH()

SET_TESTS_PROPERTIES(C_tests_simple_xy_rd PROPERTIES DEPENDS C_tests_simple_xy_wr)
SET_TESTS_PROPERTIES(C_tests_sfc_pres_temp_rd PROPERTIES DEPENDS C_tests_sfc_pres_temp_wr)
SET_TESTS_PROPERTIES(C_tests_pres_temp_4D_rd PROPERTIES DEPENDS C_tests_pres_temp_4D_wr)

SET(CLEANFILES *.nc)
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEANFILES}")
