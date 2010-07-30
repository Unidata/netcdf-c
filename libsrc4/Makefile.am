## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This automake file generates the Makefile to build netCDF-4. The
# generated makefile is not run unless the user selected to build
# netCDF-4.

# $Id: Makefile.am,v 1.194 2010/06/01 15:34:50 ed Exp $

# Initialize
AM_CPPFLAGS = -I$(top_srcdir)/include @EXTERN_CFLAGS@	\
-I${top_srcdir}/libsrc -I${top_srcdir}/libdispatch
AM_LDFLAGS = @EXTERN_LDFLAGS@
LDADD =

if USE_DAP
AM_CPPFLAGS += -I${top_srcdir}/oc
endif

# Netcdf-4 source.
NC4SOURCES = nc4internal.h error4.c \
nc4dispatch.c nc4dispatch.h nc4attr.c nc4dim.c	\
nc4file.c nc4grp.c nc4type.c nc4var.c ncfunc.c nc4internal.c nc4hdf.c

# This is our output. The big enchilada.
noinst_LTLIBRARIES = libnetcdf4.la
libnetcdf4_la_SOURCES = ${NC4SOURCES}

# These linker flags will be applied to all test program compiles.
# Currently we cannot get away from loading libnetcdf3.la and libnetcdf2.la
# and various other things
LDADD += stub4.lo ${top_builddir}/libsrc4/libnetcdf4.la	\
${top_builddir}/libsrc/libnetcdf3.la			\
${top_builddir}/libdispatch/libdispatch.la

if USE_DAP
LDADD += ${top_builddir}/oc/liboc.la
endif
if BUILD_V2
LDADD += ${top_builddir}/libdispatch/libnetcdf2.la
endif 

NC4_TESTS = tst_h_files tst_h_files2 tst_h_atts tst_h_atts3 tst_h_vars	\
tst_h_vars2 tst_h_vars3 tst_h_grps tst_h_compounds tst_h_compounds2	\
tst_h_wrt_cmp tst_h_rd_cmp tst_h_vl tst_h_opaques tst_h_strings		\
tst_h_strings1 tst_h_dimscales tst_h_dimscales1 tst_h_dimscales2	\
tst_h_dimscales3 tst_h_enums tst_dims tst_dims2 tst_dims3 tst_files	\
tst_files4 tst_vars tst_varms tst_unlim_vars tst_converts		\
tst_converts2 tst_grps tst_compounds tst_compounds2 tst_compounds3	\
tst_opaques tst_strings tst_interops tst_interops4 tst_interops5	\
tst_interops6 tst_enums tst_coords tst_coords2 tst_coords3 tst_vars3	\
tst_chunks tst_utf8 tst_fills tst_fillbug tst_xplatform tst_xplatform2	\
tst_h_atts2 tst_endian_fill

check_PROGRAMS = $(NC4_TESTS)
if LARGE_FILE_TESTS
check_PROGRAMS += tst_large tst_large2
endif

# The parallel program is run from a script.
if TEST_PARALLEL
check_PROGRAMS += tst_h_par
endif

TESTS = $(NC4_TESTS)

EXTRA_DIST = ref_tst_compounds.nc ref_tst_h_compounds.h5		\
ref_tst_h_compounds2.h5 run_par_tests.sh run_valgrind_tests.sh		\
run_hdf4_valgrind_tests.sh ref_tst_xplatform2_1.nc			\
ref_tst_xplatform2_2.nc ref_tst_dims.nc ref_tst_interops4.nc stub4.c

# These linker flags will be applied to all test program compiles.
LDADD += ${lib_LTLIBRARIES} 

if USE_HDF4
LDADD += -lmfhdf -ldf -ljpeg
endif # USE_HDF4

LDADD += -lhdf5_hl -lhdf5 -lz 

if USE_PNETCDF
LDADD += -lpnetcdf
endif

# This script will run all the parallel test programs.
if TEST_PARALLEL
TESTS += run_par_tests.sh
endif

# Add this if --enable-benchmarks was used.
if BUILD_BENCHMARKS
TESTS += tst_files2 tst_files3 tst_ar5 tst_h_files3
check_PROGRAMS += tst_files2 tst_files3 tst_ar5 tst_h_files3
endif # BUILD_BENCHMARKS

# This will run a bunch of the test programs with valgrind, the memory
# checking tool. (Valgrind must be present for this to work.)
if USE_VALGRIND_TESTS
TESTS += run_valgrind_tests.sh
if USE_HDF4
TESTS += run_hdf4_valgrind_tests.sh
endif # USE_HDF4
endif # USE_VALGRIND_TESTS

# Clean up. Each test program creates one data file, with the same
# name as the test program, and either a .h5 or .nc, for "pure" HDF5
# files and netCDF-4 files.
CLEANFILES = tst_nc_converts.nc tst_h_*.h5 tst_*.nc tst_interops*.h5	\
tst_interops2.h4 tst_interops4.nc AMSR_E_* MYD29.A2* 


