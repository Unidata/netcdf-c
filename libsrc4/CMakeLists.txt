# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018, 2019, 2020
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
# Process these files with m4.

set(libsrc4_SOURCES nc4dispatch.c nc4attr.c nc4dim.c nc4grp.c
nc4internal.c nc4type.c nc4var.c ncfunc.c nc4cache.c)

add_library(netcdf4 OBJECT ${libsrc4_SOURCES})

if (NETCDF_ENABLE_DLL)
  target_compile_definitions(netcdf4 PRIVATE DLL_NETCDF DLL_EXPORT)
endif()

if(STATUS_PARALLEL)
  target_link_libraries(netcdf4 PUBLIC MPI::MPI_C)
endif(STATUS_PARALLEL)

# Files for make dist
file(GLOB libsrc4_HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
set(CUR_EXTRA_DIST CMakeLists.txt Makefile.am)
ADD_EXTRA_DIST("${libsrc4_SOURCES}")
ADD_EXTRA_DIST("${libsrc4_HEADERS}")
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")

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
  target_link_libraries(netcdf4 PRIVATE HDF5::HDF5)
endif(USE_HDF5)

if (BUILD_DAP)
  set(ARGS_MANPAGE ${ARGS_MANPAGE} -DDAP=TRUE)
endif(BUILD_DAP)

if (BUILD_PARALLEL)
  set(ARGS_MANPAGE ${ARGS_MANPAGE} -DPARALLEL_IO=TRUE)
endif(BUILD_PARALLEL)
