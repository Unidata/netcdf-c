#    Copyright 2018, UCAR/Unidata
#    See netcdf/COPYRIGHT file for copying and redistribution conditions.

# Put Together AM_CPPFLAGS and AM_LDFLAGS
include $(top_srcdir)/lib_flags.am

AM_LDFLAGS += -module -avoid-version -shared -export-dynamic	\
 -rpath ${abs_builddir} ${NOUNDEFINED}

if ISMINGW
LDADD =  ${top_builddir}/liblib/libnetcdf.la
endif

# BZIP2 version 1.0.8 (https://sourceware.org/bzip2/)
BZIP2SRC = blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c bzlib.h bzlib_private.h

EXTRA_DIST = CMakeLists.txt \
             H5Ztemplate.c H5Zmisc.c H5Zutil.c H5Znoop.c h5noop.h NCZmisc.c \
	     NCZdefaults.c \
	     H5Zbzip2.c h5bzip2.h H5Zblosc.c H5Zblosc.h \
     	     H5Zshuffle.c H5Zdeflate.c H5Zszip.c H5Zszip.h \
	     ${BZIP2SRC} H5checksum.c

if ENABLE_FILTER_TESTING

lib_LTLIBRARIES = libh5bzip2.la
libh5bzip2_la_SOURCES = H5Zbzip2.c h5bzip2.h ${BZIP2SRC}
	     
noinst_LTLIBRARIES = libh5misc.la libh5noop.la libh5noop1.la libnczmisc.la \
		     libh5shuffle.la libh5fletcher32.la libh5deflate.la
noinst_LTLIBRARIES += libnczdefaults.la

libh5shuffle_la_SOURCES = H5Zshuffle.c
libh5fletcher32_la_SOURCES = H5Zfletcher32.c H5checksum.c
libh5deflate_la_SOURCES = H5Zdeflate.c

if ENABLE_BLOSC
noinst_LTLIBRARIES += libh5blosc.la
libh5blosc_la_SOURCES = H5Zblosc.c H5Zblosc.h
endif

if ENABLE_SZIP
noinst_LTLIBRARIES += libh5szip.la
libh5szip_la_SOURCES = H5Zszip.c H5Zszip.h
endif

libh5misc_la_SOURCES = H5Zmisc.c H5Zutil.c h5misc.h

libnczmisc_la_SOURCES = NCZmisc.c

libnczdefaults_la_SOURCES = NCZdefaults.c

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

# For reference: updating bzip2 
BZIP2DIR=/cygdrive/d/bzip2-1.0.8
updatebzip2:
	for b in ${BZIP2HDRS} ${BZIP2SRC} ; do cp -f ${BZIP2DIR}/$${b} . ; done
