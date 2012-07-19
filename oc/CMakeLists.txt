SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

# Process these files with m4.

#SET(oc_SOURCES nc4dispatch.c nc4attr.c nc4dim.c nc4file.c nc4grp.c nc4type.c nc4var.c ncfunc.c nc4internal.c nc4hdf.c error4.c)

SET(oc_SOURCES occurlfunctions.c dapparse.c daplex.c ocuri.c ochttp.c ocbytes.c oc.c occontent.c ocdebug.c ocdrno.c ocdump.c ocinternal.c oclist.c oclog.c ocnode.c ocutil.c occlientparams.c ocrc.c ocread.c xxdr.c daptab.c)

add_library(oc OBJECT ${oc_SOURCES}) 




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

