SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")
# For some reason, these appear to need to be set in each folder.
IF(ENABLE_COVERAGE_TESTS)
        SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -W -Wshadow -Wunused-variable -Wun    used-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecate    d -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
        SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
        SET(CMAKE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
ENDIF()

# Process these files with m4.
SET(m4_SOURCES attr ncx putget t_ncxx)
MACRO(GEN_m4 filename)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.c
		COMMAND m4 ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.m4 > ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.c
		VERBATIM
	)
ENDMACRO(GEN_m4)

foreach (f ${m4_SOURCES})
	GEN_m4(${f})
endforeach(f)

SET(libsrc_SOURCES v1hpg.c nclistmgr.c putget.c attr.c nc3dispatch.c nc.c var.c dim.c ncx.c lookup3.c ncio.c)

IF (BUILD_DISKLESS)
   SET(libsrc_SOURCES ${libsrc_SOURCES} memio.c)

   IF (BUILD_MMAP)
	SET(libsrc_SOURCES ${libsrc_SOURCES} mmapio.c)
   ENDIF( BUILD_MMAP)
ENDIF (BUILD_DISKLESS)

IF (USE_FFIO)
   SET(libsrc_SOURCES ${libsrc_SOURCES} ffio.c)
ELSE (USE_FFIO)
   SET(libsrc_SOURCES ${libsrc_SOURCES} posixio.c)
ENDIF (USE_FFIO)

add_library(netcdf3 OBJECT ${libsrc_SOURCES}) 

# The C API man page.
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

