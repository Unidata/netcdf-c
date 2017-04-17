## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005, see the COPYRIGHT file for more information.

# This Makefile controls the building of the dispatch layer of the
# netCDF library. The dispatch layer decides whether to call the
# netcdf-classic code, netcdf-4 code, nc3 dap code, or nc4 dap
# code. It also contains code that sit above the dispatch layer, like
# the v2 API.

# Put together AM_CPPFLAGS and AM_LDFLAGS.
include $(top_srcdir)/lib_flags.am

# This is our output, the dispatch convenience library.
noinst_LTLIBRARIES = libdispatch.la
libdispatch_la_CPPFLAGS = ${AM_CPPFLAGS}

# The source files.
libdispatch_la_SOURCES = \
daux.c dapi.c dinternal.c \
dcopy.c derror.c ddispatch.c dutf8.c dstring.c \
nclog.c ncuri.c nclist.c ncbytes.c nchashmap.c nctime.c                        \
nc.c nclistmgr.c drc.c doffsets.c dwinpath.c

# Add the utf8 codebase
libdispatch_la_SOURCES += utf8proc.c utf8proc.h

# Turn on pre-processor flag when building a DLL for windows.
if BUILD_DLL
libdispatch_la_CPPFLAGS += -DDLL_EXPORT
endif # BUILD_DLL

# Add V2 API convenience library if needed.
if BUILD_V2
noinst_LTLIBRARIES += libnetcdf2.la 
libnetcdf2_la_SOURCES = dv2i.c
libnetcdf2_la_CPPFLAGS = ${AM_CPPFLAGS} -DDLL_EXPORT
endif # BUILD_V2

# With the API refactor, we keep these around, but do not use them.
DEPRECATED =	datt.c dattget.c dattinq.c dattput.c \
		dcompound.c ddim.c denum.c dfile.c \
		dgroup.c dopaque.c dparallel.c dtype.c \
		dvar.c dvarget.c dvarinq.c dvarput.c dvlen.c

EXTRA_DIST=CMakeLists.txt ncsettings.hdr utf8proc_data.c ${DEPRECATED}

# Build ncsettings.c as follows:
# 1. copy ncsettings.hdr to ncsettings.c
# 2. append libnetcdf.settings to ncsettings.c after
#    processing it as follows:
#    1. convert tabs and cr to blanks
#    2. convert embedded double quote (") to escaped form (\").
#    3. append newline (\n) to each line
#    4. surround each line with double quotes.
# 3. finally, add a semicolon to the end of ncsettings.c
#    to complete the string constant.

ncsettings.c: $(top_srcdir)/libnetcdf.settings ncsettings.hdr
	rm -f ncsettings.c
	cat ncsettings.hdr > ncsettings.c
	tr '\t\r' '  ' <${top_srcdir}/libnetcdf.settings | \
	sed -e 's/"/\\"/g' | \
	sed -e 's/\(.*\)/\"\1\\n\"/' | \
	cat >> ncsettings.c
	echo ';' >> ncsettings.c

# Unit tests
test_ncuri_SOURCES = test_ncuri.c ncuri.c ncbytes.c nclist.c
test_pathcvt_SOURCES = test_pathcvt.c dwinpath.c
check_PROGRAMS = test_ncuri test_pathcvt
TESTS = test_ncuri test_pathcvt

