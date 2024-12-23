# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.


#####
# Installation of various netCDF headers.
#####

INSTALL(FILES ${netCDF_SOURCE_DIR}/include/netcdf.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT headers)

INSTALL(FILES ${netCDF_SOURCE_DIR}/include/netcdf_mem.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT headers)

INSTALL(FILES ${netCDF_SOURCE_DIR}/include/netcdf_aux.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT headers)

INSTALL(FILES ${netCDF_BINARY_DIR}/include/netcdf_meta.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT headers)

INSTALL(FILES ${netCDF_SOURCE_DIR}/include/netcdf_filter.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers)
INSTALL(FILES ${netCDF_SOURCE_DIR}/include/netcdf_filter_build.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers)

INSTALL(FILES ${netCDF_BINARY_DIR}/include/netcdf_dispatch.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers)

IF(NETCDF_ENABLE_PNETCDF OR NETCDF_ENABLE_PARALLEL4)
  INSTALL(FILES ${netCDF_SOURCE_DIR}/include/netcdf_par.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers)
ENDIF()

INSTALL(FILES ${netCDF_BINARY_DIR}/include/netcdf_json.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers)

INSTALL(FILES ${netCDF_BINARY_DIR}/include/netcdf_proplist.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers)

FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} Makefile.am CMakeLists.txt)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")

# One time read ncextern.h
file(READ ncexternl.h NCEXTH0)

#####
# Built Sources
#####

# Built source: netcdf_json.h
file(READ ncjson.h JSONH0)
STRING(REPLACE "NCJSON_H" "NETCDF_JSON_H" JSONH1 "${JSONH0}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/netcdf_json.h "${JSONH1}")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/netcdf_json.h "#ifdef NETCDF_JSON_H\n")
file(READ ${CMAKE_SOURCE_DIR}/libdispatch/ncjson.c JSONC0)
STRING(REGEX REPLACE "#include[ ]+\"ncjson.h\"" "" JSONC1 "${JSONC0}")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/netcdf_json.h "${JSONC1}")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/netcdf_json.h "#endif /*NETCDF_JSON_H*/\n")

# Built source: netcdf_proplist.h
file(READ ncproplist.h PLISTH0)
STRING(REPLACE "NCPROPLIST_H" "NETCDF_PROPLIST_H" PLISTH1 "${PLISTH0}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/netcdf_proplist.h "${PLISTH1}")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/netcdf_proplist.h "#ifdef NETCDF_PROPLIST_H\n")
file(READ ${CMAKE_SOURCE_DIR}/libdispatch/ncproplist.c PLISTC0)
STRING(REGEX REPLACE "#include[ ]+\"ncproplist.h\"" "" PLISTC1 "${PLISTC0}")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/netcdf_proplist.h "${PLISTC1}")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/netcdf_proplist.h "#endif /*NETCDF_PROPLIST_H*/\n")
