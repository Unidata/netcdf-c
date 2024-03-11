# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
set(dap4_SOURCES d4curlfunctions.c d4fix.c d4data.c d4file.c d4parser.c d4meta.c d4varx.c d4dump.c d4swap.c d4chunk.c d4printer.c d4read.c d4http.c d4util.c d4odom.c d4cvt.c d4debug.c ncd4dispatch.c)

## 
# Turn off inclusion of particular files when using the cmake-native
# option to turn on Unity Builds.  
#
# For more information, see:
#  * https://github.com/Unidata/netcdf-c/pull/2839/
#  * https://cmake.org/cmake/help/latest/prop_tgt/UNITY_BUILD.html
#  * https://cmake.org/cmake/help/latest/prop_tgt/UNITY_BUILD_MODE.html#prop_tgt:UNITY_BUILD_MODE
##
set_property(SOURCE d4meta.c
  PROPERTY
    SKIP_UNITY_BUILD_INCLUSION ON)

add_library(dap4 OBJECT ${dap4_SOURCES})
target_link_libraries(dap4 PUBLIC ${CURL_LIBRARIES})
target_include_directories(dap4 PUBLIC ${CURL_INCLUDE_DIRS})
target_compile_options(dap4
  PRIVATE
    -DCURL_STATICLIB=1
)

if (ENABLE_DLL)
  target_compile_definitions(dap4 PRIVATE DLL_NETCDF DLL_EXPORT)
endif()

if(STATUS_PARALLEL)
  target_link_libraries(dap4 PUBLIC MPI::MPI_C)
endif(STATUS_PARALLEL)

###
# Options related to the man page generation.
###

###
# The C API man page.
###

set(man_MANS netcdf.3)

set(ARGS_MANPAGE -DAPI=C)

if (USE_HDF5)
  set(ARGS_MANPAGE ${ARGS_MANPAGE} -DNETCDF4=TRUE)
endif(USE_HDF5)

if (BUILD_DAP)
  set(ARGS_MANPAGE ${ARGS_MANPAGE} -DDAP=TRUE)
endif(BUILD_DAP)

if (BUILD_PARALLEL)
  set(ARGS_MANPAGE ${ARGS_MANPAGE} -DPARALLEL_IO=TRUE)
endif(BUILD_PARALLEL)

FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
set(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} dce.y CMakeLists.txt Makefile.am)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
