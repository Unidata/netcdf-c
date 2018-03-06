SET(libsrcpio_SOURCES piodispatch.c)

add_library(netcdfpio OBJECT ${libsrcpio_SOURCES})

ADD_EXTRA_DIST(${libsrcpio_SOURCES} CMakeLists.txt)
