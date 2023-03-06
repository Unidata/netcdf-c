# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
SET(libdispatch_SOURCES dcopy.c dfile.c ddim.c datt.c dattinq.c dattput.c dattget.c derror.c dvar.c dvarget.c dvarput.c dvarinq.c ddispatch.c nclog.c dstring.c dutf8.c dinternal.c doffsets.c ncuri.c nclist.c ncbytes.c nchashmap.c nctime.c nc.c nclistmgr.c utf8proc.h utf8proc.c dpathmgr.c dutil.c drc.c dauth.c dreadonly.c dnotnc4.c dnotnc3.c dinfermodel.c
daux.c dinstance.c
dcrc32.c dcrc32.h dcrc64.c ncexhash.c ncxcache.c ncjson.c ds3util.c dparallel.c)

# Netcdf-4 only functions. Must be defined even if not used
SET(libdispatch_SOURCES ${libdispatch_SOURCES} dgroup.c dvlen.c dcompound.c dtype.c denum.c dopaque.c dfilter.c)

IF(BUILD_V2)
  SET(libdispatch_SOURCES ${libdispatch_SOURCES} dv2i.c)
ENDIF(BUILD_V2)

IF(ENABLE_BYTERANGE)
  SET(libdispatch_SOURCES ${libdispatch_SOURCES} dhttp.c)
ENDIF(ENABLE_BYTERANGE)

IF(ENABLE_S3_SDK)
  SET(libdispatch_SOURCES ${libdispatch_SOURCES} ncs3sdk.cpp awsincludes.h)
ENDIF()

IF(REGEDIT)
  SET(libdispatch_SOURCES ${libdispatch_SOURCES} dreg.c)
ENDIF(REGEDIT)

add_library(dispatch OBJECT ${libdispatch_SOURCES})
IF(MPI_C_INCLUDE_PATH)
    target_include_directories(dispatch PUBLIC ${MPI_C_INCLUDE_PATH})
ENDIF(MPI_C_INCLUDE_PATH)

IF(MPI_C_LIBRARIES)
    target_link_libraries(dispatch PUBLIC ${MPI_C_LIBRARIES})
ENDIF(MPI_C_LIBRARIES)

IF(ENABLE_NCZARR)
    target_include_directories(dispatch PUBLIC ../libnczarr)
ENDIF(ENABLE_NCZARR)

IF(ENABLE_S3_SDK)
target_include_directories(dispatch PUBLIC ${AWSSDK_INCLUDE_DIRS})
IF(NOT MSVC)
target_compile_features(dispatch PUBLIC cxx_std_11)
ENDIF()
ENDIF()

FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
