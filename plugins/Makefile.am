#    Copyright 2018, UCAR/Unidata
#    See netcdf/COPYRIGHT file for copying and redistribution conditions.

# Put Together AM_CPPFLAGS and AM_LDFLAGS
include $(top_srcdir)/lib_flags.am

AM_LDFLAGS += -module -avoid-version -shared -export-dynamic	\
 -rpath ${abs_builddir} ${NOUNDEFINED}

# Create an alternate directory if not installing or for noinst installs.
ALTPLUGINDIR = ${abs_top_builddir}/plugins/plugindir

# This is where the plugins are to be installed
if ENABLE_PLUGIN_DIR
plugindir = @PLUGIN_INSTALL_DIR@
else
plugindir = ${ALTPLUGINDIR}
endif

plugin_LTLIBRARIES =

# Apparently one cannot have plugin_LTLIBRARIES and also noinst_LTLIBRARIES.
# So create a tmp location for "noinst" shared libraries.
tmpdir = ${ALTPLUGINDIR}

tmp_LTLIBRARIES =

# This linker flag specifies libtool version info.
# See http://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning
# for information regarding incrementing `-version-info`.
plugin_version_info = -version-info 0:0:0

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
plugin_LTLIBRARIES += libh5fletcher32.la libh5shuffle.la libh5deflate.la
libh5shuffle_la_SOURCES = H5Zshuffle.c
libh5fletcher32_la_SOURCES = H5Zfletcher32.c H5checksum.c
libh5deflate_la_SOURCES = H5Zdeflate.c

libh5shuffle_la_LDFLAGS = ${plugin_version_info}
libh5deflate_la_LDFLAGS = ${plugin_version_info}
libh5fletcher32_la_LDFLAGS = ${plugin_version_info}

# Need our version of szip if libsz available and we are not using HDF5
if HAVE_SZ
plugin_LTLIBRARIES += libh5szip.la
libh5szip_la_SOURCES = H5Zszip.c H5Zszip.h
libh5szip_la_LDFLAGS = ${plugin_version_info}
endif

endif # ENABLE_NCZARR_FILTERS

if ENABLE_PLUGINS

# The NCZarr codec libraries
libnczstdfilters_la_SOURCES = NCZstdfilters.c
libnczhdf5filters_la_SOURCES = NCZhdf5filters.c

plugin_LTLIBRARIES += libnczhdf5filters.la
plugin_LTLIBRARIES += libnczstdfilters.la

if HAVE_BLOSC
libh5blosc_la_SOURCES = H5Zblosc.c H5Zblosc.h
libh5blosc_la_LDFLAGS = ${plugin_version_info}
plugin_LTLIBRARIES += libh5blosc.la
endif

if HAVE_ZSTD
libh5zstd_la_SOURCES = H5Zzstd.c H5Zzstd.h
libh5zstd_la_LDFLAGS = ${plugin_version_info}
plugin_LTLIBRARIES += libh5zstd.la
endif

endif #ENABLE_PLUGINS

# The noop filter is to allow testing of multifilters and filter order
# Need two distinct instances
libh5noop_la_SOURCES = H5Znoop.c H5Zutil.c h5noop.h
libh5noop1_la_SOURCES = H5Znoop1.c H5Zutil.c h5noop.h
libh5noop_la_LDFLAGS = ${plugin_version_info}
libh5noop1_la_LDFLAGS = ${plugin_version_info}

# The misc filter is to allow testing of filter arguments
libh5misc_la_SOURCES = H5Zmisc.c H5Zutil.c h5misc.h
libh5misc_la_LDFLAGS = ${plugin_version_info}
libnczmisc_la_SOURCES = NCZmisc.c
libnczmisc_la_LDFLAGS = ${plugin_version_info}

# Provide a filter to test missing filter
libh5unknown_la_SOURCES = H5Zunknown.c
libh5unknown_la_LDFLAGS = ${plugin_version_info}

tmp_LTLIBRARIES += libh5noop.la libh5noop1.la libh5misc.la libnczmisc.la libh5unknown.la

# Bzip2 is used to test more complex filters
libh5bzip2_la_SOURCES = H5Zbzip2.c h5bzip2.h
BZIP2SRC = blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c bzlib.h bzlib_private.h
EXTRA_DIST += ${BZIP2SRC} BZIP2_LICENSE
if HAVE_LOCAL_BZ2
libh5bzip2_la_SOURCES += ${BZIP2SRC}
endif
libh5bzip2_la_LDFLAGS = ${plugin_version_info}
plugin_LTLIBRARIES += libh5bzip2.la

endif #ENABLE_FILTER_TESTING

BUILT_SOURCES = H5Znoop1.c
DISTCLEANFILES = H5Znoop1.c H5Znoop2.c ncjson.h
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

# Custom clean
clean-local:
	rm -fr ${ALTPLUGINDIR}

