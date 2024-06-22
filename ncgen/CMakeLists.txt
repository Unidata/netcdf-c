# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
IF(BUILD_SHARED_LIBS AND WIN32)
  remove_definitions(-DDLL_EXPORT)
  remove_definitions(-DDLL_NETCDF)
ENDIF()

IF(USE_X_GETOPT)
  SET(XGETOPTSRC "${CMAKE_CURRENT_SOURCE_DIR}/../libdispatch/XGetopt.c")
ENDIF()

SET(ncgen_FILES bindata.c bytebuffer.c cdata.c cvt.c data.c
debug.c dump.c escapes.c f77data.c genbin.c
genc.c genchar.c generate.c generr.c genf77.c
genj.c genlib.c getfill.c jdata.c list.c
main.c ncgeny.c semantics.c
util.c bytebuffer.h data.h debug.h dump.h
generate.h generr.h genlib.h includes.h list.h
ncgen.h ncgeny.h util.h ${XGETOPTSRC})

## 
# Turn off inclusion of particular files when using the cmake-native
# option to turn on Unity Builds.  
#
# For more information, see:
#  * https://github.com/Unidata/netcdf-c/pull/2839/
#  * https://cmake.org/cmake/help/latest/prop_tgt/UNITY_BUILD.html
#  * https://cmake.org/cmake/help/latest/prop_tgt/UNITY_BUILD_MODE.html#prop_tgt:UNITY_BUILD_MODE
##
set_property(SOURCE data.c
  PROPERTY
    SKIP_UNITY_BUILD_INCLUSION ON)

# Obsolete
SET(OBSOLETE odom.c odom.h jdatastd.c jdatajni.c genjni.c cdfdata.c cmldata.c)

# don't add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

ADD_EXECUTABLE(ncgen ${ncgen_FILES})
TARGET_LINK_LIBRARIES(ncgen netcdf ${ALL_TLL_LIBS})

# Given a netcdf4 file, dump the actual chunk contents.
# Used to validate nczarr chunking code.

####
# We have to do a little tweaking
# to remove the Release/ and Debug/ directories
# in Windows builds.  This is required to get
# test scripts to work.
####
IF(WIN32)
  SET_TARGET_PROPERTIES(ncgen PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR})
  SET_TARGET_PROPERTIES(ncgen PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
    ${CMAKE_CURRENT_BINARY_DIR})
  SET_TARGET_PROPERTIES(ncgen PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
    ${CMAKE_CURRENT_BINARY_DIR})
ENDIF()

INSTALL(TARGETS ncgen DESTINATION bin COMPONENT utilities)

# Copy the input for the flex file (ncgen.l)
FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.l ${CMAKE_CURRENT_SOURCE_DIR}/*.y)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

# Stuff to build if tests are enabled.
IF(NETCDF_ENABLE_TESTS)
  FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
  FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)

  ##
  # The following tests were moved to ncdump/, to ensure
  # that they are run in the proper order.
  ##
  #add_sh_test(ncgen run_tests)
  #IF(USE_NETCDF4)
  #  add_sh_test(ncgen run_nc4_tests)
  #ENDIF()
ENDIF()

SET(MAN_FILES ncgen.1)
IF(NOT MSVC)
  INSTALL(FILES ${MAN_FILES} DESTINATION "${CMAKE_INSTALL_MANDIR}/man1"
    COMPONENT documentation)
ENDIF()
SET(CLEANFILES c0.nc c0_64.nc c0_4.nc c0_4c.nc ref_camrun.c)
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEANFILES}")

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

## Specify files to be distributed by 'make dist'
FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} ncgen.y ncgenl.c ncgen.l internals.html c0.cdl c0_4.cdl ref_camrun.cdl ncf199.cdl tst_gattenum.cdl CMakeLists.txt Makefile.am ncgen.1)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
