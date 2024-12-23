## This is a automake file, part of Unidata's netCDF package.
# Copyright 2018, see the COPYRIGHT file for more information.

# This automake file generates the Makefile to build the include
# directory.

# Ed Hartnett, Dennis Heimbigner, Ward Fisher

# Installed headers

include_HEADERS = netcdf.h netcdf_meta.h netcdf_mem.h netcdf_aux.h	\
netcdf_filter.h netcdf_filter_build.h netcdf_filter_hdf5_build.h 	\
netcdf_dispatch.h netcdf_vutils.h

include_HEADERS += netcdf_json.h netcdf_proplist.h

if BUILD_PARALLEL
include_HEADERS += netcdf_par.h
endif

noinst_HEADERS = nc_logging.h nc_tests.h fbits.h nc.h nclist.h		\
ncuri.h ncutf8.h ncdispatch.h ncdimscale.h netcdf_f.h err_macros.h	\
ncbytes.h nchashmap.h ceconstraints.h rnd.h nclog.h ncconfigure.h	\
nc4internal.h nctime.h nc3internal.h onstack.h ncrc.h ncauth.h		\
ncoffsets.h nctestserver.h nc4dispatch.h nc3dispatch.h ncexternl.h	\
ncpathmgr.h ncindex.h hdf4dispatch.h hdf5internal.h nc_provenance.h	\
hdf5dispatch.h ncmodel.h isnan.h nccrc.h ncexhash.h ncxcache.h          \
ncjson.h ncxml.h ncs3sdk.h ncproplist.h ncplugins.h

if USE_DAP
noinst_HEADERS += ncdap.h
endif

if NETCDF_ENABLE_BYTERANGE
noinst_HEADERS += nchttp.h
endif

EXTRA_DIST = CMakeLists.txt XGetopt.h netcdf_meta.h.in netcdf_dispatch.h.in

MAINTAINERCLEANFILES = netcdf_json.h netcdf_proplist.h

BUILT_SOURCES = netcdf_json.h netcdf_proplist.h

# netcdf_json.h is constructed as a header-only file for use by
# nczarr code wrappers in the plugin directory. It is
# constructed by joining libdispatch/ncjson.c with
# include/ncjson.h.  Additionally all the entry points are
# marked with a macro (OPTSTATIC) that marks the entry point as
# static inside netcdf_json.h.	This is an ugly hack to avoid
# having to reference libnetcdf in the nczarr code wrappers.
# Note that the file is built in builddir in case the build
# is out of source.

# Give the recipe for building netcdf_json.h
netcdf_json.h: ${top_srcdir}/libdispatch/ncjson.c ${top_srcdir}/include/ncjson.h
	rm -fr ${builddir}/netcdf_json.h
	cat ${srcdir}/ncjson.h \
	    | sed -e '/!NCJSON_H/d' \
    	    | sed -e 's/NCJSON_H/NETCDF_JSON_H/' \
	    >> ${builddir}/netcdf_json.h
	cat ${srcdir}/../libdispatch/ncjson.c | sed -e '/ncjson.h/d' >> ${builddir}/netcdf_json.h
	echo '#endif /*NETCDF_JSON_H*/' >> ${builddir}/netcdf_json.h

# netcdf_proplist is analogous to netcdf_json but, of course, using libdispatch/ncproplist.c and include/ncproplist.h
# Give the recipe for building netcdf_proplist.h. Basically same as for netcdf_json.h
netcdf_proplist.h: ${top_srcdir}/libdispatch/ncproplist.c ${top_srcdir}/include/ncproplist.h
	rm -fr ${builddir}/netcdf_proplist.h
	cat ${srcdir}/ncproplist.h \
	    | sed -e '/!NCPROPLIST_H/d' \
    	    | sed -e 's/NCPROPLIST_H/NETCDF_PROPLIST_H/' \
	    >> ${builddir}/netcdf_proplist.h
	cat ${srcdir}/../libdispatch/ncproplist.c | sed -e '/ncproplist.h/d' >> ${builddir}/netcdf_proplist.h
	echo '#endif /*NETCDF_PROPLIST_H*/' >> ${builddir}/netcdf_proplist.h
