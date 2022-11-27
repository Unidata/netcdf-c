## This is a automake file, part of Unidata's netCDF package.
# Copyright 2018v, see the COPYRIGHT file for more information.

# This file builds and runs DAP tests.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am
LDADD = ${top_builddir}/liblib/libnetcdf.la

# Un comment to use a more verbose test driver
#SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#TEST_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#TESTS_ENVIRONMENT = export SETX=1;

# Note which tests depend on other tests. Necessary for make -j check.
TEST_EXTENSIONS = .sh


AM_CPPFLAGS += -I$(top_srcdir)/liblib
AM_CPPFLAGS += -I$(top_srcdir)/libdap4
AM_CPPFLAGS += -DTOPSRCDIR=${abs_top_srcdir}
AM_CPPFLAGS += -DTOPBINDIR=${abs_top_builddir}

# Set up the tests; do the .sh first, then .c
check_PROGRAMS =
TESTS =

check_PROGRAMS += test_parse test_meta test_data

noinst_PROGRAMS =

TESTS += test_parse.sh test_meta.sh test_data.sh test_raw.sh 

# Note test_curlopt.sh is intended to be run manually; see comments in file.

# Note: This program name was changed to findtestserver4
# to avoid cmake complaint about duplicate targets.
noinst_PROGRAMS += findtestserver4 pingurl4
findtestserver4_SOURCES = findtestserver4.c
pingurl4_SOURCES = pingurl4.c

# Disable Dap4 Remote Tests until the test server is working
if BUILD_UTILITIES
if ENABLE_DAP_REMOTE_TESTS
TESTS += test_remote.sh
# Test require help at the server end
#  TESTS += test_hyrax.sh test_thredds.sh
endif
endif

EXTRA_DIST = CMakeLists.txt test_common.h build.sh \
	d4test_common.sh \
	test_curlopt.sh test_data.sh test_hyrax.sh test_meta.sh \
	test_parse.sh test_raw.sh test_remote.sh test_thredds.sh \
	cdltestfiles rawtestfiles \
	baseline baselineraw baselineremote baselinehyrax baselinethredds

CLEANFILES = *.exe
# This should only be left behind if using parallel io
CLEANFILES += tmp_*
CLEANFILES += ${execdir}/baselinethredds/GOES16_CONUS_20170821_020218_0.47_1km_33.3N_91.4W.nc4.thredds
 
DISTCLEANFILES = findtestserver4.c pingurl4.c

# One last thing
BUILT_SOURCES = .daprc

.daprc:
	echo "#DAPRC" >.daprc

clean-local: clean-local-check

.PHONY: clean-local-check

clean-local-check:
	-rm -rf results results_test_*
	-rm -f .dodsrc .daprc

# The shell file maketests.sh is used to build the testdata
# for dap4 testing. It creates and fills the directories
# daptestfiles, dmrtestfiles, and testsrc
# As a rule, this will only be invoked if there was a change
# in the GenerateRaw.java program in the thredds tree.
maketests::
	bash ./maketests.sh
