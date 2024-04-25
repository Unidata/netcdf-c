# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.


IF(USE_X_GETOPT)
  SET(XGETOPTSRC "${CMAKE_CURRENT_SOURCE_DIR}/../libdispatch/XGetopt.c")
ENDIF()

# See netcdf-c/COPYRIGHT file for more info.
SET(ncgen3_FILES main.c load.c escapes.c getfill.c init.c genlib.c ncgeny.c ${XGETOPTSRC})

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.sh ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

# With this option enabled, automatically generate the parser source
# files from the yacc input files when they're changed. They don't
# change very often, so this option is off by default.
if (NETCDF_GENERATE_NCGEN)
  find_program(FLEX flex REQUIRED)
  find_program(BISON bison REQUIRED)

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ncgeny.h ${CMAKE_CURRENT_SOURCE_DIR}/ncgeny.c ${CMAKE_CURRENT_SOURCE_DIR}/ncgenl.c
    DEPENDS ncgen.y ncgen.l
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}

    COMMAND ${FLEX} -Pncg -8 ncgen/ncgen.l
    COMMAND mv lex.ncg.c "${CMAKE_CURRENT_SOURCE_DIR}/ncgenl.c"

    COMMAND ${BISON} -pncg -t -d ncgen/ncgen.y
    COMMAND mv ncgen.tab.c "${CMAKE_CURRENT_SOURCE_DIR}/ncgeny.c"
    COMMAND mv ncgen.tab.h "${CMAKE_CURRENT_SOURCE_DIR}/ncgeny.h"

    # Fix the `#line` preprocessor directives with the correct source paths
    COMMAND sed -i s/ncgen.tab/ncgeny/ "${CMAKE_CURRENT_SOURCE_DIR}/ncgeny.c"
    COMMAND sed -i s/lex.ncg/ncgenl/ "${CMAKE_CURRENT_SOURCE_DIR}/ncgenl.c"
    COMMAND sed -i s/ncgen.tab/ncgeny/ "${CMAKE_CURRENT_SOURCE_DIR}/ncgeny.h"
    VERBATIM
    )
endif()

ADD_EXECUTABLE(ncgen3 ${ncgen3_FILES})
TARGET_LINK_LIBRARIES(ncgen3 netcdf ${ALL_TLL_LIBS})

####
# We have to do a little tweaking
# to remove the Release/ and Debug/ directories
# in MSVC builds.  This is required to get
# test scripts to work.
####
IF(MSVC)
  SET_TARGET_PROPERTIES(ncgen3 PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR})
  SET_TARGET_PROPERTIES(ncgen3 PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
    ${CMAKE_CURRENT_BINARY_DIR})
  SET_TARGET_PROPERTIES(ncgen3 PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
    ${CMAKE_CURRENT_BINARY_DIR})
ENDIF()



IF(ENABLE_TESTS)
  SET(NCGEN3_TESTS run_tests)
  IF(USE_HDF5)
    SET(NCGEN3_TESTS ${NCGEN3_TESTS} run_nc4_tests)
  ENDIF()

  FOREACH(F ${NCGEN3_TESTS})
    add_sh_test(ncgen3 ${F})
  ENDFOREACH()
ENDIF()
INSTALL(TARGETS ncgen3 DESTINATION bin COMPONENT utilities)

SET(MAN_FILES ncgen3.1)
IF(NOT MSVC)
  INSTALL(FILES ${MAN_FILES} DESTINATION "share/man/man1" COMPONENT documentation)
ENDIF()

## Specify files to be distributed by 'make dist'
FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am ncgen.y ncgenyy.c ncgen.l c0.cdl ncgen3.1)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")


SET(CLEANFILES c0.nc c0_64.nc c0_4.nc c0_4c.nc)
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEANFILES}")
