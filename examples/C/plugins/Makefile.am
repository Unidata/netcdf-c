# Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
# See the COPYRIGHT file for more information.

#AM_CPPFLAGS = -I$(top_srcdir)/include
#AM_LDFLAGS = ${top_builddir}/liblib/libnetcdf.la

# You may need to change this if HDF5 is installed elsewhere
HDF5LIBDIR = /usr/local/lib
# Ditto for zlib
ZLIBDIR = ${HDF5LIBDIR}

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

CLEANFILES = ${LIBNAME}


if USE_NETCDF4
if ENABLE_FILTER_EXAMPLE

# In order to create the bzip shared object,
# we provide an explicit target for it that
# must be invoked before 'make check'

#DLLSRC = ${PLUGINSRC:${srcdir}%.c=%.c} ${BZIP2SRC:${srcdir}%.c=%.c}
DLLSRC=${PLUGINSRC} ${BZIP2SRC} ${PLUGINHDRS} ${BZIP2HDRS}

lib_LTLIBRARIES = libbzip2.la
libbzip2_la_LDFLAGS = -module -avoid-version -shared -export-dynamic -no-undefined
libbzip2_la_SOURCES = ${DLLSRC}

endif #ENABLE_FILTER_EXAMPLE
endif #USE_NETCDF4
