#    Copyright 2018, UCAR/Unidata
#    See netcdf/COPYRIGHT file for copying and redistribution conditions.

# Put Together AM_CPPFLAGS and AM_LDFLAGS
include $(top_srcdir)/lib_flags.am

if ENABLE_PLUGIN_DIR
plugindir = @PLUGIN_INSTALL_DIR@
else
# Choose a random install directory (see clean-local below)
plugindir = ${abs_top_builddir}/plugins/installed
endif

# -rpath needs to be defined to force creation of the shared libraries
RPATH = -rpath ${plugindir}

AM_LDFLAGS += -module -avoid-version -shared -export-dynamic ${NOUNDEFINED} 

# There seems no guaranteed way to turn of the version in the name
#plugin_version_info = -no-version-info 0:0:0

# Use this only with installable libraries
# Not currently needed for the platforms we support.
INSTALLFLAGS = ${AM_LDFLAGS}
INSTALLFLAGS += ${RPATH}

# Use this only with test libraries
# Note: it appears necessary to set rpath for the check libraries
# so that they get generated
TESTFLAGS = ${AM_LDFLAGS}
TESTFLAGS += ${RPATH}

PLUGINLIBS =
CHECKLIBS =

if ISMINGW
LDADD =  ${top_builddir}/liblib/libnetcdf.la
endif

EXTRA_DIST = CMakeLists.txt

# The HDF5 filter wrappers
EXTRA_DIST += \
	H5Ztemplate.c H5Zmisc.c H5Zutil.c H5Znoop.c h5noop.h NCZmisc.c \
	H5Zshuffle.c H5Zdeflate.c H5Zszip.c H5Zszip.h \
        H5Zbzip2.c h5bzip2.h H5Zblosc.c H5Zblosc.h H5Zzstd.c H5Zzstd.h

# The Codec filter wrappers
EXTRA_DIST += NCZhdf5filters.c NCZstdfilters.c
# The Filter implementations
EXTRA_DIST += H5checksum.c

if ENABLE_FILTER_TESTING

if ENABLE_NCZARR_FILTERS
PLUGINLIBS += lib__nch5fletcher32.la lib__nch5shuffle.la lib__nch5deflate.la
lib__nch5shuffle_la_SOURCES = H5Zshuffle.c
lib__nch5shuffle_la_LDFLAGS = ${INSTALLFLAGS}
lib__nch5fletcher32_la_SOURCES = H5Zfletcher32.c H5checksum.c
lib__nch5fletcher32_la_LDFLAGS = ${INSTALLFLAGS}
lib__nch5deflate_la_SOURCES = H5Zdeflate.c
lib__nch5deflate_la_LDFLAGS = ${INSTALLFLAGS}

# Need our version of szip if libsz available and we are using nczarr
if HAVE_SZ
if ENABLE_NCZARR_FILTERS
PLUGINLIBS += lib__nch5szip.la
lib__nch5szip_la_SOURCES = H5Zszip.c H5Zszip.h
lib__nch5szip_la_LDFLAGS = ${INSTALLFLAGS}
endif
endif

endif # ENABLE_NCZARR_FILTERS

if ENABLE_PLUGINS

# The NCZarr codec libraries
lib__nczstdfilters_la_SOURCES = NCZstdfilters.c
lib__nczstdfilters_la_LDFLAGS = ${INSTALLFLAGS}
lib__nczhdf5filters_la_SOURCES = NCZhdf5filters.c
lib__nczhdf5filters_la_LDFLAGS = ${INSTALLFLAGS}
PLUGINLIBS += lib__nczhdf5filters.la
PLUGINLIBS += lib__nczstdfilters.la

if HAVE_BLOSC
lib__nch5blosc_la_SOURCES = H5Zblosc.c H5Zblosc.h
lib__nch5blosc_la_LDFLAGS = ${INSTALLFLAGS}
PLUGINLIBS += lib__nch5blosc.la
endif

if HAVE_ZSTD
lib__nch5zstd_la_SOURCES = H5Zzstd.c H5Zzstd.h
lib__nch5zstd_la_LDFLAGS = ${INSTALLFLAGS}
PLUGINLIBS += lib__nch5zstd.la
endif

endif #ENABLE_PLUGINS

# The noop filter is to allow testing of multifilters and filter order
# Need two distinct instances
lib__nch5noop_la_SOURCES = H5Znoop.c H5Zutil.c h5noop.h
lib__nch5noop1_la_SOURCES = H5Znoop1.c H5Zutil.c h5noop.h
lib__nch5noop_la_LDFLAGS = ${TESTFLAGS}
lib__nch5noop1_la_LDFLAGS = ${TESTFLAGS}

# The misc filter is to allow testing of filter arguments
lib__nch5misc_la_SOURCES = H5Zmisc.c H5Zutil.c h5misc.h
lib__nch5misc_la_LDFLAGS = ${TESTFLAGS}
lib__nczmisc_la_SOURCES = NCZmisc.c
lib__nczmisc_la_LDFLAGS = ${TESTFLAGS}

# Provide a filter to test missing filter
lib__nch5unknown_la_SOURCES = H5Zunknown.c
lib__nch5unknown_la_LDFLAGS = ${TESTFLAGS}

CHECKLIBS += lib__nch5noop.la lib__nch5noop1.la lib__nch5misc.la lib__nczmisc.la lib__nch5unknown.la

# Bzip2 is used to test more complex filters
lib__nch5bzip2_la_SOURCES = H5Zbzip2.c h5bzip2.h
BZIP2SRC = blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c bzlib.h bzlib_private.h
EXTRA_DIST += ${BZIP2SRC} BZIP2_LICENSE
if HAVE_LOCAL_BZ2
lib__nch5bzip2_la_SOURCES += ${BZIP2SRC}
endif
lib__nch5bzip2_la_LDFLAGS = ${INSTALLFLAGS}
PLUGINLIBS += lib__nch5bzip2.la

if ENABLE_PLUGIN_DIR
plugin_LTLIBRARIES = ${PLUGINLIBS}
check_LTLIBRARIES = ${CHECKLIBS}
else
check_LTLIBRARIES = ${PLUGINLIBS} ${CHECKLIBS}
endif

endif #ENABLE_FILTER_TESTING

BUILT_SOURCES = H5Znoop1.c
DISTCLEANFILES = H5Znoop1.c H5Znoop2.c
H5Znoop1.c: Makefile H5Znoop.c
	echo '#define NOOP_INSTANCE 1' > $@
	cat ${srcdir}/H5Znoop.c >> $@

# Record where bzip2 came from; may be out of date
BZIP2VER = 1.0.8
BZIP2DIR = bzip2-${BZIP2VER}
BZIP2URL = https://sourceware.org/pub/bzip2/${BZIP2DIR}.tar.gz
bzip2::
	rm -fr ./${BZIP2DIR} ${BZIP2SRC} BZIP2_LICENSE
	wget ${BZIP2URL}
	tar -zxf ${BZIP2DIR}.tar.gz
	cd ${BZIP2DIR}; cp ${BZIP2SRC} ..; cp LICENSE ../BZIP2_LICENSE ; cd ..
	rm -fr ./${BZIP2DIR}

# See plugindir
clean-local:
	rm -fr ${abs_top_builddir}/plugins/installed
