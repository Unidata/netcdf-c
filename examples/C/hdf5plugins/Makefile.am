#    Copyright 2018, UCAR/Unidata
#    See netcdf/COPYRIGHT file for copying and redistribution conditions.

BZIP2SRC = bzlib.h bzlib_private.h blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c

PLUGINSRC = H5Zbzip2.c h5bzip2.h

TARGETS = ${PLUGINSRC} ${BZIP2SRC}

BUILT_SOURCES = ${TARGETS}

${TARGETS}:
	for x in ${TARGETS} ; do 'cp' -f ${top_srcdir}/nc_test4/hdf5plugins/$$x . ; done

CLEANFILES = ${TARGETS}

if ENABLE_FILTER_TESTING

DLLSRC=${PLUGINSRC} ${BZIP2SRC}

lib_LTLIBRARIES = libbzip2.la

libbzip2_la_SOURCES = ${DLLSRC}
libbzip2_la_LDFLAGS = -module -avoid-version -shared -export-dynamic -no-undefined -rpath ${abs_builddir}

endif #ENABLE_FILTER_TESTING
EXTRA_DIST = CMakeLists.txt H5Zbzip2.c Makefile.am blocksort.c bzlib.c bzlib.h bzlib_private.h compress.c \
crctable.c decompress.c h5bzip2.h huffman.c randtable.c bzip2.nc
