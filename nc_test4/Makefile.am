# This is part of the netCDF package.
# Copyright 2005 University Corporation for Atmospheric Research/Unidata
# See COPYRIGHT file for conditions of use.
# 
# This entire directory will be skipped, unless the configure script
# is run with --enable-netcdf-4.
#
# $Id: Makefile.am,v 1.72 2010/05/29 00:44:02 dmh Exp $

AM_LDFLAGS = ${top_builddir}/liblib/libnetcdf.la @EXTERN_LDFLAGS@
AM_CPPFLAGS = -I$(top_srcdir) -I$(top_builddir)/liblib @EXTERN_CFLAGS@

# Unfortunately, libsrc4 contains nc_test.h
AM_CPPFLAGS += -I$(top_srcdir)/libsrc4 -I$(top_srcdir)/libsrc

# Our test programs and sources...
NC4_TEST_PROGS = t_type cdm_sea_soundings tst_camrun tst_vl tst_atts	\
tst_atts2 tst_vars2 tst_v2
cdm_sea_soundings_SOURCES = cdm_sea_soundings.c tests.h
tst_camrun_SOURCES = tst_camrun.c tests.h
check_PROGRAMS = $(NC4_TEST_PROGS)
TESTS = $(NC4_TEST_PROGS)

#if BUILD_V2
#TESTS += tst_v2 
#endif # BUILD_V2

if LARGE_FILE_TESTS
tst_large_SOURCES = tst_large.c tests.h
check_PROGRAMS += tst_large
TESTS += tst_large
endif # LARGE_FILE_TESTS

if BUILD_BENCHMARKS
bm_file_SOURCES = bm_file.c tests.h
tst_create_files_SOURCES = tst_create_files.c tests.h
tst_ar4_SOURCES = tst_ar4.c tests.h
tst_chunks_SOURCES = tst_chunks.c tests.h
check_PROGRAMS += tst_create_files bm_file tst_chunks tst_ar4	\
tst_ar4_3d tst_ar4_4d
TESTS += tst_ar4_3d tst_create_files run_bm_test1.sh run_bm_elena.sh	\
run_bm_test2.sh run_tst_chunks.sh run_bm_ar4.sh
if TEST_PARALLEL
TESTS += run_par_bm_test.sh
endif # TEST_PARALLEL
benchmarks: check
	./run_bm_radar_2D.sh	
	./run_bm_radar_2D_compression1.sh	
	./run_bm.sh
	./run_tst_chunks.sh
	./run_bm_ar4.sh
endif # BUILD_BENCHMARKS

# These are the tests for HDF4.
if USE_HDF4
check_PROGRAMS += tst_interops2 
TESTS += tst_interops2
if USE_HDF4_FILE_TESTS
check_PROGRAMS += tst_interops3
TESTS += run_get_hdf4_files.sh tst_interops3
endif # USE_HDF4_FILE_TESTS
tst_interops2_LDADD = ${lib_LTLIBRARIES} -lmfhdf -ldf -ljpeg -lhdf5_hl	\
-lhdf5 -lz
endif # USE_HDF4

CLEANFILES = cdm_sea_soundings.nc bm_chunking.nc bm_radar.nc		\
bm_radar1.nc radar_3d_compression_test.txt radar_3d_compression.txt	\
radar_2d_compression.txt radar_3d_chunking.txt tst_floats_1D.cdl	\
floats_1D_3.nc floats_1D.cdl tst_*.nc tst_floats2_*.cdl	\
tst_ints2_*.cdl tst_shorts2_*.cdl tst_elena_*.cdl tst_simple*.cdl	\
tst_chunks.cdl pr_A1.* tauu_A1.* usi_01.* thetau_01.*

# This are extra tests that will only be run if netcdf-4 is configured
# with --enable-parallel-tests.
if TEST_PARALLEL
check_PROGRAMS += tst_mpi_parallel tst_parallel tst_parallel3	\
tst_parallel4 tst_nc4perf
CLEANFILES += tst_mpi_parallel.bin tst_parallel.nc tst_parallel3.nc	\
tst_parallel4.nc tst_nc4perf.nc tst_parallel2.nc
TESTS += run_par_test.sh
if USE_PNETCDF
check_PROGRAMS += tst_parallel2
TESTS += run_pnetcdf_test.sh
endif
endif

EXTRA_DIST = run_par_test.sh run_bm.sh run_bm_test1.sh run_bm_test2.sh	\
run_bm_radar_2D.sh run_bm_radar_2D_compression1.sh run_par_bm_test.sh	\
run_bm_elena.sh run_par_bm_radar_2D.sh run_bm_radar_2D_endianness1.sh	\
run_tst_chunks.sh ref_chunks1.cdl ref_chunks2.cdl run_pnetcdf_test.sh	\
run_get_hdf4_files.sh

if USE_HDF4_FILE_TESTS
DISTCLEANFILES = AMSR_E_L2_Rain_V10_200905312326_A.hdf	\
AMSR_E_L3_DailyLand_V06_20020619.hdf			\
MYD29.A2009152.0000.005.2009153124331.hdf		\
MYD29.A2002185.0000.005.2007160150627.hdf		\
MOD29.A2000055.0005.005.2006267200024.hdf
endif # HDF4_FILE_TESTS



