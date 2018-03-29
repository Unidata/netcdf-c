# Developer Documentation for PIO Integration

This document describes the addition of advanced HPC features with the
PIO library code.

## Build System

The PIO extensions are turned on with --enable-pio configure
option. This requires a parallel build.

The CMake build system does not build PIO. In the HPC world CMake is
not used much, and autotools will be used to install netCDF. CMake
builds the windows port of netCDF, and PIO and the Windows port will
never be built together, because no HPC runs Windows.

In config.h the macro USE_PIO is defined in PIO is in use.

## Dispatch Layer

In the dispatch layer PIO impacts dfile.c. In this code file are the
NC_open() and NC_create() calls. These functions have been modified to
call the PIO dispatch layer when the flag NC_PIO is included in the
mode of either nc_open() or nc_create().

## Directory libpio

The libpio directory contains all the PIO code. The functions that
start with PIOc_ are called by the PIO dispatch table.

These functions also work the same as they did with the existing PIO
library. So netCDF with PIO can be used as a drop-in replacement for
the PIO library.

## Directory pio_test

The tests for PIO are in the pio_test directory.

The tests which begin with test_ are the tests from the PIO
library. They call the PIOc_ functions.

The tests which begin with tst_ are the pure netCDF tests, which test
PIO and netCDF without directly invoking the PIOc_ functions. Only nc_
functions are called.
