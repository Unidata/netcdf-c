################################
# PkgConfig
################################
find_package(PkgConfig QUIET)

################################
# MakeDist
################################
# Enable 'dist and distcheck'.
# File adapted from http://ensc.de/cmake/FindMakeDist.cmake
find_package(MakeDist)
# End 'enable dist and distcheck'

################################
# HDF4
################################
if(USE_HDF4)
  set(NETCDF_USE_HDF4 ON )
  # Check for include files, libraries.

  find_path(MFHDF_H_INCLUDE_DIR mfhdf.h)
  if(NOT MFHDF_H_INCLUDE_DIR)
    message(FATAL_ERROR "HDF4 Support specified, cannot find file mfhdf.h")
  else()

  target_include_directories(netcdf
    PRIVATE
      ${MFHDF_H_INCLUDE_DIR}
  )
  endif()

  find_library(HDF4_DF_LIB NAMES df libdf hdf)
  if(NOT HDF4_DF_LIB)
    message(FATAL_ERROR "Can't find or link to the hdf4 df library.")
  endif()

  find_library(HDF4_MFHDF_LIB NAMES mfhdf libmfhdf)
  if(NOT HDF4_MFHDF_LIB)
    message(FATAL_ERROR "Can't find or link to the hdf4 mfhdf library.")
  endif()

  set(HAVE_LIBMFHDF TRUE )

  set(HDF4_LIBRARIES ${HDF4_DF_LIB} ${HDF4_MFHDF_LIB} )
  # End include files, libraries.
  message(STATUS "HDF4 libraries: ${HDF4_DF_LIB}, ${HDF4_MFHDF_LIB}")

  message(STATUS "Seeking HDF4 jpeg dependency.")

  # Look for the jpeglib.h header file.
  find_path(JPEGLIB_H_INCLUDE_DIR jpeglib.h)
  if(NOT JPEGLIB_H_INCLUDE_DIR)
    message(FATAL_ERROR "HDF4 Support enabled but cannot find jpeglib.h")
  else()
    set(HAVE_JPEGLIB_H ON CACHE BOOL "")
    set(HAVE_LIBJPEG TRUE )

    target_include_directories(netcdf
      PRIVATE
        ${JPEGLIB_H_INCLUDE_DIR}
    )
  endif()

  find_library(JPEG_LIB NAMES jpeg libjpeg)
  if(NOT JPEG_LIB)
    message(FATAL_ERROR "HDF4 Support enabled but cannot find libjpeg")
  endif()
  set(HDF4_LIBRARIES ${JPEG_LIB} ${HDF4_LIBRARIES} CACHE STRING "")
  message(STATUS "Found JPEG libraries: ${JPEG_LIB}")

  target_link_libraries(netcdf
  PRIVATE
  ${HDF4_LIBRARIES}
  )
  
  # Option to enable HDF4 file tests.
  #option(NETCDF_ENABLE_HDF4_FILE_TESTS "Run HDF4 file tests.  This fetches sample HDF4 files from the Unidata resources site to test with (requires curl)." ON)

  if(NETCDF_ENABLE_HDF4_FILE_TESTS)
    find_program(PROG_CURL NAMES curl)
    if(PROG_CURL)
      set(USE_HDF4_FILE_TESTS ON )
    else()
      message(STATUS "Unable to locate 'curl'.  Disabling hdf4 file tests.")
      set(USE_HDF4_FILE_TESTS OFF )
      set(NETCDF_ENABLE_HDF4_FILE_TESTS OFF)
    endif()
    set(USE_HDF4_FILE_TESTS ${USE_HDF4_FILE_TESTS} )
  endif()
endif(USE_HDF4)

################################
# HDF5
################################
##
# Option to Enable HDF5
#
# The HDF5 cmake variables differ between platform (linux/osx and Windows),
# as well as between HDF5 versions.  As a result, this section is a bit convoluted.
#
# Note that the behavior seems much more stable across HDF5 versions under linux,
# so we do not have to do as much version-based tweaking.
#
# At the end of it, we should have the following defined:
#
# * HDF5_C_LIBRARY
# * HDF5_HL_LIBRARY
# * HDF5_LIBRARIES
# * HDF5_INCLUDE_DIR
# *
##
if(USE_HDF5)

  ##
  # Assert HDF5 version meets minimum required version.
  ##
  set(HDF5_VERSION_REQUIRED 1.8.15)

  ###
  # For now we assume that if we are building netcdf
  # as a shared library, we will use hdf5 as a shared
  # library. If we are building netcdf statically,
  # we will use a static library.  This can be toggled
  # by explicitly modifying NETCDF_FIND_SHARED_LIBS.
  ##
  #if (MSVC)
  #  if(NETCDF_FIND_SHARED_LIBS)
  #    set(HDF5_USE_STATIC_LIBRARIES OFF)
  #  else()
  #    set(HDF5_USE_STATIC_LIBRARIES ON)
  #  endif()
  #endif()

  #####
  # First, find the C and HL libraries.
  #####
  find_package(HDF5 COMPONENTS C HL REQUIRED)

  message(STATUS "Found HDF5 version: ${HDF5_VERSION}")
  if(${HDF5_VERSION} VERSION_LESS ${HDF5_VERSION_REQUIRED})
     message(FATAL_ERROR "NetCDF requires HDF5 version ${HDF5_VERSION_REQUIRED} or later; found version ${HDF5_VERSION}.")
  endif()  
  
  message(STATUS "Using HDF5 include dir: ${HDF5_INCLUDE_DIRS}")
  target_link_libraries(netcdf
    PRIVATE
    HDF5::HDF5
  )

  find_package(Threads)

  set (CMAKE_REQUIRED_INCLUDES ${HDF5_INCLUDE_DIRS})

  # Check to ensure that HDF5 was built with zlib.
  # This needs to be near the beginning since we
  # need to know whether to add "-lz" to the symbol
  # tests below.
  include(check_hdf5)
  check_hdf5_feature(HAVE_HDF5_ZLIB H5_HAVE_ZLIB_H)
  if(NOT HAVE_HDF5_ZLIB)
    message(FATAL_ERROR "HDF5 was built without zlib. Rebuild HDF5 with zlib.")
  else()
    # If user has specified the `ZLIB_LIBRARY`, use it; otherwise try to find...
    if(NOT ZLIB_LIBRARY)
      find_package(ZLIB)
      if(ZLIB_FOUND)
        set(ZLIB_LIBRARY ${ZLIB_LIBRARIES} )
      else()
        message(FATAL_ERROR "HDF5 Requires ZLIB, but cannot find libz.")
      endif()
    endif()
    set(CMAKE_REQUIRED_LIBRARIES ${ZLIB_LIBRARY} ${CMAKE_REQUIRED_LIBRARIES} )
    message(STATUS "HDF5 has zlib.")
  endif()


  # Check to see if H5Z_SZIP exists in HDF5_Libraries. If so, we must use szip library.
  check_hdf5_feature(HAVE_H5Z_SZIP H5_HAVE_FILTER_SZIP)


  ####
  # Check to see if HDF5 library is 1.10.6 or greater.
  # Used to control path name conversion
  ####
  if(${HDF5_VERSION} VERSION_GREATER "1.10.5")
    set(HDF5_UTF8_PATHS ON )
  else()
    set(HDF5_UTF8_PATHS OFF )
  endif()

  message(STATUS "-- HDF5_UTF8_PATHS (HDF5 version 1.10.6+): ${HDF5_UTF8_PATHS}")

  # Find out if HDF5 was built with parallel support.
  set(HDF5_PARALLEL ${HDF5_IS_PARALLEL})

  set(CMAKE_REQUIRED_LIBRARIES HDF5::HDF5)
  include(CheckSymbolExists)

  #Check to see if HDF5 library has collective metadata APIs, (HDF5 >= 1.10.0)
  check_symbol_exists(H5Pset_all_coll_metadata_ops "hdf5.h" HDF5_HAS_COLL_METADATA_OPS)

  # Check to see if H5Dread_chunk is available
  check_symbol_exists(H5Dread_chunk "hdf5.h" HAS_READCHUNKS)
  if(NOT HAS_READCHUNKS)
    check_symbol_exists(H5Dread_chunk2 "hdf5.h" HAS_READCHUNKS)
  endif()

  # Check to see if H5Pset_fapl_ros3 is available
  check_symbol_exists(H5Pset_fapl_ros3 "hdf5.h" HAS_HDF5_ROS3)

  # Check to see if this is hdf5-1.10.3 or later.
  if(HAS_READCHUNKS)
    set(HDF5_SUPPORTS_PAR_FILTERS ON )
    set(ENABLE_NCDUMPCHUNKS ON )
  endif()

  # Record if ROS3 Driver is available
  if(HAS_HDF5_ROS3)
    set(NETCDF_ENABLE_HDF5_ROS3 ON )
  endif()

  IF (HDF5_SUPPORTS_PAR_FILTERS)
    set(HDF5_HAS_PAR_FILTERS TRUE CACHE BOOL "" )
    set(HAS_PAR_FILTERS yes CACHE STRING "" )
  else()
    set(HDF5_HAS_PAR_FILTERS FALSE CACHE BOOL "" )
    set(HAS_PAR_FILTERS no CACHE STRING "" )
  endif()
endif(USE_HDF5)

################################
# Curl Libraries
# Only needed for DAP (DAP2 or DAP4)
# and NCZARR S3 support
# and byterange support
################################

if( NETCDF_ENABLE_DAP2 OR NETCDF_ENABLE_DAP4 OR NETCDF_ENABLE_BYTERANGE OR NETCDF_ENABLE_NCZARR_S3)
  # See if we have libcurl
  find_package(CURL)
  #target_compile_options(netcdf
  #  PRIVATE
  #    -DCURL_STATICLIB=1
  #)
  #target_include_directories(netcdf
  #  PRIVATE
  #    ${CURL_INCLUDE_DIRS}
  #)
  if(CURL_FOUND)
    set(FOUND_CURL TRUE)
    target_link_libraries(netcdf
    PRIVATE
    CURL::libcurl
  )
  else()
    set(FOUND_CURL FALSE)
    set(NETCDF_ENABLE_DAP2 OFF)
    set(NETCDF_ENABLE_DAP4 OFF)
    set(NETCDF_ENABLE_BYTERANGE OFF)
    set(NETCDF_ENABLE_S3 OFF)
  endif(CURL_FOUND)

  # Start disabling if curl not found
  if(NOT FOUND_CURL)
    message(WARNING "NETCDF_ENABLE_REMOTE_FUNCTIONALITY requires libcurl; disabling")
    set(NETCDF_ENABLE_REMOTE_FUNCTIONALITY OFF CACHE BOOL "NETCDF_ENABLE_REMOTE_FUNCTIONALITY requires libcurl; disabling" FORCE )
  endif()

  set (CMAKE_REQUIRED_INCLUDES ${CURL_INCLUDE_DIRS})
  # Check to see if we have libcurl 7.66 or later
  CHECK_C_SOURCE_COMPILES("
  #include <curl/curl.h>
  int main() {
  #if LIBCURL_VERSION_NUM < 0x074200
        choke me;
  #endif
  }" HAVE_LIBCURL_766)

  IF (HAVE_LIBCURL_766)
    # If libcurl version is >= 7.66, then can skip tests
    # for these symbols which were added in an earlier version
    set(HAVE_CURLOPT_USERNAME TRUE)
    set(HAVE_CURLOPT_PASSWORD TRUE)
    set(HAVE_CURLOPT_KEYPASSWD TRUE)
    set(HAVE_CURLINFO_RESPONSE_CODE TRUE)
    set(HAVE_CURLINFO_HTTP_CONNECTCODE TRUE)
    set(HAVE_CURLOPT_BUFFERSIZE TRUE)
    set(HAVE_CURLOPT_KEEPALIVE TRUE)
  else()
    # Check to see if CURLOPT_USERNAME is defined.
    # It is present starting version 7.19.1.
    CHECK_C_SOURCE_COMPILES("
    #include <curl/curl.h>
    int main() {int x = CURLOPT_USERNAME;}" HAVE_CURLOPT_USERNAME)

    # Check to see if CURLOPT_PASSWORD is defined.
    # It is present starting version 7.19.1.
    CHECK_C_SOURCE_COMPILES("
    #include <curl/curl.h>
    int main() {int x = CURLOPT_PASSWORD;}" HAVE_CURLOPT_PASSWORD)

    # Check to see if CURLOPT_KEYPASSWD is defined.
    # It is present starting version 7.16.4.
    CHECK_C_SOURCE_COMPILES("
    #include <curl/curl.h>
    int main() {int x = CURLOPT_KEYPASSWD;}" HAVE_CURLOPT_KEYPASSWD)

    # Check to see if CURLINFO_RESPONSE_CODE is defined.
    # It showed up in curl 7.10.7.
    CHECK_C_SOURCE_COMPILES("
    #include <curl/curl.h>
    int main() {int x = CURLINFO_RESPONSE_CODE;}" HAVE_CURLINFO_RESPONSE_CODE)

    # Check to see if CURLINFO_HTTP_CONNECTCODE is defined.
    # It showed up in curl 7.10.7.
    CHECK_C_SOURCE_COMPILES("
    #include <curl/curl.h>
    int main() {int x = CURLINFO_HTTP_CONNECTCODE;}" HAVE_CURLINFO_HTTP_CONNECTCODE)

    # Check to see if CURLOPT_BUFFERSIZE is defined.
    # It is present starting version 7.59
    CHECK_C_SOURCE_COMPILES("
    #include <curl/curl.h>
    int main() {int x = CURLOPT_BUFFERSIZE;}" HAVE_CURLOPT_BUFFERSIZE)

    # Check to see if CURLOPT_TCP_KEEPALIVE is defined.
    # It is present starting version 7.25
    CHECK_C_SOURCE_COMPILES("
    #include <curl/curl.h>
    int main() {int x = CURLOPT_TCP_KEEPALIVE;}" HAVE_CURLOPT_KEEPALIVE)
  endif()
endif()
################################
# End LibCurl stuff
################################

################################
# Math
################################
# Check for the math library so it can be explicitly linked.
if(NOT WIN32)
  find_library(HAVE_LIBM NAMES math m libm)
  if(NOT HAVE_LIBM)
    CHECK_FUNCTION_EXISTS(exp HAVE_LIBM_FUNC)
    if(NOT HAVE_LIBM_FUNC)
        message(FATAL_ERROR "Unable to find the math library.")
    else(NOT HAVE_LIBM_FUNC)
        set(HAVE_LIBM "")
    endif()
  else(NOT HAVE_LIBM)
    message(STATUS "Found Math library: ${HAVE_LIBM}")
  endif()
endif()

################################
# zlib
################################
# See if we have zlib
find_package(ZLIB)

# Define a test flag for have zlib library
if(ZLIB_FOUND)
  target_include_directories(netcdf
    PRIVATE
      ${ZLIB_INCLUDE_DIRS}
  )
  set(ENABLE_ZLIB TRUE)
else()
  set(ENABLE_ZLIB FALSE)
endif()

################################
# Zips
################################
MESSAGE(STATUS "Checking for filter libraries")
IF (NETCDF_ENABLE_FILTER_SZIP)
  find_package(Szip)
elseif(NETCDF_ENABLE_NCZARR)
  find_package(Szip)
endif()
IF (NETCDF_ENABLE_FILTER_BZ2)
  find_package(Bz2)
endif()
IF (NETCDF_ENABLE_FILTER_BLOSC)
  find_package(Blosc)
endif()
IF (NETCDF_ENABLE_FILTER_ZSTD)
  find_package(Zstd)
endif()

# Accumulate standard filters
#set(STD_FILTERS "bz2")
set(FOUND_STD_FILTERS "")
if(ENABLE_ZLIB)
  set(STD_FILTERS "deflate")
endif()
set_std_filter(Szip)
set(HAVE_SZ ${Szip_FOUND})
set(USE_SZIP ${HAVE_SZ})
set_std_filter(Blosc)
if(Zstd_FOUND)
  set_std_filter(Zstd)
else()
  set(NETCDF_ENABLE_FILTER_ZSTD OFF)
endif()
if(Bz2_FOUND)
  set_std_filter(Bz2)
else()
  # The reason we use a local version is to support a more comples test case
  message("libbz2 not found using built-in version") 
  set(HAVE_LOCAL_BZ2 ON)
  set(HAVE_BZ2 ON CACHE BOOL "")
  set(STD_FILTERS "${STD_FILTERS} bz2")
endif()

set(STD_FILTERS "${STD_FILTERS}${FOUND_STD_FILTERS}")
IF (NETCDF_ENABLE_NCZARR_ZIP)
  find_package(Zip)
  if(Zip_FOUND)
    target_include_directories(netcdf
      PRIVATE
      ${Zip_INCLUDE_DIRS}
    )
  else()
    message(STATUS "libzip development package not found, disabling NETCDF_ENABLE_NCZARR_ZIP")
    set(NETCDF_ENABLE_NCZARR_ZIP OFF CACHE BOOL "Enable NCZARR_ZIP functionality." FORCE)
  endif()
endif ()

################################
# S3
################################
# Note we check for the library after checking for enable_s3
# because for some reason this screws up if we unconditionally test for sdk
# and it is not available. Fix someday
if(NETCDF_ENABLE_S3)
  if(NETCDF_ENABLE_S3_AWS)
    # See if aws-s3-sdk is available
    find_package(AWSSDK REQUIRED COMPONENTS s3;transfer)
    if(AWSSDK_FOUND)
      set(NETCDF_ENABLE_S3_AWS ON CACHE BOOL "S3 AWS" FORCE)
      target_include_directories(netcdf
        PRIVATE
          ${AWSSDK_INCLUDE_DIR}
      )
    else(AWSSDK_FOUND)
      set(NETCDF_ENABLE_S3_AWS OFF CACHE BOOL "S3 AWS" FORCE)
    endif(AWSSDK_FOUND)
  else(NETCDF_ENABLE_S3_AWS)
    # Find crypto libraries required with testing with the internal s3 api.
    #find_library(SSL_LIB NAMES ssl openssl)
    find_package(OpenSSL REQUIRED)
    if(NOT OpenSSL_FOUND)
      message(FATAL_ERROR "Can't find an ssl library, required by S3_INTERNAL")
    endif(NOT OpenSSL_FOUND)

    #find_package(Crypto REQUIRED)
    #if(NOT CRYPTO_LIB)
    #  message(FATAL_ERROR "Can't find a crypto library, required by S3_INTERNAL")
    #endif(NOT CRYPTO_LIB)

  endif(NETCDF_ENABLE_S3_AWS)
else()
  set(NETCDF_ENABLE_S3_AWS OFF CACHE BOOL "S3 AWS" FORCE)
endif()

################################
# LibXML
################################
# see if we have libxml2
if(NETCDF_ENABLE_LIBXML2)
  find_package(LibXml2)
  if(LibXml2_FOUND)
    set(HAVE_LIBXML2 TRUE)
    target_include_directories(netcdf
      PRIVATE
        ${LIBXML2_INCLUDE_DIRS}
    )
    set(XMLPARSER "libxml2")
  else()
    set(HAVE_LIBXML2 FALSE)
  endif()
endif(NETCDF_ENABLE_LIBXML2)

################################
# MPI
################################
if(NETCDF_ENABLE_PARALLEL4 OR HDF5_PARALLEL)
  find_package(MPI REQUIRED)
endif()

################################
# Parallel IO
################################ 
if(NETCDF_ENABLE_PNETCDF)
  find_package(PNETCDF 1.6.0 REQUIRED)
  if(NOT PNETCDF_HAS_RELAXED_COORD_BOUND)
    message(FATAL_ERROR "Pnetcdf must be built with relax-coord-bound enabled")
  endif()
endif()

################################
# Doxygen
################################ 

if(NETCDF_ENABLE_DOXYGEN)
  find_package(Doxygen REQUIRED)
endif()

################################
# NC_DPKG
################################ 
if (NETCDF_PACKAGE)
  find_program(NC_DPKG NAMES dpkg)
endif()
