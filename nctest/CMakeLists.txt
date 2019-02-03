# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
SET(nctest_SRC add.c atttests.c cdftests.c dimtests.c driver.c emalloc.c error.c misctest.c rec.c slabs.c val.c vardef.c varget.c vargetg.c varput.c varputg.c vartests.c vputget.c vputgetg.c)

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

###
# Can't use macro since
# there are multiple sources.
###
ADD_EXECUTABLE(nctest ${nctest_SRC})
TARGET_LINK_LIBRARIES(nctest netcdf)
ADD_TEST(nctest ${EXECUTABLE_OUTPUT_PATH}/nctest)

add_bin_test_no_prefix(tst_rename)
add_sh_test(nctest compare_test_files)
IF(HAVE_BASH)
  SET_TESTS_PROPERTIES(nctest_compare_test_files PROPERTIES DEPENDS nctest)
ENDIF(HAVE_BASH)
