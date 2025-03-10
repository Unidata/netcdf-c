# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
set(oc_SOURCES oc.c daplex.c dapparse.c dap.tab.c occompile.c occurlfunctions.c ocdata.c ocdebug.c ocdump.c ocinternal.c ocnode.c ochttp.c ocread.c ocutil.c xxdr.c)

add_library(oc2 OBJECT ${oc_SOURCES})

if (NETCDF_ENABLE_DLL)
  target_compile_definitions(oc2 PRIVATE DLL_NETCDF DLL_EXPORT)
endif()

if(STATUS_PARALLEL)
  target_link_libraries(oc2 PUBLIC MPI::MPI_C)
endif(STATUS_PARALLEL)
target_link_libraries(oc2 PUBLIC CURL::libcurl ${CURL_LIBRARIES})
target_include_directories(oc2 PUBLIC ${CURL_INCLUDE_DIRS})
target_compile_options(oc2
  PRIVATE
    -DCURL_STATICLIB=1
)

# Apparently fails under cmake
#set(ocprint_FILES ocprint.c )
#ADD_EXECUTABLE(ocprint ${ocprint_FILES})
#TARGET_LINK_LIBRARIES(ocprint oc2 ${ALL_TLL_LIBS})

## Specify files to be distributed by 'make dist'
file(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
set(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am)
set(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} dap.y)
set(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} ocauth.html)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
