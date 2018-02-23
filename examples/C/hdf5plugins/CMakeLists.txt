SET(CMAKE_BUILD_TYPE "")

SET(libbzip2_SOURCES bzlib.h bzlib_private.h
         blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c
         H5Zbzip2.c h5bzip2.h
)

# Get the compilable sources from nc_test4/hdf5plugins
FOREACH(S ${libbzip2_SOURCES})
  FILE(COPY ${CMAKE_SOURCE_DIR}/nc_test4/hdf5plugins/${S} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
ENDFOREACH(S)

IF(ENABLE_FILTER_TESTING)
IF(BUILD_UTILITIES)

# LDFLAGS = -module -avoid-version -shared -export-dynamic -no-undefined

ADD_LIBRARY(bzip2 MODULE ${libbzip2_SOURCES})
SET_TARGET_PROPERTIES(bzip2 PROPERTIES LIBRARY_OUTPUT_NAME "bzip2")
SET_TARGET_PROPERTIES(bzip2 PROPERTIES ARCHIVE_OUTPUT_NAME "bzip2")
SET_TARGET_PROPERTIES(bzip2 PROPERTIES RUNTIME_OUTPUT_NAME "bzip2")
TARGET_LINK_LIBRARIES(bzip2 ${HDF5_HL_LIBRARIES} ${HDF5_C_LIBRARIES})

ENDIF(BUILD_UTILITIES)
ENDIF(ENABLE_FILTER_TESTING)

# Copy some test files from current source dir to out-of-tree build dir.
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
IF(MSVC)
  FILE(COPY ${COPY_FILES} DESTINATION ${RUNTIME_OUTPUT_DIRECTORY}/)
ENDIF()
