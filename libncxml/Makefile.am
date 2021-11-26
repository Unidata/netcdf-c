# WARNING: this is not the libxml2 library
# It comes from https://github.com/recp/xml

# Copyright 2009, UCAR/Unidata
# See the COPYRIGHT file for more information.

# Use automake or CMake for building under nix
# Use CMake for building under windows

# Get AM_CPPFLAGS and AM_LDFLAGS

include $(top_srcdir)/lib_flags.am

if HAVE_LIBXML2
AM_CPPFLAGS += ${XML2FLAGS}
endif

# This is our output. The ZARR convenience library.
noinst_LTLIBRARIES = libncxml.la
libncxml_la_LIBADD =
libncxml_la_LDFLAGS =

if HAVE_LIBXML2
libncxml_la_SOURCES = ncxml_xml2.c
else
libncxml_la_SOURCES = ncxml_ezxml.c ezxml.c ezxml.h
endif

EXTRA_DIST = CMakeLists.txt license.txt

# Construct ezxml from latest sources
REPO=https://downloads.sourceforge.net/project/ezxml/
EZXML=ezxml-0.8.6.tar.gz
makelib::
	echo "WARNING DO NOT RUN THIS since the patches are not in the tar file"
	exit 1
	rm -fr ./ezxml.[ch] ./license.txt ./ezxml ./ezxml.c.? ./ezxml.h.?
	tar -zxf ${EZXML}
	cat ezxml/ezxml.h > ./ezxml.h
	sed -i.1 -e '/ezxml_parse_fd/d' ezxml.h
	sed -i.2 -e '/ezxml_parse_file/d' ezxml.h
	sed -i.3 -e '/ezxml_parse_fp/d' ezxml.h
	sed -i.4 -e 's|//\(.*\)|/*\1*/|' ezxml.h
	echo "#define EZXML_NOMMAP 1" > ./ezxml.c
	cat ezxml/ezxml.c >> ./ezxml.c
	sed -i.1 -e '/<unistd.h>/d' ezxml.c
	sed -i.2 -e '/ezxml_parse_fp(FILE/i#if 0' ./ezxml.c
	sed -i.3 -e '/ezxml_ampencode(const/i#endif //0' ./ezxml.c
	sed -i.4 -e 's|//\(.*\)|/*\1*/|' ezxml.c
	cp ezxml/license.txt .
	rm -fr ezxml
	rm -f ezxml.c.? ezxml.h.?

# Define path to the xml github dir; this value assumes it is in a parallel directory to netcdf-c (YMMV)
GITSRC=${top_srcdir}/../tinyxml2
xmakelib::
	rm -f readme.md LICENSE.txt
	rm -fr ./tinyxml2.cpp ./tinyxml2.h
	cp ${GITSRC}/readme.md .
	cp ${GITSRC}/LICENSE.txt .
	cp ${GITSRC}/tinyxml2.h ${GITSRC}/tinyxml2.cpp .
