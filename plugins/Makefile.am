#    Copyright 2018, UCAR/Unidata
#    See netcdf/COPYRIGHT file for copying and redistribution conditions.

# Put Together AM_CPPFLAGS and AM_LDFLAGS
include $(top_srcdir)/lib_flags.am

AM_LDFLAGS += -module -avoid-version -shared -export-dynamic	\
 -rpath ${abs_builddir} ${NOUNDEFINED}

lib_LTLIBRARIES =
noinst_LTLIBRARIES =

if ISMINGW
LDADD =  ${top_builddir}/liblib/libnetcdf.la
endif

# BZIP2 version 1.0.8 (https://sourceware.org/bzip2/)
BZIP2SRC = blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c bzlib.h bzlib_private.h

EXTRA_DIST = CMakeLists.txt

# The HDF5 filter wrappers
EXTRA_DIST += stdinstall.in \
	H5Ztemplate.c H5Zmisc.c H5Zutil.c H5Znoop.c h5noop.h NCZmisc.c \
	H5Zshuffle.c H5Zdeflate.c H5Zszip.c H5Zszip.h \
        H5Zbzip2.c h5bzip2.h H5Zblosc.c H5Zblosc.h H5Zzstd.c H5Zzstd.h
# The Codec filter wrappers
EXTRA_DIST += NCZdefaults.c NCZszip.c
# The Filter implementations
EXTRA_DIST += H5checksum.c

EXTRA_DIST += ${BZIP2SRC} BZIP2_LICENSE

if ENABLE_FILTER_TESTING

noinst_LTLIBRARIES += libh5misc.la libh5noop.la libh5noop1.la libnczmisc.la

if ENABLE_NCZARR_FILTERS
noinst_LTLIBRARIES += libh5fletcher32.la libh5shuffle.la libh5deflate.la
libh5shuffle_la_SOURCES = H5Zshuffle.c
libh5fletcher32_la_SOURCES = H5Zfletcher32.c H5checksum.c
libh5deflate_la_SOURCES = H5Zdeflate.c

# Need our version of szip if libsz available and we are not using HDF5
if HAVE_SZ
noinst_LTLIBRARIES += libh5szip.la libnczszip.la
libh5szip_la_SOURCES = H5Zszip.c H5Zszip.h
libnczszip_la_SOURCES = NCZszip.c
endif

endif # ENABLE_NCZARR_FILTERS

if ENABLE_PLUGINS


libh5bzip2_la_SOURCES = H5Zbzip2.c h5bzip2.h
if ! HAVE_BZIP2
libh5bzip2_la_SOURCES += ${BZIP2SRC}
endif

libnczdefaults_la_SOURCES = NCZdefaults.c

if HAVE_BLOSC
libh5blosc_la_SOURCES = H5Zblosc.c H5Zblosc.h
endif

if HAVE_ZSTD
libh5zstd_la_SOURCES = H5Zzstd.c H5Zzstd.h
endif

noinst_LTLIBRARIES += libh5bzip2.la
noinst_LTLIBRARIES += libnczdefaults.la
if HAVE_BLOSC
noinst_LTLIBRARIES += libh5blosc.la
endif
if HAVE_ZSTD
noinst_LTLIBRARIES += libh5zstd.la
endif

endif #ENABLE_PLUGINS

libh5misc_la_SOURCES = H5Zmisc.c H5Zutil.c h5misc.h
libnczmisc_la_SOURCES = NCZmisc.c

# The noop filter is to allow testing of multifilters and filter order
# Need two distinct instances
libh5noop_la_SOURCES = H5Znoop.c H5Zutil.c h5noop.h
libh5noop1_la_SOURCES = H5Znoop1.c H5Zutil.c h5noop.h

endif #ENABLE_FILTER_TESTING

BUILT_SOURCES = H5Znoop1.c
DISTCLEANFILES = H5Znoop1.c ncjson.h
H5Znoop1.c: Makefile H5Znoop.c
	echo '#define NOOP_INSTANCE 1' > $@
	cat ${srcdir}/H5Znoop.c >> $@

BZIP2VER = 1.0.8
BZIP2DIR = bzip2-${BZIP2VER}
BZIP2URL = https://sourceware.org/pub/bzip2/${BZIP2DIR}.tar.gz
bzip2::
	rm -fr ./${BZIP2DIR} ${BZIP2SRC} BZIP2_LICENSE
	wget ${BZIP2URL}
	tar -zxf ${BZIP2DIR}.tar.gz
	cd ${BZIP2DIR}; cp ${BZIP2SRC} ..; cp LICENSE ../BZIP2_LICENSE ; cd ..
	rm -fr ./${BZIP2DIR}

# Custom install
install-exec-hook:
	sh ./stdinstall.sh
