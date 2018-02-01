# This is part of the netCDF package.
# Copyright 2017 University Corporation for Atmospheric Research/Unidata
# See COPYRIGHT file for conditions of use.
#
# This file builds the tests for the PIO parallel I/O features. This
# entire directory will be skipped if PIO is not enabled.
#

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# Link to our assembled library.
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la

# The tests that will be run.
PIO_TESTS = test_async_mpi test_spmd test_intercomm2			\
test_async_simple test_async_3proc test_async_4proc			\
test_iosystem2_simple test_iosystem2_simple2 test_iosystem2		\
test_iosystem3_simple test_iosystem3_simple2 test_iosystem3 test_pioc	\
test_pioc_unlim test_pioc_putget test_pioc_fill test_darray		\
test_darray_multi test_darray_multivar test_darray_multivar2		\
test_darray_multivar3 test_darray_1d test_darray_3d			\
test_decomp_uneven test_decomps test_rearr test_darray_async_simple	\
test_darray_async test_darray_async_many test_darray_2sync		\
test_async_multicomp test_async_multi2 test_async_manyproc

NC_PIO_TESTS = tst_darray_async tst_darray_1d

check_PROGRAMS = $(PIO_TESTS) $(NC_PIO_TESTS)

# Source code for each test.
test_async_mpi_SOURCES = test_async_mpi.c
test_intercomm2_SOURCES = test_intercomm2.c test_common.c pio_tests.h
test_async_simple_SOURCES = test_async_simple.c test_common.c pio_tests.h
test_async_4proc_SOURCES = test_async_4proc.c test_common.c pio_tests.h
test_iosystem2_simple_SOURCES = test_iosystem2_simple.c test_common.c pio_tests.h
test_iosystem2_simple2_SOURCES = test_iosystem2_simple2.c test_common.c pio_tests.h
test_iosystem2_SOURCES = test_iosystem2.c test_common.c pio_tests.h
test_iosystem3_simple_SOURCES = test_iosystem3_simple.c test_common.c pio_tests.h
test_iosystem3_simple2_SOURCES = test_iosystem3_simple2.c test_common.c pio_tests.h
test_iosystem3_SOURCES = test_iosystem3.c test_common.c pio_tests.h
test_pioc_SOURCES = test_pioc.c test_common.c test_shared.c pio_tests.h
test_pioc_unlim_SOURCES = test_pioc_unlim.c test_common.c test_shared.c pio_tests.h
test_pioc_putget_SOURCES = test_pioc_putget.c test_common.c test_shared.c pio_tests.h
test_pioc_fill_SOURCES = test_pioc_fill.c test_common.c test_shared.c pio_tests.h
test_darray_SOURCES = test_darray.c test_common.c pio_tests.h
test_darray_multi_SOURCES = test_darray_multi.c test_common.c pio_tests.h
test_darray_multivar_SOURCES = test_darray_multivar.c test_common.c pio_tests.h
test_darray_multivar2_SOURCES = test_darray_multivar2.c test_common.c pio_tests.h
test_darray_multivar3_SOURCES = test_darray_multivar3.c test_common.c pio_tests.h
test_darray_1d_SOURCES = test_darray_1d.c test_common.c pio_tests.h
test_darray_3d_SOURCES = test_darray_3d.c test_common.c pio_tests.h
test_decomp_uneven_SOURCES = test_decomp_uneven.c test_common.c pio_tests.h
test_decomps_SOURCES = test_decomps.c test_common.c pio_tests.h
test_rearr_SOURCES = test_rearr.c test_common.c pio_tests.h
test_darray_async_simple_SOURCES = test_darray_async_simple.c test_common.c pio_tests.h
test_darray_async_SOURCES = test_darray_async.c test_common.c pio_tests.h
test_darray_async_many_SOURCES = test_darray_async_many.c test_common.c pio_tests.h
test_darray_2sync_SOURCES = test_darray_2sync.c test_common.c pio_tests.h
test_spmd_SOURCES = test_spmd.c test_common.c pio_tests.h
test_async_3proc_SOURCES = test_async_3proc.c test_common.c pio_tests.h
test_async_multicomp_SOURCES = test_async_multicomp.c test_common.c pio_tests.h
test_async_multi2_SOURCES = test_async_multi2.c test_common.c pio_tests.h
test_async_manyproc_SOURCES = test_async_manyproc.c test_common.c pio_tests.h

tst_darray_async_SOURCES = tst_darray_async.c test_common.c pio_tests.h
tst_darray_1d_SOURCES = tst_darray_1d.c test_common.c pio_tests.h

# Tests will run from a bash script.
TESTS = run_tests.sh

# Bash script needs all tests built.
#run_tests.sh : $(PIO_TESTS) $(NC_PIO_TESTS)

# Distribute the test script.
EXTRA_DIST = run_tests.sh

# Clean up files produced during testing.
CLEANFILES = *.nc *.log

