SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

SET(libdispatch_SOURCES dparallel.c dcopy.c dfile.c ddim.c datt.c dattinq.c dattput.c dattget.c derror.c dvar.c dvarget.c dvarput.c dvarinq.c ddispatch.c dstring.c dutf8proc.c nc_uri.c nclist.c ncbytes.c nchashmap.c nctime.c dsubstrate.c)

IF(USE_NETCDF4)
	SET(libdispatch_SOURCES ${libdispatch_SOURCES} dgroup.c dvlen.c dcompound.c dtype.c denum.c dopaque.c ncaux.c)
ENDIF(USE_NETCDF4)


IF(BUILD_DLL)
	SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -DDLL_EXPORT)
	#ADD_DEFINITION(-DDLL_EXPORT)
ENDIF(BUILD_DLL)

IF(BUILD_V2)
	SET(libdispatch_SOURCES ${libdispatch_SOURCES} dv2i.c)
	#SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -DDLL_EXPORT)	
	#ADD_DEFINITION(-DDLL_EXPORT)
ENDIF(BUILD_V2)	

add_library(dispatch OBJECT ${libdispatch_SOURCES}) 

###
# Options related to the man page generation.
###

###
# The C API man page.
###

set(man_MANS netcdf.3)

set(ARGS_MANPAGE -DAPI=C)

IF (USE_NETCDF4)
	SET(ARGS_MANPAGE ${ARGS_MANPAGE} -DNETCDF4=TRUE)
ENDIF(USE_NETCDF4)

IF (BUILD_DAP)
	SET(ARGS_MANPAGE ${ARGS_MANPAGE} -DDAP=TRUE)
ENDIF(BUILD_DAP)

IF (BUILD_PARALLEL)
	SET(ARGS_MANPAGE ${ARGS_MANPAGE} -DPARALLEL_IO=TRUE)
ENDIF(BUILD_PARALLEL)

