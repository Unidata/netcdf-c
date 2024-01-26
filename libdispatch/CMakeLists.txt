# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
set(libdispatch_SOURCES dcopy.c dfile.c ddim.c datt.c dattinq.c dattput.c dattget.c derror.c dvar.c dvarget.c dvarput.c dvarinq.c ddispatch.c nclog.c dstring.c dutf8.c dinternal.c doffsets.c ncuri.c nclist.c ncbytes.c nchashmap.c nctime.c nc.c nclistmgr.c utf8proc.h utf8proc.c dpathmgr.c dutil.c drc.c dauth.c dreadonly.c dnotnc4.c dnotnc3.c dinfermodel.c
daux.c dinstance.c dinstance_intern.c
dcrc32.c dcrc32.h dcrc64.c ncexhash.c ncxcache.c ncjson.c ds3util.c dparallel.c dmissing.c)

# Netcdf-4 only functions. Must be defined even if not used
set(libdispatch_SOURCES ${libdispatch_SOURCES} dgroup.c dvlen.c dcompound.c dtype.c denum.c dopaque.c dfilter.c)

if(BUILD_V2)
  set(libdispatch_SOURCES ${libdispatch_SOURCES} dv2i.c)
endif(BUILD_V2)

if(ENABLE_BYTERANGE)
  set(libdispatch_SOURCES ${libdispatch_SOURCES} dhttp.c)
ENDIF(ENABLE_BYTERANGE)

IF(ENABLE_S3)
  if(ENABLE_S3_INTERNAL)
    set(libdispatch_SOURCES ${libdispatch_SOURCES} ncs3sdk_h5.c nch5s3comms.c nch5s3comms.h nccurl_sha256.c nccurl_sha256.h nccurl_hmac.c nccurl_hmac.h nccurl_setup.h)
  else()
    set(libdispatch_SOURCES ${libdispatch_SOURCES} ncs3sdk_aws.cpp awsincludes.h)
  endif()
endif()

if(REGEDIT)
  set(libdispatch_SOURCES ${libdispatch_SOURCES} dreg.c)
endif(REGEDIT)

add_library(dispatch OBJECT ${libdispatch_SOURCES})

if (USE_HDF5)
  target_include_directories(dispatch PRIVATE ${HDF5_INCLUDE_DIR})
endif(USE_HDF5)

if(STATUS_PARALLEL)
    target_link_libraries(dispatch PUBLIC MPI::MPI_C)
endif(STATUS_PARALLEL)

if(ENABLE_NCZARR)
    target_include_directories(dispatch PUBLIC ../libnczarr)
endif(ENABLE_NCZARR)

if(ENABLE_S3)
  if(ENABLE_S3_AWS)
    target_include_directories(dispatch PUBLIC ${AWSSDK_INCLUDE_DIRS})
    if(NOT MSVC)
      target_compile_features(dispatch PUBLIC cxx_std_11)
    endif()
  else()
    target_include_directories(dispatch PUBLIC ../libncxml)
  endif()
endif()

if(ENABLE_TESTS)
    BUILD_BIN_TEST(ncrandom)
endif()

file(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
set(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
