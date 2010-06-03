## This is an automake file, part of Unidata's NetCDF CF Library
## package. Copyright 2009, see the COPYRIGHT file for more
## information.

# This automake file is in charge of building the gridspec make_hgrid
# tools.

# $Id: Makefile.am,v 1.8 2010/05/29 01:46:33 dmh Exp $

bin_PROGRAMS = make_hgrid gs_make_hgrid
make_hgrid_SOURCES = make_hgrid.c create_conformal_cubic_grid.c		\
create_gnomonic_cubic_grid.c create_grid_from_file.c create_hgrid.h	\
create_lonlat_grid.c
gs_make_hgrid_SOURCES = gs_make_hgrid.c create_conformal_cubic_grid.c	\
create_gnomonic_cubic_grid.c create_grid_from_file.c create_hgrid.h	\
create_lonlat_grid.c


# Initalize these.
AM_CPPFLAGS =
AM_LDFLAGS =
LDADD =

AM_CPPFLAGS += -I${top_srcdir}/shared -I${top_srcdir}/shared/mosaic
AM_LDFLAGS += -L${top_builddir}/shared -L${top_builddir}/shared/mosaic -ltoolsshared -lshared

if USE_NETCDF_DIR
AM_CPPFLAGS += -I@NETCDFDIR@/include
AM_LDFLAGS += -L@NETCDFDIR@/lib
else
AM_CPPFLAGS += -I${top_srcdir}/../liblib
AM_LDFLAGS += ${top_builddir}/../liblib/libnetcdf.la
endif

AM_LDFLAGS += @EXTERN_LDFLAGS@ -lm

TESTS = run_tests
EXTRA_DIST = run_tests.in
CLEANFILES = horizontal_grid.nc gs_horizontal_grid.nc	\
horizontal_grid.cdl gs_horizontal_grid.cdl
DISTCLEANFILES = run_tests


