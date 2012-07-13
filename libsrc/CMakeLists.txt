SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")
#INCLUDE_DIRECTORIES(${NETCDF_SOURCE_DIR}/include)



#SET(libsrc_SOURCES dim.c ffio.c lookup3.c memio.c mmap.c mmapio.c nc.c nc3dispatch.c ncFile.c ncio.c nclistmgr.c ncstdio.c ncx_cray.c posixio.c t_ncio.c t_ncx.c v1hpg.c var.c winceio.c)

SET(libsrc_SOURCES v1hpg.c nclistmgr.c putget.c attr.c nc3dispatch.c nc.c var.c dim.c ncx.c lookup3.c ncio.c)

IF (BUILD_DISKLESS)
   SET(libsrc_SOURCES ${libsrc_SOURCES} memio.c)

   IF (BUILD_MMAP)
	SET(libsrc_SOURCES ${libsrc_SOURCES} mmapio.c)
   ENDIF( BUILD_MMAP)
ENDIF (BUILD_DISKLESS)

IF (USE_FFIO)
   SET(libsrc_SOURCES ${libsrc_SOURCES} ffio.c)
ENDIF (USE_FFIO)

add_library(netcdf3 ${libsrc_SOURCES}) 