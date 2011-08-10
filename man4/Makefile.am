## This is a automake file, part of Unidata's netCDF package.
# Copyright 2005-2006, see the COPYRIGHT file for more information.
# This file builds the netcdf documentation.

# The "all" target does nothing in this directory. The "dist" target
# *does* build the documentation, because the distribution contains
# all of the built documentation.

# The "install" target installs the documentation in a directory
# called $(prefix)/doc/$(PACKAGE)-$(VERSION).

# These are the source files for all the netcdf manuals.
info_TEXINFOS = netcdf-f77.texi netcdf-f90.texi 

# Get lists of the coresponding ps, info, and pdf files.
html_docs = ${info_TEXINFOS:.texi=.html} 
html_dirs = ${info_TEXINFOS:.texi=} 
ps_docs = ${info_TEXINFOS:.texi=.ps} 
dvi_docs = ${info_TEXINFOS:.texi=.dvi} 
pdf_docs = ${info_TEXINFOS:.texi=.pdf}
info_docs= ${info_TEXINFOS:.texi=.info}
txt_docs= ${info_TEXINFOS:.texi=.txt}

# These are html versions of the man pages.
html_mans = ncdump-man-1.html ncgen-man-1.html netcdf_f77-man.html	\
netcdf-man-3.html netcdf_f90-man.html ncgen3-man-1.html \
nccopy-man-1.html

# These files will be included with the dist.
EXTRA_DIST = netcdf.m4 $(pdf_docs) $(html_mans) $(txt_docs)		\
$(dvi_docs) $(ps_docs) $(info_docs) $(html_docs) netcdf_f77.3		\
DoxygenLayout.xml Doxyfile.in @NETCDF_DOC_ONLY_FILES@ footer.html

#doc_DATA = $(pdf_docs) $(html_mans) $(txt_docs) $(ps_docs)	\
#$(info_docs) $(html_docs)

# All the documents depend on defines.texi for some common
# definitions.
netcdf_f77_TEXINFOS = defines.texi
netcdf_f90_TEXINFOS = defines.texi

# Get texi2dvi to shut the hell up, and work better for parallel
# builds. The --tidy option may be useful getting parallel builds to
# work someday.
TEXI2DVI = texi2dvi -s 

# Turn off parallel builds in this directory.
.NOTPARALLEL:

# To build documentation you must have M4 in their path.
M4=m4

# Rule to generate the text copies of the documentation.
.texi.txt: 
	makeinfo -o $*.txt --no-headers $(srcdir)/$<

# Generate the HTML version of the netcdf-3 C man page.
netcdf-man-3.html: $(top_srcdir)/libsrc/netcdf.3
	groff -man -Thtml $? > $@

# Generate the HTML version of the netcdf-3 F77 man page.
netcdf_f77.3: netcdf.m4
	m4 $(M4FLAGS) $(ARGS_MANPAGE) $? >$@ || rm $@

netcdf_f77-man.html: netcdf_f77.3
	groff -man -Thtml $? > $@

# Generate the HTML version of the netcdf-3 F90 man page.
netcdf_f90-man.html: $(top_srcdir)/f90/netcdf_f90.3
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

netcdf-f77.html:
	$(MAKEINFO) --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-f77.texi
	$(MAKEINFO) --no-split --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-f77.texi

netcdf-f90.html:
	$(MAKEINFO) --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-f90.texi
	$(MAKEINFO) --no-split --html $(AM_MAKEINFOHTMLFLAGS) $(MAKEINFOFLAGS) -I$(srcdir) netcdf-f90.texi

# This builds the docs from source, if necessary, and tars up
# everything needed for the website. Run this and copy the resulting
# tarball to the /contents/netcdf/docs directory to update the on-line
# docs.
web-tarball: $(pdf_docs) $(html_mans) $(txt_docs) $(dvi_docs) $(ps_docs) $(info_docs) $(html_docs)
	tar cf netcdf_info.tar *.info*
	gzip -f netcdf_info.tar
	zip netcdf_info *.info
	tar cf netcdf_pdf.tar *.pdf
	gzip -f netcdf_pdf.tar
	zip netcdf_pdf *.pdf
	tar cf netcdf_html.tar *.html $(html_dirs)
	gzip -f netcdf_html.tar
	zip netcdf_html *.html $(html_dirs)
	tar cf netcdf_txt.tar *.txt
	gzip -f netcdf_txt.tar
	zip netcdf_txt *.txt
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
	tar cf netcdf_docs.tar $(html_dirs) *.pdf *.txt *.html *.tar.gz *.zip 
	gzip -f netcdf_docs.tar

# Clean up all the junk we created!
MAINTAINERCLEANFILES = $(ps_docs) $(pdf_docs) $(txt_docs) $(info_docs)	\
$(html_docs) $(man_MANS) ${c_examples} ${f77_examples} ${f90_examples}	\
${cxx_examples} ${cdl_examples} *.1 *.3 ncdump-man-1.html		\
ncgen-man-1.html ncgen3-man-1.html netcdf_f77-man.html netcdf-man-3.html \
netcdf_f90-man.html stamp-* version* netcdf_f77.3 nccopy-man-1.html

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