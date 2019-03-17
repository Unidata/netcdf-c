# This is part of the netCDF package. Copyright 2005-2019 University
# Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
# conditions of use.
#
# This directory holds tests for netCDF-4. It is skipped if netCDF-4
# is not enabled.
#
# Ed Hartnett, Ward Fisher

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# Un comment to use a more verbose test driver
#SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose

TEST_EXTENSIONS = .sh

# Link to our assembled library.
AM_LDFLAGS += ${top_builddir}/liblib/libnetcdf.la
LDADD = ${top_builddir}/liblib/libnetcdf.la
