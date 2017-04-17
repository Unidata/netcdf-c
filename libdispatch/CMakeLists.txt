SET(libdispatch_SOURCES 
daux.c dapi.c dinternal.c dcopy.c derror.c ddispatch.c dutf8.c dstring.c nclog.c ncuri.c nclist.c ncbytes.c nchashmap.c nctime.c nc.c nclistmgr.c drc.c doffsets.c dwinpath.c utf8proc.c)

IF(BUILD_V2)
  SET(libdispatch_SOURCES ${libdispatch_SOURCES} dv2i.c)
ENDIF(BUILD_V2)

add_library(dispatch OBJECT ${libdispatch_SOURCES})

# With the API refactor, we keep these around, but do not use them.
SET(DEPRECATED datt.c dattget.c dattinq.c dattput.c dcompound.c ddim.c denum.c dfile.c dgroup.c dopaque.c dparallel.c dtype.c dvar.c dvarget.c dvarinq.c dvarput.c dvlen.c)

FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
