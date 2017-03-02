## This is a automake file, part of Unidata's netCDF package.
# Copyright 2011, see the COPYRIGHT file for more information.

# This file builds and runs DAP tests.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

LDADD = ${top_builddir}/liblib/libnetcdf.la
AM_CPPFLAGS += -DTOPSRCDIR=${abs_top_srcdir} -I$(top_builddir)/liblib

# Set up the tests; do the .sh first, then .c
check_PROGRAMS =
TESTS =

TESTS += tst_ncdap3.sh

t_dap3a_SOURCES = t_dap3a.c t_srcdir.h
test_cvt3_SOURCES = test_cvt.c
test_vara_SOURCES = test_vara.c

check_PROGRAMS += t_dap3a test_cvt3 test_vara

TESTS += t_dap3a test_cvt3 test_vara

# remote tests are optional
# because the server may be down or inaccessible

if ENABLE_DAP_REMOTE_TESTS

# Make nctestserver be the first test to be built
check_PROGRAMS += nctestserver
nctestserver_SOURCES = nctestserver.c

check_PROGRAMS += topsrcdir
topsrcdir_SOURCES = topsrcdir.c

if BUILD_UTILITIES
TESTS += tst_remote3.sh tst_formatx.sh
endif

TESTS += test_partvar testurl.sh

if ENABLE_DAP_LONG_TESTS
 TESTS += tst_longremote3.sh
endif

test_partvar_SOURCES = test_partvar.c
test_nstride_cached_SOURCE = test_nstride_cached.c
#t_auth_SOURCES = t_auth.c
#t_auth1_SOURCES = t_auth1.c
t_misc_SOURCES = t_misc.c
test_varm3_SOURCES = test_varm3.c
#t_ncf330_SOURCES = t_ncf330.c

TESTS += test_varm3
#TESTS += t_ncf330
TESTS += test_nstride_cached
TESTS += t_misc

check_PROGRAMS += test_partvar
check_PROGRAMS += test_nstride_cached
check_PROGRAMS += t_misc
check_PROGRAMS += test_varm3
check_PROGRAMS += t_ncf330

if ENABLE_DAP_AUTH_TESTS
#check_PROGRAMS += t_auth t_auth1
#TESTS += t_auth
TESTS += testauth.sh
endif

endif #ENABLE_DAP_REMOTE_TESTS

# Need to add subdirs
SUBDIRS = testdata3 expected3 expectremote3

EXTRA_DIST = tst_ncdap3.sh  \
             tst_remote3.sh \
             tst_longremote3.sh \
             tst_ncdap.sh tst_ncdap_shared.sh tst_remote.sh \
	     t_dap.c CMakeLists.txt tst_formatx.sh testauth.sh testurl.sh \
			t_ncf330.c

CLEANFILES = test_varm3 test_cvt3 results/*.dmp results/*.das results/*.dds datadds* t_dap3a test_nstride_cached

if USE_NETCDF4
CLEANFILES += test_cvt4 t_dap4
endif

# This rule are used if someone wants to rebuild t_dap3a.c
# Otherwise never invoked, but records how to do it.
t_dap3a.c: t_dap.c
	echo "#define NETCDF3ONLY" > ./t_dap3a.c
	cat t_dap.c >> t_dap3a.c

t_dap4a.c: t_dap.c
	cat t_dap.c >> ./t_dap4a.c

# One last thing
BUILT_SOURCES = .dodsrc

.dodsrc:
	echo "#DODSRC" >.dodsrc

DISTCLEANFILES = .dodsrc results/* results
