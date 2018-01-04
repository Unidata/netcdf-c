# Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
# See the COPYRIGHT file for more information.

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS = ${top_builddir}/liblib/libnetcdf.la

# You may need to change this if HDF5 is installed elsewhere
HDF5LIBDIR = /usr/local/lib
# Ditto for zlib
ZLIBDIR = ${HDF5LIBDIR}

FILTERNAME=bzip2

BZIP2HDRS=bzlib.h bzlib_private.h
BZIP2SRC= blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c

PLUGINSRC=H5Zbzip2.c
PLUGINHDRS=h5bzip2.h

BZIP2OBJ=${BZIP2SRC:%.o=%.c}

if ISCYGWIN
LIBNAME=cyg${FILTERNAME}.dll
else
LIBNAME=lib${FILTERNAME}.so
endif

EXTRA_DIST=${PLUGINSRC} ${BZIP2SRC} ${PLUGINHDRS} ${BZIP2HDRS}

CLEANFILES = ${LIBNAME}

if USE_NETCDF4
if ENABLE_FILTER_EXAMPLE

# In order to create the bzip shared object,
# we provide an explicit target for it that
# must be invoked before 'make check'

BUILT_SOURCES = ${LIBNAME}

# Build the filter plugin as the bz2 source + the plugin adapter
${LIBNAME}: ${PLUGINSRC:${srcdir}%.c=%.c} ${BZIP2SRC:${srcdir}%.c=%.c}
	SDIR=${srcdir} ;\
	for f in ${PLUGINSRC} ${BZIP2SRC} ; do \
	    DLLSRC="$${DLLSRC} $${SDIR}/$$f" ; \
	done ; \
	gcc -g -O0 -shared -o ${LIBNAME} $${DLLSRC} \
	    -L${HDF5LIBDIR} -lhdf5_hl -lhdf5 -L${ZLIBDIR} -lz


endif #ENABLE_FILTER_EXAMPLE
endif #USE_NETCDF4
