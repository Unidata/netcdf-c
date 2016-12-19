## This is a automake file, part of Unidata's netCDF package.
# Copyright 2011, see the COPYRIGHT file for more information.

# This file builds and runs DAP tests.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

LDADD = ${top_builddir}/liblib/libnetcdf.la
AM_CPPFLAGS += -I$(top_builddir)/liblib -I$(top_builddir)/libdap4

# Set up the tests; do the .sh first, then .c
check_PROGRAMS =
TESTS =

t_dmrparse_SOURCES = t_dmrparse.c
t_dmrmeta_SOURCES = t_dmrmeta.c
t_dmrdata_SOURCES = t_dmrdata.c

check_PROGRAMS += t_dmrparse t_dmrmeta t_dmrdata

TESTS += t_dmrparse.sh t_dmrmeta.sh t_dmrdata.sh test_raw.sh

if ENABLE_DAP4_REMOTE_TESTS

if BUILD_UTILITIES
TESTS += test_remote.sh
# not yet TESTS += test_hyrax.sh
endif

endif

EXTRA_DIST = t_dmrparse.sh t_dmrmeta.sh t_dmrdata.sh \
             test_raw.sh test_remote.sh test_hyrax.sh \
             test_common.sh \
	     daptestfiles dmrtestfiles cdltestfiles \
	     baseline baselineraw baselineremote

CLEANFILES = 

# One last thing
BUILT_SOURCES = .daprc

.daprc:
	echo "#DAPRC" >.daprc

DISTCLEANFILES = .daprc results/* results

# The shell file maketests.sh is used to build the testdata
# for dap4 testing. It creates and fills the directories
# daptestfiles, dmrtestfiles, and cdltestfiles
# As a rule, this will only be invoked if there was a change
# in the GenerateRaw.java program in the thredds tree.
maketests::
	bash ./maketests.sh
