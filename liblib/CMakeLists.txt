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

if(FOUND_CURL)
  target_link_libraries(netcdf PRIVATE CURL::libcurl)
endif()

if(NETCDF_ENABLE_DAP2)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:oc2>
      $<TARGET_OBJECTS:dap2>
  )
endif()

if(NETCDF_ENABLE_DAP4)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:dap4>
      $<TARGET_OBJECTS:ncxml>
  )
endif()

if(NETCDF_ENABLE_NCZARR)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:nczarr>
  )
endif()

if(NETCDF_ENABLE_S3_INTERNAL)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:ncxml>
  )
endif()

if(NETCDF_ENABLE_PLUGINS)
  target_sources(netcdf
    PRIVATE
      $<TARGET_OBJECTS:ncpoco>
  )
endif()

target_sources(netcdf
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/nc_initialize.c 
)

if(STATUS_PARALLEL)
  target_link_libraries(netcdf PUBLIC MPI::MPI_C)
endif(STATUS_PARALLEL)

if(MOD_NETCDF_NAME)
  set_target_properties(netcdf 
    PROPERTIES 
      OUTPUT_NAME ${NETCDF_LIB_NAME}
  )
endif()

# Make sure that netcdf.dll path does not include the build configuration
#if(MSVC)
#  set_target_properties(netcdf PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
#    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
#endif()

#####
# Add dependencies required for linking.
#####

set(TLL_LIBS "")

set(TLL_LIBS ${TLL_LIBS} ${HAVE_LIBM} ${ZLIB_LIBRARY})

# Add extra dependencies specified via NC_EXTRA_DEPS
set(TLL_LIBS ${TLL_LIBS} ${EXTRA_DEPS})

if(NETCDF_ENABLE_NCZARR_ZIP)
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

if(NETCDF_ENABLE_NCZARR_ZIP)
  set(TLL_LIBS ${TLL_LIBS} ${Zip_LIBRARIES})
endif()

if(USE_HDF5)
  set(TLL_LIBS HDF5::HDF5 hdf5::hdf5_hl ${TLL_LIBS} ${Szip_LIBRARY})
endif()

if(FOUND_CURL)
  set(TLL_LIBS ${TLL_LIBS} CURL::libcurl)
endif()

if(USE_HDF4)
  set(TLL_LIBS ${HDF4_LIBRARIES} ${TLL_LIBS})
endif()

if(NETCDF_ENABLE_PNETCDF)
  SET(TLL_LIBS ${TLL_LIBS} PNETCDF::PNETCDF)
endif()


if(NETCDF_ENABLE_S3)
  if(NETCDF_ENABLE_S3_AWS)
    target_link_directories(netcdf PUBLIC ${AWSSDK_LIB_DIR})
    set(TLL_LIBS ${AWSSDK_LINK_LIBRARIES} ${TLL_LIBS})
  elseif(NETCDF_ENABLE_S3_INTERNAL)
    if(OPENSSL_FOUND)
      set(TLL_LIBS ${OPENSSL_SSL_LIBRARIES} ${OPENSSL_CRYPTO_LIBRARIES} ${TLL_LIBS})
    endif(OPENSSL_FOUND)
  endif(NETCDF_ENABLE_S3_AWS)  
endif(NETCDF_ENABLE_S3)

if(HAVE_LIBXML2)
  set(TLL_LIBS ${TLL_LIBS} ${LIBXML2_LIBRARIES})
endif()

if(NOT WIN32)
  if(NOT APPLE)
    if(CMAKE_DL_LIBS)
      target_link_libraries(netcdf PRIVATE ${CMAKE_DL_LIBS})
    endif()
  endif()
endif()

if(TLL_LIBS)
  list(REMOVE_DUPLICATES TLL_LIBS)
endif()

target_link_libraries(netcdf PRIVATE ${TLL_LIBS})

set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${TLL_LIBS})
if(MSVC)
  set_target_properties(netcdf PROPERTIES

    LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
    )
endif()

if(NETCDF_ENABLE_SHARED_LIBRARY_VERSION)
  set_target_properties(netcdf PROPERTIES
    VERSION ${netCDF_LIB_VERSION}
    SOVERSION ${netCDF_SO_VERSION}
    )
endif(NETCDF_ENABLE_SHARED_LIBRARY_VERSION)

install(
  TARGETS netcdf EXPORT netCDFTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libraries
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(ALL_TLL_LIBS ${TLL_LIBS} PARENT_SCOPE)
set(NC_LIBS ${NC_LIBS} PARENT_SCOPE)

file(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
set(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")
