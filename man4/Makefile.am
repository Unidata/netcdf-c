## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005-2006, see the COPYRIGHT file for more information.
# This file builds the netcdf documentation.

# The "all" target does nothing in this directory. The "dist" target
# *does* build the documentation, because the distribution contains
# all of the built documentation.

# The "install" target installs the documentation in a directory
# called $(prefix)/doc/$(PACKAGE)-$(VERSION).

# These are html versions of the man pages.
html_mans = ncdump-man-1.html ncgen-man-1.html netcdf-man-3.html	\
ncgen3-man-1.html nccopy-man-1.html

# These files will be included with the dist.
EXTRA_DIST = netcdf.m4 DoxygenLayout.xml Doxyfile.in	\
@NETCDF_DOC_ONLY_FILES@ footer.html

# Turn off parallel builds in this directory.
.NOTPARALLEL:

# To build documentation you must have M4 in their path.
M4=m4

# Generate the HTML version of the netcdf-3 C man page.
netcdf-man-3.html: $(top_srcdir)/libsrc/netcdf.3
	groff -man -Thtml $? > $@

# Generate the HTML version of the ncgen man page.
ncgen-man-1.html: $(top_srcdir)/ncgen/ncgen.1
	groff -man -Thtml $? > $@

# Generate the HTML version of the ncgen3 man page.
ncgen3-man-1.html: $(top_srcdir)/ncgen3/ncgen3.1
	groff -man -Thtml $? > $@

# Generate the HTML version of the ncdump man page.
ncdump-man-1.html: $(top_srcdir)/ncdump/ncdump.1
	groff -man -Thtml $? > $@

# Generate the HTML version of the nccopy man page.
nccopy-man-1.html: $(top_srcdir)/ncdump/nccopy.1
	groff -man -Thtml $? > $@

# This builds the docs from source, if necessary, and tars up
# everything needed for the website. Run this and copy the resulting
# tarball to the /contents/netcdf/docs directory to update the on-line
# docs.
web-tarball: 
	cp $(top_srcdir)/ncdump/ncdump.1 .
	cp $(top_srcdir)/ncgen/ncgen.1 .
	cp $(top_srcdir)/ncdump/nccopy.1 .
	cp $(top_srcdir)/ncgen3/ncgen3.1 .
	cp $(top_srcdir)/libsrc/netcdf.3 .
	cp $(top_srcdir)/fortran/netcdf_f77.3 .
	cp $(top_srcdir)/f90/netcdf_f90.3 .
	tar cf netcdf_man.tar netcdf.3 netcdf_f77.3 netcdf_f90.3 ncdump.1 ncgen.1 ncgen3.1
	gzip -f netcdf_man.tar
	zip netcdf_man netcdf.3 netcdf_f77.3 netcdf_f90.3 ncdump.1 ncgen.1 ncgen3.1

# Clean up all the junk we created!
MAINTAINERCLEANFILES = $(man_MANS) ${c_examples} ${cdl_examples} *.1	\
*.3 ncdump-man-1.html ncgen-man-1.html ncgen3-man-1.html		\
netcdf-man-3.html stamp-* version* nccopy-man-1.html

SUBDIRS = images

if HAVE_DOXYGEN
#directory = $(top_srcdir)/docs/man/man3/
#dist_man_MANS = $(man4directory)/man_page_1.3 $(directory)/man_page_2.3
# $(directory)/man_page_1.3: doxyfile.stamp
# $(directory)/man_page_2.3: doxyfile.stamp

doxyfile.stamp: @NETCDF_DOC_CODE_FILES@ #@NETCDF_DOC_ONLY_FILES@ 
	$(DOXYGEN) Doxyfile
	echo Timestamp > doxyfile.stamp

CLEANFILES = doxyfile.stamp html latex

all-local: doxyfile.stamp
clean-local:
	rm -rf $(top_builddir)/man4/man
	rm -rf $(top_builddir)/man4/html
	rm -rf $(top_builddir)/man4/latex
endif 