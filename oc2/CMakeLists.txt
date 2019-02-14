# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
SET(oc_SOURCES oc.c daplex.c dapparse.c dapy.c occompile.c occurlfunctions.c ocdata.c ocdebug.c ocdump.c ocinternal.c ocnode.c ochttp.c ocread.c ocutil.c xxdr.c)


add_library(oc2 OBJECT ${oc_SOURCES})

# Apparently fails under cmake
#set(ocprint_FILES ocprint.c )
#ADD_EXECUTABLE(ocprint ${ocprint_FILES})
#TARGET_LINK_LIBRARIES(ocprint oc2 ${ALL_TLL_LIBS})

## Specify files to be distributed by 'make dist'
FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} dap.y)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} ocauth.html)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
