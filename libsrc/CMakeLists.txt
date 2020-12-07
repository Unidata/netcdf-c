## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2012-2018, see the COPYRIGHT file for more information.

SET(libsrc_SOURCES v1hpg.c putget.c attr.c nc3dispatch.c
  nc3internal.c var.c dim.c ncx.c lookup3.c ncio.c)

# Process these files with m4.
SET(m4_SOURCES attr ncx putget)
foreach (f ${m4_SOURCES})
  GEN_m4(${f} dest)
  LIST(APPEND libsrc_SOURCES ${dest})
endforeach(f)

LIST(APPEND libsrc_SOURCES pstdint.h ncio.h ncx.h)

LIST(APPEND libsrc_SOURCES memio.c)

IF (BUILD_MMAP)
  LIST(APPEND libsrc_SOURCES mmapio.c)
ENDIF( BUILD_MMAP)

IF (USE_FFIO)
  LIST(APPEND libsrc_SOURCES ffio.c)
ELSEIF (USE_STDIO)
   IST(APPEND libsrc_SOURCES ncstdio.c)
ELSE (USE_FFIO)
  LIST(APPEND libsrc_SOURCES posixio.c)
ENDIF (USE_FFIO)

IF (ENABLE_BYTERANGE)
  LIST(APPEND libsrc_SOURCES httpio.c)
ENDIF(ENABLE_BYTERANGE)

add_library(netcdf3 OBJECT ${libsrc_SOURCES})

# The C API man page.

SET(MAN_FILES netcdf.3)

set(ARGS_MANPAGE -DAPI=C)

IF (USE_HDF5)
  SET(ARGS_MANPAGE ${ARGS_MANPAGE} -DNETCDF4=TRUE)
ENDIF(USE_HDF5)

IF (BUILD_DAP)
  SET(ARGS_MANPAGE ${ARGS_MANPAGE} -DDAP=TRUE)
ENDIF(BUILD_DAP)

IF (BUILD_PARALLEL)
  SET(ARGS_MANPAGE ${ARGS_MANPAGE} -DPARALLEL_IO=TRUE)
ENDIF(BUILD_PARALLEL)

# Generate the man page.
IF(HAVE_M4)

  FILE(GLOB libsrc_MANPAGE ${CMAKE_CURRENT_SOURCE_DIR}/../docs/netcdf.m4)
  FILE(COPY ${libsrc_MANPAGE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

IF(NOT MSVC)
  ADD_CUSTOM_TARGET(manpage ALL

    COMMAND ${NC_M4} ${ARGS_MANPAGE} '${CMAKE_CURRENT_BINARY_DIR}/netcdf.m4' > '${CMAKE_CURRENT_BINARY_DIR}/netcdf.3'
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/netcdf.3 DESTINATION "share/man/man3" COMPONENT documentation)
ENDIF(NOT MSVC)

ENDIF()
