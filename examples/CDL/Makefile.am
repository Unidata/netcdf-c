## This is a automake file, part of Unidata's netCDF package.

# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
# Ed Hartnett

# Note which tests depend on other tests. Necessary for make -j check.
TEST_EXTENSIONS = .sh

# All we do is run two shell scripts, which create the sample files
# and then compare them with the C versions of the examples.
TESTS = do_comps.sh

# Ship the scripts needed to create the sample files and compare them.
EXTRA_DIST = do_comps.sh simple_xy.cdl	\
sfc_pres_temp.cdl pres_temp_4D.cdl CMakeLists.txt

# Clean up files created during the process.
CLEANFILES = *.nc
