#    Copyright 2018, UCAR/Unidata
#    See netcdf/COPYRIGHT file for copying and redistribution conditions.

# Put Together AM_CPPFLAGS and AM_LDFLAGS
include $(top_srcdir)/lib_flags.am

AM_LDFLAGS += -module -avoid-version -shared -export-dynamic	\
 -rpath ${abs_builddir} ${NOUNDEFINED}

BZIP2HDRS = bzlib.h bzlib_private.h
BZIP2SRC = blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c

PLUGINSRC = H5Zbzip2.c
PLUGINHDRS = h5bzip2.h

EXTRA_DIST = ${PLUGINSRC} ${BZIP2SRC} ${PLUGINHDRS} ${BZIP2HDRS}	\
H5Ztemplate.c H5Zmisc.c H5Zutil.c H5Znoop.c h5noop.h CMakeLists.txt

# WARNING: This list must be kept consistent with the corresponding
# AC_CONFIG_LINK commands near the end of configure.ac.
HDF5PLUGINSRC = ${PLUGINSRC} ${BZIP2SRC} ${PLUGINHDRS} ${BZIP2HDRS}

if ENABLE_FILTER_TESTING

noinst_LTLIBRARIES = libmisc.la libnoop.la libnoop1.la
lib_LTLIBRARIES = libh5bzip2.la

libh5bzip2_la_SOURCES = ${HDF5PLUGINSRC}

libmisc_la_SOURCES = H5Zmisc.c H5Zutil.c h5misc.h

# The noop filter is to allow testing of multifilters and filter order
# Need two distinct instances
libnoop_la_SOURCES = H5Znoop.c H5Zutil.c h5noop.h
libnoop1_la_SOURCES = H5Znoop1.c H5Zutil.c h5noop.h

endif #ENABLE_FILTER_TESTING

BUILT_SOURCES = H5Znoop1.c
DISTCLEANFILES = H5Znoop1.c
H5Znoop1.c: Makefile H5Znoop.c
	echo '#define NOOP_INSTANCE 1' > $@
	cat ${srcdir}/H5Znoop.c >> $@
