SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

remove_definitions(-DDLL_EXPORT)

SET(ncdump_FILES ncdump.c utils.c vardata.c dumplib.c indent.c nctime0.c )
SET(nccopy_FILES nccopy.c utils.c nciter.c chunkspec.c utils.c dimmap.c)

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

ADD_EXECUTABLE(ncdump ${ncdump_FILES})
ADD_EXECUTABLE(nccopy ${nccopy_FILES})

TARGET_LINK_LIBRARIES(ncdump netcdf)
TARGET_LINK_LIBRARIES(nccopy netcdf)

INSTALL(TARGETS ncdump DESTINATION bin)
INSTALL(TARGETS nccopy DESTINATION bin)
