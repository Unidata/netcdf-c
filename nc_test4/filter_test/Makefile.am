# Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
# See the COPYRIGHT file for more information.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

FILTERNAME=bzip2

BZIP2HDRS=bzlib.h bzlib_private.h
BZIP2SRC=\
      blocksort.c  \
      huffman.c    \
      crctable.c   \
      randtable.c  \
      compress.c   \
      decompress.c \
      bzlib.c

PLUGINSRC=H5Zbzip2.c
PLUGINHDRS=h5bzip2.h

bin_PROGRAMS = fake
fake_SOURCES = fake.c H5Zbzip2.c ${BZIP2SRC}

# Link to our assembled library.
test_filter_LDFLAGS = ${top_builddir}/liblib/libnetcdf.la
test_filter_SOURCE = test_filter.c

BZIP2OBJ=${BZIP2SRC:%.o=%.c}

if ISCYGWIN
LIBNAME=cyg${FILTERNAME}.dll
else
LIBNAME=lib${FILTERNAME}.so
endif

EXTRA_DIST=${PLUGINSRC} ${BZIP2SRC} ${PLUGINHDRS} ${BZIP2HDRS} \
	tst_filter.sh bzip2.cdl CMakeLists.txt
CLEANFILES = ${LIBNAME} *.nc *.dump

if ENABLE_FILTER_TEST

check_PROGRAMS = test_filter
TESTS = tst_filter.sh

all-local:
	gcc -fPIC -g -O0 -shared \
	-o ${LIBNAME} ${PLUGINSRC} ${BZIP2OBJ} \
	-L/usr/local/lib -lhdf5_hl -lhdf5 -lz
endif


