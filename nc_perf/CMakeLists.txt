# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

# bm_file and the three tst_ar4* programs take short options. They used to
# pick getopt() up from <unistd.h>, which MSVC does not have; nc_perf_compat.h
# includes the bundled XGetopt there instead, and -- as in ncdump, ncgen and
# nczarr_test -- the target has to compile it.
if(USE_X_GETOPT)
  set(XGETOPTSRC "${CMAKE_CURRENT_SOURCE_DIR}/../libdispatch/XGetopt.c")
endif()

build_bin_test(bm_file tst_utils.c ${XGETOPTSRC})
build_bin_test(tst_chunks3 tst_utils.c)
build_bin_test(tst_ar4 tst_utils.c ${XGETOPTSRC})
build_bin_test(tst_ar4_4d tst_utils.c ${XGETOPTSRC})
build_bin_test(bm_many_objs tst_utils.c)
build_bin_test(tst_h_many_atts tst_utils.c)
build_bin_test(bm_many_atts tst_utils.c)
build_bin_test(tst_files2 tst_utils.c)
build_bin_test(tst_knmi tst_utils.c)
build_bin_test(bm_netcdf4_recs tst_utils.c)
build_bin_test(bigmeta tst_utils.c)
build_bin_test(openbigmeta tst_utils.c)

add_bin_test(nc_perf tst_ar4_3d tst_utils.c ${XGETOPTSRC})
add_bin_test(nc_perf tst_create_files tst_utils.c)
add_bin_test(nc_perf tst_files3 tst_utils.c)
add_bin_test(nc_perf tst_mem tst_utils.c)
add_bin_test(nc_perf tst_wrf_reads tst_utils.c)
add_bin_test(nc_perf tst_attsperf tst_utils.c)
add_bin_test(nc_perf tst_bm_rando tst_utils.c)
add_bin_test(nc_perf tst_compress tst_utils.c)

#add_sh_test(nc_perf run_knmi_bm)
add_sh_test(nc_perf perftest)
add_sh_test(nc_perf run_tst_chunks)
add_sh_test(nc_perf run_bm_elena)

IF(NETCDF_BUILD_UTILITIES)
add_sh_test(nc_perf run_bm_test1)
add_sh_test(nc_perf run_bm_test2)

# This will run a parallel I/O benchmark for parallel builds.
IF(TEST_PARALLEL4)
add_bin_test(nc_perf tst_compress_par tst_utils.c)
add_sh_test(nc_perf run_par_bm_test)
ENDIF(TEST_PARALLEL4)
ENDIF(NETCDF_BUILD_UTILITIES)

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

# nc_perf_tst_create_files writes tst_<type>2_<n>D.nc, tst_elena_int_3D.nc and
# tst_simple.nc into the build directory, and these three shell tests read
# exactly those files. Nothing declared the dependency, so under `ctest -j` a
# reader was free to run while the writer was still writing -- bm_file would
# reach a file mid-rewrite and return the netCDF error silently (its
# `return ret` prints nothing), which is why the log ended mid-table with no
# message. `if(TEST ...)` keeps this correct when NETCDF_BUILD_UTILITIES is off
# and the run_bm_* tests do not exist.
foreach(_nc_perf_reader nc_perf_run_bm_test1 nc_perf_run_bm_test2 nc_perf_run_bm_elena)
  if(TEST ${_nc_perf_reader})
    set_tests_properties(${_nc_perf_reader} PROPERTIES DEPENDS nc_perf_tst_create_files)
  endif()
endforeach()

