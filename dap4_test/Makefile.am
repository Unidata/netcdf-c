## This is a automake file, part of Unidata's netCDF package.
# Copyright 2018v, see the COPYRIGHT file for more information.

# This file builds and runs DAP tests.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# Un comment to use a more verbose test driver
#SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#TEST_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose

# Note which tests depend on other tests. Necessary for make -j check.
TEST_EXTENSIONS = .sh

LDADD = ${top_builddir}/liblib/libnetcdf.la
AM_CPPFLAGS += -I$(top_srcdir)/liblib
AM_CPPFLAGS += -I$(top_srcdir)/libdap4
AM_CPPFLAGS += -DTOPSRCDIR=${abs_top_srcdir}
AM_CPPFLAGS += -DTOPBINDIR=${abs_top_bindir}

# Set up the tests; do the .sh first, then .c
check_PROGRAMS =
TESTS =

check_PROGRAMS += test_parse test_meta test_data

noinst_PROGRAMS =

TESTS += test_parse.sh test_meta.sh test_data.sh test_fillmismatch.sh test_raw.sh 

# Note tst_curlopt.sh is intended to be run manually; see comments in file.

# Note: This program name was changed to findtestserver4
# to avoid cmake complaint about duplicate targets.
noinst_PROGRAMS += findtestserver4 pingurl4
findtestserver4_SOURCES = findtestserver4.c
pingurl4_SOURCES = pingurl4.c

# Disable Dap4 Remote Tests until the test server is working
if AX_IGNORE
if ENABLE_DAP_REMOTE_TESTS
if BUILD_UTILITIES
  # relies on ncdump
  TESTS += test_remote.sh
endif
endif
endif

EXTRA_DIST = test_parse.sh test_meta.sh test_data.sh \
             test_raw.sh test_remote.sh test_hyrax.sh test_fillmismatch.sh \
             tst_curlopt.sh d4test_common.sh \
	     daptestfiles dmrtestfiles cdltestfiles nctestfiles misctestfiles \
	     baseline baselineraw baselineremote CMakeLists.txt test_common.h

CLEANFILES = *.exe
# This should only be left behind if using parallel io
CLEANFILES += tmp_*

DISTCLEANFILES = findtestserver4.c pingurl4.c

# One last thing
BUILT_SOURCES = .daprc

.daprc:
	echo "#DAPRC" >.daprc

clean-local: clean-local-check

.PHONY: clean-local-check

clean-local-check:
	-rm -rf results
	-rm -f .dodsrc .daprc

# The shell file maketests.sh is used to build the testdata
# for dap4 testing. It creates and fills the directories
# daptestfiles, dmrtestfiles, and cdltestfiles
# As a rule, this will only be invoked if there was a change
# in the GenerateRaw.java program in the thredds tree.
maketests::
	bash ./maketests.sh
