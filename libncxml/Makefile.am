# WARNING: this is not the libxml2 library
# It comes from https://github.com/recp/xml

# Copyright 2009, UCAR/Unidata
# See the COPYRIGHT file for more information.

# Use automake or CMake for building under nix
# Use CMake for building under windows

# Get AM_CPPFLAGS and AM_LDFLAGS

include $(top_srcdir)/lib_flags.am

if ENABLE_LIBXML2
AM_CPPFLAGS += ${XML2FLAGS}
endif

# This is our output. The ZARR convenience library.
noinst_LTLIBRARIES = libncxml.la
libncxml_la_LIBADD =
libncxml_la_LDFLAGS =

if ENABLE_LIBXML2
libncxml_la_SOURCES = ncxml_xml2.c
else
AM_CXXFLAGS = -std=c++11
libncxml_la_SOURCES = ncxml_tinyxml2.cpp tinyxml2.cpp tinyxml2.h
endif

EXTRA_DIST = CMakeLists.txt license.txt

# Download and massage the tinyxml2 source
REPO = https://github.com/leethomason/tinyxml2.git
tinyxml2::
	rm -fr ./tinyxml2 ./license.txt
	git clone --depth=1 ${REPO}
	cat tinyxml2/LICENSE.txt > ./license.txt
	cat tinyxml2/tinyxml2.h > ./tinyxml2.h
	sed -e 's/__BORLANDC__/__APPLE__/' < tinyxml2/tinyxml2.cpp \
	| sed -e 's/ptrdiff_t/long/g' > ./tinyxml2.cpp
	rm -fr tinyxml2
