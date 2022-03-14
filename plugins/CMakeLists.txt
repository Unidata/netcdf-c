# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
SET(CMAKE_BUILD_TYPE "")

FILE(READ H5Znoop.c NOOP_SOURCE)
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/H5Znoop1.c "#define NOOP_INSTANCE 1\n")
FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/H5Znoop1.c "${NOOP_SOURCE}")

SET(h5bzip2_SOURCES blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c H5Zbzip2.c ${NCJ})

SET(h5misc_SOURCES H5Zmisc.c H5Zutil.c h5misc.h)

SET(h5noop_SOURCES H5Znoop.c H5Zutil.c h5noop.h)
SET_SOURCE_FILES_PROPERTIES(H5Znoop.c PROPERTIES COMPILE_OPTIONS -DNOOP_INSTANCE=0)

SET(h5noop1_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/H5Znoop1.c H5Zutil.c h5noop.h)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/H5Znoop1.c PROPERTIES COMPILE_OPTIONS -DNOOP_INSTANCE=1)

SET(h5shuffle_SOURCES H5Zshuffle.c)
SET(h5fletcher32_SOURCES H5Zfletcher32.c H5checksum.c)
SET(h5deflate_SOURCES H5Zdeflate.c)

SET(nczmisc_SOURCES NCZmisc.c)
SET(nczdefaults_SOURCES NCZdefaults.c)

IF(ENABLE_FILTER_TESTING)
IF(BUILD_UTILITIES)

# LDFLAGS = -module -avoid-version -shared -export-dynamic -no-undefined

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}")

MACRO(buildplugin TARGET TARGETLIB)
    ADD_LIBRARY(${TARGET} MODULE ${${TARGET}_SOURCES})
    SET_TARGET_PROPERTIES(${TARGET} PROPERTIES LIBRARY_OUTPUT_NAME ${TARGETLIB})
    SET_TARGET_PROPERTIES(${TARGET} PROPERTIES ARCHIVE_OUTPUT_NAME ${TARGETLIB})
    SET_TARGET_PROPERTIES(${TARGET} PROPERTIES RUNTIME_OUTPUT_NAME ${TARGETLIB})
    TARGET_LINK_LIBRARIES(${TARGET} ${ALL_TLL_LIBS};${ARGN})
  IF(MSVC)
    target_compile_options(${TARGET} PRIVATE /Zi)
    # Tell linker to include symbol data
    set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "/INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF")
    # Set file name & location
    set_target_properties(${TARGET} PROPERTIES COMPILE_PDB_NAME ${TARGET} COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR})
  ENDIF()
ENDMACRO()

# Note we use name h5bzip2 instead of bzip2 to avoid logical
# target name clash with examples/C/hdf5plugins
buildplugin(h5bzip2 "h5bzip2")
SET_TARGET_PROPERTIES(h5bzip2 PROPERTIES OUTPUT_NAME "bzip2")

buildplugin(h5misc "h5misc")
buildplugin(h5noop "h5noop")
buildplugin(h5noop1 "h5noop1")

buildplugin(h5shuffle "h5shuffle")
buildplugin(h5fletcher32 "h5fletcher32")
buildplugin(h5deflate "h5deflate")

buildplugin(nczmisc "nczmisc")
buildplugin(nczdefaults "nczdefaults" netcdf)

IF(ENABLE_BLOSC)
SET(h5blosc_SOURCES H5Zblosc.c)
buildplugin(h5blosc "h5blosc" netcdf;${Blosc_LIBRARIES})
ENDIF()

IF(ENABLE_SZIP)
SET(h5szip_SOURCES H5Zszip.c H5Zszip.h)
buildplugin(h5szip "h5szip" ${Szip_LIBRARIES})
ENDIF()

ENDIF(BUILD_UTILITIES)
ENDIF(ENABLE_FILTER_TESTING)

# Copy some test files from current source dir to out-of-tree build dir.
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
IF(MSVC)
  FILE(COPY ${COPY_FILES} DESTINATION ${RUNTIME_OUTPUT_DIRECTORY}/)
ENDIF()
