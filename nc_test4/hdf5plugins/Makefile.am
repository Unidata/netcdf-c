#    Copyright 2018, UCAR/Unidata
#    See netcdf/COPYRIGHT file for copying and redistribution conditions.


BZIP2HDRS=bzlib.h bzlib_private.h
BZIP2SRC= blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c

PLUGINSRC=H5Zbzip2.c
PLUGINHDRS=h5bzip2.h

EXTRA_DIST=${PLUGINSRC} ${BZIP2SRC} ${PLUGINHDRS} ${BZIP2HDRS} \
		H5Ztemplate.c H5Zmisc.c CMakeLists.txt

if ENABLE_FILTER_TESTING

DLLSRC=${PLUGINSRC} ${BZIP2SRC} ${PLUGINHDRS} ${BZIP2HDRS}

lib_LTLIBRARIES = libbzip2.la libmisc.la

libbzip2_la_SOURCES = ${DLLSRC}
libbzip2_la_LDFLAGS = -module -avoid-version -shared -export-dynamic -no-undefined -rpath ${abs_builddir}

libmisc_la_SOURCES = H5Zmisc.c h5misc.h
libmisc_la_LDFLAGS = -module -avoid-version -shared -export-dynamic -no-undefined -rpath ${abs_builddir}

endif #ENABLE_FILTER_TESTING
