#    Copyright 2018, UCAR/Unidata
#    See netcdf/COPYRIGHT file for copying and redistribution conditions.

# Put Together AM_CPPFLAGS and AM_LDFLAGS
include $(top_srcdir)/lib_flags.am

# This linker flag specifies libtool version info.
# See http://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning
# for information regarding incrementing `-version-info`.
plugin_version_info = -version-info 0:0:0

AM_LDFLAGS += -module -shared -export-dynamic $(NOUNDEFINED)

if ISMINGW
  AM_LDFLAGS += -avoid-version
else !ISMINGW
if ISCYGWIN
AM_LDFLAGS += -avoid-version
else !ISCYGWIN
AM_LDFLAGS += $(plugin_version_info)
endif !ISCYGWIN
endif !ISMINGW

# Create an alternate directory if not installing or for noinst installs.
ALTPLUGINDIR = ${abs_top_builddir}/plugins/plugindir

# This is where the plugins are to be installed
if ENABLE_PLUGIN_DIR
plugindir = @PLUGIN_INSTALL_DIR@
else
plugindir = ${ALTPLUGINDIR}
AM_LDFLAGS += -rpath $(abs_builddir)/.libs
endif

plugin_LTLIBRARIES =
plugins_to_install =

# Apparently one cannot have plugin_LTLIBRARIES and also noinst_LTLIBRARIES.
# So create a tmp location for "noinst" shared libraries.
# tmpdir = ${ALTPLUGINDIR}

check_LTLIBRARIES =
noinst_LTLIBRARIES =

if ISMINGW
LDADD =  ${top_builddir}/liblib/libnetcdf.la
LIBADD = $(LDADD)
endif
if ISCYGWIN
LDADD = $(top_builddir)/liblib/libnetcdf.la
LIBADD = $(top_builddir)/liblib/libnetcdf.la
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
plugins_to_install += lib__nch5fletcher32.la lib__nch5shuffle.la lib__nch5deflate.la
lib__nch5shuffle_la_SOURCES = H5Zshuffle.c
lib__nch5fletcher32_la_SOURCES = H5Zfletcher32.c H5checksum.c
lib__nch5deflate_la_SOURCES = H5Zdeflate.c

# Need our version of szip if libsz available and we are not using HDF5
if HAVE_SZ
plugins_to_install += lib__nch5szip.la
lib__nch5szip_la_SOURCES = H5Zszip.c H5Zszip.h
endif

endif # ENABLE_NCZARR_FILTERS

if ENABLE_PLUGINS

# The NCZarr codec libraries
lib__nczstdfilters_la_SOURCES = NCZstdfilters.c
lib__nczstdfilters_la_LIBADD = $(LIBADD)
lib__nczhdf5filters_la_SOURCES = NCZhdf5filters.c
lib__nczhdf5filters_la_LIBADD = $(LIBADD)

plugins_to_install += lib__nczhdf5filters.la
plugins_to_install += lib__nczstdfilters.la

if HAVE_BLOSC
lib__nch5blosc_la_SOURCES = H5Zblosc.c H5Zblosc.h
plugins_to_install += lib__nch5blosc.la
endif

if HAVE_ZSTD
lib__nch5zstd_la_SOURCES = H5Zzstd.c H5Zzstd.h
plugins_to_install += lib__nch5zstd.la
endif

endif #ENABLE_PLUGINS

# The noop filter is to allow testing of multifilters and filter order
# Need two distinct instances
lib__nch5noop_la_SOURCES = H5Znoop.c H5Zutil.c h5noop.h
lib__nch5noop1_la_SOURCES = H5Znoop1.c H5Zutil.c h5noop.h
lib__nch5noop_la_LDFLAGS = $(AM_LDFLAGS) -rpath $(abs_builddir)/.libs
lib__nch5noop1_la_LDFLAGS = $(AM_LDFLAGS) -rpath $(abs_builddir)/.libs

# The misc filter is to allow testing of filter arguments
lib__nch5misc_la_SOURCES = H5Zmisc.c H5Zutil.c h5misc.h
lib__nch5misc_la_LDFLAGS = $(AM_LDFLAGS) -rpath $(abs_builddir)/.libs
lib__nczmisc_la_SOURCES = NCZmisc.c
lib__nczmisc_la_LDFLAGS = $(AM_LDFLAGS) -rpath $(abs_builddir)/.libs

# Provide a filter to test missing filter
lib__nch5unknown_la_SOURCES = H5Zunknown.c
lib__nch5unknown_la_LDFLAGS = $(AM_LDFLAGS) -rpath $(abs_builddir)/.libs

check_LTLIBRARIES += lib__nch5noop.la lib__nch5noop1.la lib__nch5unknown.la
# findplugin.sh needs these plugins, and I want to see if these get built properly
noinst_LTLIBRARIES += lib__nch5misc.la lib__nczmisc.la

# Bzip2 is used to test more complex filters
lib__nch5bzip2_la_SOURCES = H5Zbzip2.c h5bzip2.h
BZIP2SRC = blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c bzlib.h bzlib_private.h
EXTRA_DIST += ${BZIP2SRC} BZIP2_LICENSE
if HAVE_LOCAL_BZ2
lib__nch5bzip2_la_SOURCES += ${BZIP2SRC}
endif
plugins_to_install += lib__nch5bzip2.la

endif #ENABLE_FILTER_TESTING

if ENABLE_PLUGIN_DIR
plugin_LTLIBRARIES += $(plugins_to_install)
else
noinst_LTLIBRARIES += $(plugins_to_install)
endif

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
