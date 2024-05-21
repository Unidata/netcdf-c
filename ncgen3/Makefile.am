# This is a automake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.
# This file builds and runs the ncgen program.

#SH_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#sh_LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#LOG_DRIVER = $(SHELL) $(top_srcdir)/test-driver-verbose
#TESTS_ENVIRONMENT = export SETX=1;

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# Link to the netCDF library.
LDADD = ${top_builddir}/liblib/libnetcdf.la

# Build ncgen from the listed sources.
bin_PROGRAMS = ncgen3
ncgen3_SOURCES = main.c load.c escapes.c getfill.c init.c	\
genlib.c generic.h ncgen.h genlib.h ncgeny.h ncgeny.c

man_MANS = ncgen3.1

# These files all need to be distributed.
EXTRA_DIST = ncgen.y ncgen.l ncgenl.c c0.cdl run_tests.sh	\
run_nc4_tests.sh $(man_MANS) CMakeLists.txt

# There is a netcdf classic and netcdf-4 test script, but don't run
# them for DLL builds.
#if !BUILD_DLL

TESTS = run_tests.sh
if USE_HDF5
TESTS += run_nc4_tests.sh
endif # USE_HDF5
#endif # !BUILD_DLL

CLEANFILES = c0.nc c0_64.nc c0_4.nc c0_4c.nc


# This is used if someone wants to rebuild the parser and lexer
# Otherwise never invoked, but records how to do it.
makeparser::
	rm -f ncgenl.c lex.ncg.c
	flex -Pncg -8 ncgen.l
	rm -f ncgenl.c
	sed -e s/lex.ncg.c/ncgenl.c/g <lex.ncg.c >ncgenl.c
	bison -pncg -t -d ncgen.y
	rm -f ncgeny.c ncgeny.h
	sed -e s/ncgen.tab.c/ncgeny.c/g -e s/ncgen.tab.h/ncgeny.h/g <ncgen.tab.c >ncgeny.c
	sed -e s/ncgen.tab.c/ncgeny.c/g -e s/ncgen.tab.h/ncgeny.h/g <ncgen.tab.h >ncgeny.h
	rm -f ncgen.tab.c ncgen.tab.h
