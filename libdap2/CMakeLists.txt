# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
SET(dap2_SOURCES constraints.c dapcvt.c dapodom.c daputil.c ncdaperr.c cdf.c cache.c dapdump.c dapdebug.c dapattr.c ncd2dispatch.c getvara.c dceconstraints.c dcetab.c dceparse.c dcelex.c)

## 
# Turn off inclusion of particular files when using the cmake-native
# option to turn on Unity Builds.  
#
# For more information, see:
#  * https://github.com/Unidata/netcdf-c/pull/2839/
#  * https://cmake.org/cmake/help/latest/prop_tgt/UNITY_BUILD.html
#  * https://cmake.org/cmake/help/latest/prop_tgt/UNITY_BUILD_MODE.html#prop_tgt:UNITY_BUILD_MODE
##
set_property(SOURCE ncd2dispatch.c
  PROPERTY
    SKIP_UNITY_BUILD_INCLUSION ON)


set_property(SOURCE ncd2dispatch.c
  PROPERTY
    SKIP_UNITY_BUILD_INCLUSION ON)

add_library(dap2 OBJECT ${dap2_SOURCES})


target_link_libraries(dap2 PUBLIC ${CURL_LIBRARIES})
target_include_directories(dap2 PUBLIC ${CURL_INCLUDE_DIRS})
target_compile_options(dap2
  PRIVATE
    -DCURL_STATICLIB=1
)

if (ENABLE_DLL)
  target_compile_definitions(dap2 PRIVATE DLL_NETCDF DLL_EXPORT)
endif()

IF(STATUS_PARALLEL)
  target_link_libraries(dap2 PUBLIC MPI::MPI_C)
ENDIF(STATUS_PARALLEL)

###
# Options related to the man page generation.
###

###
# The C API man page.
###

set(man_MANS netcdf.3)

set(ARGS_MANPAGE -DAPI=C)

IF (USE_HDF5)
  SET(ARGS_MANPAGE ${ARGS_MANPAGE} -DNETCDF4=TRUE)
ENDIF(USE_HDF5)

IF (BUILD_DAP)
  SET(ARGS_MANPAGE ${ARGS_MANPAGE} -DDAP=TRUE)
ENDIF(BUILD_DAP)

IF (BUILD_PARALLEL)
  SET(ARGS_MANPAGE ${ARGS_MANPAGE} -DPARALLEL_IO=TRUE)
ENDIF(BUILD_PARALLEL)

FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} dce.y CMakeLists.txt Makefile.am)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
