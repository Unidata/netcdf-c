# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

add_library(netcdfp OBJECT ${libsrcp_SOURCES})
target_link_libraries(netcdfp PUBLIC PNETCDF::PNETCDF)

target_sources(netcdfp PRIVATE ncpdispatch.c)

if (NETCDF_ENABLE_DLL)
    target_compile_definitions(netcdfp PRIVATE DLL_NETCDF DLL_EXPORT)
endif()

add_extra_dist(${libsrcp_SOURCES} CMakeLists.txt)
