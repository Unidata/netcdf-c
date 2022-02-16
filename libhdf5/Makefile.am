# This is part of Unidata's netCDF package. Copyright 2018-2022,
# see the COPYRIGHT file for more information.

# Build the HDF5 dispatch layer.

# Ed Hartnett

# Get AM_CPPFLAGS.
include $(top_srcdir)/lib_flags.am
libnchdf5_la_CPPFLAGS = ${AM_CPPFLAGS}

# This is our output. The HDF5 convenience library.
noinst_LTLIBRARIES = libnchdf5.la

# The source files.
libnchdf5_la_SOURCES = nc4hdf.c nc4info.c hdf5file.c hdf5attr.c		\
hdf5dim.c hdf5grp.c hdf5type.c hdf5internal.c hdf5create.c hdf5open.c	\
hdf5var.c nc4mem.c nc4memcb.c hdf5dispatch.c hdf5filter.c   \
hdf5set_format_compatibility.c hdf5debug.c hdf5debug.h hdf5err.h

if ENABLE_BYTERANGE
libnchdf5_la_SOURCES += H5FDhttp.c H5FDhttp.h
endif

# Package this for cmake build.
EXTRA_DIST = CMakeLists.txt
