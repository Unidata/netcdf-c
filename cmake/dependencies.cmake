################################
# PkgConfig
################################
FIND_PACKAGE(PkgConfig QUIET)

################################
# MakeDist
################################
# Enable 'dist and distcheck'.
# File adapted from http://ensc.de/cmake/FindMakeDist.cmake
FIND_PACKAGE(MakeDist)
# End 'enable dist and distcheck'

################################
# HDF4
################################
IF(ENABLE_HDF4)
  SET(USE_HDF4 ON )
  # Check for include files, libraries.

  FIND_PATH(MFHDF_H_INCLUDE_DIR mfhdf.h)
  IF(NOT MFHDF_H_INCLUDE_DIR)
    MESSAGE(FATAL_ERROR "HDF4 Support specified, cannot find file mfhdf.h")
  ELSE()
    INCLUDE_DIRECTORIES(${MFHDF_H_INCLUDE_DIR})
  ENDIF()

  FIND_LIBRARY(HDF4_DF_LIB NAMES df libdf hdf)
  IF(NOT HDF4_DF_LIB)
    MESSAGE(FATAL_ERROR "Can't find or link to the hdf4 df library.")
  ENDIF()

  FIND_LIBRARY(HDF4_MFHDF_LIB NAMES mfhdf libmfhdf)
  IF(NOT HDF4_MFHDF_LIB)
    MESSAGE(FATAL_ERROR "Can't find or link to the hdf4 mfhdf library.")
  ENDIF()

  SET(HAVE_LIBMFHDF TRUE )

  SET(HDF4_LIBRARIES ${HDF4_DF_LIB} ${HDF4_MFHDF_LIB} )
  # End include files, libraries.
  MESSAGE(STATUS "HDF4 libraries: ${HDF4_DF_LIB}, ${HDF4_MFHDF_LIB}")

  MESSAGE(STATUS "Seeking HDF4 jpeg dependency.")

  # Look for the jpeglib.h header file.
  FIND_PATH(JPEGLIB_H_INCLUDE_DIR jpeglib.h)
  IF(NOT JPEGLIB_H_INCLUDE_DIR)
    MESSAGE(FATAL_ERROR "HDF4 Support enabled but cannot find jpeglib.h")
  ELSE()
    SET(HAVE_JPEGLIB_H ON CACHE BOOL "")
    SET(HAVE_LIBJPEG TRUE )
    INCLUDE_DIRECTORIES(${JPEGLIB_H_INCLUDE_DIR})
  ENDIF()

  FIND_LIBRARY(JPEG_LIB NAMES jpeg libjpeg)
  IF(NOT JPEG_LIB)
    MESSAGE(FATAL_ERROR "HDF4 Support enabled but cannot find libjpeg")
  ENDIF()
  SET(HDF4_LIBRARIES ${JPEG_LIB} ${HDF4_LIBRARIES} )
  MESSAGE(STATUS "Found JPEG libraries: ${JPEG_LIB}")

  # Option to enable HDF4 file tests.
  OPTION(ENABLE_HDF4_FILE_TESTS "Run HDF4 file tests.  This fetches sample HDF4 files from the Unidata ftp site to test with (requires curl)." ON)
  IF(ENABLE_HDF4_FILE_TESTS)
    FIND_PROGRAM(PROG_CURL NAMES curl)
    IF(PROG_CURL)
      SET(USE_HDF4_FILE_TESTS ON )
    ELSE()
      MESSAGE(STATUS "Unable to locate 'curl'.  Disabling hdf4 file tests.")
      SET(USE_HDF4_FILE_TESTS OFF )
    ENDIF()
    SET(USE_HDF4_FILE_TESTS ${USE_HDF4_FILE_TESTS} )
  ENDIF()
ENDIF()

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
IF(USE_HDF5)

  ##
  # Assert HDF5 version meets minimum required version.
  ##
  SET(HDF5_VERSION_REQUIRED 1.8.10)


  ##
  # Accommodate developers who have hdf5 libraries and
  # headers on their system, but do not have a the hdf
  # .cmake files.  If this is the case, they should
  # specify HDF5_HL_LIBRARY, HDF5_LIBRARY, HDF5_INCLUDE_DIR manually.
  #
  # This script will attempt to determine the version of the HDF5 library programatically.
  ##
  IF(HDF5_C_LIBRARY AND HDF5_HL_LIBRARY AND HDF5_INCLUDE_DIR)
    SET(HDF5_LIBRARIES ${HDF5_C_LIBRARY} ${HDF5_HL_LIBRARY} )
    SET(HDF5_C_LIBRARIES ${HDF5_C_LIBRARY} )
    SET(HDF5_C_LIBRARY_hdf5 ${HDF5_C_LIBRARY} )
    SET(HDF5_HL_LIBRARIES ${HDF5_HL_LIBRARY} )
    INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIR})
    MESSAGE(STATUS "Using HDF5 C Library: ${HDF5_C_LIBRARY}")
    MESSAGE(STATUS "Using HDF5 HL LIbrary: ${HDF5_HL_LIBRARY}")
    if (EXISTS "${HDF5_INCLUDE_DIR}/H5pubconf.h")
      file(READ "${HDF5_INCLUDE_DIR}/H5pubconf.h" _hdf5_version_lines
        REGEX "#define[ \t]+H5_VERSION")
      string(REGEX REPLACE ".*H5_VERSION .*\"\(.*\)\".*" "\\1" _hdf5_version "${_hdf5_version_lines}")
      set(HDF5_VERSION "${_hdf5_version}" CACHE STRING "")
      set(HDF5_VERSION ${HDF5_VERSION} )
      unset(_hdf5_version)
      unset(_hdf5_version_lines)
    endif ()
    MESSAGE(STATUS "Found HDF5 libraries version ${HDF5_VERSION}")
    ###
    # If HDF5_VERSION is still empty, we have a problem.
    # Error out.
    ###
    IF("${HDF5_VERSION}" STREQUAL "")
      MESSAGE(FATAL_ERR "Unable to determine HDF5 version.  NetCDF requires at least version ${HDF5_VERSION_REQUIRED}. Please ensure that libhdf5 is installed and accessible.")
    ENDIF()

    ###
    # Now that we know HDF5_VERSION isn't empty, we can check for minimum required version,
    # and toggle various options.
    ###
    IF(${HDF5_VERSION} VERSION_LESS ${HDF5_VERSION_REQUIRED})
      MESSAGE(FATAL_ERROR "netCDF requires at least HDF5 ${HDF5_VERSION_REQUIRED}. Found ${HDF5_VERSION}.")
    ENDIF()

  ELSE(HDF5_C_LIBRARY AND HDF5_HL_LIBRARY AND HDF5_INCLUDE_DIR) # We are seeking out HDF5 with Find Package.
    ###
    # For now we assume that if we are building netcdf
    # as a shared library, we will use hdf5 as a shared
    # library. If we are building netcdf statically,
    # we will use a static library.  This can be toggled
    # by explicitly modifying NC_FIND_SHARED_LIBS.
    ##
    IF(NC_FIND_SHARED_LIBS)
      SET(NC_HDF5_LINK_TYPE "shared")
      SET(NC_HDF5_LINK_TYPE_UPPER "SHARED")
      ADD_DEFINITIONS(-DH5_BUILT_AS_DYNAMIC_LIB)
    ELSE(NC_FIND_SHARED_LIBS)
      SET(NC_HDF5_LINK_TYPE "static")
      SET(NC_HDF5_LINK_TYPE_UPPER "STATIC")
      ADD_DEFINITIONS(-DH5_BUILT_AS_STATIC_LIB )
    ENDIF(NC_FIND_SHARED_LIBS)

    #####
    # First, find the C and HL libraries.
    #
    # This has been updated to reflect what is in the hdf5
    # examples, even though the previous version of what we
    # had worked.
    #####
    IF(MSVC)
      SET(SEARCH_PACKAGE_NAME ${HDF5_PACKAGE_NAME})
      FIND_PACKAGE(HDF5 NAMES ${SEARCH_PACKAGE_NAME} COMPONENTS C HL CONFIG REQUIRED ${NC_HDF5_LINK_TYPE})
    ELSE(MSVC)
      FIND_PACKAGE(HDF5 COMPONENTS C HL REQUIRED)
    ENDIF(MSVC)

    ##
    # Next, check the HDF5 version. This will inform which
    # HDF5 variables we need to munge.
    ##

    # Some versions of HDF5 set HDF5_VERSION_STRING instead of HDF5_VERSION
    IF(HDF5_VERSION_STRING AND NOT HDF5_VERSION)
      SET(HDF5_VERSION ${HDF5_VERSION_STRING})
    ENDIF()


    ###
    # If HDF5_VERSION is undefined, attempt to determine it programatically.
    ###
    IF("${HDF5_VERSION}" STREQUAL "")
      MESSAGE(STATUS "HDF5_VERSION not detected. Attempting to determine programatically.")
      IF (EXISTS "${HDF5_INCLUDE_DIR}/H5pubconf.h")
        file(READ "${HDF5_INCLUDE_DIR}/H5pubconf.h" _hdf5_version_lines
          REGEX "#define[ \t]+H5_VERSION")
        string(REGEX REPLACE ".*H5_VERSION .*\"\(.*\)\".*" "\\1" _hdf5_version "${_hdf5_version_lines}")
        set(HDF5_VERSION "${_hdf5_version}" CACHE STRING "")
        unset(_hdf5_version)
        unset(_hdf5_version_lines)
        MESSAGE(STATUS "Found HDF5 libraries version ${HDF5_VERSION}")
      ENDIF()
    ELSE()
      SET(HDF5_VERSION ${HDF5_VERSION} CACHE STRING "")
    ENDIF()

    ###
    # If HDF5_VERSION is still empty, we have a problem.
    # Error out.
    ###
    IF("${HDF5_VERSION}" STREQUAL "")
      MESSAGE(FATAL_ERR "Unable to determine HDF5 version.  NetCDF requires at least version ${HDF5_VERSION_REQUIRED}. Please ensure that libhdf5 is installed and accessible.")
    ENDIF()

    ###
    # Now that we know HDF5_VERSION isn't empty, we can check for minimum required version,
    # and toggle various options.
    ###

    IF(${HDF5_VERSION} VERSION_LESS ${HDF5_VERSION_REQUIRED})
      MESSAGE(FATAL_ERROR "netCDF requires at least HDF5 ${HDF5_VERSION_REQUIRED}. Found ${HDF5_VERSION}.")
    ENDIF()



    ##
    # Include the HDF5 include directory.
    ##
    IF(HDF5_INCLUDE_DIRS AND NOT HDF5_INCLUDE_DIR)
      SET(HDF5_INCLUDE_DIR ${HDF5_INCLUDE_DIRS} )
    ENDIF()
    MESSAGE(STATUS "Using HDF5 include dir: ${HDF5_INCLUDE_DIR}")
    INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIR})

    ###
    # This is the block where we figure out what the appropriate
    # variables are, and we ensure that we end up with
    # HDF5_C_LIBRARY, HDF5_HL_LIBRARY and HDF5_LIBRARIES.
    ###
    IF(MSVC)
      ####
      # Environmental variables in Windows when using MSVC
      # are a hot mess between versions.
      ####

      ##
      # HDF5 1.8.15 defined HDF5_LIBRARIES.
      ##
      IF(${HDF5_VERSION} VERSION_LESS "1.8.16")
        SET(HDF5_C_LIBRARY hdf5 )
        SET(HDF5_C_LIBRARY_hdf5 hdf5 )
      ENDIF(${HDF5_VERSION} VERSION_LESS "1.8.16")

      IF(${HDF5_VERSION} VERSION_GREATER "1.8.15")
        IF(NOT HDF5_LIBRARIES AND HDF5_C_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY AND HDF5_HL_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY)
          SET(HDF5_C_LIBRARY ${HDF5_C_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY} )
          SET(HDF5_C_LIBRARY_hdf5 ${HDF5_C_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY} )
          SET(HDF5_HL_LIBRARY ${HDF5_HL_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY} )

      	  SET(HDF5_LIBRARIES ${HDF5_C_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY} ${HDF5_HL_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY} )
        ENDIF()
      ENDIF(${HDF5_VERSION} VERSION_GREATER "1.8.15")

    ELSE(MSVC)

      # Depending on the install, either HDF5_hdf_library or
      # HDF5_C_LIBRARIES may be defined.  We must check for either.
      IF(HDF5_C_LIBRARIES AND NOT HDF5_hdf5_LIBRARY)
        SET(HDF5_hdf5_LIBRARY ${HDF5_C_LIBRARIES} )
      ENDIF()

      # Some versions of FIND_PACKAGE set HDF5_C_LIBRARIES, but not HDF5_C_LIBRARY
      # We use HDF5_C_LIBRARY below, so need to make sure it is set.
      IF(HDF5_C_LIBRARIES AND NOT HDF5_C_LIBRARY)
        SET(HDF5_C_LIBRARY ${HDF5_C_LIBRARIES} )
      ENDIF()

      # Same issue as above...
      IF(HDF5_HL_LIBRARIES AND NOT HDF5_HL_LIBRARY)
        SET(HDF5_HL_LIBRARY ${HDF5_HL_LIBRARIES} )
      ENDIF()

    ENDIF(MSVC)
    IF(NOT HDF5_C_LIBRARY)
      SET(HDF5_C_LIBRARY hdf5 )
    ENDIF()

  ENDIF(HDF5_C_LIBRARY AND HDF5_HL_LIBRARY AND HDF5_INCLUDE_DIR)

  FIND_PACKAGE(Threads)

  # There is a missing case in the above code so default it
  IF(NOT HDF5_C_LIBRARY_hdf5 OR "${HDF5_C_LIBRARY_hdf5}" STREQUAL "" )
    SET(HDF5_C_LIBRARY_hdf5 "${HDF5_C_LIBRARY}" )
  ENDIF()

  FIND_PATH(HAVE_HDF5_H hdf5.h PATHS ${HDF5_INCLUDE_DIR} NO_DEFAULT_PATH)
  IF(NOT HAVE_HDF5_H)
    MESSAGE(FATAL_ERROR "Compiling a test with hdf5 failed. Either hdf5.h cannot be found, or the log messages should be checked for another reason.")
  ELSE(NOT HAVE_HDF5_H)
    INCLUDE_DIRECTORIES(${HAVE_HDF5_H})
  ENDIF(NOT HAVE_HDF5_H)

  set (CMAKE_REQUIRED_INCLUDES ${HDF5_INCLUDE_DIR})

  # Check to ensure that HDF5 was built with zlib.
  # This needs to be near the beginning since we
  # need to know whether to add "-lz" to the symbol
  # tests below.
  CHECK_C_SOURCE_COMPILES("#include <H5pubconf.h>
   #if !H5_HAVE_ZLIB_H
   #error
   #endif
   int main() {
   int x = 1;}" HAVE_HDF5_ZLIB)
  IF(NOT HAVE_HDF5_ZLIB)
    MESSAGE(FATAL_ERROR "HDF5 was built without zlib. Rebuild HDF5 with zlib.")
  ELSE()
    # If user has specified the `ZLIB_LIBRARY`, use it; otherwise try to find...
    IF(NOT ZLIB_LIBRARY)
      find_package(ZLIB)
      IF(ZLIB_FOUND)
        SET(ZLIB_LIBRARY ${ZLIB_LIBRARIES} )
      ELSE()
        MESSAGE(FATAL_ERROR "HDF5 Requires ZLIB, but cannot find libz.")
      ENDIF()
    ENDIF()
    SET(CMAKE_REQUIRED_LIBRARIES ${ZLIB_LIBRARY} ${CMAKE_REQUIRED_LIBRARIES} )
    MESSAGE(STATUS "HDF5 has zlib.")
  ENDIF()

  #Check to see if H5Z_SZIP exists in HDF5_Libraries. If so, we must use szip library.
  CHECK_C_SOURCE_COMPILES("#include <H5pubconf.h>
   #if !H5_HAVE_FILTER_SZIP
   #error
   #endif
   int main() {
   int x = 1;}" USE_HDF5_SZIP)
  IF(USE_HDF5_SZIP)
    SET(HAVE_H5Z_SZIP yes )
  ENDIF()

  ####
  # Check to see if HDF5 library is 1.10.6 or greater.
  # Used to control path name conversion
  ####
  IF(${HDF5_VERSION} VERSION_GREATER "1.10.5")
    SET(HDF5_UTF8_PATHS ON )
  ELSE()
    SET(HDF5_UTF8_PATHS OFF )
  ENDIF()

  MESSAGE("-- HDF5_UTF8_PATHS (HDF5 version 1.10.6+): ${HDF5_UTF8_PATHS}")

  # Find out if HDF5 was built with parallel support.
  # Do that by checking for the targets H5Pget_fapl_mpiposx and
  # H5Pget_fapl_mpio in ${HDF5_LIB}.

  # H5Pset_fapl_mpiposix and H5Pget_fapl_mpiposix have been removed since HDF5 1.8.12.
  # Use H5Pset_fapl_mpio and H5Pget_fapl_mpio, instead.
  # CHECK_LIBRARY_EXISTS(${HDF5_C_LIBRARY_hdf5} H5Pget_fapl_mpiposix "" HDF5_IS_PARALLEL_MPIPOSIX)

  CHECK_LIBRARY_EXISTS(${HDF5_C_LIBRARY_hdf5} H5Pget_fapl_mpio "" HDF5_IS_PARALLEL_MPIO)
  IF(HDF5_IS_PARALLEL_MPIO)
    SET(HDF5_PARALLEL ON)
  ELSE()
    SET(HDF5_PARALLEL OFF)
  ENDIF()
  SET(HDF5_PARALLEL ${HDF5_PARALLEL} )

  #Check to see if HDF5 library has collective metadata APIs, (HDF5 >= 1.10.0)
  CHECK_LIBRARY_EXISTS(${HDF5_C_LIBRARY_hdf5} H5Pset_all_coll_metadata_ops "" HDF5_HAS_COLL_METADATA_OPS)

  IF(HDF5_PARALLEL)
	SET(HDF5_CC h5pcc )
  ELSE()
	SET(HDF5_CC h5cc )
  ENDIF()

  # Check to see if H5Dread_chunk is available
  CHECK_LIBRARY_EXISTS(${HDF5_C_LIBRARY_hdf5} H5Dread_chunk "" HAS_READCHUNKS)

  # Check to see if H5Pset_fapl_ros3 is available
  CHECK_LIBRARY_EXISTS(${HDF5_C_LIBRARY_hdf5} H5Pset_fapl_ros3 "" HAS_HDF5_ROS3)

  # Check to see if this is hdf5-1.10.3 or later.
  IF(HAS_READCHUNKS)
    SET(HDF5_SUPPORTS_PAR_FILTERS ON )
    SET(ENABLE_NCDUMPCHUNKS ON )
  ENDIF()

  # Record if ROS3 Driver is available
  IF(HAS_HDF5_ROS3)
    SET(ENABLE_HDF5_ROS3 ON )
  ENDIF()

  IF (HDF5_SUPPORTS_PAR_FILTERS)
    SET(HDF5_HAS_PAR_FILTERS TRUE CACHE BOOL "" )
    SET(HAS_PAR_FILTERS yes CACHE STRING "" )
  ELSE()
    SET(HDF5_HAS_PAR_FILTERS FALSE CACHE BOOL "" )
    SET(HAS_PAR_FILTERS no CACHE STRING "" )
  ENDIF()

  FIND_PATH(HAVE_HDF5_H hdf5.h PATHS ${HDF5_INCLUDE_DIR} NO_DEFAULT_PATH)
  IF(NOT HAVE_HDF5_H)
    MESSAGE(FATAL_ERROR "Compiling a test with hdf5 failed. Either hdf5.h cannot be found, or the log messages should be checked for another reason.")
  ELSE(NOT HAVE_HDF5_H)
    INCLUDE_DIRECTORIES(${HAVE_HDF5_H})
  ENDIF(NOT HAVE_HDF5_H)

  #option to include HDF5 High Level header file (hdf5_hl.h) in case we are not doing a make install
  INCLUDE_DIRECTORIES(${HDF5_HL_INCLUDE_DIR})

ENDIF(USE_HDF5)

################################
# Curl
################################
# See if we have libcurl
FIND_PACKAGE(CURL)
ADD_DEFINITIONS(-DCURL_STATICLIB=1)
INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIRS})

# Define a test flag for have curl library
IF(CURL_LIBRARIES OR CURL_LIBRARY)
  SET(FOUND_CURL TRUE)
ELSE()
  SET(FOUND_CURL FALSE)
ENDIF()
SET(FOUND_CURL ${FOUND_CURL} TRUE )

# Start disabling if curl not found
IF(NOT FOUND_CURL)
  MESSAGE(WARNING "ENABLE_REMOTE_FUNCTIONALITY requires libcurl; disabling")
  SET(ENABLE_REMOTE_FUNCTIONALITY OFF CACHE BOOL "ENABLE_REMOTE_FUNCTIONALITY requires libcurl; disabling" FORCE )
ENDIF()

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
ELSE()
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
ENDIF()

################################
# Math
################################
# Check for the math library so it can be explicitly linked.
IF(NOT WIN32)
  FIND_LIBRARY(HAVE_LIBM NAMES math m libm)
  IF(NOT HAVE_LIBM)
    CHECK_FUNCTION_EXISTS(exp HAVE_LIBM_FUNC)
    IF(NOT HAVE_LIBM_FUNC)
        MESSAGE(FATAL_ERROR "Unable to find the math library.")
    ELSE(NOT HAVE_LIBM_FUNC)
        SET(HAVE_LIBM "")
    ENDIF()
  ELSE(NOT HAVE_LIBM)
    MESSAGE(STATUS "Found Math library: ${HAVE_LIBM}")
  ENDIF()
ENDIF()

################################
# zlib
################################
# See if we have zlib
FIND_PACKAGE(ZLIB)

# Define a test flag for have zlib library
IF(ZLIB_FOUND)
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
  SET(ENABLE_ZLIB TRUE)
ELSE()
  SET(ENABLE_ZLIB FALSE)
ENDIF()

################################
# Zips
################################
IF (ENABLE_FILTER_SZIP)
  FIND_PACKAGE(Szip)
ELSEIF(ENABLE_NCZARR)
  FIND_PACKAGE(Szip)
ENDIF()
IF (ENABLE_FILTER_BZ2)
  FIND_PACKAGE(Bz2)
ENDIF()
IF (ENABLE_FILTER_BLOSC)
  FIND_PACKAGE(Blosc)
ENDIF()
IF (ENABLE_FILTER_ZSTD)
  FIND_PACKAGE(Zstd)
ENDIF()

# Accumulate standard filters
set(STD_FILTERS "deflate") # Always have deflate*/
set_std_filter(Szip)
SET(HAVE_SZ ${Szip_FOUND})
set_std_filter(Blosc)
IF(Zstd_FOUND)
  set_std_filter(Zstd)
  SET(HAVE_ZSTD ON)
ENDIF()
IF(Bz2_FOUND)
  set_std_filter(Bz2)
ELSE()
  # The reason we use a local version is to support a more comples test case
  MESSAGE("libbz2 not found using built-in version")
  SET(HAVE_LOCAL_BZ2 ON)
  SET(HAVE_BZ2 ON CACHE BOOL "")
  set(STD_FILTERS "${STD_FILTERS} bz2")
ENDIF()

IF (ENABLE_NCZARR_ZIP)
  FIND_PACKAGE(Zip REQUIRED)
  INCLUDE_DIRECTORIES(${Zip_INCLUDE_DIRS})
ENDIF ()

################################
# S3
################################
# Note we check for the library after checking for enable_s3
# because for some reason this screws up if we unconditionally test for sdk
# and it is not available. Fix someday
IF(ENABLE_S3)
  IF(NOT ENABLE_S3_INTERNAL)
    # See if aws-s3-sdk is available
    find_package(AWSSDK REQUIRED COMPONENTS s3;transfer)
    IF(AWSSDK_FOUND)
      SET(ENABLE_S3_AWS ON CACHE BOOL "S3 AWS" FORCE)
      INCLUDE_DIRECTORIES(${AWSSDK_INCLUDE_DIR})
    ELSE(AWSSDK_FOUND)
      SET(ENABLE_S3_AWS OFF CACHE BOOL "S3 AWS" FORCE)
    ENDIF(AWSSDK_FOUND)
  ELSE(NOT ENABLE_S3_INTERNAL)
    # Find crypto libraries required with testing with the internal s3 api.
    #FIND_LIBRARY(SSL_LIB NAMES ssl openssl)
    find_package(OpenSSL REQUIRED)
    IF(NOT OpenSSL_FOUND)
      MESSAGE(FATAL_ERROR "Can't find an ssl library, required by S3_INTERNAL")
    ENDIF(NOT OpenSSL_FOUND)
    
    #find_package(Crypto REQUIRED)
    #IF(NOT CRYPTO_LIB)
    #  MESSAGE(FATAL_ERROR "Can't find a crypto library, required by S3_INTERNAL")
    #ENDIF(NOT CRYPTO_LIB)

  ENDIF(NOT ENABLE_S3_INTERNAL)
ELSE()
  SET(ENABLE_S3_AWS OFF CACHE BOOL "S3 AWS" FORCE)
ENDIF()

################################
# LibXML
################################
# see if we have libxml2
IF(ENABLE_LIBXML2)
  find_package(LibXml2)
  IF(LibXml2_FOUND)
    SET(HAVE_LIBXML2 TRUE)
    INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIRS})
    SET(XMLPARSER "libxml2")
  ELSE()
    SET(HAVE_LIBXML2 FALSE)
  ENDIF()
ENDIF(ENABLE_LIBXML2)

################################
# MPI
################################ 
OPTION(ENABLE_PARALLEL4 "Build netCDF-4 with parallel IO" "${HDF5_PARALLEL}")
IF(ENABLE_PARALLEL4 AND ENABLE_HDF5 AND HDF5_PARALLEL)
  FIND_PACKAGE(MPI REQUIRED)
ENDIF()

################################
# parallel IO
################################ 
IF(ENABLE_PNETCDF)
  FIND_LIBRARY(PNETCDF NAMES pnetcdf)
  FIND_PATH(PNETCDF_INCLUDE_DIR pnetcdf.h)
  IF(NOT PNETCDF)
    MESSAGE(STATUS "Cannot find PnetCDF library. Disabling PnetCDF support.")
    SET(USE_PNETCDF OFF CACHE BOOL "")
  ENDIF
ENDIF()

################################
# Doxygen
################################ 
IF(ENABLE_DOXYGEN)
  FIND_PACKAGE(Doxygen REQUIRED)
ENDIF()