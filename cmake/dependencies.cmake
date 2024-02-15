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
if(ENABLE_HDF4)
  set(USE_HDF4 ON )
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
  set(HDF4_LIBRARIES ${JPEG_LIB} ${HDF4_LIBRARIES} )
  message(STATUS "Found JPEG libraries: ${JPEG_LIB}")

  # Option to enable HDF4 file tests.
  option(ENABLE_HDF4_FILE_TESTS "Run HDF4 file tests.  This fetches sample HDF4 files from the Unidata ftp site to test with (requires curl)." ON)
  if(ENABLE_HDF4_FILE_TESTS)
    find_program(PROG_CURL NAMES curl)
    if(PROG_CURL)
      set(USE_HDF4_FILE_TESTS ON )
    else()
      message(STATUS "Unable to locate 'curl'.  Disabling hdf4 file tests.")
      set(USE_HDF4_FILE_TESTS OFF )
    endif()
    set(USE_HDF4_FILE_TESTS ${USE_HDF4_FILE_TESTS} )
  endif()
endif()

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
  set(HDF5_VERSION_REQUIRED 1.8.10)


  ##
  # Accommodate developers who have hdf5 libraries and
  # headers on their system, but do not have a the hdf
  # .cmake files.  If this is the case, they should
  # specify HDF5_HL_LIBRARY, HDF5_LIBRARY, HDF5_INCLUDE_DIR manually.
  #
  # This script will attempt to determine the version of the HDF5 library programatically.
  ##
  if(HDF5_C_LIBRARY AND HDF5_HL_LIBRARY AND HDF5_INCLUDE_DIR)
    set(HDF5_LIBRARIES ${HDF5_C_LIBRARY} ${HDF5_HL_LIBRARY} )
    set(HDF5_C_LIBRARIES ${HDF5_C_LIBRARY} )
    set(HDF5_C_LIBRARY_hdf5 ${HDF5_C_LIBRARY} )
    set(HDF5_HL_LIBRARIES ${HDF5_HL_LIBRARY} )
    target_include_directories(netcdf
      PRIVATE
        ${HDF5_INCLUDE_DIR}
    )
    message(STATUS "Using HDF5 C Library: ${HDF5_C_LIBRARY}")
    message(STATUS "Using HDF5 HL LIbrary: ${HDF5_HL_LIBRARY}")
    if (EXISTS "${HDF5_INCLUDE_DIR}/H5pubconf.h")
      file(READ "${HDF5_INCLUDE_DIR}/H5pubconf.h" _hdf5_version_lines
        REGEX "#define[ \t]+H5_VERSION")
      string(REGEX REPLACE ".*H5_VERSION .*\"\(.*\)\".*" "\\1" _hdf5_version "${_hdf5_version_lines}")
      set(HDF5_VERSION "${_hdf5_version}" CACHE STRING "")
      set(HDF5_VERSION ${HDF5_VERSION} PARENT_SCOPE)
      unset(_hdf5_version)
      unset(_hdf5_version_lines)
    endif ()
    message(STATUS "Found HDF5 libraries version ${HDF5_VERSION}")
    ###
    # If HDF5_VERSION is still empty, we have a problem.
    # Error out.
    ###
    if("${HDF5_VERSION}" STREQUAL "")
      message(FATAL_ERR "Unable to determine HDF5 version.  NetCDF requires at least version ${HDF5_VERSION_REQUIRED}. Please ensure that libhdf5 is installed and accessible.")
    endif()

    ###
    # Now that we know HDF5_VERSION isn't empty, we can check for minimum required version,
    # and toggle various options.
    ###
    if(${HDF5_VERSION} VERSION_LESS ${HDF5_VERSION_REQUIRED})
      message(FATAL_ERROR "netCDF requires at least HDF5 ${HDF5_VERSION_REQUIRED}. Found ${HDF5_VERSION}.")
    endif()

  else(HDF5_C_LIBRARY AND HDF5_HL_LIBRARY AND HDF5_INCLUDE_DIR) # We are seeking out HDF5 with Find Package.
    ###
    # For now we assume that if we are building netcdf
    # as a shared library, we will use hdf5 as a shared
    # library. If we are building netcdf statically,
    # we will use a static library.  This can be toggled
    # by explicitly modifying NC_FIND_SHARED_LIBS.
    ##
    if(NC_FIND_SHARED_LIBS)
      set(NC_HDF5_LINK_TYPE "shared")
      set(NC_HDF5_LINK_TYPE_UPPER "SHARED")
      ADD_DEFINITIONS(-DH5_BUILT_AS_DYNAMIC_LIB)
    else(NC_FIND_SHARED_LIBS)
      set(NC_HDF5_LINK_TYPE "static")
      set(NC_HDF5_LINK_TYPE_UPPER "STATIC")
      ADD_DEFINITIONS(-DH5_BUILT_AS_STATIC_LIB )
    endif(NC_FIND_SHARED_LIBS)

    #####
    # First, find the C and HL libraries.
    #
    # This has been updated to reflect what is in the hdf5
    # examples, even though the previous version of what we
    # had worked.
    #####
    if(MSVC)
      set(SEARCH_PACKAGE_NAME ${HDF5_PACKAGE_NAME})
      find_package(HDF5 NAMES ${SEARCH_PACKAGE_NAME} COMPONENTS C HL CONFIG REQUIRED ${NC_HDF5_LINK_TYPE})
    else(MSVC)
      find_package(HDF5 COMPONENTS C HL REQUIRED)
    endif(MSVC)

    ##
    # Next, check the HDF5 version. This will inform which
    # HDF5 variables we need to munge.
    ##

    # Some versions of HDF5 set HDF5_VERSION_STRING instead of HDF5_VERSION
    if(HDF5_VERSION_STRING AND NOT HDF5_VERSION)
      set(HDF5_VERSION ${HDF5_VERSION_STRING})
    endif()


    ###
    # If HDF5_VERSION is undefined, attempt to determine it programatically.
    ###
    if("${HDF5_VERSION}" STREQUAL "")
      message(STATUS "HDF5_VERSION not detected. Attempting to determine programatically.")
      IF (EXISTS "${HDF5_INCLUDE_DIR}/H5pubconf.h")
        file(READ "${HDF5_INCLUDE_DIR}/H5pubconf.h" _hdf5_version_lines
          REGEX "#define[ \t]+H5_VERSION")
        string(REGEX REPLACE ".*H5_VERSION .*\"\(.*\)\".*" "\\1" _hdf5_version "${_hdf5_version_lines}")
        set(HDF5_VERSION "${_hdf5_version}" CACHE STRING "")
        unset(_hdf5_version)
        unset(_hdf5_version_lines)
        message(STATUS "Found HDF5 libraries version ${HDF5_VERSION}")
      endif()
    else()
      set(HDF5_VERSION ${HDF5_VERSION} CACHE STRING "")
    endif()

    ###
    # If HDF5_VERSION is still empty, we have a problem.
    # Error out.
    ###
    if("${HDF5_VERSION}" STREQUAL "")
      message(FATAL_ERR "Unable to determine HDF5 version.  NetCDF requires at least version ${HDF5_VERSION_REQUIRED}. Please ensure that libhdf5 is installed and accessible.")
    endif()

    ###
    # Now that we know HDF5_VERSION isn't empty, we can check for minimum required version,
    # and toggle various options.
    ###

    if(${HDF5_VERSION} VERSION_LESS ${HDF5_VERSION_REQUIRED})
      message(FATAL_ERROR "netCDF requires at least HDF5 ${HDF5_VERSION_REQUIRED}. Found ${HDF5_VERSION}.")
    endif()



    ##
    # Include the HDF5 include directory.
    ##
    if(HDF5_INCLUDE_DIRS AND NOT HDF5_INCLUDE_DIR)
      set(HDF5_INCLUDE_DIR ${HDF5_INCLUDE_DIRS} )
    endif()
    message(STATUS "Using HDF5 include dir: ${HDF5_INCLUDE_DIR}")
    target_include_directories(netcdf
      PRIVATE
        ${HDF5_INCLUDE_DIR}
    )

    ###
    # This is the block where we figure out what the appropriate
    # variables are, and we ensure that we end up with
    # HDF5_C_LIBRARY, HDF5_HL_LIBRARY and HDF5_LIBRARIES.
    ###
    if(MSVC)
      ####
      # Environmental variables in Windows when using MSVC
      # are a hot mess between versions.
      ####

      ##
      # HDF5 1.8.15 defined HDF5_LIBRARIES.
      ##
      if(${HDF5_VERSION} VERSION_LESS "1.8.16")
        set(HDF5_C_LIBRARY hdf5 )
        set(HDF5_C_LIBRARY_hdf5 hdf5 )
      endif(${HDF5_VERSION} VERSION_LESS "1.8.16")

      if(${HDF5_VERSION} VERSION_GREATER "1.8.15")
        if(NOT HDF5_LIBRARIES AND HDF5_C_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY AND HDF5_HL_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY)
          set(HDF5_C_LIBRARY ${HDF5_C_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY} )
          set(HDF5_C_LIBRARY_hdf5 ${HDF5_C_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY} )
          set(HDF5_HL_LIBRARY ${HDF5_HL_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY} )

      	  set(HDF5_LIBRARIES ${HDF5_C_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY} ${HDF5_HL_${NC_HDF5_LINK_TYPE_UPPER}_LIBRARY} )
        endif()
      endif(${HDF5_VERSION} VERSION_GREATER "1.8.15")

    else(MSVC)

      # Depending on the install, either HDF5_hdf_library or
      # HDF5_C_LIBRARIES may be defined.  We must check for either.
      if(HDF5_C_LIBRARIES AND NOT HDF5_hdf5_LIBRARY)
        set(HDF5_hdf5_LIBRARY ${HDF5_C_LIBRARIES} )
      endif()

      # Some versions of find_package set HDF5_C_LIBRARIES, but not HDF5_C_LIBRARY
      # We use HDF5_C_LIBRARY below, so need to make sure it is set.
      if(HDF5_C_LIBRARIES AND NOT HDF5_C_LIBRARY)
        set(HDF5_C_LIBRARY ${HDF5_C_LIBRARIES} )
      endif()

      # Same issue as above...
      if(HDF5_HL_LIBRARIES AND NOT HDF5_HL_LIBRARY)
        set(HDF5_HL_LIBRARY ${HDF5_HL_LIBRARIES} )
      endif()

    endif(MSVC)
    if(NOT HDF5_C_LIBRARY)
      set(HDF5_C_LIBRARY hdf5 )
    endif()

  endif(HDF5_C_LIBRARY AND HDF5_HL_LIBRARY AND HDF5_INCLUDE_DIR)

  find_package(Threads)

  # There is a missing case in the above code so default it
  if(NOT HDF5_C_LIBRARY_hdf5 OR "${HDF5_C_LIBRARY_hdf5}" STREQUAL "" )
    set(HDF5_C_LIBRARY_hdf5 "${HDF5_C_LIBRARY}" )
  endif()

  find_path(HAVE_HDF5_H hdf5.h PATHS ${HDF5_INCLUDE_DIR} NO_DEFAULT_PATH)
  if(NOT HAVE_HDF5_H)
    message(FATAL_ERROR "Compiling a test with hdf5 failed. Either hdf5.h cannot be found, or the log messages should be checked for another reason.")
  else(NOT HAVE_HDF5_H)
    target_include_directories(netcdf
      PRIVATE
        ${HAVE_HDF5_H}
    )
  endif(NOT HAVE_HDF5_H)

  include(cmake/check_hdf5.cmake)

  # Check to ensure that HDF5 was built with zlib.
  # This needs to be near the beginning since we
  # need to know whether to add "-lz" to the symbol
  # tests below.

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

  message("-- HDF5_UTF8_PATHS (HDF5 version 1.10.6+): ${HDF5_UTF8_PATHS}")

  # Find out if HDF5 was built with parallel support.
  # Do that by checking for the targets H5Pget_fapl_mpiposx and
  # H5Pget_fapl_mpio in ${HDF5_LIB}.

  # H5Pset_fapl_mpiposix and H5Pget_fapl_mpiposix have been removed since HDF5 1.8.12.
  # Use H5Pset_fapl_mpio and H5Pget_fapl_mpio, instead.
  # CHECK_LIBRARY_EXISTS(${HDF5_C_LIBRARY_hdf5} H5Pget_fapl_mpiposix "" HDF5_IS_PARALLEL_MPIPOSIX)

  CHECK_LIBRARY_EXISTS(${HDF5_C_LIBRARY_hdf5} H5Pget_fapl_mpio "" HDF5_IS_PARALLEL_MPIO)
  if(HDF5_IS_PARALLEL_MPIO)
    set(HDF5_PARALLEL ON)
  else()
    set(HDF5_PARALLEL OFF)
  endif()
  set(HDF5_PARALLEL ${HDF5_PARALLEL} )

  #Check to see if HDF5 library has collective metadata APIs, (HDF5 >= 1.10.0)
  CHECK_LIBRARY_EXISTS(${HDF5_C_LIBRARY_hdf5} H5Pset_all_coll_metadata_ops "" HDF5_HAS_COLL_METADATA_OPS)

  if(HDF5_PARALLEL)
	set(HDF5_CC h5pcc )
  else()
	set(HDF5_CC h5cc )
  endif()

  # Check to see if H5Dread_chunk is available
  CHECK_LIBRARY_EXISTS(${HDF5_C_LIBRARY_hdf5} H5Dread_chunk "" HAS_READCHUNKS)

  # Check to see if H5Pset_fapl_ros3 is available
  CHECK_LIBRARY_EXISTS(${HDF5_C_LIBRARY_hdf5} H5Pset_fapl_ros3 "" HAS_HDF5_ROS3)

  # Check to see if this is hdf5-1.10.3 or later.
  if(HAS_READCHUNKS)
    set(HDF5_SUPPORTS_PAR_FILTERS ON )
    set(ENABLE_NCDUMPCHUNKS ON )
  endif()

  # Record if ROS3 Driver is available
  if(HAS_HDF5_ROS3)
    set(ENABLE_HDF5_ROS3 ON )
  endif()

  IF (HDF5_SUPPORTS_PAR_FILTERS)
    set(HDF5_HAS_PAR_FILTERS TRUE CACHE BOOL "" )
    set(HAS_PAR_FILTERS yes CACHE STRING "" )
  else()
    set(HDF5_HAS_PAR_FILTERS FALSE CACHE BOOL "" )
    set(HAS_PAR_FILTERS no CACHE STRING "" )
  endif()

  find_path(HAVE_HDF5_H hdf5.h PATHS ${HDF5_INCLUDE_DIR} NO_DEFAULT_PATH)
  if(NOT HAVE_HDF5_H)
    message(FATAL_ERROR "Compiling a test with hdf5 failed. Either hdf5.h cannot be found, or the log messages should be checked for another reason.")
  else(NOT HAVE_HDF5_H)
    target_include_directories(netcdf
      PRIVATE
        ${HAVE_HDF5_H}
    )
  endif(NOT HAVE_HDF5_H)

  #option to include HDF5 High Level header file (hdf5_hl.h) in case we are not doing a make install
  target_include_directories(netcdf
    PRIVATE
      ${HDF5_HL_INCLUDE_DIR}
  )

endif(USE_HDF5)

################################
# Curl
################################
# See if we have libcurl
find_package(CURL)
target_compile_options(netcdf
  PRIVATE
    -DCURL_STATICLIB=1
)
target_include_directories(netcdf
  PRIVATE
    ${CURL_INCLUDE_DIRS}
)

# Define a test flag for have curl library
if(CURL_LIBRARIES OR CURL_LIBRARY)
  set(FOUND_CURL TRUE)
else()
  set(FOUND_CURL FALSE)
endif()
set(FOUND_CURL ${FOUND_CURL} TRUE )

# Start disabling if curl not found
if(NOT FOUND_CURL)
  message(WARNING "ENABLE_REMOTE_FUNCTIONALITY requires libcurl; disabling")
  set(ENABLE_REMOTE_FUNCTIONALITY OFF CACHE BOOL "ENABLE_REMOTE_FUNCTIONALITY requires libcurl; disabling" FORCE )
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
IF (ENABLE_FILTER_SZIP)
  find_package(Szip)
elseif(ENABLE_NCZARR)
  find_package(Szip)
endif()
IF (ENABLE_FILTER_BZ2)
  find_package(Bz2)
endif()
IF (ENABLE_FILTER_BLOSC)
  find_package(Blosc)
endif()
IF (ENABLE_FILTER_ZSTD)
  find_package(Zstd)
endif()

# Accumulate standard filters
set(STD_FILTERS "deflate") # Always have deflate*/
set_std_filter(Szip)
set(HAVE_SZ ${Szip_FOUND})
set(USE_SZIP ${HAVE_SZ})
set_std_filter(Blosc)
if(Zstd_FOUND)
  set_std_filter(Zstd)
  set(HAVE_ZSTD ON)
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

IF (ENABLE_NCZARR_ZIP)
  find_package(Zip REQUIRED)
  target_include_directories(netcdf
    PRIVATE
      ${Zip_INCLUDE_DIRS}
  )
endif ()

################################
# S3
################################
# Note we check for the library after checking for enable_s3
# because for some reason this screws up if we unconditionally test for sdk
# and it is not available. Fix someday
if(ENABLE_S3)
  if(NOT ENABLE_S3_INTERNAL)
    # See if aws-s3-sdk is available
    find_package(AWSSDK REQUIRED COMPONENTS s3;transfer)
    if(AWSSDK_FOUND)
      set(ENABLE_S3_AWS ON CACHE BOOL "S3 AWS" FORCE)
      target_include_directories(netcdf
        PRIVATE
          ${AWSSDK_INCLUDE_DIR}
      )
    else(AWSSDK_FOUND)
      set(ENABLE_S3_AWS OFF CACHE BOOL "S3 AWS" FORCE)
    endif(AWSSDK_FOUND)
  else(NOT ENABLE_S3_INTERNAL)
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

  endif(NOT ENABLE_S3_INTERNAL)
else()
  set(ENABLE_S3_AWS OFF CACHE BOOL "S3 AWS" FORCE)
endif()

################################
# LibXML
################################
# see if we have libxml2
if(ENABLE_LIBXML2)
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
endif(ENABLE_LIBXML2)

################################
# MPI
################################ 
if(ENABLE_PARALLEL4 OR HDF5_PARALLEL)
  find_package(MPI REQUIRED)
endif()

################################
# parallel IO
################################ 
if(ENABLE_PNETCDF)
  find_library(PNETCDF NAMES pnetcdf)
  find_path(PNETCDF_INCLUDE_DIR pnetcdf.h)
  if(NOT PNETCDF)
    message(STATUS "Cannot find PnetCDF library. Disabling PnetCDF support.")
    set(USE_PNETCDF OFF CACHE BOOL "")
  endif()
endif()

################################
# Doxygen
################################ 

if(ENABLE_DOXYGEN)
  find_package(Doxygen REQUIRED)
endif()

################################
# NC_DPKG
################################ 
if (NETCDF_PACKAGE)
  find_program(NC_DPKG NAMES dpkg)
endif()
