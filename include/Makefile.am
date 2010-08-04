## This is a automake file, part of Unidata's netCDF package.
# Copyright 2010, see the COPYRIGHT file for more information.

# This automake file generates the Makefile to build the include
# directory.

include_HEADERS = netcdf.h 

if BUILD_PARALLEL
include_HEADERS += netcdf_par.h 
endif 

noinst_HEADERS = nc_logging.h nc_tests.h 


.PHONY: visualstudio
visualstudio:
	sh ${top_srcdir}/visualbuild add ${abs_srcdir} "${include_HEADERS} ${noinst_HEADERS}"

