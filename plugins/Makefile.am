#    Copyright 2018, UCAR/Unidata
#    See netcdf/COPYRIGHT file for copying and redistribution conditions.

# Put Together AM_CPPFLAGS and AM_LDFLAGS
include $(top_srcdir)/lib_flags.am

# This linker flag specifies libtool version info.
# See http://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning
# for information regarding incrementing `-version-info`.
plugin_version_info = -version-info 0:0:0

AM_LDFLAGS += -module -avoid-version -shared -export-dynamic $(NOUNDEFINED)

# Create an alternate directory if not installing.
ALTPLUGINDIR = ${abs_top_builddir}/plugins/plugindir

RPATH = -rpath $(abs_top_builddir)/.libs

# This is where the plugins are to be installed
if ENABLE_PLUGIN_DIR
  plugindir = @PLUGIN_INSTALL_DIR@
else
  plugindir = ${ALTPLUGINDIR}
  AM_LDFLAGS += ${RPATH}
endif

plugin_LTLIBRARIES =
plugins_to_install =

# These libraries are for testing only
check_LTLIBRARIES =

if ISMINGW
LDADD =  ${top_builddir}/liblib/libnetcdf.la
LIBADD = $(LDADD)
endif
if ISCYGWIN
LDADD = $(top_builddir)/liblib/libnetcdf.la
LIBADD = $(top_builddir)/liblib/libnetcdf.la
endif

EXTRA_DIST = CMakeLists.txt findplugin.in

# The HDF5 filter wrappers
EXTRA_DIST += \
	H5Ztemplate.c H5Zmisc.c H5Zutil.c H5Znoop.c h5noop.h NCZmisc.c \
	H5Zshuffle.c H5Zdeflate.c H5Zszip.c H5Zszip.h \
        H5Zbzip2.c h5bzip2.h H5Zblosc.c H5Zblosc.h H5Zzstd.c H5Zzstd.h

# The Codec filter wrappers
EXTRA_DIST += NCZhdf5filters.c NCZstdfilters.c
# The Filter implementations
EXTRA_DIST += H5checksum.c

plugins_to_install += lib__nch5fletcher32.la lib__nch5shuffle.la 
lib__nch5shuffle_la_SOURCES = H5Zshuffle.c
lib__nch5fletcher32_la_SOURCES = H5Zfletcher32.c H5checksum.c
if HAVE_DEFLATE
plugins_to_install += lib__nch5deflate.la
lib__nch5deflate_la_SOURCES = H5Zdeflate.c
endif

# Need our version of szip if libsz available and we are not using HDF5
if HAVE_SZ
plugins_to_install += lib__nch5szip.la
lib__nch5szip_la_SOURCES = H5Zszip.c H5Zszip.h
endif

if HAVE_BLOSC
lib__nch5blosc_la_SOURCES = H5Zblosc.c H5Zblosc.h
plugins_to_install += lib__nch5blosc.la
endif

if HAVE_ZSTD
lib__nch5zstd_la_SOURCES = H5Zzstd.c H5Zzstd.h
plugins_to_install += lib__nch5zstd.la
endif

if NETCDF_ENABLE_NCZARR_FILTERS
# The NCZarr codec libraries (they need libnetcdf)
lib__nczstdfilters_la_SOURCES = NCZstdfilters.c
lib__nczstdfilters_la_LIBADD = $(LIBADD)  $(top_builddir)/liblib/libnetcdf.la
lib__nczhdf5filters_la_SOURCES = NCZhdf5filters.c
lib__nczhdf5filters_la_LIBADD = $(LIBADD) $(top_builddir)/liblib/libnetcdf.la

plugins_to_install += lib__nczhdf5filters.la
plugins_to_install += lib__nczstdfilters.la
endif

# The noop filter is to allow testing of multifilters and filter order
# Need two distinct instances
lib__nch5noop_la_SOURCES = H5Znoop.c H5Zutil.c h5noop.h
lib__nch5noop1_la_SOURCES = H5Znoop1.c H5Zutil.c h5noop.h
lib__nch5noop_la_LDFLAGS = $(AM_LDFLAGS) ${RPATH}
lib__nch5noop1_la_LDFLAGS = $(AM_LDFLAGS) ${RPATH}

if NETCDF_ENABLE_FILTER_TESTING
# The misc filter is to allow testing of filter arguments
lib__nch5misc_la_SOURCES = H5Zmisc.c H5Zutil.c h5misc.h
lib__nch5misc_la_LDFLAGS = ${top_builddir}/liblib/libnetcdf.la $(AM_LDFLAGS) ${RPATH}
lib__nczmisc_la_SOURCES = NCZmisc.c
lib__nczmisc_la_LDFLAGS = $(AM_LDFLAGS) ${RPATH}

# Provide a filter to test missing filter
lib__nch5unknown_la_SOURCES = H5Zunknown.c
lib__nch5unknown_la_LDFLAGS = $(AM_LDFLAGS) ${RPATH}

check_LTLIBRARIES += lib__nch5noop.la lib__nch5noop1.la lib__nch5unknown.la
check_LTLIBRARIES += lib__nch5misc.la lib__nczmisc.la

endif

# Bzip2 is used to test more complex filters
lib__nch5bzip2_la_SOURCES = H5Zbzip2.c h5bzip2.h
BZIP2SRC = blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c bzlib.h bzlib_private.h
EXTRA_DIST += ${BZIP2SRC} BZIP2_LICENSE
if HAVE_LOCAL_BZ2
lib__nch5bzip2_la_SOURCES += ${BZIP2SRC}
endif
plugins_to_install += lib__nch5bzip2.la

if ENABLE_PLUGIN_DIR
plugin_LTLIBRARIES += $(plugins_to_install)
else
check_LTLIBRARIES += $(plugins_to_install)
endif

DISTCLEANFILES = H5Znoop1.c H5Znoop2.c ncjson.h findplugin.sh

BUILT_SOURCES = H5Znoop1.c
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
