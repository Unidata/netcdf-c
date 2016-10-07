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
TESTS_ENVIRONMENT=TOPSRCDIR=${abs_top_srcdir}

t_dmrparse_SOURCES = t_dmrparse.c
t_dmrmeta_SOURCES = t_dmrmeta.c
t_dmrdata_SOURCES = t_dmrdata.c

check_PROGRAMS += t_dmrparse t_dmrmeta t_dmrdata

TESTS += t_dmrparse.sh t_dmrmeta.sh t_dmrdata.sh

EXTRA_DIST = testfiles baseline t_dmrparse.sh t_dmrmeta.sh t_dmrdata.sh

CLEANFILES = 

# One last thing
BUILT_SOURCES = .daprc

.daprc:
	echo "#DAPRC" >.daprc

DISTCLEANFILES = .daprc results/* results

t_dmrparse.c: ../libdap4/t_dmrparse.c ../libdap4/t_dmr.h
	rm -f ./t_dmrparse.c ./t_dmr.h
	cp ../libdap4/t_dmrparse.c ../libdap4/t_dmr.h .

t_dmrmeta.c: ../libdap4/t_dmrmeta.c ../libdap4/t_dmr.h
	rm -f ./t_dmrmeta.c ./t_dmr.h
	cp ../libdap4/t_dmrmeta.c ../libdap4/t_dmr.h .

t_dmrdata.c: ../libdap4/t_dmrdata.c ../libdap4/t_dmr.h
	rm -f ./t_dmrdata.c ./t_dmr.h
	cp ../libdap4/t_dmrdata.c ../libdap4/t_dmr.h .
