#    Copyright 2018, UCAR/Unidata
#    See netcdf/COPYRIGHT file for copying and redistribution conditions.

BZIP2HDRS=bzlib.h bzlib_private.h
BZIP2SRC= blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c

PLUGINSRC=H5Zbzip2.c
PLUGINHDRS=h5bzip2.h

BZIP2OBJ=${BZIP2SRC:%.o=%.c}

if ISCYGWIN
LIBNAME=cygbzip2.dll
else
LIBNAME=libbzip2.so
endif

EXTRA_DIST=${PLUGINSRC} ${BZIP2SRC} ${PLUGINHDRS} ${BZIP2HDRS}

if ENABLE_FILTER_TESTING

DLLSRC=${PLUGINSRC} ${BZIP2SRC} ${PLUGINHDRS} ${BZIP2HDRS}

lib_LTLIBRARIES = libbzip2.la
libbzip2_la_LDFLAGS = -module -avoid-version -shared -export-dynamic -no-undefined
libbzip2_la_SOURCES = ${DLLSRC}

endif #ENABLE_FILTER_TESTING

