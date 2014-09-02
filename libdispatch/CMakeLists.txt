SET(libdispatch_SOURCES dparallel.c dcopy.c dfile.c ddim.c datt.c dattinq.c dattput.c dattget.c derror.c dvar.c dvarget.c dvarput.c dvarinq.c ddispatch.c nclog.c dstring.c dutf8proc.c ncuri.c nclist.c ncbytes.c nchashmap.c nctime.c dsubstrate.c nc.c nclistmgr.c ncsettings.c)

IF(USE_NETCDF4)
  SET(libdispatch_SOURCES ${libdispatch_SOURCES} dgroup.c dvlen.c dcompound.c dtype.c denum.c dopaque.c ncaux.c)
ENDIF(USE_NETCDF4)

IF(BUILD_V2)
  SET(libdispatch_SOURCES ${libdispatch_SOURCES} dv2i.c)
ENDIF(BUILD_V2)	

add_library(dispatch OBJECT ${libdispatch_SOURCES}) 

FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am ncsettings.hdr)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")

FILE(REMOVE "${CMAKE_CURRENT_SOURCE_DIR}/ncsettings.c")
FILE(READ "${CMAKE_SOURCE_DIR}/libnetcdf.settings" settings)


STRING(REGEX REPLACE "	" " " settings "${settings}")
STRING(REGEX REPLACE "\r" " " settings "${settings}")
STRING(REPLACE "\"" "\\\"" settings "${settings}")
STRING(REGEX REPLACE "^" "\"\\1" settings "${settings}")
FILE(READ "${CMAKE_CURRENT_SOURCE_DIR}/ncsettings.hdr" hdr)
FILE(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/ncsettings.c" "${hdr}" "${settings}")
FILE(APPEND "${CMAKE_CURRENT_SOURCE_DIR}/ncsettings.c" ";")
