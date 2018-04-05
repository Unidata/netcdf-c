# This is part of Unidata's netCDF package. Copyright 2018, see the
# COPYRIGHT file for more information.

# Build the HDF5 dispatch layer.

# Ed Hartnett

# Get AM_CPPFLAGS.
include $(top_srcdir)/lib_flags.am
libnetcdf4_la_CPPFLAGS = ${AM_CPPFLAGS}

# This is our output. The HDF5 convenience library.
noinst_LTLIBRARIES = libnchdf5.la

# The source files.
# libnchdf5_la_SOURCES = nc4hdf.c hdf5file.c hdf5dispatch.c hdf5var.c	\
# hdf5internal.c hdf5type.c hdf5func.c hdf5attr.c
libnchdf5_la_SOURCES = nc4hdf.c

# Package this for cmake build.
EXTRA_DIST = CMakeLists.txt





