SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

# Process these files with m4.


SET(liblib_LIBS dispatch netcdf3)

ADD_LIBRARY(netcdf stub.c)
TARGET_LINK_LIBRARIES(netcdf ${liblib_LIBS})
#add_library(netcdf ${liblib_LIBS})
