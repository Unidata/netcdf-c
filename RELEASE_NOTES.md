Release Notes       {#RELEASE_NOTES}
=============

\brief Release notes file for the netcdf-c package.

This file contains a high-level description of this package's evolution. Releases are in reverse chronological order (most recent first). Note that, as of netcdf 4.2, the `netcdf-c++` and `netcdf-fortran` libraries have been separated into their own libraries.

## 4.9.1 - T.B.D.

* [Bug Fix] Check for libxml2 header file in support of [Github #2410](https://github.com/Unidata/netcdf-c/issues/2410)
* [Enhancement] Provide a simple API to allow user access to the internal .rc file table: supports get/set/overwrite of entries of the form "key=value". See [Github #2408](https://github.com/Unidata/netcdf-c/pull/2408).
* [Bug Fix] Use env variable USERPROFILE instead of HOME for windows and mingw. See [Github #2405](https://github.com/Unidata/netcdf-c/pull/2405).
* [Bug Fix] Fix the nc_def_var_fletcher32 code in hdf5 to properly test value of the fletcher32 argument. See [Github #2403](https://github.com/Unidata/netcdf-c/pull/2403).

## 4.9.0 - June 10, 2022

* [Enhancement] Improve filter installation process to avoid use of an extra shell script. See [Github #2348](https://github.com/Unidata/netcdf-c/pull/2348).
* [Bug Fix] Get "make distcheck" to work See [Github #2343](https://github.com/Unidata/netcdf-c/pull/2343).
* [Enhancement] Allow the read/write of JSON-valued Zarr attributes to allow
for domain specific info such as used by GDAL/Zarr. See [Github #2278](https://github.com/Unidata/netcdf-c/pull/2278).
* [Enhancement] Turn on the XArray convention for NCZarr files by default. WARNING, this means that the mode should explicitly specify nczarr" or "zarr" even if "xarray" or "noxarray" is specified. See [Github #2257](https://github.com/Unidata/netcdf-c/pull/2257).
* [Enhancement] Update the documentation to match the current filter capabilities  See [Github #2249](https://github.com/Unidata/netcdf-c/pull/2249).
* [Enhancement] Support installation of pre-built standard filters into user-specified location. See [Github #2318](https://github.com/Unidata/netcdf-c/pull/2318).
* [Enhancement] Improve filter support. More specifically (1) add nc_inq_filter_avail to check if a filter is available, (2) add the notion of standard filters, (3) cleanup szip support to fix interaction with NCZarr. See [Github #2245](https://github.com/Unidata/netcdf-c/pull/2245).
* [Enhancement] Switch to tinyxml2 as the default xml parser implementation. See [Github #2170](https://github.com/Unidata/netcdf-c/pull/2170).
* [Bug Fix] Require that the type of the variable in nc_def_var_filter is not variable length. See [Github #/2231](https://github.com/Unidata/netcdf-c/pull/2231).
* [File Change] Apply HDF5 v1.8 format compatibility when writing to previous files, as well as when creating new files.  The superblock version remains at 2 for newly created files.  Full backward read/write compatibility for netCDF-4 is maintained in all cases.  See [Github #2176](https://github.com/Unidata/netcdf-c/issues/2176).
* [Enhancement] Add ability to set dataset alignment for netcdf-4/HDF5 files. See [Github #2206](https://github.com/Unidata/netcdf-c/pull/2206).
* [Bug Fix] Improve UTF8 support on windows so that it can use utf8 natively. See [Github #2222](https://github.com/Unidata/netcdf-c/pull/2222).
* [Enhancement] Add complete bitgroom support to NCZarr. See [Github #2197](https://github.com/Unidata/netcdf-c/pull/2197).
* [Bug Fix] Clean up the handling of deeply nested VLEN types. Marks nc_free_vlen() and nc_free_string as deprecated in favor of ncaux_reclaim_data(). See [Github #2179](https://github.com/Unidata/netcdf-c/pull/2179).
* [Bug Fix] Make sure that netcdf.h accurately defines the flags in the open/create mode flags. See [Github #2183](https://github.com/Unidata/netcdf-c/pull/2183).
* [Enhancement] Improve support for msys2+mingw platform. See [Github #2171](https://github.com/Unidata/netcdf-c/pull/2171).
* [Bug Fix] Clean up the various inter-test dependencies in ncdump for CMake. See [Github #2168](https://github.com/Unidata/netcdf-c/pull/2168).
* [Bug Fix] Fix use of non-aws appliances. See [Github #2152](https://github.com/Unidata/netcdf-c/pull/2152).
* [Enhancement] Added options to suppress the new behavior from [Github #2135](https://github.com/Unidata/netcdf-c/pull/2135).  The options for `cmake` and `configure` are, respectively `-DENABLE_LIBXML2` and `--(enable/disable)-libxml2`. Both of these options defaul to 'on/enabled'.  When disabled, the bundled `ezxml` XML interpreter is used regardless of whether `libxml2` is present on the system.
* [Enhancement] Support optional use of libxml2, otherwise default to ezxml. See [Github #2135](https://github.com/Unidata/netcdf-c/pull/2135) -- H/T to [Egbert Eich](https://github.com/e4t).
* [Bug Fix] Fix several os related errors. See [Github #2138](https://github.com/Unidata/netcdf-c/pull/2138).
* [Enhancement] Support byte-range reading of netcdf-3 files stored in private buckets in S3. See [Github #2134](https://github.com/Unidata/netcdf-c/pull/2134)
* [Enhancement] Support Amazon S3 access for NCZarr. Also support use of the existing Amazon SDK credentials system. See [Github #2114](https://github.com/Unidata/netcdf-c/pull/2114)
* [Bug Fix] Fix string allocation error in H5FDhttp.c. See [Github #2127](https://github.com/Unidata/netcdf-c/pull/2127).
* [Bug Fix] Apply patches for ezxml and for selected  oss-fuzz detected errors. See [Github #2125](https://github.com/Unidata/netcdf-c/pull/2125).
* [Bug Fix] Ensure that internal Fortran APIs are always defined. See [Github #2098](https://github.com/Unidata/netcdf-c/pull/2098).
* [Enhancement] Support filters for NCZarr. See [Github #2101](https://github.com/Unidata/netcdf-c/pull/2101)
* [Bug Fix] Make PR 2075 long file name be idempotent. See [Github #2094](https://github.com/Unidata/netcdf-c/pull/2094).
## What's Changed
* hashmap for quick var and dim retrieval -- nc3 files by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/238
* Nc3 hashmap fix by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/246
* Remove dispatch substrate mechanism by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/248
* Add provenance info for netcdf-4 files. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/260
* Finally get around to removing all that obsolete pnetcdf related code in libsrc4. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/261
* Use SOURCE_DATE_EPOCH instead of current date. by @sebastic in https://github.com/Unidata/netcdf-c/pull/263
* re: github issue https://github.com/Unidata/netcdf-c/issues/265 by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/266
* Check for valid MPI_Comm before freeing by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/268
* re: https://github.com/Unidata/netcdf-c/issues/269 by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/270
* HDF5 generates unnecessary error messages when netcdf4 logging enabled by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/272
* The name hash for hdf4 variables was not being computed. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/273
* Make ncdump/nccopy return a better error when given a bad constraint in a dap url by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/279
* Eliminate c++ or c99-style comment by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/274
* mutiple typedefs causing compile problems by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/287
* Fix variable and dimension renaming for nc3 with hashmap by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/286
* This patch changes the algorithm for determining the extended size of a dataset... by @brtnfld in https://github.com/Unidata/netcdf-c/pull/278
* Fix att_name size by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/288
* Fix incorrect test by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/289
* Test all three files for compatibility by @tbeu in https://github.com/Unidata/netcdf-c/pull/294
* Use correct symbol by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/293
* Some small fixes for nc-config by @ZedThree in https://github.com/Unidata/netcdf-c/pull/305
* Fix for Github issue netcdf-c 300 by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/301
* Usage function called wrong in nc-config by @ZedThree in https://github.com/Unidata/netcdf-c/pull/306
* Fix invalid array access by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/309
* Fix HDF5 include directory with CMake >= 3.6.0. by @sebastic in https://github.com/Unidata/netcdf-c/pull/311
* Fix spelling errors. by @sebastic in https://github.com/Unidata/netcdf-c/pull/312
* Fix for Github issue 314. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/315
* Use an hdf5-api function that eliminates code by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/290
* Replace linked list with array for var storage in netcdf-4 format by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/328
* Add missing #include "err_macros.h" to tst_h_par.c by @opoplawski in https://github.com/Unidata/netcdf-c/pull/333
* Eliminate an MPI_Allreduce in many cases by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/338
* Modified ncdap_test to allow a different method to get TOPSRCDIR by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/348
* Fix aliasing/type-pun problem in ncdump by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/353
* Cmake parallel fix by @WardF in https://github.com/Unidata/netcdf-c/pull/356
* Use HDF5 collective metadata operations if available. by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/335
* Ncdump fails under visual studio for certain dap urls by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/366
* HDF4-visual studio interference by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/370
* Clean up dap2 code to remove now obsolete functionality by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/367
* Resolve license issue with the utf8proc code. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/364
* Avoiding conditional directives that break statements by @RomeroMalaquias in https://github.com/Unidata/netcdf-c/pull/346
* bug fix: record size alignment for NC_USHORT is missing by @wkliao in https://github.com/Unidata/netcdf-c/pull/331
* Use H5Aiterate2 in read_var. by @brtnfld in https://github.com/Unidata/netcdf-c/pull/276
* Suppress selected orphan opendap attributes by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/376
* Fix bug in ncgen handling of global, enum valued, attribute. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/378
* Switch NC_CHAR on netCDF4 to use ASCII by @shoyer in https://github.com/Unidata/netcdf-c/pull/316
* Ncgen treats an integer with just a U/u suffix as uint64 instead of uint32. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/386
* patch for issue 384 - NC_ELATEFILL error for NetCDF-4 files by @wkliao in https://github.com/Unidata/netcdf-c/pull/387
* patch for #388 - setting global _FillValue by @wkliao in https://github.com/Unidata/netcdf-c/pull/389
* Set default fill/no_fill behavior on newly defined variable (nc4) by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/336
* Primary change: add dap4 support by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/374
* Fix NC_DISKLESS returns garbage data for certain files by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/403
* fix nc_open_mem by @thehesiod in https://github.com/Unidata/netcdf-c/pull/400
* check the number of legal large variables at open time by @wkliao in https://github.com/Unidata/netcdf-c/pull/393
* Use H5free_memory instead of free by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/411
* Remove X_INT64_MIN, X_INT64_MAX, and X_UINT64_MAX from netcdf.h by @wkliao in https://github.com/Unidata/netcdf-c/pull/414
* Fix memory leak in NC4_put_propattr() by @rouault in https://github.com/Unidata/netcdf-c/pull/415
* struct stat.st_size is of type off_t by @wkliao in https://github.com/Unidata/netcdf-c/pull/422
* Remove more old utf8 code by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/430
* Fix undefined left shift in ncx_get_size_t() by @rouault in https://github.com/Unidata/netcdf-c/pull/427
* Fix case error when opendap keyword is used as an identifier. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/434
* Update install.md by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/409
* Add additional path name conversion checks by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/439
* Temporarily suppress <MAP> vars referring to struct fields by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/406
* Some temporary files are being left in a tempdir by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/436
* Reclaim DAP temporary files by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/451
* Fix stack buffer overflow in nc4_check_name() by @rouault in https://github.com/Unidata/netcdf-c/pull/450
* close files in tst_formatx_pnetcdf.c by @wkliao in https://github.com/Unidata/netcdf-c/pull/455
* Fixed computation of recsize for NC_64BIT_DATA files, fixes #453 by @pastewka in https://github.com/Unidata/netcdf-c/pull/457
* fixed table and link in Large File section by @edhartnett in https://github.com/Unidata/netcdf-c/pull/459
* Upgrade authentication documentation by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/467
* avoid calling NC3_inq_var_fill when ERANGE_FILL is enabled by @wkliao in https://github.com/Unidata/netcdf-c/pull/464
* Fix a bunch of references, and some formatting, in the doxygen C guide by @edhartnett in https://github.com/Unidata/netcdf-c/pull/465
* Fix (I think) the --has-dap and --has-dap4 output by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/473
* Fix bad output from nc-config by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/476
* Important fix to d4rc.c by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/482
* Wrap version specific curl flags. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/488
* Fix bug in ocrc.c#combinecredentials by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/472
* Added detail about _FillValue being undefined. by @pelson in https://github.com/Unidata/netcdf-c/pull/470
* Update dispatch table documentation by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/497
* Fix problem with hangs in parallel collective output by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/449
* Fix dap4 related warnings in https://github.com/Unidata/netcdf-c/issues/501. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/503
* fixed memory problem in tst_h_dimscales by @edhartnett in https://github.com/Unidata/netcdf-c/pull/511
* fixed compile warning in tst_h_files.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/519
* fix for tst_vars.c memory overflow by @edhartnett in https://github.com/Unidata/netcdf-c/pull/513
* fixed most compile warnings in nc_test4 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/521
* fixed memory leaks and warnings in nc_test4/tst_empty_vlen_unlim.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/522
* fixed warnings in libsrc4/nc4hdf.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/527
* fixed compile warnings in nc4info.c/nc4var.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/529
* fixed warnings in nc_test/tst_utf8_validate.c and nc_test/tst_atts3,c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/533
* fixed more warnings in nc_test by @edhartnett in https://github.com/Unidata/netcdf-c/pull/536
* fixed memory leak in nc_open when cmode includes both NC_MPIIO and NC_MPIPOSIX by @edhartnett in https://github.com/Unidata/netcdf-c/pull/516
* fixed warning in libsrc/posixio.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/538
* moved tests from tst_h_atts2 into tst_xplatform2, fixed warnings and some memory issues by @edhartnett in https://github.com/Unidata/netcdf-c/pull/544
* Fix memory issues in nc_open and some tests by @edhartnett in https://github.com/Unidata/netcdf-c/pull/547
* fixed memory error in handling of (ignored) reference atts in HDF5 file by @edhartnett in https://github.com/Unidata/netcdf-c/pull/550
* fixed memory error in tst_fills2.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/551
* Better handling of --enable-pnetcdf and --enable-parallel4 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/548
* fix for leaks in nc_create as seen in nc_test4/tst_files by @edhartnett in https://github.com/Unidata/netcdf-c/pull/557
* typedef schar to signed char by @wkliao in https://github.com/Unidata/netcdf-c/pull/419
* added error code message for NC_EMPI by @edhartnett in https://github.com/Unidata/netcdf-c/pull/566
* Eliminate NC_MAX_VARS|DIMS|ATTRS limits by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/443
* check against NC_MAX_VAR_DIMS by @wkliao in https://github.com/Unidata/netcdf-c/pull/490
* Fix url parameter handling for DAP2 code by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/570
* Fix dap legal name character handling by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/573
* Modify nc_test/tst_parallel2.c to remove a failure. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/545
* Fix handling of corrupt files + proper offset handling for hdf5 files by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/552
* Refactor common code between DAP2(oc2) and DAP4 by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/477
* Fix broken link to NCO within FAQ by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/587
* removed archaic code to check memory use by @edhartnett in https://github.com/Unidata/netcdf-c/pull/571
* documentation for nc_par_var_access(), nc_inq_libvers() and nc_open_par() by @edhartnett in https://github.com/Unidata/netcdf-c/pull/572
* added missing documentation nc_def_var_deflate(), nc_def_var_fletcher32(), nc_def_var_fill(), nc_def_var_endian() in dvars.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/576
* fixed some warnings and incorrect assert by @edhartnett in https://github.com/Unidata/netcdf-c/pull/583
* Fix some tests use of ENABLE_CDF5 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/591
* refurbished tst_cdf5format.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/594
* fixed warnings in tst_interops5.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/595
* BUGFIX: fixed tst_enums memory issue when nc_close() does not succeed on first attempt by @edhartnett in https://github.com/Unidata/netcdf-c/pull/559
* removed Makefile.am mention of historic file no longer used: nc_test/ncx.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/561
* DOCUMENTATION: added v2 documentation by @edhartnett in https://github.com/Unidata/netcdf-c/pull/579
* BUGFIX: Fixes broken --disable-utilities by @edhartnett in https://github.com/Unidata/netcdf-c/pull/586
* fixed file name in test which was breaking make -j check in h5_test by @edhartnett in https://github.com/Unidata/netcdf-c/pull/601
* took out most uses of EXTRA_TESTS, returned it to disabled by default by @edhartnett in https://github.com/Unidata/netcdf-c/pull/609
* Finalize the compression support. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/399
* BUGFIX: Fixes 3 memory leaks, in tst_filterparser.c, nc4internal.c, and dfilter.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/649
* Ejh all documentation PRs combined by @edhartnett in https://github.com/Unidata/netcdf-c/pull/652
* DOCUMENTATION: add docs for nc_create_par() and other functions by @edhartnett in https://github.com/Unidata/netcdf-c/pull/599
* DOCUMENTATION: fixed typos in CMake build documentation by @edhartnett in https://github.com/Unidata/netcdf-c/pull/604
* DOCUMENTATION: note of memory issues in nc_inq_var_fill() by @edhartnett in https://github.com/Unidata/netcdf-c/pull/637
* DOCUMENTATION: fixed doxygen warnings nc_get_att() and related functions in dattget.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/640
* set DOXYGEN_SERVER_BASED_SEARCH in configure.ac by @edhartnett in https://github.com/Unidata/netcdf-c/pull/641
* DOXUMENTATION: fixed doxygen warnings by adding docs in dfile.c, dparallel.c, and dvarget.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/646
* DOCUMENTATION: fixed doxygen warnings in dattinq.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/651
* fixed commented out tests in nc_test4/tst_vars2.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/578
* removed unused files by @edhartnett in https://github.com/Unidata/netcdf-c/pull/600
* FIxes for all remaining easy warnings for make all by @edhartnett in https://github.com/Unidata/netcdf-c/pull/611
* noted dependancy between tests, allowing make check to work in parallel in nctest by @edhartnett in https://github.com/Unidata/netcdf-c/pull/615
* fixed names of output files to enable parallel make check in nc_test4 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/618
* turned on some commented out test code by @edhartnett in https://github.com/Unidata/netcdf-c/pull/626
* BUILD BUGFIX: Remove use of BUILT_SOURCES in ncdump directory... by @edhartnett in https://github.com/Unidata/netcdf-c/pull/633
* now nctest also tests CDF5 if it is built by @edhartnett in https://github.com/Unidata/netcdf-c/pull/636
* BUGFIX: fixed pnetcdf issues by @edhartnett in https://github.com/Unidata/netcdf-c/pull/650
* Combined PRs from Ed, except one... by @edhartnett in https://github.com/Unidata/netcdf-c/pull/653
* fix codes might break strict-aliasing rules by @wkliao in https://github.com/Unidata/netcdf-c/pull/654
* Address strict null byte padding by @WardF in https://github.com/Unidata/netcdf-c/pull/675
* Autotools build system tweaks and fixes to get make -j check and make -j distcheck working by @edhartnett in https://github.com/Unidata/netcdf-c/pull/665
* Begin changing over to use strlcat instead of strncat by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/683
* Code changes to make libnetcdf compile with vs2008 by @nehaljwani in https://github.com/Unidata/netcdf-c/pull/691
* improve parallel builds, fixed SEGFAULT for NULL parameter for nc_inq_format(), added testing, documentation by @edhartnett in https://github.com/Unidata/netcdf-c/pull/680
* parallelize cmake tests similar to work done with autotools tests by @WardF in https://github.com/Unidata/netcdf-c/pull/715
* Updating with additional parallel test fenceposts by @WardF in https://github.com/Unidata/netcdf-c/pull/717
* more tests, more internal documentation, minor bug fixes by @edhartnett in https://github.com/Unidata/netcdf-c/pull/711
* Corrected links to cdash dashboard. by @WardF in https://github.com/Unidata/netcdf-c/pull/728
* Fix buffer overrun in tabs[MAX_NESTS] by adding space for \0 terminator by @mingwandroid in https://github.com/Unidata/netcdf-c/pull/729
* Make certain tests conditional on dependencies by @nehaljwani in https://github.com/Unidata/netcdf-c/pull/692
* Fixes of warnings, check return values, added guards for headers that needed them, remove unneeded HDF4 checks by @edhartnett in https://github.com/Unidata/netcdf-c/pull/773
* fix vpath build for regenerating c files from m4 files by @wkliao in https://github.com/Unidata/netcdf-c/pull/466
* Allow multiple vars with different chunks on the same unlimited dim by @Lnaden in https://github.com/Unidata/netcdf-c/pull/760
* Eliminate potential buffer overflow by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/764
* Eliminate warning from clang compilers. by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/765
* Make it clear that limits are not enforced by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/766
* Fix plugin building for filter tests and add filter example by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/768
* Combination of 9 outstanding non-controversial PRs by @edhartnett in https://github.com/Unidata/netcdf-c/pull/789
* fix tst_converts.c when enable-erange-fill is set by @wkliao in https://github.com/Unidata/netcdf-c/pull/790
* Recent refactoring removed uses of variables by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/802
* Incorrect reference to byteswap8 in libdispatch/dfilter.c by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/807
* Gh809.wif by @WardF in https://github.com/Unidata/netcdf-c/pull/812
* Ensure NC_HAS_META_H is defined by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/809
* Avoid cmake CMP0002 by using different logical name for plugin libs by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/813
* Gsjaardema batch merge by @WardF in https://github.com/Unidata/netcdf-c/pull/833
* Fix compression level settings by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/827
* a clean commit for #383 by @wkliao in https://github.com/Unidata/netcdf-c/pull/731
* CDF-5 fix: let NC_var.len be the true size of variable by @wkliao in https://github.com/Unidata/netcdf-c/pull/478
* Batch EJH Pull requests by @WardF in https://github.com/Unidata/netcdf-c/pull/844
* fix dim bad paramter issues by @edhartnett in https://github.com/Unidata/netcdf-c/pull/817
* doc changes only by @edhartnett in https://github.com/Unidata/netcdf-c/pull/820
* extra tests, plus fill value fixes for string type by @edhartnett in https://github.com/Unidata/netcdf-c/pull/821
* Fixes for attributes bad parameters, more testing by @edhartnett in https://github.com/Unidata/netcdf-c/pull/822
* another rename fix by @edhartnett in https://github.com/Unidata/netcdf-c/pull/830
* warnings fix by @edhartnett in https://github.com/Unidata/netcdf-c/pull/832
* Ejh makefile cleanup unidata by @edhartnett in https://github.com/Unidata/netcdf-c/pull/837
* Address a Windows NC3 Memory issue. by @WardF in https://github.com/Unidata/netcdf-c/pull/846
* HDF5 ZLIB detection and CRT configuration by @WardF in https://github.com/Unidata/netcdf-c/pull/854
* changes to detect HDF5 HL, build with CRT and detect ZLIB in HDF5 by @pedro-vicente in https://github.com/Unidata/netcdf-c/pull/853
* Corrected overflow error in support of https://github.com/Unidata/net… by @WardF in https://github.com/Unidata/netcdf-c/pull/862
* Minor cleanups by @mathstuf in https://github.com/Unidata/netcdf-c/pull/814
* Pull #849 plus a note in the release notes. by @WardF in https://github.com/Unidata/netcdf-c/pull/868
* Move HDF4 to its own dispatch layer by @edhartnett in https://github.com/Unidata/netcdf-c/pull/849
* Some extra attribute and rename testing that I neglected to include in previous PRs by @edhartnett in https://github.com/Unidata/netcdf-c/pull/860
* Higher performance hash for metadata: step 0 by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/855
* Gh881 ac par.wif by @WardF in https://github.com/Unidata/netcdf-c/pull/883
* Revert change that broke extant NCO workflow by @WardF in https://github.com/Unidata/netcdf-c/pull/889
* Second Try: Aggregation of Pull Requests by @WardF in https://github.com/Unidata/netcdf-c/pull/893
* fix error reporting by @wkliao in https://github.com/Unidata/netcdf-c/pull/480
* fix issue 791: validate ncid for classic files by @wkliao in https://github.com/Unidata/netcdf-c/pull/804
* onstack: check whether HAVE_ALLOCA_H is defined or not by @mathstuf in https://github.com/Unidata/netcdf-c/pull/852
* Unused valgrind removal from build by @edhartnett in https://github.com/Unidata/netcdf-c/pull/886
* large files on 32-bit machines by @wkliao in https://github.com/Unidata/netcdf-c/pull/632
* Move HDF4 tests to their own directory by @edhartnett in https://github.com/Unidata/netcdf-c/pull/895
* V4.6.1 release branch.wif by @WardF in https://github.com/Unidata/netcdf-c/pull/901
* null test triple->host by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/888
* Give nccopy control of output filter suppression by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/894
* This is the second step in refactoring the libsrc4 code. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/878
* The ncdap_tests were a mess, so I decided to clean them up by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/907
* Fix bug: Index exceeds dimension bound by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/904
* fixed spaces by @edhartnett in https://github.com/Unidata/netcdf-c/pull/912
* Attempt to fix the problem of ftp failing when getting the hdf4 from our ftp site. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/918
* new method of using valgrind by @edhartnett in https://github.com/Unidata/netcdf-c/pull/910
* Final refactoring of libsrc4. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/903
* added merged HDF4 changes by @edhartnett in https://github.com/Unidata/netcdf-c/pull/922
* Removed some unused files and unused m4 macros from acinclude.m4 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/898
* Replace quotes when including system includes by @WardF in https://github.com/Unidata/netcdf-c/pull/929
* reset ncp->begin_var if errors occur when calling NC_begins() by @wkliao in https://github.com/Unidata/netcdf-c/pull/479
* Use hdf5-1.8 compatible output format if possible by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/934
* Enabled CDF5 support by default by @WardF in https://github.com/Unidata/netcdf-c/pull/935
* added fill mode and var rename tests by @edhartnett in https://github.com/Unidata/netcdf-c/pull/919
* Combine several pull requests by @WardF in https://github.com/Unidata/netcdf-c/pull/945
* Fix external symbol name conflicts in libdispatch/crc32.c by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/921
* create separate plugins top-level directory by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/937
* Fix  signedness warnings in nchashmap.c by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/944
* Add missing ERR Macro by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/949
* nctest: Make cmake pass under a windows cmd prompt by @t-b in https://github.com/Unidata/netcdf-c/pull/950
* Misc. source comment typos by @luzpaz in https://github.com/Unidata/netcdf-c/pull/902
* removed unused HDF4 code, fixed warnings in HDF4 test tst_h4_lendian.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/940
* Centos fix by @WardF in https://github.com/Unidata/netcdf-c/pull/952
* moved netcdf-4 tests from nc_test to nc_test4 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/938
* Expand the NC_INMEMORY capabilities  by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/879
* Consolidating Several PR's by @WardF in https://github.com/Unidata/netcdf-c/pull/956
* ncx: use __inline with MSVC by @mathstuf in https://github.com/Unidata/netcdf-c/pull/850
* Update the netCDF-Extractor entry re: esupport QHB-286994 by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/953
* Fix unused variable compiler warning by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/954
* created libhdf5, moved some files by @edhartnett in https://github.com/Unidata/netcdf-c/pull/958
* Adding Appveyor Support by @WardF in https://github.com/Unidata/netcdf-c/pull/983
* Testing Appveyor branch exclusion syntax. by @WardF in https://github.com/Unidata/netcdf-c/pull/984
* Small aggregation of PRs by @WardF in https://github.com/Unidata/netcdf-c/pull/992
* moved hdf5.h include in test nc_test/tst_inmemory.c to allow classic only builds to work by @edhartnett in https://github.com/Unidata/netcdf-c/pull/960
* Fix some issues with the inmemory code as identified by Ed by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/964
* removed unused function by @edhartnett in https://github.com/Unidata/netcdf-c/pull/988
* Cleanup some of the ncgen memory problems identified by Ed by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/558
* Alias stringlength url parameter and maxstrlen by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/619
* more progress towards separating HDF5 code from libsrc4 code by @edhartnett in https://github.com/Unidata/netcdf-c/pull/980
* Misc changes.wif by @WardF in https://github.com/Unidata/netcdf-c/pull/1005
* No longer attempting to use MPI_COMM_WORLD as default anywhere in library code by @edhartnett in https://github.com/Unidata/netcdf-c/pull/990
* Next step in HDF5 separation by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1004
* Fix use of 'int' to represent 'hid_t' that caused HDF5 1.10 to fail. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1008
* Test code for parallel I/O issue, enum issue, and some other issues by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1007
* removed dead code associated with old handling of NC_LONG, fixed test checking NC_ELATEFILL by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1013
* Misc changes by @WardF in https://github.com/Unidata/netcdf-c/pull/1015
* Re-implement the nc_get/put_vars operations for netcdf-4 using the corresponding HDF5 operations. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1001
* user defined formats by @edhartnett in https://github.com/Unidata/netcdf-c/pull/977
* added filter/szip tests by @edhartnett in https://github.com/Unidata/netcdf-c/pull/974
* changed macro STREQ to NCSTREQ to avoid name collusion with HDF4 library by @edhartnett in https://github.com/Unidata/netcdf-c/pull/967
* After this PR, nc_set_log_level() will always be defined by @edhartnett in https://github.com/Unidata/netcdf-c/pull/969
* Removed some dead code, now use nc4_get/put_vars for vara calls by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1023
* Lazy read of attributes by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1026
* Move to a new remote test server on Jetstream by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1032
* renamed NC_HDF5_FILE_INFO to NC_FILE_INFO by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1028
* fix cdf5 configure option by @wkliao in https://github.com/Unidata/netcdf-c/pull/1033
* Revert "fix cdf5 configure option" by @WardF in https://github.com/Unidata/netcdf-c/pull/1036
* ifdef cleanup of netcdf.h by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1030
* Fix clang warnings in tst_vars3.c, hdf5file.c, d4read.c, ncuri.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1050
* Second Try by @WardF in https://github.com/Unidata/netcdf-c/pull/1060
* Combining some PR's by @WardF in https://github.com/Unidata/netcdf-c/pull/1063
* install.md: building netCDF with PnetCDF support by @wkliao in https://github.com/Unidata/netcdf-c/pull/1043
* Enable metadata-perf test for CMake build by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/1059
* Fix esupport # HYV-329576 by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1062
* Merging multiple pull requests by @WardF in https://github.com/Unidata/netcdf-c/pull/1073
* 32bit lfs by @wkliao in https://github.com/Unidata/netcdf-c/pull/1022
* replace USE_CDF5 with ENABLE_CDF5 by @wkliao in https://github.com/Unidata/netcdf-c/pull/1038
* add missing note about CDF-5 format by @wkliao in https://github.com/Unidata/netcdf-c/pull/1044
* Fix missing ERR in tst_parallel5.c by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/1053
* moved hdf5 specific header stuff to hdf5internal.h by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1061
* cmake warning fix for nc_set_log_level warning from netcdf.h by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1069
* Eliminate NC_Dispatch redefinition compiler warning by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/1045
* Split hdf5file.c into hdf5file.c, hdf5create.c, and hdf5open.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1071
* introduce error code NC_EPNETCDF for errors at PnetCDF level by @wkliao in https://github.com/Unidata/netcdf-c/pull/1093
* YAA (Yet Another Aggregation)  by @WardF in https://github.com/Unidata/netcdf-c/pull/1096
* add NC_RELAX_COORD_BOUND to netcdf_meta.h.in by @wkliao in https://github.com/Unidata/netcdf-c/pull/1064
* add NC_HAS_PARALLEL4 to netcdf_meta.h.in by @wkliao in https://github.com/Unidata/netcdf-c/pull/1067
* nc_var_par_access by @wkliao in https://github.com/Unidata/netcdf-c/pull/1068
* Clean up the handling of chunking parameters (the -c flag) in nccopy. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1087
* Moving rest of HDF5 code from libsrc4 to libhdf5 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1075
* Correct ocprint build when DAP is turned off. by @WardF in https://github.com/Unidata/netcdf-c/pull/1097
* Clear up coverity complaints by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1100
* Move hid_t from NC_FILE_INFO_T to new struct NC_HDF5_FILE_INFO_T by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1076
* Make global atts read much, much faster by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1081
* Migrate script-based tests from bash to sh by @WardF in https://github.com/Unidata/netcdf-c/pull/1109
* Cleanup up some functions in preparation for lazy var reads (take 2) by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1092
* Warning, coverity warning, error handling, documentation, and hdf5_set_log_level fixes, all on the path to the lazy var feature by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1103
* Combined PR by @WardF in https://github.com/Unidata/netcdf-c/pull/1127
* error out when conflicts with PnetCDF are detected by @wkliao in https://github.com/Unidata/netcdf-c/pull/1070
* check PnetCDF version at compile time by @wkliao in https://github.com/Unidata/netcdf-c/pull/1089
* Eliminate double printing of nccopy program name in usage output by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/1118
* Fix handling of NULL start/count/stride, remove unneeded file lookups, moved get/put code from nc4hdf5.c to hdf5var.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1112
* fix problem of setting the default file format by @wkliao in https://github.com/Unidata/netcdf-c/pull/1090
* The identifier typename is apparently a reserved word in C++. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1132
* Combined Pull Requests by @WardF in https://github.com/Unidata/netcdf-c/pull/1135
* Fix compression level logic by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/828
* to be NC_EINDEFINE or NC_ENOTINDEFINE by @wkliao in https://github.com/Unidata/netcdf-c/pull/1125
* Fix the NC_INMEMORY code to work in all cases with HDF5 1.10. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1133
* Fix checking for HDF5 max dims, no longer re-create atts if not needed, confirm behavior for HDF5 cyclical files, allow user to set mpiexec by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1119
* Improve provenance information by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1124
* Add the ability to set some additional curlopt values by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1126
* Remove the set_fmode by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1139
* Clean up unneeded object lookups... by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1128
* Fix szip filter handling code and corresponding tests by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1142
* Fix benchmark programs by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1129
* Modify DAP2 and DAP4 to optionally allow Fillvalue/Variable mismatch by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1155
* Separate the HDF5 cache code by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1141
* Fix memory problems when using HDF5 version 1.10.x and later. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1157
* Remove unneeded include of netcdf.h from netcdf_mem.h by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1149
* Ignore NC_MPIIO and NC_MPIPOSIX by @wkliao in https://github.com/Unidata/netcdf-c/pull/1147
* various fixes for PnetCDF driver by @wkliao in https://github.com/Unidata/netcdf-c/pull/1143
* Revert/Improve nc_create + NC_DISKLESS behavior by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1158
* Fix HDF4 config issues by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1153
* Restore lost chunking documentation by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1159
* Multiple PR's in aggregate by @WardF in https://github.com/Unidata/netcdf-c/pull/1165
* Cmake fixups by @mathstuf in https://github.com/Unidata/netcdf-c/pull/1161
* more latefill tests by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1164
* Recreating 4.6.2 release branch. by @WardF in https://github.com/Unidata/netcdf-c/pull/1166
* Incorrect flag is set test for readonly by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/1170
* Fix many (but not all) memory leaks using gcc -fsanitize=address by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1173
* Merge v4.6.2 release branch back upstream by @WardF in https://github.com/Unidata/netcdf-c/pull/1177
* Separate the closing of HDF5 objects from freeing of internal metadata by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1167
* Fix test run_diskless2.sh with LARGE_FILE_TESTS by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1175
* Added DAP4.dox to doxygen template in support of https://github.com/U… by @WardF in https://github.com/Unidata/netcdf-c/pull/1179
* V4.6.2 release branch.wif by @WardF in https://github.com/Unidata/netcdf-c/pull/1191
* Next step in HDF5 separation - moving att HDF5 info to libhdf5 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1182
* Corrected VS2010 compilation issue in support of https://github.com/U… by @WardF in https://github.com/Unidata/netcdf-c/pull/1185
* Next step in libhdf5/libsrc4 separation - moving hdf5-specific dim info to libhdf5 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1186
* fixed curl configure issue by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1188
* added release notes by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1189
* minor doc change by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1190
* Fix failure in the nccopy -c option by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1192
* Next step in HDF5/libsrc4 separation - Moving HDF5 group info to libhdf5. by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1193
* Fixing text output in nc_test4/tst_files6.c and tst_rename.c by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1187
* V4.6.2 release branch.wif by @WardF in https://github.com/Unidata/netcdf-c/pull/1201
* RPATH handling is not needed for archive library by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/1176
* Remove extra argument by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/1198
* Eliminate compiler warnings (and code error) by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/1199
* Fix more memory leaks in netcdf-c library by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1202
* V4.6.2 release branch by @WardF in https://github.com/Unidata/netcdf-c/pull/1203
* add CMAKE_C_FLAGS_RELEASE in specify_static_crt_flag by @Jin-Whu in https://github.com/Unidata/netcdf-c/pull/947
* Some fixes for configure.ac by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1200
* V4.6.2 release branch by @WardF in https://github.com/Unidata/netcdf-c/pull/1212
* change rm in test script to use specific file names so parallel builds continue to work by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1214
* Pull Request Aggregation by @WardF in https://github.com/Unidata/netcdf-c/pull/1229
* Finally removed rest of HDF5 fields from nc4internal.h by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1218
* Eliminate double-free in realloc'd memory by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/1219
* Next step in HDF5 separation - separate initialization of HDF5 dispatch by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1224
* Move lazy att code to libhdf5 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1225
* Fixes #803 by @WardF in https://github.com/Unidata/netcdf-c/pull/1227
* Update nc_test/tst_inmemory to cover more cases by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1228
* Forgot to change realloc -> H5resize_memory in H5Zbzip2.c by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1221
* Combined Pull Requests, second take by @WardF in https://github.com/Unidata/netcdf-c/pull/1250
* Remove // style comments from source code. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1210
* Fix error report coming out of nc4info.c by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1213
* Almost done separating HDF5 from libsrc4 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1230
* Completing code separation of libsrc4 and libhdf5  by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1231
* adjust tests and utils for split of libsrc4 and libhdf5 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1234
* License update.wif by @WardF in https://github.com/Unidata/netcdf-c/pull/1243
* YAA (Yet Another Aggregation) by @WardF in https://github.com/Unidata/netcdf-c/pull/1264
* Fix failings of nc_test/run_inmemory.sh by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1240
* Eliminate nested comment by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/1244
* Fix some issues, add comments, and prepare for lazy(-ish) var reads by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1248
* Some small fixes and further preparation for lazy var metadata reading by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1258
* Separate out the -ansi comment fixes. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1252
* Latest PR Aggregation by @WardF in https://github.com/Unidata/netcdf-c/pull/1283
* deprecate non-relax coord bounds checking, also fix parallel builds for --enable-pnetcdf by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1238
* Make the netcdf-c library compile with gcc -ansi. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1242
* Use coordinates hidden att to speed opens... by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1262
* Fix handling of NULL strides, some warnings, and missing config.h includes in tests by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1266
* Add autogen.sh script for helping novice users by @t-b in https://github.com/Unidata/netcdf-c/pull/1280
* Y.A.A. (Yet Another Another Aggregation) by @WardF in https://github.com/Unidata/netcdf-c/pull/1293
* Fixes for warnings for parallel builds by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1271
* Tests on equalp are always true by @gsjaardema in https://github.com/Unidata/netcdf-c/pull/1276
* CMakeLists.txt: Fix compilation with default settings on debian stretch by @t-b in https://github.com/Unidata/netcdf-c/pull/1277
* Added newline after ncdump -k output by @WardF in https://github.com/Unidata/netcdf-c/pull/1285
* Fix issues: excess variables, broken LOG statements, parallel build issue, benchmark build issue by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1289
* When doing prefetch in DAP2, ignore invisible variables. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1301
* Rinse and Repeat: PR Edition by @WardF in https://github.com/Unidata/netcdf-c/pull/1309
* Fix warnings by @mathstuf in https://github.com/Unidata/netcdf-c/pull/1281
* change atts_not_read to atts_read in NC_GRP_INTO_T and NC_VAR_INFO_T by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1290
* Fix rename bug name dim same as existing var by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1294
* Clean up some macros in CMakeLists.txt, CMake-based build system. by @WardF in https://github.com/Unidata/netcdf-c/pull/1296
* Fix for bug renaming non-coord var to same name as dim without a var by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1298
* Fix errors when building on big-endian machine by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1302
* Added int i and j declarations by @simonholgate in https://github.com/Unidata/netcdf-c/pull/1314
* Add hack to deal with DAP2 signed byte conversion. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1317
* URL with username+pwd in url is not working. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1312
* Fix handling of '/' characters in names in DAP2. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1319
* Combination of the remaining rename-related PR's by @WardF in https://github.com/Unidata/netcdf-c/pull/1318
* Fix bug which changed varids after var rename by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1308
* Extend nccopy -F option syntax. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1311
* Fix additional big-endian machine error in dap4. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1321
* Preparing Maintenance Release by @WardF in https://github.com/Unidata/netcdf-c/pull/1322
* whitespace changes only by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1328
* Fix nc_test4/tst_filter.sh for big endian by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1339
* Fix ncgen handling of octal constants (with leading 0). by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1345
* 4.6.3 prep, upstream propegation by @WardF in https://github.com/Unidata/netcdf-c/pull/1332
* Fix typo in tst_filterparser.c by @opoplawski in https://github.com/Unidata/netcdf-c/pull/1337
* Documentation fixes for nc_rename_dim() by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1342
* Modify tst_filterparser to use baseline array by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1343
* provide NDEBUG version of macro NC_CHECK which does not use code file name by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1326
* Remove all references to __FILE__ in ncgen by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1327
* Fix nccopy to use NC_PERSIST so that -w actually persists the output. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1366
* move checking for flag conflict to nc_create_par by @wkliao in https://github.com/Unidata/netcdf-c/pull/1346
* Provide byte-range reading of remote datasets by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1267
* Support installation of the bzip2 plugin under name libh5bzip2.so by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1348
* non controversial parts of last PR by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1364
* Fix minor --ansi warnings in dinfermodel.c and bzlib.c by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1377
* Fix STRNG typo in CMakeList by @jschueller in https://github.com/Unidata/netcdf-c/pull/1367
* Move performance testing from nc_test4 to new directory nc_perf by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1371
* Minor Maintenance PR by @WardF in https://github.com/Unidata/netcdf-c/pull/1378
* Fix MinGW build by @jschueller in https://github.com/Unidata/netcdf-c/pull/1368
* Simplify libhdf5/nc5info.c to move to avoid parsing by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1361
* Bring behavior of nc-config in line with pkgconfig --libs --static netcdf by @WardF in https://github.com/Unidata/netcdf-c/pull/1383
* Backport relevant changes from 4.7.0 release into master by @WardF in https://github.com/Unidata/netcdf-c/pull/1386
* Allow programmatic definition of filters by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1375
* Minor config.h changes to support filters in Fortran by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1387
* Thread safety: step 1: cleanup by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1379
* Fix cmake wrt mmap by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1385
* Centralize calls to curl_global_init and curl_global_cleanup by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1389
* Option --disable-byterange fails if libcurl is not found by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1392
* uncommented test by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1394
* fix compile warnings in tst_large2, tst_rename2, and tst_rename3, and some build issues, including partial fix for building with HDF4 but without HDF5 by @edhartnett in https://github.com/Unidata/netcdf-c/pull/1404
* Nccopy was overriding default chunking when it should not. by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1409
* Add Wei-King Liao's ncvalidator program by @DennisHeimbigner in https://github.com/Unidata/netcdf-c/pull/1407
* Modified how libsz is being detected in automake-based builds. by @WardF in https://github.com/Unidata/netcdf-c/pull/1406
* Corrected mistake regarding VS version. by @WardF in https://github.com/Unidata/netcdf-c/pull/1418

## New Contributors
* @RomeroMalaquias made their first contribution in https://github.com/Unidata/netcdf-c/pull/346
* @shoyer made their first contribution in https://github.com/Unidata/netcdf-c/pull/316
* @thehesiod made their first contribution in https://github.com/Unidata/netcdf-c/pull/400
* @pastewka made their first contribution in https://github.com/Unidata/netcdf-c/pull/457
* @pelson made their first contribution in https://github.com/Unidata/netcdf-c/pull/470
* @nehaljwani made their first contribution in https://github.com/Unidata/netcdf-c/pull/691
* @mingwandroid made their first contribution in https://github.com/Unidata/netcdf-c/pull/729
* @Lnaden made their first contribution in https://github.com/Unidata/netcdf-c/pull/760
* @pedro-vicente made their first contribution in https://github.com/Unidata/netcdf-c/pull/853
* @t-b made their first contribution in https://github.com/Unidata/netcdf-c/pull/950
* @luzpaz made their first contribution in https://github.com/Unidata/netcdf-c/pull/902
* @Jin-Whu made their first contribution in https://github.com/Unidata/netcdf-c/pull/947
* @simonholgate made their first contribution in https://github.com/Unidata/netcdf-c/pull/1314
* @jschueller made their first contribution in https://github.com/Unidata/netcdf-c/pull/1367

**Full Changelog**: https://github.com/Unidata/netcdf-c/compare/v4.3.1-rc1...v4.9.0
## 4.8.1 - August 18, 2021

* [Bug Fix] Fix multiple bugs in libnczarr. See [Github #2066](https://github.com/Unidata/netcdf-c/pull/2066).
* [Enhancement] Support windows network paths (e.g. \\svc\...). See [Github #2065](https://github.com/Unidata/netcdf-c/pull/2065).
* [Enhancement] Convert to a new representation of the NCZarr meta-data extensions: version 2. Read-only backward compatibility is provided. See [Github #2032](https://github.com/Unidata/netcdf-c/pull/2032).
* [Bug Fix] Fix dimension_separator bug in libnczarr. See [Github #2035](https://github.com/Unidata/netcdf-c/pull/2035).
* [Bug Fix] Fix bugs in libdap4. See [Github #2005](https://github.com/Unidata/netcdf-c/pull/2005).
* [Bug Fix] Store NCZarr fillvalue as a singleton instead of a 1-element array. See [Github #2017](https://github.com/Unidata/netcdf-c/pull/2017).
* [Bug Fixes] The netcdf-c library was incorrectly determining the scope of dimension; similar to the type scope problem. See [Github #2012](https://github.com/Unidata/netcdf-c/pull/2012) for more information.
* [Bug Fix] Re-enable DAP2 authorization testing. See [Github #2011](https://github.com/Unidata/netcdf-c/issues/2011).
* [Bug Fix] Fix bug with windows version of mkstemp that causes failure to create more than 26 temp files. See [Github #1998](https://github.com/Unidata/netcdf-c/pull/1998).
* [Bug Fix] Fix ncdump bug when printing VLENs with basetype char. See [Github #1986](https://github.com/Unidata/netcdf-c/issues/1986).
* [Bug Fixes] The netcdf-c library was incorrectly determining the scope of types referred to by nc_inq_type_equal. See [Github #1959](https://github.com/Unidata/netcdf-c/pull/1959) for more information.
* [Bug Fix] Fix bug in use of XGetopt when building under Mingw. See [Github #2009](https://github.com/Unidata/netcdf-c/issues/2009).
* [Enhancement] Improve the error reporting when attempting to use a filter for which no implementation can be found in HDF5_PLUGIN_PATH. See [Github #2000](https://github.com/Unidata/netcdf-c/pull/2000) for more information.
* [Bug Fix] Fix `make distcheck` issue in `nczarr_test/` directory. See [Github #2007](https://github.com/Unidata/netcdf-c/issues/2007).
* [Bug Fix] Fix bug in NCclosedir in dpathmgr.c. See [Github #2003](https://github.com/Unidata/netcdf-c/issues/2003).
* [Bug Fix] Fix bug in ncdump that assumes that there is a relationship between the total number of dimensions and the max dimension id. See [Github #2004](https://github.com/Unidata/netcdf-c/issues/2004).
* [Bug Fix] Fix bug in JSON processing of strings with embedded quotes. See [Github #1993](https://github.com/Unidata/netcdf-c/issues/1993).
* [Enhancement] Add support for the new "dimension_separator" enhancement to Zarr v2. See [Github #1990](https://github.com/Unidata/netcdf-c/pull/1990) for more information.
* [Bug Fix] Fix hack for handling failure of shell programs to properly handle escape characters. See [Github #1989](https://github.com/Unidata/netcdf-c/issues/1989).
* [Bug Fix] Allow some primitive type names to be used as identifiers depending on the file format. See [Github #1984](https://github.com/Unidata/netcdf-c/issues/1984).
* [Enhancement] Add support for reading/writing pure Zarr storage format that supports the XArray _ARRAY_DIMENSIONS attribute. See [Github #1952](https://github.com/Unidata/netcdf-c/pull/1952) for more information.
* [Update] Updated version of bzip2 used in filter testing/functionality, in support of [Github #1969](https://github.com/Unidata/netcdf-c/issues/1969).
* [Bug Fix] Corrected HDF5 version detection logic as described in [Github #1962](https://github.com/Unidata/netcdf-c/issues/1962).

## 4.8.0 - March 30, 2021

* [Enhancement] Bump the NC_DISPATCH_VERSION from 2 to 3, and as a side effect, unify the definition of NC_DISPATCH_VERSION so it only needs to be defined in CMakeLists.txt and configure.ac. See [Github #1945](https://github.com/Unidata/netcdf-c/pull/1945) for more information.
* [Enhancement] Provide better cross platform path name management. This converts paths for various platforms (e.g. Windows, MSYS, etc.) so that they are in the proper format for the executing platform. See [Github #1958](https://github.com/Unidata/netcdf-c/pull/1958) for more information.
* [Bug Fixes] The nccopy program was treating -d0 as turning deflation on rather than interpreting it as "turn off deflation". See [Github #1944](https://github.com/Unidata/netcdf-c/pull/1944) for more information.
* [Enhancement] Add support for storing NCZarr data in zip files. See [Github #1942](https://github.com/Unidata/netcdf-c/pull/1942) for more information.
* [Bug Fixes] Make fillmismatch the default for DAP2 and DAP4; too many servers ignore this requirement.
* [Bug Fixes] Fix some memory leaks in NCZarr, fix a bug with long strides in NCZarr. See [Github #1913](https://github.com/Unidata/netcdf-c/pull/1913) for more information.
* [Enhancement] Add some optimizations to NCZarr, dosome cleanup of code cruft, add some NCZarr test cases, add a performance test to NCZarr. See [Github #1908](https://github.com/Unidata/netcdf-c/pull/1908) for more information.
* [Bug Fix] Implement a better chunk cache system for NCZarr. The cache now uses extendible hashing plus a linked list for provide a combination of expandibility, fast access, and LRU behavior. See [Github #1887](https://github.com/Unidata/netcdf-c/pull/1887) for more information.
* [Enhancement] Provide .rc fields for S3 authentication: HTTP.S3.ACCESSID and HTTP.S3.SECRETKEY.
* [Enhancement] Give the client control over what parts of a DAP2 URL are URL encoded (i.e. %xx). This is to support the different decoding rules that servers apply to incoming URLS. See [Github #1884](https://github.com/Unidata/netcdf-c/pull/1884) for more information.
* [Bug Fix] Fix incorrect time offsets from `ncdump -t`, in some cases when the time `units` attribute contains both a **non-zero** time-of-day, and a time zone suffix containing the letter "T", such as "UTC".  See [Github #1866](https://github.com/Unidata/netcdf-c/pull/1866) for more information.
* [Bug Fix] Cleanup the NCZarr S3 build options. See [Github #1869](https://github.com/Unidata/netcdf-c/pull/1869) for more information.
* [Bug Fix] Support aligned access for selected ARM processors.  See [Github #1871](https://github.com/Unidata/netcdf-c/pull/1871) for more information.
* [Documentation] Migrated the documents in the NUG/ directory to the dedicated NUG repository found at https://github.com/Unidata/netcdf
* [Bug Fix] Revert the internal filter code to simplify it. From the user's point of view, the only visible change should be that (1) the functions that convert text to filter specs have had their signature reverted and renamed and have been moved to netcdf_aux.h, and (2) Some filter API functions now return NC_ENOFILTER when inquiry is made about some filter. Internally, the dispatch table has been modified to get rid of the complex structures.
* [Bug Fix] If the HDF5 byte-range Virtual File Driver is available )HDf5 1.10.6 or later) then use it because it has better performance than the one currently built into the netcdf library.
* [Bug Fix] Fixed byte-range support with cURL > 7.69. See [https://github.com/Unidata/netcdf-c/pull/1798].
* [Enhancement] Added new test for using compression with parallel I/O: nc_test4/tst_h_par_compress.c. See [https://github.com/Unidata/netcdf-c/pull/1784].
* [Bug Fix] Don't return error for extra calls to nc_redef() for netCDF/HDF5 files, unless classic model is in use. See [https://github.com/Unidata/netcdf-c/issues/1779].
* [Enhancement] Added new parallel I/O benchmark program to mimic NOAA UFS data writes, built when --enable-benchmarks is in configure. See [https://github.com/Unidata/netcdf-c/pull/1777].
* [Bug Fix] Now allow szip to be used on variables with unlimited dimension [https://github.com/Unidata/netcdf-c/issues/1774].
* [Enhancement] Add support for cloud storage using a variant of the Zarr storage format. Warning: this feature is highly experimental and is subject to rapid evolution [https://www.unidata.ucar.edu/blogs/developer/en/entry/overview-of-zarr-support-in].
* [Bug Fix] Fix nccopy to properly set default chunking parameters when not otherwise specified. This can significantly improve performance in selected cases. Note that if seeing slow performance with nccopy, then, as a work-around, specifically set the chunking parameters. [https://github.com/Unidata/netcdf-c/issues/1763].
* [Bug Fix] Fix some protocol bugs/differences between the netcdf-c library and the OPeNDAP Hyrax server. Also cleanup checksum handling [https://github.com/Unidata/netcdf-c/issues/1712].* [Bug Fix] IMPORTANT: Ncgen was not properly handling large
data sections. The problem manifests as incorrect ordering of
data in the created file. Aside from examining the file with
ncdump, the error can be detected by running ncgen with the -lc
flag (to produce a C file). Examine the file to see if any
variable is written in pieces as opposed to a single call to
nc_put_vara. If multiple calls to nc_put_vara are used to write
a variable, then it is probable that the data order is
incorrect. Such multiple writes can occur for large variables
and especially when one of the dimensions is unlimited.
* [Bug Fix] Add necessary __declspec declarations to allow compilation
of netcdf library without causing errors or (_declspec related)
warnings [https://github.com/Unidata/netcdf-c/issues/1725].
* [Enhancement] When a filter is applied twice with different
parameters, then the second set is used for writing the dataset
[https://github.com/Unidata/netcdf-c/issues/1713].
* [Bug Fix] Now larger cache settings are used for sequential HDF5 file creates/opens on parallel I/O capable builds; see [Github #1716](https://github.com/Unidata/netcdf-c/issues/1716) for more information.
* [Bug Fix] Add functions to libdispatch/dnotnc4.c to support
dispatch table operations that should work for any dispatch
table, even if they do not do anything; functions such as
nc_inq_var_filter [https://github.com/Unidata/netcdf-c/issues/1693].
* [Bug Fix] Fixed a scalar annotation error when scalar == 0; see [Github #1707](https://github.com/Unidata/netcdf-c/issues/1707) for more information.
* [Bug Fix] Use proper CURLOPT values for VERIFYHOST and VERIFYPEER; the semantics for VERIFYHOST in particular changed. Documented in NUG/DAP2.md. See  [https://github.com/Unidata/netcdf-c/issues/1684].
* [Bug Fix][cmake] Correct an issue with parallel filter test logic in CMake-based builds.
* [Bug Fix] Now allow nc_inq_var_deflate()/nc_inq_var_szip() to be called for all formats, not just HDF5. Non-HDF5 files return NC_NOERR and report no compression in use. This reverts behavior that was changed in the 4.7.4 release. See [https://github.com/Unidata/netcdf-c/issues/1691].
* [Bug Fix] Compiling on a big-endian machine exposes some missing forward delcarations in dfilter.c.
* [File Change] Change from HDF5 v1.6 format compatibility, back to v1.8 compatibility, for newly created files.  The superblock changes from version 0 back to version 2.  An exception is when using libhdf5 deprecated versions 1.10.0 and 1.10.1, which can only create v1.6 compatible format.  Full backward read/write compatibility for netCDF-4 is maintained in all cases.  See [Github #951](https://github.com/Unidata/netcdf-c/issues/951).

## 4.7.4 - March 27, 2020

* [Windows] Bumped packaged HDF5 to 1.10.6, HDF4 to 4.2.14, and libcurl to 7.60.0.
* [Enhancement] Support has been added for HDF5-1.12.0.  See [https://github.com/Unidata/netcdf-c/issues/1528].
* [Bug Fix] Correct behavior for the command line utilities when directly accessing a directory using utf8 characters. See [Github #1669](https://github.com/Unidata/netcdf-c/issues/1669), [Github #1668](https://github.com/Unidata/netcdf-c/issues/1668) and [Github #1666](https://github.com/Unidata/netcdf-c/issues/1666) for more information.
* [Bug Fix] Attempts to set filters or chunked storage on scalar vars will now return NC_EINVAL. Scalar vars cannot be chunked, and only chunked vars can have filters. Previously the library ignored these attempts, and always storing scalars as contiguous storage. See [https://github.com/Unidata/netcdf-c/issues/1644].
* [Enhancement] Support has been added for multiple filters per variable.  See [https://github.com/Unidata/netcdf-c/issues/1584].
* [Enhancement] Now nc_inq_var_szip retuns 0 for parameter values if szip is not in use for var. See [https://github.com/Unidata/netcdf-c/issues/1618].
* [Enhancement] Now allow parallel I/O with filters, for HDF5-1.10.3 and later. See [https://github.com/Unidata/netcdf-c/issues/1473].
* [Enhancement] Increased default size of cache buffer to 16 MB, from 4 MB. Increased number of slots to 4133. See [https://github.com/Unidata/netcdf-c/issues/1541].
* [Enhancement] Allow zlib compression to be used with parallel I/O writes, if HDF5 version is 1.10.3 or greater. See [https://github.com/Unidata/netcdf-c/issues/1580].
* [Enhancement] Restore use of szip compression when writing data (including writing in parallel if HDF5 version is 1.10.3 or greater). See [https://github.com/Unidata/netcdf-c/issues/1546].
* [Enhancement] Enable use of compact storage option for small vars in netCDF/HDF5 files. See [https://github.com/Unidata/netcdf-c/issues/1570].
* [Enhancement] Updated benchmarking program bm_file.c to better handle very large files. See [https://github.com/Unidata/netcdf-c/issues/1555].
* [Enhancement] Added version number to dispatch table, and now check version with nc_def_user_format(). See [https://github.com/Unidata/netcdf-c/issues/1599].
* [Bug Fix] Fixed user setting of MPI launcher for parallel I/O HDF5 test in h5_test. See [https://github.com/Unidata/netcdf-c/issues/1626].
* [Bug Fix] Fixed problem of growing memory when netCDF-4 files were opened and closed. See [https://github.com/Unidata/netcdf-c/issues/1575 and https://github.com/Unidata/netcdf-c/issues/1571].
* [Enhancement] Increased size of maximum allowed name in HDF4 files to NC_MAX_NAME. See [https://github.com/Unidata/netcdf-c/issues/1631].

## 4.7.3 - November 20, 2019

* [Bug Fix]Fixed an issue where installs from tarballs will not properly compile in parallel environments.
* [Bug Fix] Library was modified so that rewriting the same attribute happens without deleting the attribute, to avoid a limit on how many times this may be done in HDF5. This fix was thought to be in 4.6.2 but was not. See [https://github.com/Unidata/netcdf-c/issues/350].
* [Enhancement] Add a dispatch version number to netcdf_meta.h and libnetcdf.settings, in case we decide to change dispatch table in future. See [https://github.com/Unidata/netcdf-c/issues/1469].
* [Bug Fix] Now testing that endianness can only be set on atomic ints and floats. See [https://github.com/Unidata/netcdf-c/issues/1479].
* [Bug Fix] Fix for subtle error involving var and unlimited dim of the same name, but unrelated, in netCDF-4. See [https://github.com/Unidata/netcdf-c/issues/1496].
* [Enhancement] Update for attribute documentation. See [https://github.com/Unidata/netcdf-c/issues/1512].
* [Bug Fix][Enhancement] Corrected assignment of anonymous (a.k.a. phony) dimensions in an HDF5 file. Now when a dataset uses multiple dimensions of the same size, netcdf assumes they are different dimensions. See [GitHub #1484](https://github.com/Unidata/netcdf-c/issues/1484) for more information.

## 4.7.2 - October 22, 2019

* [Bug Fix][Enhancement] Various bug fixes and enhancements.
* [Bug Fix][Enhancement] Corrected an issue where protected memory was being written to with some pointer slight-of-hand.  This has been in the code for a while, but appears to be caught by the compiler on OSX, under circumstances yet to be completely nailed down.  See [GitHub #1486](https://github.com/Unidata/netcdf-c/issues/1486) for more information.
* [Enhancement] [Parallel IO] Added support for parallel functions in MSVC. See [Github #1492](https://github.com/Unidata/netcdf-c/pull/1492) for more information.
* [Enhancement] Added a function for changing the ncid of an open file.  This function should only be used if you know what you are doing, and is meant to be used primarily with PIO integration. See [GitHub #1483](https://github.com/Unidata/netcdf-c/pull/1483) and [GitHub #1487](https://github.com/Unidata/netcdf-c/pull/1487) for more information.

## 4.7.1 - August 27, 2019

* [Enhancement] Added unit_test directory, which contains unit tests for the libdispatch and libsrc4 code (and any other directories that want to put unit tests there). Use --disable-unit-tests to run without unit tests (ex. for code coverage analysis). See [GitHub #1458](https://github.com/Unidata/netcdf-c/issues/1458).

* [Bug Fix] Remove obsolete _CRAYMPP and LOCKNUMREC macros from code. Also brought documentation up to date in man page. These macros were used in ancient times, before modern parallel I/O systems were developed. Programmers interested in parallel I/O should see nc_open_par() and nc_create_par(). See [GitHub #1459](https://github.com/Unidata/netcdf-c/issues/1459).

* [Enhancement] Remove obsolete and deprecated functions nc_set_base_pe() and nc_inq_base_pe() from the dispatch table. (Both functions are still supported in the library, this is an internal change only.) See [GitHub #1468](https://github.com/Unidata/netcdf-c/issues/1468).

* [Bug Fix] Reverted nccopy behavior so that if no -c parameters are given, then any default chunking is left to the netcdf-c library to decide. See [GitHub #1436](https://github.com/Unidata/netcdf-c/issues/1436).

## 4.7.0 - April 29, 2019

* [Enhancement] Updated behavior of `pkgconfig` and `nc-config` to allow the use of the `--static` flags, e.g. `nc-config --libs --static`, which will show information for linking against `libnetcdf` statically. See [Github #1360](https://github.com/Unidata/netcdf-c/issues/1360) and [Github #1257](https://github.com/Unidata/netcdf-c/issues/1257) for more information.

* [Enhancement] Provide byte-range reading of remote datasets. This allows
read-only access to, for example, Amazon S3 objects and also Thredds Server
datasets via the HTTPService access method.
See [GitHub #1251](https://github.com/Unidata/netcdf-c/issues/1251).

* Update the license from the home-brewed NetCDF license to the standard 3-Clause BSD License.  This change does not result in any new restrictions; it is merely the adoption of a standard, well-known and well-understood license in place of the historic NetCDF license written at Unidata.  This is part of a broader push by Unidata to adopt modern, standardized licensing.

## 4.6.3 - February 28, 2019

* [Bug Fix] Correctly generated `netcdf.pc` generated either by `configure` or `cmake`.  If linking against a static netcdf, you would need to pass the `--static` argument to `pkg-config` in order to list all of the downstream dependencies.  See [Github #1324](https://github.com/Unidata/netcdf-c/issues/1324) for more information.
* Now always write hidden coordinates attribute, which allows faster file opens when present. See [Github #1262](https://github.com/Unidata/netcdf-c/issues/1262) for more information.
* Some fixes for rename, including fix for renumbering of varids after a rename (#1307), renaming var to dim without coordinate var. See [Github #1297](https://github.com/Unidata/netcdf-c/issues/1297).
* Fix of NULL parameter causing segfaults in put_vars functions. See [Github #1265](https://github.com/Unidata/netcdf-c/issues/1265) for more information.
* Fix of --enable-benchmark benchmark tests [Github #1211](https://github.com/Unidata/netcdf-c/issues/1211)
* Update the license from the home-brewed NetCDF license to the standard 3-Clause BSD License.  This change does not result in any new restrictions; it is merely the adoption of a standard, well-known and well-understood license in place of the historic NetCDF license written at Unidata.  This is part of a broader push by Unidata to adopt modern, standardized licensing.
* [BugFix] Corrected DAP-related issues on big-endian machines. See [Github #1321](https://github.com/Unidata/netcdf-c/issues/1321), [Github #1302](https://github.com/Unidata/netcdf-c/issues/1302) for more information.
* [BugFix][Enhancement]  Various and sundry bugfixes and performance enhancements, thanks to \@edhartnett, \@gsjaardema, \@t-b, \@wkliao, and all of our other contributors.
* [Enhancement] Extended `nccopy -F` syntax to support multiple variables with a single invocation. See [Github #1311](https://github.com/Unidata/netcdf-c/issues/1311) for more information.
* [BugFix] Corrected an issue where DAP2 was incorrectly converting signed bytes, resulting in an erroneous error message under some circumstances. See [GitHub #1317](https://github.com/Unidata/netcdf-c/issues/1317) for more information.  See [Github #1319](https://github.com/Unidata/netcdf-c/issues/1319) for related information.
* [BugFix][Enhancement] Modified `nccopy` so that `_NCProperties` is not copied over verbatim but is instead generated based on the version of `libnetcdf` used when copying the file.  Additionally, `_NCProperties` are displayed if/when associated with a netcdf3 file, now. See [GitHub #803](https://github.com/Unidata/netcdf-c/issues/803) for more information.

## 4.6.2 - November 16, 2018

* [Enhancement] Lazy att read - only read atts when user requests one of them.  See [GitHub #857](https://github.com/Unidata/netcdf-c/issues/857).
* [Enhancement] Fast global att read - when global atts are read, they are read much more quickly.  See [GitHub #857](https://github.com/Unidata/netcdf-c/issues/857).

## 4.6.2-rc2 November 1, 2018


* [Enhancement] Add nccopy command options for per-variable chunk sizing, and minimum chunk size.  See [GitHub #1087](https://github.com/Unidata/netcdf-c/pull/1087).
* [Bug Fix] Fix nccopy handling of user specified chunk sizes.  See [GitHub #725](https://github.com/Unidata/netcdf-c/issues/725),[#1087](https://github.com/Unidata/netcdf-c/issues/1087).
* [Bug Fix] Avoid limit on number of times a netCDF4 attribute can be updated.  Not a complete fix for the HDF5 "maximum creation order" problem, but should greatly reduce occurrences in many real-world cases.  See [GitHub #350](https://github.com/Unidata/netcdf-c/issues/350).
* [Bug Fix] The use of NC_DISKLESS has been modified to make it cleaner. This adds a new flag called NC_PERSIST that takes over the now obsolete NC_MPIPOSIX.
* [Obsolete] Obsolete the MPIPOSIX flag.
* [Bug Fix] When using filters with HDF5 1.10.x or later, it is necessary to utilize the HDF5 replacements for malloc, realloc, and free in the filter code.

## 4.6.2-rc1 - September 19, 2018

* [Enhancement] Create a new version of _NCProperties provenance attribute. This version (version 2) supports arbitrary key-value pairs. It is the default when new files are created. Version 1 continues to be accepted.
* [Enhancement] Allow user to set http read buffersize for DAP2 and  DAP4 using the tag HTTP.READ.BUFFERSIZE in the .daprc file.
* [Enhancement] Allow user to set http keepalive for DAP2 and  DAP4 using the tag HTTP.KEEPALIVE in the .daprc file (see the OPeNDAP documentation for details).
* [Enhancement] Support DAP4 remote tests using a new remote test server located on the Unidata JetStream project.
* [Enhancement] Improved the performance of the nc_get/put_vars operations by using the equivalent slab capabilities of hdf5. Result is a significant speedup of these operations.  See [GitHub #1001](https://github.com/Unidata/netcdf-c/pull/1001) for more information.
* [Enhancement] Expanded the capabilities of `NC_INMEMORY` to support writing and accessing the final modified memory.  See [GitHub #879](https://github.com/Unidata/netcdf-c/pull/879) for more information.
* [Enhancement] Made CDF5 support enabled by default.  See [Github #931](https://github.com/Unidata/netcdf-c/issues/931) for more information.
* [Bug Fix] Corrected a number of memory issues identified in `ncgen`.  See [GitHub #558 for more information](https://github.com/Unidata/netcdf-c/pull/558).

## 4.6.1 - March 19, 2018

* [Bug Fix] Corrected an issue which could result in a dap4 failure. See [Github #888](https://github.com/Unidata/netcdf-c/pull/888) for more information.
* [Bug Fix][Enhancement] Allow `nccopy` to control output filter suppresion.  See [Github #894](https://github.com/Unidata/netcdf-c/pull/894) for more information.
* [Enhancement] Reverted some new behaviors that, while in line with the netCDF specification, broke existing workflows.  See [Github #843](https://github.com/Unidata/netcdf-c/issues/843) for more information.
* [Bug Fix] Improved support for CRT builds with Visual Studio, improves zlib detection in hdf5 library. See [Github #853](https://github.com/Unidata/netcdf-c/pull/853) for more information.
* [Enhancement][Internal] Moved HDF4 into a distinct dispatch layer. See [Github #849](https://github.com/Unidata/netcdf-c/pull/849) for more information.

## 4.6.0 - January 24, 2018
* [Enhancement] Full support for using HDF5 dynamic filters, both for reading and writing. See the file docs/filters.md.
* [Enhancement] Added an option to enable strict null-byte padding for headers; this padding was specified in the spec but was not enforced.  Enabling this option will allow you to check your files, as it will return an E_NULLPAD error.  It is possible for these files to have been written by older versions of libnetcdf.  There is no effective problem caused by this lack of null padding, so enabling these options is informational only.  The options for `configure` and `cmake` are `--enable-strict-null-byte-header-padding` and `-DENABLE_STRICT_NULL_BYTE_HEADER_PADDING`, respectively.  See [Github #657](https://github.com/Unidata/netcdf-c/issues/657) for more information.
* [Enhancement] Reverted behavior/handling of out-of-range attribute values to pre-4.5.0 default. See [Github #512](https://github.com/Unidata/netcdf-c/issues/512) for more information.
* [Bug] Fixed error in tst_parallel2.c. See [Github #545](https://github.com/Unidata/netcdf-c/issues/545) for more information.
* [Bug] Fixed handling of corrupt files + proper offset handling for hdf5 files. See [Github #552](https://github.com/Unidata/netcdf-c/issues/552) for more information.
* [Bug] Corrected a memory overflow in `tst_h_dimscales`, see [Github #511](https://github.com/Unidata/netcdf-c/issues/511), [Github #505](https://github.com/Unidata/netcdf-c/issues/505), [Github #363](https://github.com/Unidata/netcdf-c/issues/363) and [Github #244](https://github.com/Unidata/netcdf-c/issues/244) for more information.

## 4.5.0 - October 20, 2017

* Corrected an issue which could potential result in a hang while using parallel file I/O. See [Github #449](https://github.com/Unidata/netcdf-c/pull/449) for more information.
* Addressed an issue with `ncdump` not properly handling dates on a 366 day calendar. See [GitHub #359](https://github.com/Unidata/netcdf-c/issues/359) for more information.

### 4.5.0-rc3 - September 29, 2017

* [Update] Due to ongoing issues, native CDF5 support has been disabled by **default**.  You can use the options mentioned below (`--enable-cdf5` or `-DENABLE_CDF5=TRUE` for `configure` or `cmake`, respectively).  Just be aware that for the time being, Reading/Writing CDF5 files on 32-bit platforms may result in unexpected behavior when using extremely large variables.  For 32-bit platforms it is best to continue using `NC_FORMAT_64BIT_OFFSET`.
* [Bug] Corrected an issue where older versions of curl might fail. See [GitHub #487](https://github.com/Unidata/netcdf-c/issues/487) for more information.
* [Enhancement] Added options to enable/disable `CDF5` support at configure time for autotools and cmake-based builds.  The options are `--enable/disable-cdf5` and `ENABLE_CDF5`, respectively.  See [Github #484](https://github.com/Unidata/netcdf-c/issues/484) for more information.
* [Bug Fix] Corrected an issue when subsetting a netcdf3 file via `nccopy -v/-V`. See [Github #425](https://github.com/Unidata/netcdf-c/issues/425) and [Github #463](https://github.com/Unidata/netcdf-c/issues/463) for more information.
* [Bug Fix] Corrected `--has-dap` and `--has-dap4` output for cmake-based builds. See [GitHub #473](https://github.com/Unidata/netcdf-c/pull/473) for more information.
* [Bug Fix] Corrected an issue where `NC_64BIT_DATA` files were being read incorrectly by ncdump, despite the data having been written correctly.  See [GitHub #457](https://github.com/Unidata/netcdf-c/issues/457) for more information.
* [Bug Fix] Corrected a potential stack buffer overflow.  See [GitHub #450](https://github.com/Unidata/netcdf-c/pull/450) for more information.

### 4.5.0-rc2 - August 7, 2017

* [Bug Fix] Addressed an issue with how cmake was implementing large file support on 32-bit systems. See [GitHub #385](https://github.com/Unidata/netcdf-c/issues/385) for more information.
* [Bug Fix] Addressed an issue where ncgen would not respect keyword case. See [GitHub #310](https://github.com/Unidata/netcdf-c/issues/310) for more information.

### 4.5.0-rc1 - June 5, 2017

* [Enhancement] DAP4 is now included. Since dap2 is the default for urls, dap4 must be specified by
(1) using "dap4:" as the url protocol, or
(2) appending "#protocol=dap4" to the end of the url, or
(3) appending "#dap4" to the end of the url
Note that dap4 is enabled by default but remote-testing is
disabled until the testserver situation is resolved.
* [Enhancement] The remote testing server can now be specified with the `--with-testserver` option to ./configure.
* [Enhancement] Modified netCDF4 to use ASCII for NC_CHAR.  See [Github Pull request #316](https://github.com/Unidata/netcdf-c/pull/316) for more information.
* [Bug Fix] Corrected an error with how dimsizes might be read. See [Github #410](https://github.com/unidata/netcdf-c/issues/410) for more information.
* [Bug Fix] Corrected an issue where 'make check' would fail if 'make' or 'make all' had not run first.  See [Github #339](https://github.com/Unidata/netcdf-c/issues/339) for more information.
* [Bug Fix] Corrected an issue on Windows with Large file tests. See [Github #385](https://github.com/Unidata/netcdf-c/issues/385]) for more information.
* [Bug Fix] Corrected an issue with diskless file access, see [Pull Request #400](https://github.com/Unidata/netcdf-c/issues/400) and [Pull Request #403](https://github.com/Unidata/netcdf-c/issues/403) for more information.
* [Upgrade] The bash based test scripts have been upgraded to use a common test_common.sh include file that isolates build specific information.
* [Upgrade] The bash based test scripts have been upgraded to use a common test_common.sh include file that isolates build specific information.
* [Refactor] the oc2 library is no longer independent of the main netcdf-c library. For example, it now uses ncuri, nclist, and ncbytes instead of its homegrown equivalents.
* [Bug Fix] `NC_EGLOBAL` is now properly returned when attempting to set a global `_FillValue` attribute. See [GitHub #388](https://github.com/Unidata/netcdf-c/issues/388) and [GitHub #389](https://github.com/Unidata/netcdf-c/issues/389) for more information.
* [Bug Fix] Corrected an issue where data loss would occur when `_FillValue` was mistakenly allowed to be redefined.  See [Github #390](https://github.com/Unidata/netcdf-c/issues/390), [GitHub #387](https://github.com/Unidata/netcdf-c/pull/387) for more information.
* [Upgrade][Bug] Corrected an issue regarding how "orphaned" DAS attributes were handled. See [GitHub #376](https://github.com/Unidata/netcdf-c/pull/376) for more information.
* [Upgrade] Update utf8proc.[ch] to use the version now maintained by the Julia Language project (https://github.com/JuliaLang/utf8proc/blob/master/LICENSE.md).
* [Bug] Addressed conversion problem with Windows sscanf.  This primarily affected some OPeNDAP URLs on Windows.  See [GitHub #365](https://github.com/Unidata/netcdf-c/issues/365) and [GitHub #366](https://github.com/Unidata/netcdf-c/issues/366) for more information.
* [Enhancement] Added support for HDF5 collective metadata operations when available. Patch submitted by Greg Sjaardema, see [Pull request #335](https://github.com/Unidata/netcdf-c/pull/335) for more information.
* [Bug] Addressed a potential type punning issue. See [GitHub #351](https://github.com/Unidata/netcdf-c/issues/351) for more information.
* [Bug] Addressed an issue where netCDF wouldn't build on Windows systems using MSVC 2012. See [GitHub #304](https://github.com/Unidata/netcdf-c/issues/304) for more information.
* [Bug] Fixed an issue related to potential type punning, see [GitHub #344](https://github.com/Unidata/netcdf-c/issues/344) for more information.
* [Enhancement] Incorporated an enhancement provided by Greg Sjaardema, which may improve read/write times for some complex files.  Basically, linked lists were replaced in some locations where it was safe to use an array/table.  See [Pull request #328](https://github.com/Unidata/netcdf-c/pull/328) for more information.

## 4.4.1.1 - November 21, 2016

* [Bug] Fixed an issue where `ncgen` would potentially crash or write incorrect netCDF4 binary data under very specific circumstances.  This bug did *not* affect data written on 32-bit systems or by using the netCDF library; it was specific to `ncgen`.  This would only happen when writing a compound data type containing an 8-byte data type followed by a 4-byte data type *and* the 4-byte data type was not properly aligned; this would *possibly* result in incorrect padding. This did not affect 32-bit systems, or data written directly by the library.  See [GitHub #323](https://github.com/Unidata/netcdf-c/issues/323) for more information.
* [Documentation] Updated documentation related to netCDF variable names and DAP2 access to reflect the undefined behavior potentially observed when DAP2 reserved keywords are used as netCDF variable names. See [GitHub #308](https://github.com/Unidata/netcdf-c/issues/308) for more information.
* [Bug] Fixed an issue with `nc_inq_type()` not returning proper value in some circumstances.  See [GitHub #317](https://github.com/Unidata/netcdf-c/issues/317) for more information.
* [Bug] Corrected an issue related to test failures when `--disable-utilities` or `-DENABLE_UTILITIES=OFF` are specified when building with autotools or cmake, respectively.  See [GitHub #313](https://github.com/Unidata/netcdf-c/issues/313) for more information.
* [Bug][Enhancement] Corrected a behavioral issue with the `_NCProperties` attribute taking up too much space.  See [GitHub #300](https://github.com/Unidata/netcdf-c/issues/300) and [GitHub #301](https://github.com/Unidata/netcdf-c/pull/301) for more information.

* [Bug] Corrected behavior for `nc-config` so that, if `nf-config` is found in system, the proper fortran-related information will be conveyed.  See [GitHub #296](https://github.com/Unidata/netcdf-c/issues/296] for more information.

## 4.4.1 - June 28, 2016

* [File Change] Starting with release 4.4.1, netCDF-4 files are created with HDF5 v1.6 format compatibility, rather than v1.8.  The superblock changes from version 2, as was observed in previous netCDF versions, to version 0.  This is due to a workaround required to avoid backwards binary incompatibility when using libhdf5 1.10.x or greater.  Superblock versions 0 and 2 appear to be forward and backward compatible for netCDF purposes.  Other than a different superblock number, the data should remain consistent.  See [GitHub #250](https://github.com/Unidata/netcdf-c/issues/250).
* [Enhancement] Added better error reporting when ncdump/nccopy are given a bad constraint in a DAP url. See [GitHub #279](https://github.com/Unidata/netcdf-c/pull/279) for more information.

### 4.4.1-RC3 - June 17, 2016

* [Bug Fix] Misc. bug fixes and improvements.
* [Bug Fix] Corrected an issue where adding a \_FillValue attribute to a variable would result in other attributes being lost. See [GitHub #239](https://github.com/Unidata/netcdf-c/issues/239) for more details.
* [Bug Fix][Parallel I/O] Corrected an issue reported by Kent Yang at the HDF group related to Collective Parallel I/O and a potential hang.

### 4.4.1-RC2 - May 13, 2016

* [Enhancement] Added provenance information to files created.  This information consists of a persistent attribute named `_NCProperties` plus two computed attributes, `_IsNetcdf4` and `_SuperblockVersion`.  Associated documentation was added to the file `docs/attribute_conventions.md`.  See [GitHub pull request #260](https://github.com/Unidata/netcdf-c/pull/260) for more information.
* [Bug Fix] Cleaned up some dead links in the doxygen-generated documentation.
* [Bug Fix] Corrected several issues related to building under Visual Studio 2014.
* [Bug Fix] Corrected several test failures related to HDF5 `1.10.0`
* [Bug Fix] Reverted SOVersion *current* to 11 from 12; it was incorrectly incremented in netCDF-C release 4.4.1-RC1.
* [Enhancement][Windows] Bumped the included libhdf5 to 1.8.16 from 1.8.15 for pre-built Visual Studio installer files.


### 4.4.1-RC1 - April 15, 2016

* [Bug Fix][Enhancement] Fixed an issue with netCDF4 files generated using version `1.10.0` of the HDF5 library.  The 1.10 release potentially changed the underlying file format, introducing a backwards compatibility issue with the files generated.  HDF5 provided an API for retaining the 1.8.x file format, which is now on by default.  See [GitHub Issue #250](https://github.com/Unidata/netcdf-c/issues/250) for more information.
* [Bug Fix] Corrected an issue with autotools-based builds performed out-of-source-tree.  See [GitHub Issue #242](https://github.com/Unidata/netcdf-c/issues/242) for more information.
* [Enhancement] Modified `nc_inq_type()` so that it would work more broadly without requiring a valid ncid.  See [GitHub Issue #240](https://github.com/Unidata/netcdf-c/issues/240) for more information.
* [Enhancement] Accepted a patch code which added a hashmap lookup for rapid var and dim retrieval in nc3 files, contributed by Greg Sjaardema.  See [GitHub Pull Request #238](https://github.com/Unidata/netcdf-c/pull/238) for more information.
* [Bug Fix] Accepted a contributed pull request which corrected an issue with how the cmake-generated `nc-config` file determined the location of installed files. See [GitHub Pull Request #235](https://github.com/Unidata/netcdf-c/pull/235) for more information.
* [Enhancement] Added an advanced option for CMake-based builds, `ENABLE_SHARED_LIBRARY_VERSION`.  This option is `ON` by default, but if turned off, only `libnetcdf.dylib` will be generated, instead of files containing the SOVERSION in the file name.  This is a requested feature most people might not care about.  See [GitHub #228](https://github.com/Unidata/netcdf-c/issues/228) for more information.
* [Bug Fix] Corrected an issue with duplicated error codes defined in multiple header files.  See [GitHub #213](https://github.com/Unidata/netcdf-c/issues/213) for more information.
* [Bug Fix] Addressed an issue specific to Visual Studio 2015 on Windows.  On very large files, some calls to the `fstat` class of functions would fail for no apparent reason. This behavior was **not** observed under Visual Studio 2013. This has now been mitigated.  See [GitHub #188](https://github.com/Unidata/netcdf-c/issues/188) for more information.
* [Enhancement] Updated `nc-config` to report whether `logging` is enabled in netcdf.  Additionally, if `f03` is available in an installed netcdf-fortran library, it will now be reported as well.
* [Bug Fix] Addressed an issue where `netcdf_mem.h` was not being installed by cmake. See [GitHub #227](https://github.com/Unidata/netcdf-c/issues/227) for more information.
* [Bug Fix] Addressed an issue where `ncdump` would crash when trying to read a netcdf file containing an empty ragged `VLEN` variable in an unlimited dimension. See [GitHub #221](https://github.com/Unidata/netcdf-c/issues/221) for more information.

## 4.4.0 Released - January 13, 2016

* Bumped SO version to 11.0.0.

* Modified `CMakeLists.txt` to work with the re-organized cmake configuration used by the latest HDF5, `1.8.16`, on Windows. Before this fix, netCDF would fail to locate hdf5 1.8.16 when using cmake on Windows.  See [GitHub #186](https://github.com/Unidata/netcdf-c/issues/186) for more information.

* Addressed an issue with `ncdump` when annotations were used.  The indices for the last row suffered from an off-by-1 error.  See [GitHub issue #181](https://github.com/Unidata/netcdf-c/issues/181) for more information.

* Addressed an issue on platforms where `char` is `unsigned` by default (such as `ARM`), as well as an issue describing regarding undefined behavior, again on `ARM`.  See [GitHub issue #159](https://github.com/Unidata/netcdf-c/issues/159) for detailed information.

* Fixed an ambiguity in the grammar for cdl files.  See [GitHub #178](https://github.com/Unidata/netcdf-c/issues/178) for more information.

* Updated documentation for `nc_get_att_string()` to reflect the fact that it returns allocated memory which must be explicitly free'd using `nc_free_string()`. Reported by Constantine Khroulev, see [GitHub Issue 171](https://github.com/Unidata/netcdf-c/issues/171) for more information.

* Modified ncgen to properly handle the L and UL suffixes for integer constants
  to keep backward compatibility. Now it is the case the single L suffix
  (e.g. 111L) is treated as a 32 bit integer. This makes it consistent with
  the fact that NC_LONG (netcdf.h) is an alias for NC_INT. Existing .cdl
  files should be examined for occurrences of the L prefix to ensure that
  this change will not affect them.
  (see Github issue 156[https://github.com/Unidata/netcdf-c/issues/156]).

* Updated documentation to reference the new `NodeJS` interface to netcdf4, by Sven Willner.  It is available from [https://www.npmjs.com/package/netcdf4](https://www.npmjs.com/package/netcdf4) or from the GitHub repository at [https://github.com/swillner/netcdf4-js](https://github.com/swillner/netcdf4-js).

* Incorporated pull request https://github.com/Unidata/netcdf-c/pull/150 from Greg Sjaardema to remove the internal hard-wired use of `NC_MAX_DIMS`, instead using a dynamic memory allocation.

### 4.4.0-RC5 Released - November 11, 2015

* Added a fix for https://github.com/Unidata/netcdf-c/issues/149, which was reported several times in quick succession within an hour of the RC4 release.

### 4.4.0-RC4 Released - November 10, 2015

* Added CDM-5 support via new mode flag called NC_64BIT_DATA (alias NC_CDF5).

	Major kudos to Wei-Keng Liao for all the effort he put into getting this to work.

    This cascaded into a number of other changes.

    1. Renamed libsrcp5 -> libsrcp because PnetCDF can do parallel io for CDF-1, CDF-2 and CDF-5, not just CDF-5.
    2. Given #1, then the NC_PNETCDF mode flag becomes a subset of NC_MPIIO, so made NC_PNETCDF an alias for NC_MPII.
    3. NC_FORMAT_64BIT is now deprecated.  Use NC_FORMAT_64BIT_OFFSET.

Further information regarding the CDF-5 file format specifrication may be found here: http://cucis.ece.northwestern.edu/projects/PnetCDF/CDF-5.html

* Modified configure.ac to provide finer control over parallel
  support. Specifically, add flags for:

    1. HDF5_PARALLEL when hdf5 library has parallel enabled
    2. --disable-parallel4 to be used when we do not want
     netcdf-4 to use parallelism even if hdf5 has it enabled.


* Deprecating various extended format flags.

The various extended format flags of the format `NC_FORMAT_FOO` have been refactored into the form `NC_FORMATX_FOO`.  The old flags still exist but have been marked as deprecated and will be removed at some point.  This was done to avoid confusion between the extended format flags and the format flags `NC_FORMAT_CLASSIC`, `NC_FORMAT_64BIT_OFFSET`, etc.  The mapping of deprecated-to-new flags is as follows:

Deprecated | Replaced with
-----------|-------------
NC\_FORMAT\_NC3       | NC\_FORMATX\_NC3
NC\_FORMAT\_NC\_HDF5  | NC\_FORMATX\_NC\_HDF5
NC\_FORMAT\_NC4       | NC\_FORMATX\_NC4
NC\_FORMAT\_NC\_HDF4  | NC\_FORMATX\_NC\_HDF4
NC\_FORMAT\_PNETCDF   | NC\_FORMATX\_PNETCDF
NC\_FORMAT\_DAP2      | NC\_FORMATX\_DAP2
NC\_FORMAT\_DAP4      | NC\_FORMATX\_DAP4
NC\_FORMAT\_UNDEFINED | NC\_FORMATX\_UNDEFINED

* Reduced minimum cmake version to `2.8.11` from `2.8.12`. This will allow for cmake use on a broader set of popular linux platforms without having to do a custom cmake install.  See https://github.com/Unidata/netcdf-c/issues/135 for more information.

* The documentation section `The Default Chunking Scheme` has been updated with more information.  This lives in the `guide.dox` file in the `docs/` directory, or can be found online in the appropriate location (typically https://docs.unidata.ucar.edu/netcdf-c/), once this release has been published.

### 4.4.0-RC3 2015-10-08

* Addressed an inefficiency in how bytes would be swapped when converting between `LITTLE` and `BIG` ENDIANNESS.  See [NCF-338](https://bugtracking.unidata.ucar.edu/browse/NCF-338) for more information.

* Addressed an issue where an interrupted read on a `POSIX` system would return an error even if errno had been properly set to `EINTR`.  This issue was initially reported by David Knaak at Cray.  More information may be found at [NCF-337](https://bugtracking.unidata.ucar.edu/browse/NCF-337).

* Added a note to the install directions pointing out that parallel make
cannot be used for 'make check'.

### 4.4.0-RC2 Released 2015-07-09

* Minor bug fixes and cleanup of issues reported with first release candidate.

### 4.4.0-RC1 Released 2015-06-09

* The pre-built Windows binaries are now built using `Visual Studio 2012`, instead of `Visual Studio 2010`.  Source-code compilation remains function with `Visual Studio 2010`, this is just a change in the pre-built binaries.

* Added support for opening in-memory file content. See `include/netcdf_mem.h` for the procedure signature. Basically, it allows one to fill a chunk of memory with the equivalent of some netCDF file and then open it and read from it as if it were any other file. See [NCF-328](https://bugtracking.unidata.ucar.edu/browse/NCF-328) for more information.

* Addressed an issue when reading hdf4 files with explicit little-endian datatypes. This issue was [reported by Tim Burgess at GitHub](https://github.com/Unidata/netcdf-c/issues/113).  See [NCF-332](https://bugtracking.unidata.ucar.edu/browse/NCF-332) for more information.

* Addressed an issue with IBM's `XL C` compiler on AIX and how it handled some calls to malloc.  Also, as suggested by Wolfgang Hayek, developers using this compiler may need to pass `CPPFLAGS=-D_LINUX_SOURCE_COMPAT` to avoid some test failures.

* Addressed an issure in netcdf4 related to specifying an endianness explicitly.  When specifying an endianness for `NC_FLOAT`, the value would appear to not be written to file, if checked with `ncdump -s`.  The issue was more subtle; the value would be written but was not being read from file properly for non-`NC_INT`.  See [GitHub Issue](https://github.com/Unidata/netcdf-c/issues/112) or [NCF-331](https://bugtracking.unidata.ucar.edu/browse/NCF-331) for more information.

* Addressed an issue in netcdf4 on Windows w/DAP related to how byte values were copied with sscanf.  Issue originally reported by Ellen Johnson at Mathworks, see [NCF-330](https://bugtracking.unidata.ucar.edu/browse/NCF-330) for more information.

* Addressed in issue in netcdf4 files on Windows, built with Microsoft Visual Studio, which could result in a memory leak.  See [NCF-329](https://bugtracking.unidata.ucar.edu/browse/NCF-329) for more information.

* Addressed an issue in netcdf4 files where writing unlimited dimensions that were not declared at head of the dimensions list, as reported by Ellen Johnson at Mathworks.  See [NCF-326](https://bugtracking.unidata.ucar.edu/browse/NCF-326) for more information.

* Added an authorization reference document as oc2/ocauth.html.

* Fixed bug resulting in segmentation violation when trying to add a
  _FillValue attribute to a variable in an existing netCDF-4 file
  defined without it (thanks to Alexander Barth). See
  [NCF-187](https://bugtracking.unidata.ucar.edu/browse/NCF-187) for
  more information.

## 4.3.3.1 Released 2015-02-25

* Fixed a bug related to renaming the attributes of coordinate variables in a subgroup. See [NCF-325](https://bugtracking.unidata.ucar.edu/browse/NCF-325) for more information.

## 4.3.3 Released 2015-02-12

* Fixed bug resulting in error closing a valid netCDF-4 file with a dimension and a non-coordinate variable with the same name. [NCF-324](https://bugtracking.unidata.ucar.edu/browse/NCF-324)

* Enabled previously-disabled shell-script-based tests for Visual Studio when `bash` is detected.

### 4.3.3-rc3 Released 2015-01-14

* Added functionality to make it easier to build `netcdf-fortran` as part of the `netcdf-c` build for *NON-MSVC* builds.  This functionality is enabled at configure time by using the following **Highly Experimental** options:

 * CMake:  `-DENABLE_REMOTE_FORTRAN_BOOTSTRAP=ON`
 * Autotools: `--enable-remote-fortran-bootstrap`

Details are as follows:

----

Enabling these options creates two new make targets:

*  `build-netcdf-fortran`
* `install-netcdf-fortran`

Example Work Flow from netcdf-c source directory:

* $ `./configure --enable-remote-fortran-bootstrap --prefix=$HOME/local`
* $ `make check`
* $ `make install`
* $ `make build-netcdf-fortran`
* $ `make install-netcdf-fortran`

> These make targets are **only** valid after `make install` has been invoked.  This cannot be enforced rigidly in the makefile for reasons we will expand on in the documentation, but in short: `make install` may require sudo, but using sudo will discard environmental variables required when attempting to build netcdf-fortran in this manner.<br><br>

> It is important to note that this is functionality is for *convenience only*. It will remain possible to build `netcdf-c` and `netcdf-fortran` manually.  These make targets should hopefully suffice for the majority of our users, but for corner cases it may still be required of the user to perform a manual build.  [NCF-323](https://bugtracking.unidata.ucar.edu/browse/NCF-323)

----

* Added a failure state if the `m4` utility is not found on non-Windows systems; previously, the build would fail when it reached the point of invoking m4.

* Added an explicit check in the build systems (autotools, cmake) for the CURL-related option `CURLOPT_CHUNK_BGN_FUNCTION`.  This option was introduced in libcurl version `7.21.0`.  On installations which require libcurl and have this version, `CURLOPT_CHUNK_BGN_FUNCTION` will be available. Otherwise, it will not.

* The PnetCDF support was not properly being used to provide mpi parallel io for netcdf-3 classic files. The wrong dispatch table was being used. [NCF-319](https://bugtracking.unidata.ucar.edu/browse/NCF-319)

* In nccopy utility, provided proper default for unlimited dimension in chunk-size specification instead of requiring explicit chunk size. Added associated test. [NCF-321](https://bugtracking.unidata.ucar.edu/browse/NCF-321)

* Fixed documentation typo in FILL_DOUBLE definition in classic format specification grammar. Fixed other typos and inconsistencies in Doxygen version of User Guide.

* For nccopy and ncgen, added numeric options (-3, -4, -6, -7) for output format, to provide less confusing format version specifications than the error-prone equivalent -k options (-k1, -k2, -k3, -k4). The new numeric options are compatible with NCO's mnemonic version options. The old -k numeric options will still be accepted but are deprecated, due to easy confusion between format numbers and format names. [NCF-314](https://bugtracking.unidata.ucar.edu/browse/NCF-314)

* Fixed bug in ncgen. When classic format was in force (k=1 or k=4), the "long" datatype should be treated as int32. Was returning an error. [NCF-318](https://bugtracking.unidata.ucar.edu/browse/NCF-318)

* Fixed bug where if the netCDF-C library is built with the HDF5 library but without the HDF4 library and one attempts to open an HDF4 file, an abort occurs rather than returning a proper error code (NC_ENOTNC). [NCF-317](https://bugtracking.unidata.ucar.edu/browse/NCF-317)

* Added a new option, `NC_EXTRA_DEPS`, for cmake-based builds.  This is analogous to `LIBS` in autotools-based builds.  Example usage:

    $ cmake .. -NC_EXTRA_DEPS="-lcustom_lib"

More details may be found at the Unidata JIRA Dashboard.  [NCF-316](https://bugtracking.unidata.ucar.edu/browse/NCF-316)


### 4.3.3-rc2 Released 2014-09-24

* Fixed the code for handling character constants
  in datalists in ncgen. Two of the problems were:
  1. It failed on large constants
  2. It did not handle e.g. var = 'a', 'b', ...
     in the same way that ncgen3 did.
  See [NCF-309](https://bugtracking.unidata.ucar.edu/browse/NCF-309).

* Added a new file, `netcdf_meta.h`.  This file is generated automatically at configure time and contains information related to the capabilities of the netcdf library.  This file may be used by projects dependent upon `netcdf` to make decisions during configuration, based on how the `netcdf` library was built.  The macro `NC_HAVE_META_H` is defined in `netcdf.h`.  Paired with judicious use of `ifdef`'s, this macro will indicate to developers whether or not the meta-header file is present. See [NCF-313](https://bugtracking.unidata.ucar.edu/browse/NCF-313).

    > Determining the presence of `netcdf_meta.h` can also be accomplished by methods common to autotools and cmake-based build systems.

* Changed `Doxygen`-generated documentation hosted by Unidata to use more robust server-based searching.
* Corrected embedded URLs in release notes.
* Corrected an issue where building with HDF4 support with Visual Studio would fail.

### 4.3.3-rc1 Released 2014-08-25

* Added `CMake`-based export files, contributed by Nico Schlömer. See https://github.com/Unidata/netcdf-c/pull/74.

* Documented that ncgen input can come from standard input.

* Regularized generation of libnetcdf.settings file to make parsing it easier.

* Fixed ncdump bug for char variables with multiple unlimited dimensions and added an associated test.  Now the output CDL properly disambiguates dimension groupings, so that ncgen can generate the original file from the CDL. [NCF-310](https://bugtracking.unidata.ucar.edu/browse/NCF-310)

* Converted the [Manually-maintained FAQ page](https://docs.unidata.ucar.edu/netcdf-c/current/faq.html) into markdown and added it to the `docs/` directory.  This way the html version will be generated when the rest of the documentation is built, the FAQ will be under version control, and it will be in a more visible location, hopefully making it easier to maintain.

* Bumped minimum required version of `cmake` to `2.8.12`.  This was necessitated by the adoption of the new `CMAKE_MACOSX_RPATH` property, for use on OSX.

* Jennifer Adams has requested a reversion in behavior so that all dap requests include a constraint. Problem is caused by change in prefetch where if all variables are requested, then no constraint is generated.  Fix is to always generate a constraint in prefetch.
  [NCF-308](https://bugtracking.unidata.ucar.edu/browse/NCF-308)

* Added a new option for cmake-based builds, `ENABLE_DOXYGEN_LATEX_OUTPUT`.  On those systems with `make` and `pdflatex`, setting this option **ON** will result in pdf versions of the documentation being built.  This feature is experimental.

* Bumped minimum CMake version to `2.8.9` from `2.8.8` as part of a larger pull request contributed by Nico Schlömer. [Pull Request #64](https://github.com/Unidata/netcdf-c/pull/64)

* Replaced the `NetCDF Library Architecture` image with an updated version from the 2012 NetCDF Workshop slides.

* Fix HDF4 files to support chunking.
  [NCF-272](https://bugtracking.unidata.ucar.edu/browse/NCF-272)

* NetCDF creates a `libnetcdf.settings` file after configuration now, similar to those generated by `HDF4` and `HDF5`.  It is installed into the same directory as the libraries. [NCF-303](https://bugtracking.unidata.ucar.edu/browse/NCF-303).


* Renamed `man4/` directory to `docs/` to make the purpose and contents clearer. See [man4 vs. docs #60](https://github.com/Unidata/netcdf-c/issues/60).

* Removed redundant variable `BUILD_DOCS` from the CMake configuration file.  See the issue at github: [#59](https://github.com/Unidata/netcdf-c/issues/59).

* Added missing documentation templates to `man4/Makefile.am`, to correct an issue when trying to build the local `Doxygen`-generated documentation. This issue was reported by Nico Schlömer and may be viewed on github.  [Releases miss Doxygen files #56](https://github.com/Unidata/netcdf-c/issues/56)

* When the NC_MPIPOSIX flag is given for parallel I/O access and the HDF5 library does not have the MPI-POSIX VFD configured in, the NC_MPIPOSIX flag is transparently aliased to the NC_MPIIO flag within the netCDF-4 library.

## 4.3.2 Released 2014-04-23

* As part of an ongoing project, the Doxygen-generated netcdf documentation has been reorganized.  The goal is to make the documentation easier to parse, and to eliminate redundant material.  This project is ongoing.

* The oc .dodsrc reader was improperly handling the user name and password entries. [NCF-299](https://bugtracking.unidata.ucar.edu/browse/NCF-299)

* CTestConfig.cmake has been made into a template so that users may easily specify the location of an alternative CDash-based Dashboard using the following two options:

	* `NC_TEST_DROP_SITE` - Specify an alternative Dashboard by URL or IP address.

	* `NC_CTEST_DROP_LOC_PREFIX` - Specify a prefix on the remote webserver relative to the root directory. This lets CTest accommodate dashboards that do not live at the top level of the web server.

* Return an error code on open instead of an assertion violation for truncated file.

* Documented limit on number of Groups per netCDF-4 file (32767).

### 4.3.2-rc2 Released 2014-04-15

* Cleaned up a number of CMake inconsistencies related to CMake usage, parallel builds.
* Now passing -Wl,--no-undefined to linker when appropriate.
* Corrected an issue preventing large file tests from running correctly under Windows.
* Misc Bug Fixes detected by static analysis.

### 4.3.2-rc1 Released 2014-03-20

* Pre-built Windows downloads will now be bundled with the latest (as of the time of this writing) versions of the various dependencies:
	* `hdf5: 1.8.12`
	* `zlib: 1.2.8`
	* `libcurl: 7.35.0`

* Added a separate flag to enable DAP AUTH tests. These tests are disabled by default.  The flags for autotools and CMAKE-based builds are (respectively):
	* --enable-dap-auth-tests
	* -DENABLE\_DAP\_AUTH\_TESTS

* Fixed small default chunk size for 1-dimensional record variables.  [NCF-211](https://bugtracking.unidata.ucar.edu/browse/NCF-211)

* Cleaned up type handling in netCDF-4 to fix bugs with fill-values.

* Corrected "BAIL" macros to avoid infinite loop when logging is disabled and an error occurs.

* Refactored how types are used for attributes, variables, and committed types, clarifying and categorizing fields in structs, and eliminating duplicated type information between variables and types they use.

* Made type structure shareable by committed datatypes and variables that use it.

* Handled string datatypes correctly, particularly for fill value attributes. Expanded testing for string fill values.

* Simplified iteration of objects in the file when it's opened, tracking fewer objects and using less memory.

* Enabled netCDF-4 bit-for-bit reproducibility for nccopy and other applications (thanks to Rimvydas Jasinskas and Quincey Koziol) by turning off HDF5 object creation, access, and modification time tracking.  [NCF-290](https://bugtracking.unidata.ucar.edu/browse/NCF-290)

* Addressed an issue where `cmake`-based builds would not properly create a `pkg-config` file. This file is now created properly by `cmake`.  [NCF-288](https://bugtracking.unidata.ucar.edu/browse/NCF-288)

* Addressed an issue related to old DAP servers. [NCF-287](https://bugtracking.unidata.ucar.edu/browse/NCF-287)

* Modified nc_{get/put}_vars to no longer use
  nc_get/put_varm. They now directly use nc_get/put_vara
  directly. This means that nc_get/put_vars now work
  properly for user defined types as well as atomic types.
  [NCF-228] (https://bugtracking.unidata.ucar.edu/browse/NCF-228)

## 4.3.1.1 Released 2014-02-05

This is a bug-fix-only release for version 4.3.1.

* Corrected a DAP issue reported by Jeff Whitaker related to non-conforming servers.

* Corrected an issue with DAP tests failing in a 64-bit Cygwin environment. [NCF-286](https://bugtracking.unidata.ucar.edu/browse/NCF-286)

## 4.3.1 Released 2014-01-16

* Add an extended format inquiry method to the netCDF API: nc\_inq\_format\_extended. NC\_HAVE\_INQ\_FORMAT\_EXTENDED is defined in netcdf.h [NCF-273]

[NCF-273]:https://bugtracking.unidata.ucar.edu/browse/NCF-273


### 4.3.1-rc6 Released 2013-12-19

* Fixed fill value handling for string types in nc4\_get\_vara().

* Corrected behavior of nc\_inq\_unlimdim and nv\_inq\_unlimdims to report dimids
  in same order as nc\_inq\_dimids.

* Addressed an issue reported by Jeff Whitaker regarding `nc_inq_nvars` returning an incorrect number of dimensions (this issue was introduced in 4.3.1-rc5).  Integrated a test contributed by Jeff Whitaker.

* A number of previously-disabled unit tests were reviewed and made active.


### 4.3.1-rc5 Released 2013-12-06

* When opening a netCDF-4 file, streamline the iteration over objects in the underlying HDF5 file.

* Fixed netCDF-4 failure when renaming a dimension and renaming a variable using that dimension, in either order. [NCF-177]

[NCF-177]:https://bugtracking.unidata.ucar.edu/browse/NCF-177

* When compiling with `hdf4` support, both autotools and cmake-based builds now properly look for the `libjpeg` dependency and will link against it when found (or complain if it's not).  Also added `ENABLE_HDF4_FILE_TESTS` option to CMake-based builds.

* Fixed bug in ncgen; it was not properly filling empty string constants ("") to be the proper length. [NCF-279]

[NCF-279]:https://bugtracking.unidata.ucar.edu/browse/NCF-279

* Fixed bug in ncgen where it was interpreting int64 constants
  as uint64 constants. [NCF-278]

[NCF-278]:https://bugtracking.unidata.ucar.edu/browse/NCF-278

* Fixed bug in handling Http Basic Authorization. The code was actually there but was not being executed. [NCF-277]

[NCF-277]:https://bugtracking.unidata.ucar.edu/browse/NCF-277

* Added hack to the DAP code to address a problem with the Columbia.edu server. That server does not serve up proper DAP2 DDS replies. The Dataset {...} name changes depending on if the request has certain kinds of constraints. [NCF-276]

[NCF-276]:https://bugtracking.unidata.ucar.edu/browse/NCF-276

* Fixed bugs with ncdump annotation of values, using -b or -f
  options. [NCF-275]

[NCF-275]:https://bugtracking.unidata.ucar.edu/browse/NCF-275


### 4.3.1-rc4 Released 2013-11-06

* Addressed an issue on Windows where `fstat` would report an incorrect file size on files > 4GB.  [NCF-219]


* Added better documentation about accessing ESG datasets.
  See https://docs.unidata.ucar.edu/netcdf-c/current/esg.html.

* Corrected an issue with CMake-based builds enabling HDF4 support where the HDF4 libraries were in a non-standard location.

* Fix bug introduced by [NCF-267] where octal constants above
'\177' were not recognized as proper octal constants. [NCF-271]

[NCF-271]:https://bugtracking.unidata.ucar.edu/browse/NCF-271

* Fixed an issue where the `netcdf.3` man page was not being installed by CMake-based builds. [Github](https://github.com/Unidata/netcdf-c/issues/3)



### 4.3.1-rc3 Released 2013-09-24

* Modify ncgen to support NUL characters in character array
  constants. [NCF-267]

[NCF-267]:https://bugtracking.unidata.ucar.edu/browse/NCF-267

* Modify ncgen to support disambiguating references to
  an enum constant in a data list. [NCF-265]

[NCF-265]:https://bugtracking.unidata.ucar.edu/browse/NCF-265

* Corrected bug in netCDF-4 dimension ID ordering assumptions, resulting in access that works locally but fails through DAP server. [NCF-166]

[NCF-166]:https://bugtracking.unidata.ucar.edu/browse/NCF-166

* Added a new configuration flag, `NC_USE_STATIC_CRT` for CMake-based Windows builds.  The default value is 'OFF'.  This will allow the user to define whether to use the shared CRT libraries (\\MD) or static CRT libraries (\\MT) in Visual Studio builds.

* Ensure netCDF-4 compiles with OpenMPI as an alternative to MPICH2. [NCF-160]

[NCF-160]:https://bugtracking.unidata.ucar.edu/browse/NCF-160

* Addressed issue with hanging Parallel netCDF-4 using HDF5 1.8.10. [NCF-240]

[NCF-240]:https://bugtracking.unidata.ucar.edu/browse/NCF-240

* Addressed issue with Large File Support on Windows, using both 32 and 64-bit builds. [NCF-219]

[NCF-219]:https://bugtracking.unidata.ucar.edu/browse/NCF-219

* Removed deprecated directories:
	* librpc/
	* udunits/
	* libcf/
	* libcdmr/

### 4.3.1-rc2 Released 2013-08-19

* Added `configure` and accompanying configuration files/templates to release repository.  **These will only be added to tagged releases on GitHub**.

* Integrated a fix by Quincey Koziol which addressed a variation of [NCF-250], *Fix issue of netCDF-4 parallel independent access with unlimited dimension hanging*.

[NCF-250]:https://bugtracking.unidata.ucar.edu/browse/NCF-250

* Integrated change contributed by Orion Poplawski which integrated GNUInstallDirs into the netCDF-C CMake system; this will permit systems that install into lib64 (such as Fedora) to `make install` without problem.

* Corrected an error with the CMake config files that resulted in the `netcdf.3` manpage not being built or installed.

### 4.3.1-rc1 Released 2013-08-09

* Migrated from the netCDF-C `subversion` repository to a publicly available GitHub repository available at https://github.com/Unidata/netCDF-C.  This repository may be checked out (cloned) with the following command:

	$ git clone https://github.com/Unidata/netCDF-C.git

* Note: in this release, it is necessary to generate the `configure` script and makefile templates using `autoreconf` in the root netCDF-C directory.:

	$ autoreconf -i -f

* Added `nc_rename_grp` to allow for group renaming in netCDF-4 files. [NCF-204]

[NCF-204]: https://bugtracking.unidata.ucar.edu/browse/NCF-204

* Added a `NC_HAVE_RENAME_GRP` macro to netcdf.h, [as per a request by Charlie Zender][cz1]. This will allow software compiling against netcdf to easily query whether or not nc\_rename\_grp() is available.

[cz1]: https://bugtracking.unidata.ucar.edu/browse/NCF-204

* Added Greg Sjaardema's contributed optimization for the nc4\_find\_dim\_len function in libsrc4/nc4internal.c. The patch eliminates several malloc/free calls that exist in the original coding.

* Added support for dynamic loading, to compliment the dynamic loading support introduced in hdf 1.8.11.  Dynamic loading support depends on libdl, and is enabled as follows: [NCF-258]
	* autotools-based builds: --enable-dynamic-loading
	* cmake-based builds: -DENABLE\_DYNAMIC\_LOADING=ON

[NCF-258]: https://bugtracking.unidata.ucar.edu/browse/NCF-258

* Fix issue of netCDF-4 parallel independent access with unlimited dimension hanging.  Extending the size of an unlimited dimension in HDF5 must be a collective operation, so now an error is returned if trying to extend in independent access mode. [NCF-250]

[NCF-250]: https://bugtracking.unidata.ucar.edu/browse/NCF-250

* Fixed bug with netCDF-4's inability to read HDF5 scalar numeric attributes. Also allow, in addition to zero length strings, a new NULL pointer as a string value. to improve interoperability with HDF5. This required a new CDL constant, 'NIL', that can be output from ncdump for such a string value in an HDF5 or netCDF-4 file. The ncgen utility was also modified to properly handle such NIL values for strings. [NCF-56]

[NCF-56]: https://bugtracking.unidata.ucar.edu/browse/NCF-56

* Parallel-build portability fixes, particularly for OpenMPI and gcc/gfortran-4.8.x on OSX.

* Fix contributed by Nath Gopalaswamy to large file problem reading netCDF classic or 64-bit offset files that have a UINT32_MAX flag for large last record size of a variable that has values larger than 1 byte.  This problem had previously been fixed for *writing* such data, but was only tested with an ncbyte variable.

* Fixed various minor documentation problems.

## 4.3.0 Released 2013-04-29

* fsync: Changed default in autotools config file; fsync must now be
explicitly enabled instead of explicitly disabled. [NCF-239]

[NCF-239]: https://bugtracking.unidata.ucar.edu/browse/NCF-239

* Fixed netCDF-4 bug where odometer code for libdap2 mishandled stride \> 1. Bug reported by Ansley Manke. [NCF-249]

[NCF-249]: https://bugtracking.unidata.ucar.edu/browse/NCF-249

* Fixed netCDF-4 bug so netCDF just ignores objects of HDF5 reference type in
the file, instead of rejecting the file. [NCF-29]

[NCF-29]: https://bugtracking.unidata.ucar.edu/browse/NCF-29

* Fixed netCDF-4 bug with particular order of creation of dimensions,
coordinate variables, and subgroups resulting in two dimensions with the
same dimension ID. [NCF-244]

[NCF-244]: https://bugtracking.unidata.ucar.edu/browse/NCF-244

* Fixed netCDF-4 bug with a multidimensional coordinate variable in a
subgroup getting the wrong dimension IDs for its dimensions. [NCF-247]

[NCF-247]: https://bugtracking.unidata.ucar.edu/browse/NCF-247

* Fixed bug with incorrect fixed-size variable offsets in header getting
written when schema changed for files created by PnetCDF Thanks
to Wei-keng Liao for developing and contributing the fix. [NCF-234]

[NCF-234]: https://bugtracking.unidata.ucar.edu/browse/NCF-234

* Fixed bug in handling old servers that do not do proper Grid to
Structure conversions. [NCF-232]

[NCF-232]: https://bugtracking.unidata.ucar.edu/browse/NCF-232

* Replaced the oc library with oc2.0

* Fix bug with nc\_get\_var1\_uint() not accepting unsigned ints larger
than 2\*\*31. [NCF-226]

[NCF-226]: https://bugtracking.unidata.ucar.edu/browse/NCF-226

* Fix to convert occurrences of '/' in DAP names to %2f. [NCF-223]

[NCF-223]: https://bugtracking.unidata.ucar.edu/browse/NCF-223

* Fix bug in netCDF-4 with scalar non-coordinate variables with same name
as dimensions. [NCF-222]

[NCF-222]: https://bugtracking.unidata.ucar.edu/browse/NCF-222

* Fix bug in which calling netCDF-4 functions in which behavior that
should not depend on order of calls sometimes produces the wrong
results. [NCF-217]

[NCF-217]: https://bugtracking.unidata.ucar.edu/browse/NCF-217

* Merged in nccopy additions from Martin van Driel to support -g and -v
options for specifying which groups or variables are to be copied.
[NCF-216]

[NCF-216]: https://bugtracking.unidata.ucar.edu/browse/NCF-216

* Merged in PnetCDF bugs fixes from Greg Sjaardema. [NCF-214]

[NCF-214]: https://bugtracking.unidata.ucar.edu/browse/NCF-214

* Modify ncgen so that if the incoming file has a special attribute, then
it is used to establish the special property of the netcdf file, but the
attribute is not included as a real attribute in the file. [NCF-213].

[NCF-213]: https://bugtracking.unidata.ucar.edu/browse/NCF-213

* Added library version info to the user-agent string so that the server
logs will be more informative. [NCF-210]

[NCF-210]: https://bugtracking.unidata.ucar.edu/browse/NCF-210

* Added work around for bad servers that sometimes sends DAP dataset with
duplicate field names. [NCF-208]

[NCF-208]: https://bugtracking.unidata.ucar.edu/browse/NCF-208

* Fixed bug with strided access for NC\_STRING type. [NCF-206]

[NCF-206]: https://bugtracking.unidata.ucar.edu/browse/NCF-206

* Prevented adding an invalid \_FillValue attribute to a variable (with
nonmatching type or multiple values), to avoid later error when any
record variable is extended. [NCF-190]

[NCF-190]: https://bugtracking.unidata.ucar.edu/browse/NCF-190

* Fix bug in which some uses of vlen within compounds causes HDF5 errors.
[NCF-155]

[NCF-155]: https://bugtracking.unidata.ucar.edu/browse/NCF-155

* Fixed ncdump bug in display of data values of variables that use
multiple unlimited dimensions. [NCF-144]

[NCF-144]: https://bugtracking.unidata.ucar.edu/browse/NCF-144

* Fix bug in which interspersing def\_var calls with put\_var calls can
lead to corrupt metadata in a netCDF file with groups and inherited
dimensions. [NCF-134]

[NCF-134]: https://bugtracking.unidata.ucar.edu/browse/NCF-134

* Building shared libraries works with DAP and netCDF4 functionality.
[NCF-205] [NCF-57]

[NCF-205]: https://bugtracking.unidata.ucar.edu/browse/NCF-205
[NCF-57]: https://bugtracking.unidata.ucar.edu/browse/NCF-57

* 32-and-64-bit builds are working under MinGW on Windows. [NCF-112]

[NCF-112]: https://bugtracking.unidata.ucar.edu/browse/NCF-112

* Config.h for Windows compiles are included in the build. [NCF-98]

[NCF-98]: https://bugtracking.unidata.ucar.edu/browse/NCF-98

* NetCDF-4 dependency on NC\_MAX\_DIMS has been removed. [NCF-71]

[NCF-71]: https://bugtracking.unidata.ucar.edu/browse/NCF-71

* 64-bit DLL's are produced on Windows. [NCF-65]

[NCF-65]: https://bugtracking.unidata.ucar.edu/browse/NCF-65

* DLL Packaging issues are resolved. [NCF-54]

[NCF-54]: https://bugtracking.unidata.ucar.edu/browse/NCF-54

* The CMake build system (with related ctest and cdash systems for
testing) has been integrated into netCDF-C. This allows for Visual
Studio-based builds in addition to gcc-based builds. This requires at
least CMake version 2.8.8. This replaces/supplements the cross-compiled
set of Visual-Studio compatible netCDF libraries introduced in netCDF
4.2.1-rc1.

## 4.2.1.1 Released 2012-08-03

* Patched libdap2/ncdap3.c to fix DAP performance bug remotely accessing large files (> 2GiB).

* Patched ncdump/dumplib.c to properly escape special characters in CDL output from ncdump for netCDF-4 string data.


### 4.2.1 Released 2012-07-18

* Added a specific NC\_MMAP mode flag to modify behavior of NC\_DISKLESS.

* Changed the file protections for NC\_DISKLESS created files to 0666
[NCF-182]

* Fixed ncdump to report error when an unsupported option is specified.
[NCF-180]

* Fixed documentation of CDL char constants in Users Guide and ncgen man
page.

* Fixed memory leak detected by valgrind in one of the HDF5 tests.

* Fixed problem with \#elif directives in posixio.c revealed by PGI
compilers.

### 4.2.1-rc1 Released 2012-06-18

* Ported static and shared libraries (DLL's) for both 32- and 64-bit
Windows, including support for DAP remote access, with netCDF-3 and
netCDF-4/HDF5 support enabled. The environment for this build is
MSYS/MinGW/MinGW64, but the resulting DLLs may be used with Visual
Studio. [NCF-112] [NCF-54] [NCF-57] [NCF-65]

* Implemented diskless files for all netCDF formats. For nc\_create(),
diskless operation performs all operations in memory and then optionally
persists the results to a file on close. For nc\_open(), but only for
netcdf classic files, diskless operation caches the file in-memory,
performs all operations on the memory resident version and then writes
all changes back to the original file on close.
[NCF-110][NCF-109][NCF-5]

* Added MMAP support. If diskless file support is enabled, then it is
possible to enable implementation of diskless files using the operating
system's MMAP facility (if available). The enabling flag is
"--enable-mmap". This is most useful when using nc\_open() and when only
parts of files, a single variable say, need to be read.

* Added configure flag for --disable-diskless.

* Added nccopy command-line options to exploit diskless files, resulting
in large speedups for some operations, for example converting unlimited
dimension to fixed size or rechunking files for faster access. Upgraded
doxygen and man-page documentation for ncdump and nccopy utilities,
including new -w option for diskless nccopy, with an example.

* Modified Makefile to allow for concurrent builds and to support builds
outside the source tree, e.g. 'mkdir build; cd build;
SOURCE-DIR/configure' where SOURCE-DIR is the top-level source
directory.

* Fixed some netCDF-4 bugs with handling strings in non-netCDF-4 HDF5
files. [NCF-150]

* Fixed bug using nccopy to compress with shuffling that doesn't compress
output variables unless they were already compressed in the input file.
[NCF-162]

* Fixed bug in 64-bit offset files with large records, when last record
variable requires more than 2\*\*32 bytes per record. [NCF-164]

* Fix bug in which passing a NULL path to nc\_open causes failure.
[NCF-173]

* Fixed ncgen bugs in parsing and handling opaque data.

* Fixed ncdump bug, not escaping characters special to CDL in enumeration
labels. [NCF-169]

* Fixed bug reading netCDF int into a C longlong or writing from longlong
to external int on 32-bit platforms with classic format files. The upper
32 bits of the longlong were not cleared on read or used on write.
[NCF-171]

* Resolved some erroneous returns of BADTYPE errors and RANGE errors due
to conflating C memory types with external netCDF types when accessing
classic or 64-bit offset files. [NCF-172]

* Fixed bug with ncdump -t interpreting unit attribute without base time
as a time unit. [NCF-175]

* Changed port for testing remote access test server to increase
reliability of tests.

* Modified ncio mechanism to support multiple ncio packages, so that it is
possible to have e.g. posixio and memio operating at the same time.

* Generation of documentation is disabled by default. Use --enable-doxygen
to generate. [NCF-168]

* Added description of configure flags to installation guide.

* Clarified documentation of arguments to nc**open() and nc**create() and
their default values.

* Fixed doxygen installation guide source file to preserve line breaks in
code and scripts. [NCF-174]

* Cleaned up a bunch of lint issues (unused variables, etc.) and some
similar problems reported by clang static analysis.

* Updated and fixed pkg-config source file netcdf.pc.in to work with
separated netCDF language-specific packages. Also fixed nc-config to
call nf-config, ncxx-config, and ncxx4-config for for backward
compatibility with use of nc-config in current Makefiles. [NCF-165]
[NCF-179]

## 4.2.0 2012-05-01

* Completely rebuilt the DAP constraint handling. This primarily affects
users who specify a DAP constraint as part of their URL. [NCF-120]

* Fixed cause of slow nccopy performance on file systems with many records
and large disk block size or many record variables, by accessing data a
record at a time instead of a variable at a time. [NCF-142]

* Performance improvement to DAP code to support fetching partial
variables into the cache; especially important when using nc\_get\_var()
API. A partial variable is one that has ranges attached to the
projection variables (e.g. x[1:10][20:21]) [NCF-157]

* Separate the Fortran and C++ libraries and release the C library and
ncdump/ncgen/nccopy without Fortran or C++. [NCF-24]

* Documentation mostly migrated to Doxygen, from Texinfo. [NCF-26]

* Properly convert vara start/count parameters to DAP [NCF-105][NCF-106]

* Fixed major wasted space from previous default variable chunk sizes
algorithm. [NCF-81]

* Fixed bug in nccopy, in which compression and chunking options were
ignored for netCDF-4 input files. [NCF-79]

* Fixed bug in ncgen in which large variables (more than 2**18 elements)
duplicates the first 2**18 values into subsequent chunks of data
[NCF-154].

* Applied Greg Sjaardema's nccopy bug fix, not compressing output
variables f they were not already using compression on the input file
when shuffle specified. [NCF-162]

* Fixed problem when a URL is provided that contains only a host name.
[NCF-103]

* Fixed behavior of ncgen flags so that -o =\> -lb and, in the absence of
any other markers, make the default be -k1 [NCF-158]

* Created a text INSTALL file for netCDF-4.2 release. [NCF-161]

* Fixed bug in ncgen for vlen arrays as fields of compound types where
datalists for those types was improperly interpreted [NCF-145] (but see
NCF-155).

* Improve use of chunk cache in nccopy utility, making it practical for
rechunking large files. [NCF-85]

* Fixed nccopy bug copying a netCDF-4 file with a chunksize for an
unlimited dimension that is larger than the associated dimension size.
[NCF-139]

* Fixed nccopy bug when rechunking a netCDF-4 file with a chunkspec option
that doesn't explicitly specify all dimensions. [NCF-140]

* Fixed bug in netCDF-4 files with non-coordinate variable with the same
name as a dimension. [NCF-141]

* Incorporated Wei Huang's fix for bug where netCDF-4 sometimes skips over
too many values before adding fill values to an in-memory buffer.
[NCF-143]

* Fixed ncgen bug with netCDF-4 variable-length constants (H/T to Lynton
Appel). [NCF-145]

* Incorporated Peter Cao's performance fixes using HDF5 link iterator for
any group with many variables or types. [NCF-148]

* Incorporated Constantine Khroulev's bug fix for invalid usage of
MPI\_Comm\_f2c in nc\_create\_par. [NCF-135]

* Fixed turning off fill values in HDF5 layers when NOFILL mode is set in
netCDF-4 API (thanks to Karen Schuchardt). [NCF-151]

* Fixed bug with scalar coordinate variables in netCDF-4 files, causing
failure with --enable-extra-tests [NCF-149]

* Cleaned up the definition and use of nulldup. [NCF-92][NCF-93][NCF-94]

* Fixed various '\#include' bugs. [NCF-91][NCF-96][NCF-127]

* v2 API functions modified to properly call the external API instead of
directly calling the netcdf-3 functions. [NCF-100]

* Fixed problem with 64-bit offset format where writing more than 2\*\*31
records resulted in erroneous NC\_EINVALCOORDS error. [NCF-101]

* Restored original functionality of ncgen so that a call with no flags,
only does the syntax check. [NCF-104]

* Corrected misc. test bugs [NCF-107]

* Modified ncdump to properly output various new types (ubyte, ushort,
uint, int64, and uint64). [NCF-111]

* Fixed incorrect link flag for szip in configure.ac [NCF-116]

* ncdump -t now properly parses ISO "T" separator in date-time strings.
[NCF-16]

* ncdump -t "human time" functionality now available for attributes and
bounds variables [NCF-70]

* In ncdump, add -g option to support selection of groups for which data
is displayed. [NCF-11]

* Now supports bluefire platform [NCF-52]

* ncdump now properly displays values of attributes of type NC\_USHORT as
signed shorts [NCF-82]

* Rename some code files so that there are no duplicate filenames.
[NCF-99]

* Demonstration of netCDF-4 Performance Improvement with KNMI Data
[NCF-113]

* Dimension size in classic model netCDF-4 files now allows larger sizes
than allowed for 64-bit offset classic files. [NCF-117]

* ncdump now reports correct error message when "-x" option specifying
NcML output is used on netCDF-4 enhanced model input. [NCF-129]

* Fixed bug causing infinite loop in ncdump -c of netCDF-4 file with
subgroup with variables using inherited dimensions. [NCF-136]

## 4.1.3 2011-06-17

* Replace use of --with-hdf5= and other such configure options that
violate conventions and causes build problems. Set environment variables
CPPFLAGS, LDFLAGS, and LD\_LIBRARY\_PATH instead, before running
configure script. [NCF-20]

* Detect from configure script when szlib is needed [NCF-21]

* Fix bug that can silently zero out portions of a file when writing data
in nofill mode beyond the end of a file, crossing disk-block boundaries
with region to be written while in-memory buffer is in a specific state.
This bug was observed disabling fill mode using Lustre (or other large
blksize file system) and writing data slices in reverse order on disk.
[NCF-22]

* Fix bug that prevents netCDF-4/HDF5 files created with netCDF-4.1.2 from
being read by earlier versions of netCDF or HDF5 versions before 1.8.7.
[NCF-23]

* Fix bug in configure that did not make the search for the xdr library
depend on --enable-dap. [NCF-41]

* Fix ncgen bug that did not use the value of a \_Format attribute in the
input CDL file to determine the kind of output file created, when not
specified by the -k command-line flag. [NCF-42]

* Fix ncgen bug, not properly handling unsigned longlong parsing. [NCF-43]

* Fix DAP client code to suppress variables with names such as "x.y",
which DAP protocol interprets as variable "y" inside container "x". Such
variables will be invisible when accessed through DAP client. [NCF-47]

* Define uint type for unsigned integer, if not otherwise available.
Symptom was compile error involving uint in putget.c. [NCF-49]

* Fix username+password handling in the DAP client code. [NCF-50]

* Add test for handling parallel I/O problem from f77 when user forgets to
turn on one of the two MPI flags. [NCF-60]

* Resolved "make check" problems when ifort compiler. Some "make install"
problems remain when using MPI and shared libraries. [NCF-61]

* Fix problem with f90\_def\_var not always handle deflate setting when
compiler was ifort. [NCF-67]

* Check that either MPIIO or MPIPOSIX flag is set when parallel create or
open is called. Also fix examples that didn't set at least one of these
flags. [NCF-68]

* Improve documentation on handling client-side certificates [NCF-48]

* Document that array arguments, except in varm functions, must point to
contiguous blocks of memory. [NCF-69]

* Get netCDF-4 tests working for DLLs generated with mingw. [NCF-6]

* Make changes necessary for upgrading to HDF5 1.8.7 [NCF-66]

### 4.1.3-rc1 2011-05-06

* Stop looking for xdr if --disable-dap is used.

* Don't try to run (some) fortran configure tests on machines with no
fortran.

* Allow nccopy to rechunk with chunksizes larger than current dimension
lengths.

* Initial implementation of CDMREMOTE is complete; needs comprehensive
testing.

### 4.1.3-beta1 2011-04-29

* Fixed szlib not linking bug.

* Fixed dreaded "nofill bug", lurking in netCDF classic since at least
1999. Writing more than a disk block's worth of data that crossed disk
block boundaries more than a disk block beyond the end of file in nofill
mode could zero out recently written earlier data that hadn't yet been
flushed to disk.

* Changed setting for H5Pset\_libver\_bounds to ensure that all netCDF-4
files can be read by HDF5 1.8.x.

* Merged libncdap3 and libncdap4 into new libdap2 library. The suffix dap2
now refers to the dap protocol. This is in prep for adding dap4 protocol
support.

* Took out --with-hdf5 and related options due to high cost of maintaining
this non-standard way of finding libraries.

## 4.1.2 2011-03-29

* Changes in build system to support building dlls on cygwin/mingw32.

* Changes to fix portability problems and get things running on all test
platforms.

* Some minor documentation fixes.

* Fixed opendap performance bug for nc\_get\_vars; required adding
nc\_get\_var{s,m} to the dispatch table.

* Now check for libz in configure.ac.

* Fixed some bugs and some performance problems with default chunksizes.

### 4.1.2-beta2 2011-01-11

* Add "-c" option to nccopy to specify chunk sizes used in output in terms
of list of dimension names.

* Rewrite netCDF-4 attribute put code for a large speedup when writing
lots of attributes.

* Fix nc-config --libs when static dependent libraries are not installed
in the same directory as netCDF libraries (thanks to Jeff Whitaker).

* Build shared libraries by default, requiring separate Fortran library.
Static libraries now built only with --disable-shared.

* Refactor of HDF5 file metadata scan for large speedup in opening files,
especially large files.

* Complete rewrite of the handling of character datalist constants. The
heuristics are documented in ncgen.1.

* Eliminate use of NC\_MAX\_DIMS and NC\_MAX\_VARS in ncdump and nccopy,
allocating memory as needed and reducing their memory footprint.

* Add documentation for new nc\_inq\_path() function.

* Use hashing to speedup lookups by name for files with lots of dimensions
and variables (thanks to Greg Sjaardema).

* Add options to nccopy to support uniform compression of variables in
output, shuffling, and fixing unlimited dimensions. Documented in
nccopy.1 man page and User's Guide.

### 4.1.2-beta1 2010-07-09

* Fix "ncdump -c" bug identifying coordinate variables in groups.

* Fix bug in libsrc/posixio.c when providing sizehint larger than default,
which then doesn't get used (thanks to Harald Anlauf).

* Fix netCDF-4 bug caused when doing enddef/redef and then defining
coordinate variable out of order.

* Fixed bug in man4 directory automake file which caused documentation to
be rebuilt after make clean.

* Turned off HDF5 caching when parallel I/O is in use because of its
memory use.

* Refactoring of netCDF code with dispatch layer to decide whether to call
netCDF classic, netCDF-4, or opendap version of a function.

* Refactoring of netCDF-4 memory internals to reduce memory use and end
dependence on NC\_MAX\_DIMS and NC\_MAX\_NAME.

* Modified constraint parser to be more compatible with a java version of
the parser.

* Modified ncgen to utilize iterators internally; should be no user
visible effect.

* Fixed two large-file bugs with using classic format or 64-bit offset
format and accessing multidimensional variables with more than 2\*\*32
values.

## 4.1.1 2010-04-01

* Fixed various build issues.

* Fixed various memory bugs.

* Fixed bug for netCDF-4 files with dimensions and coord vars written in
different orders, with data writes interspersed.

* Added test for HDF5-1.8.4 bug.

* Added new C++ API from Lynton Appel.

## 4.1 2010-01-30

* Much better memory leak checking with valgrind.

* Added per-variable chunk cache control for better performance. Use
nc\_set\_var\_chunk\_cache / nf\_set\_var\_chunk\_cache /
nf90\_set\_var\_chunk\_cache to set the per-variable cache.

* Automatically set per-variable chunk cache when opening a file, or
creating a variable, so that the cache is big enough for more than one
chunk. (Can be overridden by user). Settings may be changed with
configure options --max-default-chunk-size and
--default-chunks-in-cache.

* Better default chunks size. Now chunks are sized to fit inside the
DEFAULT\_CHUNK\_SIZE (settable at configure time with
--with-default-chunk-size= option.)

* Added nccopy utility for converting among netCDF format variants or to
copy data from DAP servers to netCDF files.

* The oc library has been modified to allow the occurrence of alias
definitions in the DAS, but they will be ignored.

* The old ncgen has been moved to ncgen3 and ncgen is now the new ncgen4.

* Modified --enable-remote-tests to be on by default.

* Fixed the nc\_get\_varm code as applied to DAP data sources.

* Added tests for nc-config.

* Many documentation fixes.

* Added capability to use the PnetCDF library to
perform parallel I/O on classic and 32-bit offset files. Use the
NC\_PNETCDF mode flag to get parallel I/O for non-netcdf-4 files.

* Added libcf library to netCDF distribution. Turn it on with configure
option --with-libcf.

* Added capability to read HDF4 files created with the SD (Scientific
Data) API.

* The DAP support was revised to closely mimic the original libnc-dap
support.

* Significantly revised the data handling mechanism in ncgen4 to more
closely mimic the output from the original ncgen.

* Added prototype NcML output capability to ncgen4. It is specified by the
-lcml flag.

* Added capability to read HDF5 files without dimension scales. This will
allow most existing HDF5 datasets to be read by netCDF-4.

* Fixed bug with endianness of default fill values for integer types when
variables are created with a non-native endianness and use the default
fill value.

* Significant refactoring of HDF5 type handling to improve performance and
handle complicated nesting of types in cross-platform cases.

* Added UDUNITS2 to the distribution. Use --with-udunits to build udunits
along with netcdf.

* Made changes suggested by HDF5 team to relax creation-order requirement
(for read-only cases) which allows HDF5 1.6.x files to be retrofitted
with dimension scales, and be readable to netCDF-4.

* Handle duplicate type names within different groups in ncdump. Fix group
path handling in absolute and relative variable names for "-v" option.

* Added nc-config shell script to help users build netCDF programs without
having to figure out all the compiler options they will need.

* Fixed ncdump -s bug with displaying special attributes for classic and
64-bit offset files.

* For writers, nc\_sync() now calls fsync() to flush data to disk sooner.

* The nc\_inq\_type() function now works for primitive types.

## 4.0.1 2009-03-26

* Added optional arguments to F90 API to nf90\_open/create,
nf90\_create\_var, and nf90\_inquire\_variable so that all netCDF-4
settings may be accomplished with optional arguments, instead of
separate function calls.

* Added control of HDF5 chunk cache to allow for user performance tuning.

* Added parallel example program in F90.

* Changed default chunking to better handle very large variables.

* Made contiguous the default for fixed size data sets with no filters.

* Fixed bug in nc\_inq\_ncid; now it returns NC\_ENOGRP if the named group
is not found.

* Fixed man pages for C and F77 so that netCDF-4 builds will result in man
pages that document new netCDF-4 functions.

* Added OPeNDAP support based on a new C-only implementation. This is
enabled using --enable-dap option and requires libcurl. The configure
script will attempt to locate libcurl, but if it fails, then its
location must be specified by the --with-curl option.

### 4.0.1-beta2 2008-12-26

* Changed chunksizes to size\_t from int.

* Fixed fill value problem from F77 API.

* Fixed problems in netcdf-4 files with multi-dimensional coordinate
variables.

* Fixed ncgen to properly handle CDL input that uses Windows line endings
("\r\n"), instead of getting a syntax error.

* Added "-s" option to ncdump to display performance characteristics of
netCDF-4 files as special virtual attributes, such as \_Chunking,
\_DeflateLevel, \_Format, and \_Endianness.

* Added "-t" option to ncdump to display times in human readable form as
strings. Added code to interpret "calendar" attribute according to CF
conventions, if present, in displaying human-readable times.

* Added experimental version of ncgen4 capable of generating netcdf-4 data
files and C code for creating them. In addition, it supports the special
attributes \_Format, etc.

* 4.0.1-beta1 2008-10-16

* Fixed Fortran 90 int64 problems.

* Rewrote HDF5 read/write code in accordance with performance advice from
Kent.

* Fixed memory leaks in gets/puts of HDF5 data.

* Fixed some broken tests for parallel I/O (i.e. MPI) builds.

* Fixed some cross-compile problems.

* Rewrote code which placed bogus errors on the HDF5 error stack, trying
to open non-existent attributes and variables. Now no HDF5 errors are
seen.

* Removed man subdirectory. Now man4 subdirectory is used for all builds.

* Changed build so that users with access to parallel make can use it.

* Added experimental support for accessing data through OPeNDAP servers
using the DAP protocol (use --enable-opendap to build it).

* Fixed ncdump bugs with array field members of compound type variables.
Fixed ncdump bug of assuming default fill value for data of type
unsigned byte.

## 4.0 2008-05-31

* Introduced the use of HDF5 as a storage layer, which allows use of
groups, user-defined types, multiple unlimited dimensions, compression,
data chunking, parallel I/O, and other features. See the netCDF Users
Guide for more information.

## 3.6.3 2008-05-31

* In ncdump and ncgen, added CDL support for UTF-8 encoding of characters
in names and for escaped special chars in names. Made sure UTF-8 names
are normalized using NFC rules before storing or comparing.

* Handle IEEE NaNs and infinities in a platform-independent way in ncdump
output.

* Added support for ARM representation of doubles, (thanks to Warren
Turkal).

* Fixed bug in C++ API creating 64-bit offset files. (See
https://docs.unidata.ucar.edu/netcdf-c/current/known_problems.html#cxx_64-bit).

* Fixed bug for variables larger than 4 GB. (See
https://docs.unidata.ucar.edu/netcdf-c/current/known_problems.html#large_vars_362).

* Changed the configure.ac to build either 3.6.x or 4.x build from the
same configure.ac.

* Build now checks gfortran version and handles it cleanly, also Portland
Group in Intel fortran, with various configurations.

* A Fortran netcdf.inc file is now created at build time, based on the
setting of --disable-v2.

* Documentation has been fixed in several places.

* Upgraded to automake 1.10, autoconf 2.62, and libtool 2.2.2.

* Includes missing Windows Visual Studio build files.

* Fixed missing include of config.h in a C++ test program.

* Fixed maintainer-clean in man directory.

* Fixed --enable-c-only and make check.

* Fixed behavior when opening a zero-length file.

* Many portability enhancements to build cleanly on various platforms.

* Turned on some old test programs which were not being used in the build.

## 3.6.2 2007-03-05

* Released.

### 3.6.2 beta6 2007-01-20

* Fine tuning of build system to properly handle cygwin, Mingw, and
strange configuration issues.

* Automake 1.10 has a problem with running our tests on MinGW, so I'm
switching back to automake 1.9.6 for this release.

### 3.6.2 beta5 2006-12-30

* Now netCDF configuration uses autoconf 2.61, and automake 1.10. (Thanks
to Ralf Wildenhues for the patches, and all the autotools help in
general!)

* Final major revision of netCDF tutorial before the 3.6.2 release.

* Now netCDF builds under MinGW, producing a windows DLL with the C and
F77 APIs. Use the --enable-shared --enable-dll --disable-cxx
--disable-f90 flags to configure. (C++ and F90 have never been built as
windows DLLs, but might be in a future release if there is user
interest). This has all been documented in the netCDF Porting and
Installation Guide.

* Now extreme numbers (i.e. those close to the limits of their type) can
be turned off in nc\_test/nf\_test, with --disable-extreme-numbers. It
is turned off automatically for Solaris i386 systems.

* Added --enable-c-only option to configure. This causes only the core
netCDF-3 C library to be built. It's the same as --disable-f77
--disable-cxx --disable-v2 --disable-utilities.

* Added --disable-utilities to turn off building and testing of
ncgen/ncdump.

* Fix a long-standing bug in nf90\_get\_att\_text() pointed out by Ryo
Furue, to make sure resulting string is blank-padded on return. This is
fixed in the Fortran-90 interface, but is impractical to fix in the
Fortran-77 interface implemented via cfortran.h.

* Now large file tests are run if --enable-large-file-tests is used in the
configure.

* For Cray users, the ffio module is used if the --enable-ffio option is
passed to configure.

* Unrolled loops in byte-swapping code used on little-endian platforms to
reduce loop overhead. This optimization resulted in a 22% speedup for
some applications accessing floats or ints (e.g. NCO utilities ncap and
ncbo) and a smaller speedup for shorts or doubles.

* Added "-k" option to ncdump and ncgen, for identifying and specifying
the kind of netCDF file, one of "classic", "64-bit-offset", "hdf5", or
"hdf5-nc3". Removed output of kind of netCDF file in CDL comment
produced by ncdump.

* Fixed bug of ncdump seg-faulting if invoked incorrectly with option like
"-c" or "-h" but no file name.

### 3.6.2 beta4 2006-08-15

* Changed F77/F90 man pages from netcdf.3f and netcdf.3f90 to
netcdf\_f77.3 and netcdf\_f90.3. Also fixed broken install of man pages.

* Changed configure script so that "-g -O2" is no longer set as CFLAGS,
CXXFLAGS, and FFLAGS by default if a GNU compiler is being used. Now
nothing is set.

* Changed configure script so that fortran flag is set in config.h.

* Updated Installation and Porting Guide, C++ Interface Guide, F77 and F90
Interface Guides.

* Build with static libraries by default.

* Added configure option --enable-separate-fortran, which causes the
fortran library to be built separately. This is turned on automatically
for shared libraries.

* Improved clarity of error messages.

* Changed configuration to get cygwin DLL and mingw DLL builds working,
for the C library only (i.e. no F77, F90, or C++ APIs).

* Changed type of ncbyte in C++ interface from unsigned char to signed
char, for consistency with C interface. The C++ documentation warned
this change would eventually occur.

* Changed the C++ interface to use only the netCDF-3 C interface instead
of the older netCDF-2 C interface. This has the added benefit that
on-the-fly numeric conversions are now supported using get methods, for
example you can get data of any type as double. When using --disable-v2
flag to configure, the C++ interface can now be built and installed.

### 3.6.2 beta3 2006-05-24

* Changed to use default prefix of /usr/local instead of package-based
prefix of previous releases of netCDF. Use the --prefix argument to the
configure script to override the default.

* Made separate fortran library file, instead of appending fortran library
functions to the C library file, if --enable-separate-fortran is used
during configure (it's turned on automatically if --enable-shared is
used). If uses, the fortran API users must link to *both* the C library
and the new fortran library, like this: -lnetcdff -lnetcdf

* Added netCDF examples in C, C++, F77, F90, and CDL. See the examples
subdirectory.

* Added the NetCDF Tutorial.

* Minor fixes to some of the netCDF documentation.

* Made it possible to build without V2 API using --disable-v2 from
configure.

* Switched to new build system, with automake and libtool. Now shared
libraries are built (as well as static ones) on platforms which support
it. For more information about shared libraries, see
https://docs.unidata.ucar.edu/netcdf-c/current/faq.html#shared_intro

* Fixed ncdump crash that happened when no arguments were used.

* Fixed for building with gfortran 4.1.0.

* Important fix for machines whose SIZEOF\_SIZE\_T != SIZEOF\_LONG, such
as NEC-SX, thanks to Stephen Leak.

* Fixed C++ on AIX platform.

* Fixed 64-bit builds on AIX platform.

* Removed bad assertion that could be triggered in rare cases when reading
a small file.

* Added comments in v1hpg.c to clarify purpose of each internal function.

* Make sure filesize is determined in nc\_close() *after* buffers get
flushed.

* Fix long-standing problem resulting in files up to 3 bytes longer than
necessary if there is exactly one record variable of type byte, char, or
short and if the number of values per record for that variable is not
divisible by 4 (or 2 in the case of short). Now the filesize determined
from header info by NC\_calcsize should be correct in all cases.

## 3.6.1 2006-01-31

* Updated installation manual for 3.6.1.

* Changed installation to try to provide correct compiler flags for
compiling in 64-bit mode on Sun, Irix, AIX, and HPUX. (HPUX doesn't work
for me, however). Now run configure with --enable-64bit to get a 64 bit
compile.

* Fixed long-standing bug that would cause small netCDF files to be padded
on the end with zero bytes to 4096 bytes when they were opened and
changed. Now small files should stay small after you change a value.

* Fixed bug in assertions in putget.c that would only be noticed if you
change the manifest constant NC\_MAX\_DIMS in netcdf.h to be different
from NC\_MAX\_VAR\_DIMS.

* Moved test ftest.F from fortran to nf\_test directory, and fixed bug in
ftest.F which caused it to return 0 even if tests failed (no tests were
failing, however). Also renamed some test output files to make things a
little clearer.

* If open for writing, pad with up to 3 extra zero bytes before close to
the correct canonical length, calculated from the header. Previously
files could be short due to not padding when writing in NOFILL mode.

* Doubled arbitrary limits on number of dimensions, variables, attributes,
and length of names.

* Change name of nc\_get\_format() to nc\_inq\_format(). Add analogous
interfaces for nf\_inq\_format(), nf90\_inquire(), and
NcFile::get\_format() to f77, f90, and C++ interfaces. Document new
function in texinfo files. Add minimal test to nc\_test, nf\_test.

### 3.6.1-beta3 2005-02-17

* Added function nc\_get\_format(int ncid, int\* formatp) that returns
either NC\_FORMAT\_CLASSIC or NC\_FORMAT\_64BIT for a CDF1 or CDF2 file,
respectively.

* Added test to nc\_test that detects whether format version was changed
after a file is reopened and define mode is entered.

* Correctly configure for Intel ifort Fortran compiler on Linux.

### 3.6.0-p1 2005-02-18

* Fixed bug that changes CDF2 files to CDF1 files if CDF2 file is reopened
for write access and either an attribute is changed or define mode is
entered.

### 3.6.1-beta2 2005-1-6

* Fixed absoft compile problem. Maybe.

### 3.6.1-beta1 2005-1-3

* Fixed Cygwin C++ problem.

* Fixed large file problem in MS Visual C++.NET environment.

* More information in installation and porting guide.

## 3.6.0 2004-12-16

* Added texinfo source for the documentation.

* Added large file tests to Windows directory in distribution.

* Modified win32 visual studio project files so that m4 is no longer
required to build netcdf under visual studio.

* Modified rules.make to use install instead of cp, fixing install problem
for cygwin users.

* Modified configure/install stuff to support HP-UX.

* Modified configure/install stuff to support G95.

* In the f90 interface, applied Arnaud Desitter's fixes to correct
mismatches between scalar and array arguments, eliminating (legitimate)
complaints by the NAGWare f95 compiler. Also fixed bugs introduced in
3.6.0-beta5 in the mapped array interfaces.

### 3.6.0-beta6 2004-10-05

* Fixed AIX 64-bit/largefile install problems.

* Removed FAQ section from netcdf.texi User's Guide, in deference to
online version that can be kept up to date more easily.

### 3.6.0-beta5 2004-10-04

* Fixed assertion violation on 64-bit platforms when size of last fixed
size variable exceeds 2\^32 - 1.

* Removed another restriction on file size by making record size (derived
from other sizes, not part of the format) an off\_t instead of a
size\_t, when an off\_t is larger than a size\_t. This permits records
to be *much* larger in either classic format or 64-bit-offset format.

* Incorporated patch from Mathis Rosenhauer to improve performance of
Fortran 90 interface for calls to nf90\_put\_var\_TYPE(),
nf90\_get\_var\_TYPE(), nf90\_put\_vara\_TYPE(), and
nf90\_get\_vara\_TYPE() functions by not emulating them with the
corresponding nf90\_put\_varm\_TYPE() and nf90\_get\_varm\_TYPE() calls.

* Added tests for invalid offsets in classic format when defining multiple
large variables.

* Improved installation ease. Have configure script use Large File Support
as a default, if available.

* Add "extra\_test" as a target for testing Large File Support.

### 3.6.0-beta3 2004-08-24

* Upgraded to recent autoconf, changed configure to (hopefully) improve
installation. Also added macros to deal with large file systems.

* Added nf\_set\_default\_format to Fortran interface.

* Added testing to the set\_default\_format functions to nc\_test and
nf\_test.

* Added documentation to the man page for set\_default\_format functions.

* Added two new error return codes to C, f77, and f90 interfaces for
invalid dimension size and for bad variable size. Made test for max
dimension size depend on whether 64-bit offsets used. Fixed bug with
dimension sizes between 2\^31 and 2\^32 (for byte variables).

* Fixed ncdump to properly print dimensions larger than 2\^31.

* Fixed ncgen to properly handle dimensions between 2\^31 and 2\^32.

### 3.6.0-beta2

* Added -v2 (version 2 format with 64-bit offsets) option to
ncgen, to specify that generated files or generated C/Fortran code
should create 64-bit offset files. Also added -x option to ncgen to
specify use of no-fill mode for fast creation of large files.

* Added function to set default create mode to C interface
(nc\_set\_default\_create).

* Added win32 directory, with NET subdirectory to hold .NET port of
netCDF. To use, open netcdf.sln with Visual Studio, and do a clean and
then a build of either the debug or release builds. Tests will be run as
part of the build process. VC++ with managed extensions is required
(i.e. VC++.NET).

* Added windows installer files to build windows binary installs.

### 3.6.0-beta1

* By incorporating Greg Sjaardema's patch, added support for
64-bit offset files, which remove many of the restrictions relating to
very large files (i.e. larger than 2 GB.) This introduces a new data
format for the first time since the original netCDF format was
introduced. Files in this new 64-bit offset format can't be read by
earlier versions of netCDF. Users should continue to use the netCDF
classic format unless they need to create very large files.

* The test suite, nc\_test, will now be run twice, once for netCDF classic
format testing, and once for 64-bit offset format testing.

* The implementation of the Fortran-77 interface has been adapted to
version 4.3 of Burkhard Burow's "cfortran.h".

### 3.6.0-alpha

* Added NEC SX specific optimization for NFILL tunable
parameter in libsrc/putget.c

Added support for the ifc Fortran-90 compiler creating files "netcdf.d"
and "typesizes.d" (instead of ".mod" files).

* Fixed access to iargc and getarg functions from Fortran-90 for NAG f90
compiler, contributed by Harald Anlauf.

## 3.5.1 2004-02-03

* Updated INSTALL.html for Mac OS X (Darwin).

* Made the installation of the netCDF Fortran-90 module file more robust
regarding the name of the file.

* Added support for eight-byte integers in Fortran90 interface.

* Increased advisory limits in C netcdf.h and Fortran netcdf.inc for
maximum number of dimensions, variables, and attributes.

* Changed C++ declarations "friend NcFile" to "friend class NcFile" in
cxx/netcdfcpp.h to conform to standard.

* Added Dan Schmitt's backward compatible extension to the C++ record
interface to work with arbitrary dimension slices.

* Added C++ documentation note that caller is responsible for deleting
pointer returned by Variable::values() method when no longer needed.

* Made C++ interface more standard; the result may not compile on some old
pre-standard C++ compilers.

* Fixed bug in ncgen when parsing values of a multidimensional char
variable that resulted in failure to pad a value with nulls on IRIX.

* Fixed ncdump bug adding extra quote to char variable data when using -fc
or -ff option.

* Fixed so compiling with -DNO\_NETCDF\_2 will work for building without
backward-compatibility netCDF-2 interfaces.

* Eliminated use of ftruncate(), because it fails on FAT32 file systems
under Linux.

* Initialized a pointer in putget.m4 (used to generate putget.c) that was
involved in uninitialized memory references when nc\_test is run under
Purify. Two users had reported seeing crashes resulting from this
problem in their applications.

* Reverted pointer initializations in putget.m4, after testing revealed
these caused a performance problem, resulting in many extra calls to
px\_pgin and px\_pgout when running nc\_test.

* Added checking of size of "dimids" vector in function
nf90\_inquire\_variable(...) and error-returning if it isn't
sufficiently capacious.

* Added variable index to ncvarget() and ncattinq() error messages and
attribute name to ncattinq() error message.

* Tweaked configure script to work with recent C++ compilers.

* Fixed a memory leak in C++ interface, making sure NcVar::cur\_rec[] gets
deleted in NcVar destructor.

* Reimplemented nc\_sync() fix of version 3.5.0 to eliminate performance
penalty when synchronization is unnecessary.

* Changed order of targets in Makefile to build Fortran interface last, as
a workaround for problem with make on AIX platforms.

## 3.5.0 2001-03-23

* Added Fortran 90 interface.

* Changed C macro TIMELEN in file cxx/nctst.cpp to TIMESTRINGLEN to avoid
clash with macro defined on AIX systems in /usr/include/time.h.

* Fixed miswriting of netCDF header when exiting define mode. Because the
header was always written correctly later, this was only a problem if
there was another reader of the netCDF file.

* Fixed explicit synchronizing between netCDF writer and readers via the
nc\_sync(), nf\_sync(), and ncsync() functions.

* Fixed a number of bugs related to attempts to support shrinking the
header in netCDF files when attributes are rewritten or deleted. Also
fixed the problem that nc\_\_endef() did not work as intended in
reserving extra space in the file header, since the extra space would be
compacted again on calling nc\_close().

* Fixed the "redef bug" that occurred when nc\_enddef() or nf\_enddef() is
called after nc\_redef() or nf\_redef(), the file is growing such that
the new beginning of a record variable is in the next "chunk", and the
size of at least one record variable exceeds the chunk size (see
netcdf.3 man page for a description of this tuning parameter and how to
set it). This bug resulted in corruption of some values in other
variables than the one being added.

* The "\*\*" tuning functions for the Fortran interface, nf\*\*create,
nf\*\*open, and nf\*\*enddef, are now documented in the Fortran interface
man pages.

* Add an 'uninstall' target to all the Makefiles. Dave Glowacki
<dglo@SSEC.WISC.EDU> 199810011851.MAA27335

* Added support for multiprocessing on Cray T3E. Hooks added by Glenn, but
the majority of the work was done at NERSC. Also includes changes to
ffio option specification. Patch rollup provided by R. K. Owen
<rkowen@Nersc.GOV>. The following functions are added to the public
interface. nc**create\_mp() nc**open\_mp() nc\_set\_base\_pe()
nc\_inq\_base\_pe()

* Fixed makefile URL for Win32 systems in INSTALL file.

* Made test for UNICOS system in the configure script case independent.

* Ported to the following systems: AIX 4.3 (both /bin/xlc and
/usr/vac/bin/xlc compilers) IRIX 6.5 IRIX64 6.5

* Changed the extension of C++ files from ".cc" to ".cpp". Renamed the C++
interface header file "netcdfcpp.h" instead of "netcdf.hh", changing
"netcdf.hh" to include "netcdfcpp.h" for backward compatibility.

* Treat "FreeBSD" systems the same as "BSD/OS" system w.r.t. Fortran and
"whatis" database.

* Corrected manual pages: corrected spelling of "enddef" (was "endef") and
ensured that the words "index" and "format" will be correctly printed.

* Updated support for Fortran-calling-C interface by updating
"fortran/cfortran.h" from version 3.9 to version 4.1. This new version
supports the Portland Group Fortran compiler (C macro "pgiFortran") and
the Absoft Pro Fortran compiler (C macro "AbsoftProFortran").

* Corrected use of non-integral-constant-expression in specifying size of
temporary arrays in file "libsrc/ncx\_cray.c".

* Added Compaq Alpha Linux workstation example to INSTALL file.

* Ported cfortran.h to Cygnus GNU Win32 C compiler (gcc for Windows).

* Fixed bug in ncdump using same CDL header name when called with multiple
files.

* Added new NULL data type NC\_NAT (Not A Type) to facilitate checking
whether a variable object has had its type defined yet, for example when
working with packed values.

* Fixed use of compile-time macro NO\_NETCDF\_2 so it really doesn't
include old netCDF-2 interfaces, as intended.

* Ported to MacOS X Public Beta (Darwin 1.2/PowerPC).

* Fixed C++ friend declarations to conform to C++ standard.

* Changed INSTALL file to INSTALL.html instead.

## 3.4 1998-03-09

* Fixed ncx\_cray.c to work on all CRAY systems, not just CRAY1. Reworked
USE\_IEG, which was incorrect. Reworked short support. Now USE\_IEG and
otherwise both pass t\_ncx.

* To better support parallel systems, static and malloc'ed scratch areas
which were shared in the library were eliminated. These were made
private and on the stack where possible. To support this, the macros
ALLOC\_ONSTACK and FREE\_ONSTACK are defined in onstack.h.

* The buffered i/o system implementation in posixio.c was reimplemented to
limit the number and size of read() or write() system calls and use
greater reliance on memory to memory copy. This saves a great deal of
wall clock time on slow (NFS) filesystems, especially during
nc\_endef().

* Added performance tuning "underbar underbar" interfaces nc**open(),
nc**create(), and nc\_\_enddef().

* The 'sizehint' contract between the higher layers and the ncio layer is
consistently enforced.

* The C++ interface has been updated so that the deprecated "nclong"
typedef should no longer be required, and casts to nclong no longer
necessary. Just use int or long as appropriate. nclong is still
supported for backwards compatibility.

* The ncdump utility now displays byte values as signed, even on platforms
where the type corresponding to a C char is unsigned (SGI, for example).
Also the ncdump and ncgen utilities have been updated to display and
accept byte attributes as signed numeric values (with a "b" suffix)
instead of using character constants.

* In libsrc/error.c:nc\_strerror(int), explain that NC\_EBADTYPE applies
to "\_FillValue type mismatch".

* Some changes to configure scripts (aclocal.m4), macros.make.in and
ncgen/Makefile to support NEC SUPER-UX 7.2.

* The "usage" messages of ncgen and ncdump include the string returned
from nc\_inq\_libvers().

* Corrected some casts in the library so that all phases of the arithmetic
computing file offsets occurs with "off\_t" type. This allows certain
larger netcdf files to be created and read on systems with larger
(64bit) off\_t.

* In ncgen, multidimensional character variables are now padded to the
length of last dimension, instead of just concatenating them. This
restores an undocumented but convenient feature of ncgen under netCDF-2.
Also, a syntax error is now reliably reported if the netcdf name is
omitted in CDL input.

* Fortran and C code generated by ncgen for netCDF components whose names
contain "-" characters will now compile and run correctly instead of
causing syntax errors.

* The library allows "." characters in names as well as "\_" and "-"
characters. A zero length name "" is explicitly not allowed. The ncgen
utility will now permit "." characters in CDL names as well.

* Memory leaks in the C++ interface NcVar::as\_\*() member functions and
NcFile::add\_var() member function are fixed. The documentation was
fixed where it indicated incorrectly that the library managed value
blocks that the user is actually responsible for deleting.

* he values of the version 2 Fortran error codes have been modified to
make the version 2 Fortran interface more backward compatible at the
source level.

* Added support for systems whose Fortran INTEGER*1 and INTEGER*2 types
are equivalent to the C "long" type but whose C "int" and "long" types
differ. An example of such a system is the NEC SX-4 with the "-ew"
option to the f90 compiler (sheesh, what a system!).

* Fixed Version 2 Fortran compatibility bug: NCVGTG, NCVGGC, NCVPTG, and
NCVPGC didn't work according to the Version 2 documentation if the
innermost mapping value (i.e. IMAP[1]) was zero (indicating that the
netCDF structure of the variable should be used).

## 3.3.1 1997-06-16

* One can now inquire about the number of attributes that a variable has
using the global variable ID.

* The FORTRAN interface should now work on more systems. In particular:

* It should now work with FORTRAN compilers whose "integer*1" datatype is
either a C "signed char", "short", or "int" and whose "integer*2"
datatype is either a C "short" or "int".

* It should now work with FORTRAN compilers that are extremely picky about
source code formatting (e.g. the NAG f90 compiler).

* The dependency on the non-POSIX utility m4(1) for generating the C and
FORTRAN manual pages has been eliminated.

* EXTERNAL statements have been added to the FORTRAN include-file
"netcdf.inc" to eliminate excessive warnings about "unused" variables
(which were actually functions) by some compilers (e.g. SunOS 4.1.3's
f77(1) version 1.x).

* Building the netCDF-3 package no longer requires the existence of the
Standard C macro RAND\_MAX.

* Fixed an ncdump bug resulting in ncdump reporting Attempt to convert
between text & numbers when \_FillValue attribute of a character
variable set to the empty string "".

* Made ncgen tests more stringent and fixed various bugs this uncovered.
These included bugs in handling byte attributes on platforms on which
char is unsigned, initializing scalar character variables in generated C
code under "-c" option, interspersing DATA statements with declaration
statements in generated Fortran code under "-f" option, handling empty
string as a value correctly in generated C and Fortran, and handling
escape characters in strings. The Fortran output under the "-f" option
was also made less obscure and more portable, using automatic conversion
with netCDF-3 interfaces instead of "BYTE", "INTEGER*1", or "INTEGER*2"
declarations.

* Fixed a C++ interface problem that prevented compiling the C++ library
with Digital's cxx compiler.

* Made ncgen "make test" report failure and stop if test resulted in a
failure of generated C or Fortran code.

* The file that you are now reading was created to contain a high-level
description of the evolution of the netCDF-3 package.

## 3.3 1997-05-15

* The production version of the netCDF-3 package was released.

* A comparison of the netCDF-2 and netCDF-3 releases can be found in the
file COMPATIBILITY.

*/
