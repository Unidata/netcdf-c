# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

#####
# Add target objects/modules based on options.
#####
target_sources(netcdf
  PRIVATE
    $<TARGET_OBJECTS:netcdf3>
    $<TARGET_OBJECTS:dispatch>
)

if(USE_NETCDF4)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:netcdf4>
  )
endif()

if(USE_HDF5)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:netcdfhdf5>
  )
endif()

if(USE_PNETCDF)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:netcdfp>
  )
endif()

if(USE_HDF4)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:netcdfhdf4>
  )
endif()

if(ENABLE_DAP2)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:oc2>
      $<TARGET_OBJECTS:dap2>
  )
endif()

if(ENABLE_DAP4)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:dap4>
      $<TARGET_OBJECTS:ncxml>
  )
endif()

if(ENABLE_NCZARR)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:nczarr>
  )
endif()

if(ENABLE_S3_INTERNAL)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:ncxml>
  )
endif()

if(ENABLE_PLUGINS)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:ncpoco>
  )
endif()

target_sources(netcdf
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/nc_initialize.c 
)

if(STATUS_PARALLEL)
  # TODO: Make PUBLIC when other dependencies have PUBLIC/PRIVATE specified
  target_link_libraries(netcdf MPI::MPI_C)
endif(STATUS_PARALLEL)

if(TARGET MPI::MPI_C)
  target_link_libraries(netcdf MPI::MPI_C)
endif(TARGET MPI::MPI_C)

if(MOD_NETCDF_NAME)
  set_target_properties(netcdf PROPERTIES LIBRARY_OUTPUT_NAME ${NETCDF_LIB_NAME})
  set_target_properties(netcdf PROPERTIES ARCHIVE_OUTPUT_NAME ${NETCDF_LIB_NAME})
  set_target_properties(netcdf PROPERTIES RUNTIME_OUTPUT_NAME ${NETCDF_LIB_NAME})
endif()

# Make sure that netcdf.dll path does not include the build configuration
if(MSVC)
  set_target_properties(netcdf PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
endif()

#####
# Add dependencies required for linking.
#####

set(TLL_LIBS "")

set(TLL_LIBS ${TLL_LIBS} ${HAVE_LIBM} ${ZLIB_LIBRARY})

# Add extra dependencies specified via NC_EXTRA_DEPS
set(TLL_LIBS ${TLL_LIBS} ${EXTRA_DEPS})

if(ENABLE_NCZARR_ZIP)
  set(TLL_LIBS ${TLL_LIBS} ${Zip_LIBRARIES})
endif()

if(Blosc_FOUND)
  set(TLL_LIBS ${TLL_LIBS} ${Blosc_LIBRARIES})
endif()
if(Zstd_FOUND)
  set(TLL_LIBS ${TLL_LIBS} ${Zstd_LIBRARIES})
endif()
if(Bz2_FOUND)
  set(TLL_LIBS ${TLL_LIBS} ${Bz2_LIBRARIES})
endif()
if(SZIP_FOUND)
  set(TLL_LIBS ${TLL_LIBS} ${Szip_LIBRARY})
endif()

if(HAVE_LIBDL)
  set(TLL_LIBS ${LIBDL} ${TLL_LIBS})
endif()

if(ENABLE_NCZARR_ZIP)
  set(TLL_LIBS ${TLL_LIBS} ${Zip_LIBRARIES})
endif()

if(USE_HDF5)
  if(NOT MSVC)
    # Some version of cmake define HDF5_hdf5_LIBRARY instead of
    # HDF5_LIBRARY. Same with HDF5_HL_LIBRARIES
    if(HDF5_hdf5_LIBRARY AND NOT HDF5_C_LIBRARIES)
      set(HDF5_C_LIBRARIES ${HDF5_hdf5_LIBRARY})
    endif()
    if(HDF5_hdf5_hl_LIBRARY AND NOT HDF5_HL_LIBRARIES)
      set(HDF5_HL_LIBRARIES ${HDF5_hdf5_hl_LIBRARY})
    endif()
    # The order of the libraries is important here for static
    # builds:
    # Make sure that HDF5_C_LIBRARY appears *after*
    # HDF5_HL_LIBRARY.
    set(TLL_LIBS ${HDF5_HL_LIBRARIES} ${HDF5_C_LIBRARIES} ${HDF5_LIBRARIES} ${TLL_LIBS} ${Szip_LIBRARY})
  else() # Windows CMake defines HDF5_LIBRARIES.
    set(TLL_LIBS ${HDF5_LIBRARIES} ${TLL_LIBS} ${Szip_LIBRARY})
  endif()
endif()

if(FOUND_CURL)
  set(TLL_LIBS ${TLL_LIBS} ${CURL_LIBRARIES})
endif()

if(USE_HDF4)
  set(TLL_LIBS ${HDF4_MFHDF_LIB} ${HDF4_DF_LIB} ${TLL_LIBS})
endif()

if(ENABLE_PNETCDF AND PNETCDF)
  set(TLL_LIBS ${TLL_LIBS} ${PNETCDF})
endif()


if(ENABLE_S3)
  if(ENABLE_S3_AWS)
    TARGET_LINK_DIRECTORIES(netcdf PUBLIC ${AWSSDK_LIB_DIR})
    set(TLL_LIBS ${AWSSDK_LINK_LIBRARIES} ${TLL_LIBS})
  elseif(ENABLE_S3_INTERNAL)
    if(OPENSSL_FOUND)
      set(TLL_LIBS ${OPENSSL_SSL_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARIES} ${TLL_LIBS})
    endif(OPENSSL_FOUND)
  endif(ENABLE_S3_AWS)  
endif(ENABLE_S3)

if(HAVE_LIBXML2)
  set(TLL_LIBS ${TLL_LIBS} ${LIBXML2_LIBRARIES})
endif()

if(NOT WIN32)
  if(NOT APPLE)
    if(CMAKE_DL_LIBS)
      target_link_libraries(netcdf ${CMAKE_DL_LIBS})
    endif()
  endif()
endif()

if(TLL_LIBS)
  list(REMOVE_DUPLICATES TLL_LIBS)
endif()

target_link_libraries(netcdf ${TLL_LIBS})

set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${TLL_LIBS})
if(MSVC)
  set_target_properties(netcdf PROPERTIES
    LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
    )
endif()

if(NOT MSVC)
  if(BUILD_SHARED_LIBS)
    set_target_properties(netcdf PROPERTIES LINK_FLAGS -shared)
  endif()
endif()

if(ENABLE_SHARED_LIBRARY_VERSION)
  set_target_properties(netcdf PROPERTIES
    VERSION ${netCDF_LIB_VERSION}
    SOVERSION ${netCDF_SO_VERSION}
    )
endif(ENABLE_SHARED_LIBRARY_VERSION)

###
# The INCLUDES directive was introduced in 2.8.12.
# However, 2.8.11 is still the default on some
# popular linux distributions. See
# https://github.com/Unidata/netcdf-c/issues/135
# for more information.
###
if(${CMAKE_VERSION} VERSION_LESS 2.8.12)
  install(
    TARGETS netcdf EXPORT netCDFTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libraries
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
else()
    install(
    TARGETS netcdf EXPORT netCDFTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libraries
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

set(ALL_TLL_LIBS ${TLL_LIBS} PARENT_SCOPE)
set(NC_LIBS ${NC_LIBS} PARENT_SCOPE)

file(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
set(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")

# [ 10%] Linking C shared library libnetcdf.dylib
# /opt/homebrew/Cellar/cmake/3.27.1/bin/cmake -E cmake_link_script CMakeFiles/netcdf.dir/link.txt --verbose=1
# /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc  
#   -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -g -Wall -Wshorten-64-to-32 -Wconversion -arch arm64 
#   -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.0.sdk -dynamiclib -Wl,-headerpad_max_install_names 
#   -shared -L/opt/homebrew/opt/llvm/lib/c++ -Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++ -compatibility_version 19.0.0 -current_version 19.0.0 
#   -o libnetcdf.19.dylib -install_name @rpath/libnetcdf.19.dylib 
#   CMakeFiles/netcdf.dir/liblib/nc_initialize.c.o libsrc4/CMakeFiles/netcdf4.dir/nc4dispatch.c.o libsrc4/CMakeFiles/netcdf4.dir/nc4attr.c.o 
#   libsrc4/CMakeFiles/netcdf4.dir/nc4dim.c.o libsrc4/CMakeFiles/netcdf4.dir/nc4grp.c.o libsrc4/CMakeFiles/netcdf4.dir/nc4internal.c.o libsrc4/CMakeFiles/netcdf4.dir/nc4type.c.o libsrc4/CMakeFiles/netcdf4.dir/nc4var.c.o libsrc4/CMakeFiles/netcdf4.dir/ncfunc.c.o libsrc4/CMakeFiles/netcdf4.dir/ncindex.c.o libsrc4/CMakeFiles/netcdf4.dir/nc4cache.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/nc4hdf.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/nc4info.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5file.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5attr.c.o 
#   libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5dim.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5grp.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5type.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5internal.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5create.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5open.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5var.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/nc4mem.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/nc4memcb.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5dispatch.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5filter.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5set_format_compatibility.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/hdf5debug.c.o libhdf5/CMakeFiles/netcdfhdf5.dir/H5FDhttp.c.o 
#   oc2/CMakeFiles/oc2.dir/oc.c.o oc2/CMakeFiles/oc2.dir/daplex.c.o oc2/CMakeFiles/oc2.dir/dapparse.c.o oc2/CMakeFiles/oc2.dir/dapy.c.o oc2/CMakeFiles/oc2.dir/occompile.c.o oc2/CMakeFiles/oc2.dir/occurlfunctions.c.o oc2/CMakeFiles/oc2.dir/ocdata.c.o oc2/CMakeFiles/oc2.dir/ocdebug.c.o oc2/CMakeFiles/oc2.dir/ocdump.c.o oc2/CMakeFiles/oc2.dir/ocinternal.c.o oc2/CMakeFiles/oc2.dir/ocnode.c.o oc2/CMakeFiles/oc2.dir/ochttp.c.o oc2/CMakeFiles/oc2.dir/ocread.c.o oc2/CMakeFiles/oc2.dir/ocutil.c.o oc2/CMakeFiles/oc2.dir/xxdr.c.o 
#   libdap2/CMakeFiles/dap2.dir/constraints.c.o libdap2/CMakeFiles/dap2.dir/dapcvt.c.o libdap2/CMakeFiles/dap2.dir/dapodom.c.o libdap2/CMakeFiles/dap2.dir/daputil.c.o libdap2/CMakeFiles/dap2.dir/ncdaperr.c.o libdap2/CMakeFiles/dap2.dir/cdf.c.o libdap2/CMakeFiles/dap2.dir/cache.c.o libdap2/CMakeFiles/dap2.dir/dapdump.c.o libdap2/CMakeFiles/dap2.dir/dapdebug.c.o libdap2/CMakeFiles/dap2.dir/dapattr.c.o libdap2/CMakeFiles/dap2.dir/ncd2dispatch.c.o libdap2/CMakeFiles/dap2.dir/getvara.c.o libdap2/CMakeFiles/dap2.dir/dceconstraints.c.o libdap2/CMakeFiles/dap2.dir/dcetab.c.o libdap2/CMakeFiles/dap2.dir/dceparse.c.o libdap2/CMakeFiles/dap2.dir/dcelex.c.o 
#   libdap4/CMakeFiles/dap4.dir/d4curlfunctions.c.o libdap4/CMakeFiles/dap4.dir/d4fix.c.o libdap4/CMakeFiles/dap4.dir/d4data.c.o libdap4/CMakeFiles/dap4.dir/d4file.c.o libdap4/CMakeFiles/dap4.dir/d4parser.c.o libdap4/CMakeFiles/dap4.dir/d4meta.c.o libdap4/CMakeFiles/dap4.dir/d4varx.c.o libdap4/CMakeFiles/dap4.dir/d4dump.c.o libdap4/CMakeFiles/dap4.dir/d4swap.c.o libdap4/CMakeFiles/dap4.dir/d4chunk.c.o libdap4/CMakeFiles/dap4.dir/d4printer.c.o libdap4/CMakeFiles/dap4.dir/d4read.c.o libdap4/CMakeFiles/dap4.dir/d4http.c.o libdap4/CMakeFiles/dap4.dir/d4util.c.o libdap4/CMakeFiles/dap4.dir/d4odom.c.o libdap4/CMakeFiles/dap4.dir/d4cvt.c.o libdap4/CMakeFiles/dap4.dir/d4debug.c.o libdap4/CMakeFiles/dap4.dir/ncd4dispatch.c.o 
#   libncxml/CMakeFiles/ncxml.dir/ncxml_xml2.c.o 
#   libnczarr/CMakeFiles/nczarr.dir/zarr.c.o libnczarr/CMakeFiles/nczarr.dir/zattr.c.o libnczarr/CMakeFiles/nczarr.dir/zxcache.c.o libnczarr/CMakeFiles/nczarr.dir/zchunking.c.o libnczarr/CMakeFiles/nczarr.dir/zclose.c.o libnczarr/CMakeFiles/nczarr.dir/zcreate.c.o libnczarr/CMakeFiles/nczarr.dir/zcvt.c.o libnczarr/CMakeFiles/nczarr.dir/zdim.c.o libnczarr/CMakeFiles/nczarr.dir/zdispatch.c.o libnczarr/CMakeFiles/nczarr.dir/zfile.c.o libnczarr/CMakeFiles/nczarr.dir/zfilter.c.o libnczarr/CMakeFiles/nczarr.dir/zgrp.c.o libnczarr/CMakeFiles/nczarr.dir/zinternal.c.o libnczarr/CMakeFiles/nczarr.dir/zmap.c.o libnczarr/CMakeFiles/nczarr.dir/zmap_file.c.o libnczarr/CMakeFiles/nczarr.dir/zodom.c.o libnczarr/CMakeFiles/nczarr.dir/zopen.c.o libnczarr/CMakeFiles/nczarr.dir/zprov.c.o libnczarr/CMakeFiles/nczarr.dir/zsync.c.o libnczarr/CMakeFiles/nczarr.dir/ztype.c.o libnczarr/CMakeFiles/nczarr.dir/zutil.c.o libnczarr/CMakeFiles/nczarr.dir/zvar.c.o libnczarr/CMakeFiles/nczarr.dir/zwalk.c.o libnczarr/CMakeFiles/nczarr.dir/zdebug.c.o 
#   libncpoco/CMakeFiles/ncpoco.dir/ncpoco.c.o libncpoco/CMakeFiles/ncpoco.dir/cp_unix.c.o  
#   /opt/homebrew/lib/libhdf5_hl.dylib /opt/homebrew/lib/libhdf5.dylib 
#   /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.0.sdk/usr/lib/libm.tbd 
#   /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.0.sdk/usr/lib/libz.tbd 
#   /opt/homebrew/lib/libzstd.dylib 
#   /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.0.sdk/usr/lib/libbz2.tbd 
#   /opt/homebrew/lib/libsz.dylib 
#   /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.0.sdk/usr/lib/libcurl.tbd 
#   /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.0.sdk/usr/lib/libxml2.tbd
