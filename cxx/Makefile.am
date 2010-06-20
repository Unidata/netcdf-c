## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.
# This file builds the C++ interface.

# $Id: Makefile.am,v 1.54 2010/05/29 18:45:45 dmh Exp $

# Here's what the hand-crafted Makefile did to build cxx:
# c++ -c -g -O2 -I../libsrc -I.   -Df2cFortran netcdf.cpp
# c++ -c -g -O2 -I../libsrc -I.   -Df2cFortran ncvalues.cpp
# ar cru libnetcdf_c++.a netcdf.o ncvalues.o
# ranlib libnetcdf_c++.a
# c++ -c -g -O2 -I../libsrc -I.   -Df2cFortran nctst.cpp
# c++ -o nctst -g -O2  nctst.o libnetcdf_c++.a -L../libsrc -lnetcdf 

AM_CPPFLAGS = -I$(top_srcdir)
AM_LDFLAGS =

CLEANFILES = nctst_classic.nc nctst_64bit_offset.nc nctst_netcdf4.nc	\
nctst_netcdf4_classic.nc nctst_classic.cdl nctst_64bit_offset.cdl	\
tst_failure.nc

EXTRA_DIST = ref_nctst.cdl ref_nctst_64bit_offset.cdl			\
ref_nctst_netcdf4.cdl ref_nctst_netcdf4_classic.cdl run_nc_tests.sh	\
run_nc4_tests.sh

# nctst creates a simple data file in each format, then uses ncdump to
# create CDL files, which are compared with some reference files
# shipped with the code. tst_failure is a program that is intended to
# fail.
TESTFILES = nctst$(EXEEXT) tst_failure$(EXEEXT)
nctst_SOURCES = nctst.cpp
tst_failure_SOURCES = tst_failure.cpp
XFAIL_TESTS = tst_failure$(EXEEXT)

# Build and run these tests.
check_PROGRAMS = $(TESTFILES)
TESTS = $(TESTFILES)

# If the utilities have been built, run these extra tests.
if BUILD_UTILITIES
TESTS += run_nc_tests.sh
endif

AM_CPPFLAGS += -I${top_srcdir}/liblib
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la @EXTERN_LDFLAGS@
LDADD = $(top_builddir)/cxx/libnetcdf_c++.la  ${AM_LDFLAGS}

# These headers will be installed in the users header directory.
include_HEADERS = netcdfcpp.h ncvalues.h netcdf.hh

# This is our output library. 
lib_LTLIBRARIES = libnetcdf_c++.la

# Library source.
libnetcdf_c___la_SOURCES = netcdf.cpp ncvalues.cpp

test: check

