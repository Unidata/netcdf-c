SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")
#INCLUDE_DIRECTORIES(${NETCDF_SOURCE_DIR}/include)



SET(libsrc_SOURCES dim.c ffio.c lookup3.c memio.c mmap.c mmapio.c nc.c nc3dispatch.c ncFile.c ncio.c nclistmgr.c ncstdio.c ncx_cray.c posixio.c t_ncio.c t_ncx.c v1hpg.c var.c winceio.c)

add_library(netcdf3 ${libsrc_SOURCES}) 