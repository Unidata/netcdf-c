SET(libhdf4_SOURCES hdf4attr.c hdf4dim.c hdf4dispatch.c hdf4file.c hdf4func.c
  hdf4grp.c hdf4type.c hdf4var.c)

add_library(netcdfhdf4 OBJECT ${libhdf4_SOURCES})

ADD_EXTRA_DIST(${libhdf4_SOURCES} CMakeLists.txt)
