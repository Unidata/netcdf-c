SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

SET(ncgen3_FILES main.c load.c escapes.c getfill.c init.c genlib.c ncgentab.c)

ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ncgentab.c ${CMAKE_CURRENT_SOURCE_DIR}/ncgentab.h
	COMMAND bison -pncg -d ${NetCDF_SOURCE_DIR}/ncgen3/ncgen.y
	COMMAND mv ncgen.tab.c ${NetCDF_SOURCE_DIR}/ncgen3/ncgentab.c
	COMMAND mv ncgen.tab.h ${NetCDF_SOURCE_DIR}/ncgen3/ncgentab.h
	VERBATIM
)

ADD_EXECUTABLE(ncgen3 ${ncgen3_FILES})

TARGET_LINK_LIBRARIES(ncgen3 netcdf)

INSTALL(TARGETS ncgen3 DESTINATION bin)
