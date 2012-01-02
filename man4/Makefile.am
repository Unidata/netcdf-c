## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005-2011, see the COPYRIGHT file for more information.
# This file builds the netcdf documentation.

# These files will be included with the dist.
EXTRA_DIST = netcdf.m4 DoxygenLayout.xml Doxyfile.in	\
@NETCDF_DOC_ONLY_FILES@ footer.html 

# Turn off parallel builds in this directory.
.NOTPARALLEL:

# To build documentation you must have M4 in their path.
M4=m4

SUBDIRS = images

if BUILD_DOCS

# Copy man pages.
#directory = $(top_srcdir)/docs/man/man3/
#dist_man_MANS = $(man4directory)/man_page_1.3 $(directory)/man_page_2.3
# $(directory)/man_page_1.3: doxyfile.stamp
# $(directory)/man_page_2.3: doxyfile.stamp

# Timestamp to prevent rebuilds.
doxyfile.stamp: 
	$(DOXYGEN) Doxyfile
	echo Timestamp > doxyfile.stamp

CLEANFILES = doxyfile.stamp html latex man

all-local: doxyfile.stamp
clean-local:
	rm -rf $(top_builddir)/man4/man
	rm -rf $(top_builddir)/man4/html
	rm -rf $(top_builddir)/man4/latex

# This builds the docs from source, if necessary, and tars up
# everything needed for the website. Run this and copy the resulting
# tarball to the /contents/netcdf/docs directory to update the on-line
# docs.
web-tarball: doxyfile.stamp
	cd html; tar cf ../netcdf_docs.tar *
	gzip -f netcdf_docs.tar

endif 
