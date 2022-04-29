## This is a automake file, part of Unidata's netCDF package.
# Copyright 2008, see the COPYRIGHT file for more information.

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
libdispatch_la_SOURCES = dcopy.c dfile.c ddim.c datt.c dattinq.c	\
dattput.c dattget.c derror.c dvar.c dvarget.c dvarput.c dvarinq.c	\
dinternal.c ddispatch.c dutf8.c nclog.c dstring.c ncuri.c nclist.c	\
ncbytes.c nchashmap.c nctime.c nc.c nclistmgr.c dauth.c doffsets.c	\
dpathmgr.c dutil.c dreadonly.c dnotnc4.c dnotnc3.c dinfermodel.c	\
daux.c dinstance.c dcrc32.c dcrc32.h dcrc64.c ncexhash.c ncxcache.c	\
ncjson.c ds3util.c dparallel.c

# Add the utf8 codebase
libdispatch_la_SOURCES += utf8proc.c utf8proc.h

# The rc code is currently only used by libdap2 and libdap4,
# but in the future, it will be expanded to be used as the
# general .rc file for the netcdf-c library. So, always compile it.
libdispatch_la_SOURCES += drc.c 

# Add functions only found in netCDF-4.
# They are always defined, even if they just return an error
libdispatch_la_SOURCES += dgroup.c dvlen.c dcompound.c dtype.c denum.c	\
dopaque.c dfilter.c

# Add V2 API convenience library if needed.
if BUILD_V2
noinst_LTLIBRARIES += libnetcdf2.la
libnetcdf2_la_SOURCES = dv2i.c
libnetcdf2_la_CPPFLAGS = ${AM_CPPFLAGS} -DDLL_EXPORT
endif # BUILD_V2

if ENABLE_BYTERANGE
libdispatch_la_SOURCES += dhttp.c
endif # ENABLE_BYTERANGE

if ENABLE_S3_SDK
libdispatch_la_SOURCES += ncs3sdk.cpp awsincludes.h
AM_CXXFLAGS = -std=c++11
endif

if REGEDIT
libdispatch_la_SOURCES += dreg.c
endif

EXTRA_DIST=CMakeLists.txt ncsettings.hdr utf8proc_data.c

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

# Show what is needed to insert a new version of ezxml
# primary fix: The original ezxml.[ch] uses '//' comments;
# unpack and replace with '/*..*/'

REPO=https://downloads.sourceforge.net/project/ezxml/
EZXML=ezxml-0.8.6.tar.gz
ezxml::
	rm -fr ./ezxml ./ezxml.[ch] ./license.txt
	tar -zxf ./${EZXML}
	sed -e 's|//\(.*\)|/*\1*/|' <ezxml/ezxml.c >./ezxml.c
	sed -e 's|//\(.*\)|/*\1*/|' <ezxml/ezxml.h >./ezxml.h
	cp ezxml/license.txt .
