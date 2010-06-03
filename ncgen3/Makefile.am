# This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.
# This file builds and runs the ncgen program.

# $Id: Makefile.am,v 1.54 2010/05/29 00:42:11 dmh Exp $

AM_LDFLAGS =
AM_CPPFLAGS =
LDADD =

LDADD += ${top_builddir}/liblib/libnetcdf.la @EXTERN_LDFLAGS@
AM_CPPFLAGS += -I${top_srcdir}/liblib

# Build ncgen from the listed sources.
bin_PROGRAMS = ncgen3
ncgen3_SOURCES = main.c load.c escapes.c getfill.c init.c	\
genlib.c generic.h ncgen.h genlib.h ncgentab.h ncgentab.c

man_MANS = ncgen3.1

# These files all need to be distributed.
EXTRA_DIST = ncgen.y ncgenyy.c ncgen.l c0.cdl run_tests.sh run_nc4_tests.sh \
             $(man_MANS)

# This shell script causes ncgen3 to build a classic and a 64-bit
# offset file from a cdl file shipped with the distribution.
TESTS = run_tests.sh 

# The tests produce these files.
CLEANFILES = c0.nc c0_64.nc

if USE_NETCDF4

# This shell script causes ncgen to build a netcdf-4 and netcdf-4
# classic file from a cdl file shipped with the distribution.
TESTS += run_nc4_tests.sh

# The nc4 tests produce these files.
CLEANFILES += c0_4.nc c0_4c.nc

endif # USE_NETCDF4

LDADD += -lm

# Here is the sequence of tests that need to be run:
# ./ncgen3 -b -o c0.nc c0.cdl
# ../ncdump/ncdump -n c1 c0.nc > c1.cdl
# *** ncgen3 -b test successful ***
# ./ncgen3 -c -o ctest0.nc c0.cdl > ctest.c && \
# cc -o ctest -g  -I../libsrc -I.   -Df2cFortran ctest.c ../libsrc/libnetcdf.a  && \
# ./ctest	&& \
# ../ncdump/ncdump -n c1 ctest0.nc > ctest1.cdl
# *** ncgen3 -c test successful ***

test: check

# This is used if someone wants to rebuild ncgenyy.c or ncgentab.c
# Otherwise never invoked, but records how to do it. Don't forget to
# manually insert #include <config.h> in ncgenyy.c!
ncgenyy.c: ncgen.l
	flex -Pncg -8 ncgen.l
	mv lex.ncg.c ncgenyy.c

ncgentab.c ncgentab.h: ncgen.y
	bison -pncg -d ncgen.y
	mv ncgen.tab.c ncgentab.c
	mv ncgen.tab.h ncgentab.h
