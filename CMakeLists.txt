## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2012-2018, see the COPYRIGHT file for more information.
#

##################################
# Set Project Properties
##################################

cmake_minimum_required(VERSION 3.20.0)

#Project Name
project(netCDF
  LANGUAGES C CXX
  HOMEPAGE_URL "https://www.unidata.ucar.edu/software/netcdf/"
  DESCRIPTION "NetCDF is a set of software libraries and machine-independent data formats that support the creation, access, and sharing of array-oriented scientific data."
  VERSION 4.9.4
)

#####
# Version Info:
#
# Release Version
# Library Version
# SO Version
#
# SO Version is computed from library version. See:
# http://www.gnu.org/software/libtool/manual/libtool.html#Libtool-versioning
#####

set(NC_VERSION_NOTE "-development")
set(netCDF_VERSION ${PROJECT_VERSION}${NC_VERSION_NOTE})
set(VERSION ${netCDF_VERSION})
set(NC_VERSION ${netCDF_VERSION})
set(PACKAGE_VERSION ${VERSION})

# These values should match those in configure.ac
set(netCDF_LIB_VERSION 22)
set(netCDF_SO_VERSION 22)

#Add custom CMake Module
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${PROJECT_SOURCE_DIR}/cmake")

set(PACKAGE "netCDF" CACHE STRING "")

include(netcdf_functions_macros)
include(deprecated)


# Backport of built-in `PROJECT_IS_TOP_LEVEL` from CMake 3.21
if (NOT DEFINED NETCDF_IS_TOP_LEVEL)
  set(NETCDF_IS_TOP_LEVEL OFF)
  if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(NETCDF_IS_TOP_LEVEL ON)
  endif ()
endif ()


################################
# The target
################################

##
# Default building shared libraries.
# BUILD_SHARED_LIBS is provided by/used by
# CMake directly.
##
option(BUILD_SHARED_LIBS "Configure netCDF as a shared library." ON)
if(BUILD_SHARED_LIBS)
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

add_library(netcdf)
add_library(netCDF::netcdf ALIAS netcdf)



# Version of the dispatch table. This must match the value in
# configure.ac.
set(NC_DISPATCH_VERSION 5)

# Version of the plugin path dispatch table. This must match the value in
# configure.ac.
set(NC_PLUGINPATH_DISPATCH_VERSION 1)

# Get system configuration, Use it to determine osname, os release, cpu. These
# will be used when committing to CDash.
find_program(UNAME NAMES uname)
if(UNAME)
  getuname(osname -s)
  getuname(osrel  -r)
  getuname(cpu    -m)
  getuname(host   -n)
  set(TMP_BUILDNAME "${osname}-${osrel}-${cpu}")
endif()

find_program(GETFATTR NAMES getfattr)
if(GETFATTR)
  set(HAVE_GETFATTR TRUE)
endif()

# Define some Platforms
if(osname MATCHES "CYGWIN.*")
  set(ISCYGWIN yes)
endif()
  if(osname MATCHES "Darwin.*")
  set(ISOSX yes)
endif()
if(MSVC)
  set(ISMSVC yes)
endif()
if(osname MATCHES "MINGW.*" OR osname MATCHES "MSYS.*")
  set(ISMINGW yes)
  set(MINGW yes)
endif()

###
# Allow for some customization of the buildname.
# This will make it easier to identify different builds,
# based on values passed from command line/shell scripts.
#
# For ctest scripts, we can use CTEST_BUILD_NAME.
###

set(BUILDNAME_PREFIX "" CACHE STRING "")
set(BUILDNAME_SUFFIX "" CACHE STRING "")

if(BUILDNAME_PREFIX)
  set(TMP_BUILDNAME "${BUILDNAME_PREFIX}-${TMP_BUILDNAME}")
endif()

if(BUILDNAME_SUFFIX)
  set(TMP_BUILDNAME "${TMP_BUILDNAME}-${BUILDNAME_SUFFIX}")
endif()

if(NOT BUILDNAME)
  set(BUILDNAME "${TMP_BUILDNAME}" CACHE STRING "Build name variable for CDash")
endif()
###
# End BUILDNAME customization.
###

# For CMAKE_INSTALL_LIBDIR
include(GNUInstallDirs)

if(MSVC)
  set(GLOBAL PROPERTY USE_FOLDERS ON)
  target_compile_options(netcdf PUBLIC "/utf-8")
endif()

# auto-configure style checks, other CMake modules.
include(CheckLibraryExists)
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)
include(CheckCSourceCompiles)
include(TestBigEndian)
include(CheckSymbolExists)
include(GetPrerequisites)
include(CheckCCompilerFlag)

# A check to see if the system is big endian
TEST_BIG_ENDIAN(BIGENDIAN)
if(${BIGENDIAN})
  set(WORDS_BIGENDIAN "1")
endif(${BIGENDIAN})

# Define a function to convert various true or false values
# to either TRUE|FALSE (uppercase).
# If value is not a recognized boolean, then return NOTFOUND
#1, ON, YES, TRUE, Y, 
#0, OFF, NO, FALSE, N, IGNORE, NOTFOUND -NOTFOUND ""

set(TRUELIST "on;yes;y;true")
set(FALSELIST "off;no;n;false;0;ignore;notfound")

# Set the build type.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE DEBUG CACHE STRING "Choose the type of build, options are: None, Debug, Release."
    FORCE)
endif()

# Set build type uppercase
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)

# Determine the configure date.

if(DEFINED ENV{SOURCE_DATE_EPOCH})
	execute_process(
	  COMMAND "date" "-u" "-d" "@$ENV{SOURCE_DATE_EPOCH}"
	  OUTPUT_VARIABLE CONFIG_DATE
	  )
else()
	execute_process(
	  COMMAND date
	  OUTPUT_VARIABLE CONFIG_DATE
	  )
endif()
if(CONFIG_DATE)
	string(STRIP ${CONFIG_DATE} CONFIG_DATE)
endif()
##
# Allow for extra dependencies.
##

set(EXTRA_DEPS "")

################################
# End Project Properties
################################

################################
# Set CTest Properties
################################

enable_testing()
include(CTest)

# Set Memory test program for non-MSVC based builds.
# Assume valgrind for now.
if((NOT MSVC) AND (NOT MINGW) AND (NOT ISCYGWIN))
  set(CTEST_MEMORYCHECK_COMMAND valgrind CACHE STRING "")
endif()

# Set variable to define the build type.
include(GenerateExportHeader)

################################
# End CTest Properties
################################


################################
# Compiler and Linker Configuration
################################

# Set in support of https://github.com/Unidata/netcdf-c/issues/2700
if(${CMAKE_C_COMPILER_ID} MATCHES "Intel")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fhonor-infinities")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fhonor-infinities")
endif()

option(NETCDF_FIND_SHARED_LIBS "Find dynamically-built versions of dependent libraries" ${BUILD_SHARED_LIBS})

##
# We've had a request to allow for non-versioned shared libraries.
# This seems reasonable enough to accommodate.  See
# https://github.com/Unidata/netcdf-c/issues/228 for more info.
##
option(NETCDF_ENABLE_SHARED_LIBRARY_VERSION "Encode the library SO version in the file name of the generated library file." ON)

# Set some default linux gcc & apple compiler options for
# debug builds.
if(CMAKE_COMPILER_IS_GNUCC OR APPLE)
  option(NETCDF_ENABLE_COVERAGE_TESTS "Enable compiler flags needed to perform coverage tests." OFF)
  option(NETCDF_ENABLE_CONVERSION_WARNINGS "Enable warnings for implicit conversion from 64 to 32-bit datatypes." ON)
  option(NETCDF_ENABLE_LARGE_FILE_TESTS "Enable large file tests." OFF)

  # Debugging flags
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")

  # Check to see if -Wl,--no-undefined is supported.
  CHECK_C_LINKER_FLAG("-Wl,--no-undefined" LIBTOOL_HAS_NO_UNDEFINED)

  if(LIBTOOL_HAS_NO_UNDEFINED)
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -Wl,--no-undefined")
  endif()
  set(CMAKE_REQUIRED_FLAGS "${TMP_CMAKE_REQUIRED_FLAGS}")

  # Coverage tests need to have optimization turned off.
  if(NETCDF_ENABLE_COVERAGE_TESTS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    message(STATUS "Coverage Tests: On.")
  endif()

  # Warnings for 64-to-32 bit conversions.
  if(NETCDF_ENABLE_CONVERSION_WARNINGS)
    CHECK_C_COMPILER_FLAG(-Wconversion CC_HAS_WCONVERSION)
    CHECK_C_COMPILER_FLAG(-Wshorten-64-to-32 CC_HAS_SHORTEN_64_32)

    if(CC_HAS_SHORTEN_64_32)
      set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wshorten-64-to-32")
    endif()
    if(CC_HAS_WCONVERSION)
      set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wconversion")
    endif()

  endif(NETCDF_ENABLE_CONVERSION_WARNINGS)

endif(CMAKE_COMPILER_IS_GNUCC OR APPLE)

# End default linux gcc & apple compiler options.

# Use relative pathnames in __FILE__ macros on MINGW:
if(MINGW)
  CHECK_C_COMPILER_FLAG("-fmacro-prefix-map='${CMAKE_SOURCE_DIR}'=." CC_HAS_MACRO_PREFIX_MAP)
  if(CC_HAS_MACRO_PREFIX_MAP)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmacro-prefix-map='${CMAKE_SOURCE_DIR}'=.")
  endif()
endif()

# Suppress CRT Warnings.
# Only necessary for Windows
if(MSVC)
  target_compile_definitions(netcdf PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# Support ANSI format specifiers for *printf on MINGW:
if(MINGW)
  target_compile_definitions(netcdf PRIVATE __USE_MINGW_ANSI_STDIO=1)
endif()

#####
# System inspection checks
#####
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/oc2)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libsrc)
set(CMAKE_REQUIRED_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/libsrc)

################################
# End Compiler Configuration
################################

##
# Configuration for post-install RPath
# Adapted from http://www.cmake.org/Wiki/CMake_RPATH_handling
##
if(NOT WIN32 AND BUILD_SHARED_LIBS)
  # use, i.e. don't skip the full RPATH for the build tree
  set(CMAKE_SKIP_BUILD_RPATH  FALSE)

  # when building, don't use the install RPATH already
  # (but later on when installing)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

  if(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
  endif(APPLE)

  # add the automatically determined parts of the RPATH
  # which point to directories outside the build tree to the install RPATH
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

  # the RPATH to be used when installing,
  # but only if it's not a system directory
  list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
  if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
  endif("${isSystemDir}" STREQUAL "-1")

endif()

##
# End configuration for post-install RPath
##

################################
# Option checks
################################

# Default Cache variables.
set(DEFAULT_CHUNK_SIZE 16777216 CACHE STRING "Default Chunk Cache Size.")
set(DEFAULT_CHUNK_CACHE_SIZE 16777216U CACHE STRING "Default Chunk Cache Size.")
set(DEFAULT_CHUNKS_IN_CACHE 1000 CACHE STRING "Default number of chunks in cache.")
set(DEFAULT_CHUNK_CACHE_PREEMPTION 0.75 CACHE STRING "Default file chunk cache preemption policy (a number between 0 and 1, inclusive.")

# HDF5 default cache size values
set(CHUNK_CACHE_SIZE ${DEFAULT_CHUNK_CACHE_SIZE} CACHE STRING "Default HDF5 Chunk Cache Size.")
set(CHUNK_CACHE_NELEMS ${DEFAULT_CHUNKS_IN_CACHE} CACHE STRING "Default maximum number of elements in cache.")
set(CHUNK_CACHE_PREEMPTION ${DEFAULT_CHUNK_CACHE_PREEMPTION} CACHE STRING "Default file chunk cache preemption policy for HDf5 files(a number between 0 and 1, inclusive.")

set(NETCDF_LIB_NAME "" CACHE STRING "Default name of the netcdf library.")
set(TEMP_LARGE "." CACHE STRING "Where to put large temp files if large file tests are run.")
set(NCPROPERTIES_EXTRA "" CACHE STRING "Specify extra pairs for _NCProperties.")

if(NOT NETCDF_LIB_NAME STREQUAL "")
  set(MOD_NETCDF_NAME ON)
endif()

# Set the appropriate compiler/architecture for universal OSX binaries.
if(${CMAKE_SYSTEM_NAME} EQUAL "Darwin")
  set(CMAKE_OSX_ARCHITECTURES i386;x86_64)
endif(${CMAKE_SYSTEM_NAME} EQUAL "Darwin")


# Option to use Static Runtimes in MSVC
if(MSVC)
  option(NETCDF_USE_STATIC_CRT "Use static CRT Libraries ('\\MT')." OFF)
  if(NETCDF_USE_STATIC_CRT)
    set(USE_STATIC_CRT ON)
    specify_static_crt_flag()
  endif()
endif()

# Option to build netCDF Version 2
OPTION (NETCDF_ENABLE_V2_API "Build netCDF Version 2." ON)
set(BUILD_V2 ${NETCDF_ENABLE_V2_API})
if(NOT NETCDF_ENABLE_V2_API)
  set(NO_NETCDF_2 ON)
else(NOT NETCDF_ENABLE_V2_API)
  set(USE_NETCDF_2 TRUE)
endif(NOT NETCDF_ENABLE_V2_API)

# Option to build utilities
option(NETCDF_BUILD_UTILITIES "Build ncgen, ncgen3, ncdump." ON)
option(NETCDF_GENERATE_NCGEN "Automatically regenerate ncgen C files if required" OFF)


# Option to use MMAP
option(NETCDF_ENABLE_MMAP "Use MMAP." ON)

# Option to use examples.
option(NETCDF_ENABLE_EXAMPLES "Build Examples" ON)

###
# Allow the user to specify libraries
# to link against, similar to automakes 'LIBS' variable.
###
set(NC_EXTRA_DEPS "" CACHE STRING "Additional libraries to link against.")
if(NC_EXTRA_DEPS)
  string(REPLACE " " ";" DEPS_LIST ${NC_EXTRA_DEPS})
  foreach(_DEP ${DEPS_LIST})
    string(REGEX REPLACE "^-l" "" _LIB ${_DEP})
    FIND_LIBRARY("${_LIB}_DEP" NAMES "${_LIB}" "lib${_LIB}")
    message(STATUS ${${_LIB}_DEP})
    if("${${_LIB}_DEP}" STREQUAL "${_LIB}_DEP-NOTFOUND")
      message(FATAL_ERROR "Error finding ${_LIB}.")
    else()
      message(STATUS "Found ${_LIB}: ${${_LIB}_DEP}")
    endif()
    set(EXTRA_DEPS ${EXTRA_DEPS} "${${_LIB}_DEP}")
  endforeach()
  message(STATUS "Extra deps: ${EXTRA_DEPS}")
  list(REMOVE_DUPLICATES EXTRA_DEPS)
  set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${EXTRA_DEPS})
endif()
###
# End user-specified dependent libraries.
###

################################
# Format Option checks
################################

# As a long term goal, and because it is now the case that
# NETCDF_ENABLE_NCZARR => USE_NETCDF4, so make the external options
# NETCDF_ENABLE_NETCDF_4 and NETCDF_ENABLE_NETCDF4 obsolete
# in favor of NETCDF_ENABLE_HDF5.
# We will do the following for one more release cycle.
# 1. Make NETCDF_ENABLE_NETCDF_4 be an alias for NETCDF_ENABLE_NETCDF4.
# 2. Make NETCDF_ENABLE_NETCDF4 an alias for NETCDF_ENABLE_HDF5.
# 3. Internally, convert most (but not all) uses of USE_NETCDF_4 and USE_NETCDF4 to USE_HDF5.

# Collect the values of NETCDF_ENABLE_NETCDF_4, NETCDF_ENABLE_NETCDF4, and NETCDF_ENABLE_HDF5.

# Figure out which options are defined and process options
if(DEFINED NETCDF_ENABLE_NETCDF_4)
    set(UNDEF_NETCDF_4 OFF CACHE BOOL "") 
    option(NETCDF_ENABLE_NETCDF_4 "" ON)
else()
    set(UNDEF_NETCDF_4 ON CACHE BOOL "") 
endif()
if(DEFINED NETCDF_ENABLE_NETCDF4)
    set(UNDEF_NETCDF4 OFF CACHE BOOL "") 
    option(NETCDF_ENABLE_NETCDF4 "" ON)
else()
    set(UNDEF_NETCDF4 ON CACHE BOOL "") 
endif()
if(DEFINED NETCDF_ENABLE_HDF5)
    set(UNDEF_HDF5 OFF CACHE BOOL "") 
    option(NETCDF_ENABLE_HDF5 "" ON)
else()
    set(UNDEF_HDF5 ON CACHE BOOL "") 
endif()

if(NOT UNDEF_NETCDF_4)
message(WARNING "NETCDF_ENABLE_NETCDF_4 is deprecated; please use NETCDF_ENABLE_HDF5")
endif()

if(NOT UNDEF_NETCDF4)
message(WARNING "NETCDF_ENABLE_NETCDF4 is deprecated; please use NETCDF_ENABLE_HDF5")
endif()

# NETCDF_ENABLE_NETCDF_4 overrides NETCDF_ENABLE_NETCDF4 if latter not defined.

if((NOT "${UNDEF_NETCDF_4}") AND UNDEF_NETCDF4)
set(NETCDF_ENABLE_NETCDF4 ${NETCDF_ENABLE_NETCDF_4} CACHE BOOL "" FORCE) 
endif()
# NETCDF_ENABLE_NETCDF4 overrides NETCDF_ENABLE_HDF5 if latter not defined.
if((NOT "${UNDEF_NETCDF4}") AND UNDEF_HDF5)
set(NETCDF_ENABLE_HDF5 "${NETCDF_ENABLE_HDF5}" CACHE BOOL "" FORCE) 
endif()
# Otherwise, use NETCDF_ENABLE_HDF5 default
if(UNDEF_HDF5)
set(NETCDF_ENABLE_HDF5 ON CACHE BOOL "" FORCE) 
endif()

# Turn off NETCDF_ENABLE_NETCDF4 because it will be used
# as a shorthand for NETCDF_ENABLE_HDF5|NETCDF_ENABLE_HDF4|NETCDF_ENABLE_NCZARR
set(NETCDF_ENABLE_NETCDF4 OFF CACHE BOOL "" FORCE)
option(NETCDF_ENABLE_DAP "Enable DAP2 and DAP4 Client." ON)
option(NETCDF_ENABLE_NCZARR "Enable NCZarr Client." ON)
option(NETCDF_ENABLE_PNETCDF "Build with parallel I/O for CDF-1, 2, and 5 files using PnetCDF." OFF)
set(NETCDF_ENABLE_CDF5 AUTO CACHE STRING "AUTO")
option(NETCDF_ENABLE_CDF5 "Enable CDF5 support" ON)
option(NETCDF_ENABLE_HDF4 "Enable HDF4 Read Support" OFF)
option(NETCDF_ENABLE_HDF4_FILE_TESTS "Enable HDF4 File Tests" ${NETCDF_ENABLE_HDF4})
if(NETCDF_ENABLE_HDF4)
  set(USE_HDF4 ON)
endif()

# Netcdf-4 support (i.e. libsrc4) is required by more than just HDF5 (e.g. NCZarr)
# So depending on what above formats are enabled, enable netcdf-4
if(NETCDF_ENABLE_HDF5 OR NETCDF_ENABLE_HDF4 OR NETCDF_ENABLE_NCZARR)
  set(NETCDF_ENABLE_NETCDF4 ON CACHE BOOL "Enable netCDF-4 API" FORCE)
endif()

# enable|disable all forms of network access
option(NETCDF_ENABLE_REMOTE_FUNCTIONALITY "Enable|disable all forms remote data access (DAP, S3, etc)" ON)
if(NOT NETCDF_ENABLE_REMOTE_FUNCTIONALITY AND NETCDF_ENABLE_DAP)
message(WARNING "NETCDF_ENABLE_REMOTE_FUNCTIONALITY=NO => NETCDF_ENABLE_DAP[4]=NO")
set(NETCDF_ENABLE_DAP OFF CACHE BOOL "NETCDF_ENABLE_REMOTE_FUNCTIONALITY=NO => NETCDF_ENABLE_DAP=NO" FORCE)
set(NETCDF_ENABLE_DAP2 OFF CACHE BOOL "NETCDF_ENABLE_REMOTE_FUNCTIONALITY=NO => NETCDF_ENABLE_DAP2=NO" FORCE)
set(NETCDF_ENABLE_DAP4 OFF CACHE BOOL "NETCDF_ENABLE_REMOTE_FUNCTIONALITY=NO => NETCDF_ENABLE_DAP4=NO" FORCE)
endif()

# Option to Build DLL
if(WIN32)
  option(NETCDF_ENABLE_DLL "Build a Windows DLL." ${BUILD_SHARED_LIBS})
  if(NETCDF_ENABLE_DLL)
    set(BUILD_DLL ON CACHE BOOL "")
    target_compile_definitions(netcdf PRIVATE DLL_EXPORT)
    target_compile_definitions(netcdf PUBLIC DLL_NETCDF)
  endif()
endif()
# Did the user specify a default minimum blocksize for posixio?
set(NCIO_MINBLOCKSIZE 256 CACHE STRING "Minimum I/O Blocksize for netCDF classic and 64-bit offset format files.")

if(NETCDF_ENABLE_NETCDF4)
  set(USE_NETCDF4 ON CACHE BOOL "")
  set(NETCDF_ENABLE_NETCDF4 ON CACHE BOOL "")
else()
  set(USE_HDF4_FILE_TESTS OFF)
  set(USE_HDF4 OFF)
  set(NETCDF_ENABLE_HDF4_FILE_TESTS OFF)
  set(NETCDF_ENABLE_HDF4 OFF)
endif()

# Option Logging, only valid for netcdf4 dispatchers.
option(NETCDF_ENABLE_LOGGING "Enable Logging." OFF)
if(NOT NETCDF_ENABLE_NETCDF4)
  set(NETCDF_ENABLE_LOGGING OFF)
endif()

set(LOGGING ${NETCDF_ENABLE_LOGGING})
set(NETCDF_ENABLE_SET_LOG_LEVEL ${NETCDF_ENABLE_LOGGING})

# Option to allow for strict null file padding.
# See https://github.com/Unidata/netcdf-c/issues/657 for more information
option(NETCDF_ENABLE_STRICT_NULL_BYTE_HEADER_PADDING "Enable strict null byte header padding." OFF)

if(NETCDF_ENABLE_STRICT_NULL_BYTE_HEADER_PADDING)
  set(USE_STRICT_NULL_BYTE_HEADER_PADDING ON CACHE BOOL "")
endif(NETCDF_ENABLE_STRICT_NULL_BYTE_HEADER_PADDING)

# Note that szip management is tricky.
# This is because we have three things to consider:
# 1. is libsz available?
# 2. is szip enabled in HDF5?
# 3. is nczarr enabled?
# We need separate flags for cases 1 and 2

set(USE_HDF5 ${NETCDF_ENABLE_HDF5})

if(NETCDF_ENABLE_DAP)
  set(USE_DAP ON CACHE BOOL "")
  set(NETCDF_ENABLE_DAP2 ON CACHE BOOL "")

  if(NETCDF_ENABLE_HDF5)
    message(STATUS "Enabling DAP4")
    set(NETCDF_ENABLE_DAP4 ON CACHE BOOL "")
  else()
    message(STATUS "Disabling DAP4")
    set(NETCDF_ENABLE_DAP4 OFF CACHE BOOL "")
  endif(NETCDF_ENABLE_HDF5)

else()
  set(NETCDF_ENABLE_DAP2 OFF CACHE BOOL "")
  set(NETCDF_ENABLE_DAP4 OFF CACHE BOOL "")
endif()

# Option to support byte-range reading of remote datasets
option(NETCDF_ENABLE_BYTERANGE "Enable byte-range access to remote datasets.." ${NETCDF_ENABLE_DAP})
if(NOT NETCDF_ENABLE_REMOTE_FUNCTIONALITY AND NETCDF_ENABLE_BYTERANGE)
message(WARNING "NETCDF_ENABLE_REMOTE_FUNCTIONALITY=NO => NETCDF_ENABLE_BYTERANGE=NO")
set(NETCDF_ENABLE_BYTERANGE OFF CACHE BOOL "NETCDF_ENABLE_REMOTE_FUNCTIONALITY=NO => NETCDF_ENABLE_BYTERANGE=NO" FORCE)
endif()

# Option to Enable DAP long tests, remote tests.
option(NETCDF_ENABLE_DAP_REMOTE_TESTS "Enable DAP remote tests." ON)
option(NETCDF_ENABLE_EXTERNAL_SERVER_TESTS "Enable external Server remote tests." OFF)
option(NETCDF_ENABLE_DAP_LONG_TESTS "Enable DAP long tests." OFF)

if(NOT NETCDF_ENABLE_DAP)
set(NETCDF_ENABLE_DAP_REMOTE_TESTS OFF CACHE BOOL "" FORCE)
set(NETCDF_ENABLE_EXTERNAL_SERVER_TESTS OFF CACHE BOOL "" FORCE)
set(NETCDF_ENABLE_DAP_LONG_TESTS OFF CACHE BOOL "" FORCE)
endif()

# Provide a global control for remotetest.
if ("$ENV{REMOTETESTDOWN}" STREQUAL "yes" AND NETCDF_ENABLE_DAP_REMOTE_TESTS)
message(WARNING "ENV(REMOTETESTDOWN) => NETCDF_ENABLE_DAP_REMOTE_TESTS == OFF")
set(NETCDF_ENABLE_DAP_REMOTE_TESTS OFF CACHE BOOL "" FORCE)
endif()

set(REMOTETESTSERVERS "remotetest.unidata.ucar.edu" CACHE STRING "test servers to use for remote test")
set(REMOTETESTSERVERS "remotetest.unidata.ucar.edu" CACHE STRING "test servers to use for remote test")

# Locate some compressors
option(NETCDF_ENABLE_PLUGINS "Enable netCDF Plugins." ON)
option(NETCDF_ENABLE_FILTER_SZIP  "Enable use of Szip compression library if it is available. Required if NETCDF_ENABLE_NCZARR is true." ON)
option(NETCDF_ENABLE_FILTER_BZ2   "Enable use of Bz2 compression library if it is available." ON)
option(NETCDF_ENABLE_FILTER_BLOSC "Enable use of blosc compression library if it is available." ON)
option(NETCDF_ENABLE_FILTER_ZSTD  "Enable use of Zstd compression library if it is available." ON)

# If user wants, then install selected plugins (default on)
set(NETCDF_PLUGIN_INSTALL_DIR "YES" CACHE STRING "Whether and where we should install plugins; defaults to yes")
if(NOT NETCDF_ENABLE_PLUGINS)
  unset(NETCDF_PLUGIN_INSTALL_DIR CACHE)
endif()

# This is ugly, but seems necessary because of CMake's boolean structure
set(boolval FALSE)
if(DEFINED NETCDF_PLUGIN_INSTALL_DIR)
  booleanize(${NETCDF_PLUGIN_INSTALL_DIR} boolval)
  if(boolval)
    set(ENABLE_PLUGIN_INSTALL YES)
    # No actual value was specified
    unset(NETCDF_PLUGIN_INSTALL_DIR CACHE)
  else()
    if(boolval STREQUAL "NOTFOUND")
      # Must be an actual value
      set(ENABLE_PLUGIN_INSTALL YES)
    else()
      set(ENABLE_PLUGIN_INSTALL NO)
    endif()
  endif()
else()
  set(ENABLE_PLUGIN_INSTALL NO)
endif()

# Ensure no defined plugin dir if not enabled
if(NOT ENABLE_PLUGIN_INSTALL)
    unset(NETCDF_PLUGIN_INSTALL_DIR CACHE)
endif()

if(ENABLE_PLUGIN_INSTALL)
    if(NOT DEFINED NETCDF_PLUGIN_INSTALL_DIR)
      # Default to HDF5_PLUGIN_PATH or its default directories
      if(DEFINED ENV{HDF5_PLUGIN_PATH})
        set(NETCDF_PLUGIN_INSTALL_DIR "$ENV{HDF5_PLUGIN_PATH}")
      else()
        if(ISMSVC OR ISMINGW)
	  set(NETCDF_PLUGIN_INSTALL_DIR "$ENV{ALLUSERSPROFILE}\\hdf5\\lib\\plugin")
	else()
	  set(NETCDF_PLUGIN_INSTALL_DIR "/usr/local/hdf5/lib/plugin")
	endif()
      endif()
        message(STATUS "Defaulting to -DPLUGIN_INSTALL_DIR=${NETCDF_PLUGIN_INSTALL_DIR}")
    endif()
endif()

if(ENABLE_PLUGIN_INSTALL)
  # Use the lowest priority dir in the path
  if(NOT ISMSVC AND NOT ISMINGW)
    string(REPLACE ":" ";" PATH_LIST ${NETCDF_PLUGIN_INSTALL_DIR})
  else()
    set(PATH_LIST ${NETCDF_PLUGIN_INSTALL_DIR})
  endif()

  # Get last element
  list(GET PATH_LIST -1 NETCDF_PLUGIN_INSTALL_DIR)
  set(PLUGIN_INSTALL_DIR_SETTING "${NETCDF_PLUGIN_INSTALL_DIR}")
  message(STATUS "Final value of-DPLUGIN_INSTALL_DIR=${NETCDF_PLUGIN_INSTALL_DIR}")
else() # No option specified
    unset(NETCDF_PLUGIN_INSTALL_DIR)
    unset(NETCDF_PLUGIN_INSTALL_DIR CACHE)
    set(PLUGIN_INSTALL_DIR_SETTING "N.A.")
endif()

message(STATUS "ENABLE_PLUGIN_INSTALL=${ENABLE_PLUGIN_INSTALL} PLUGIN_INSTALL_DIR=${NETCDF_PLUGIN_INSTALL_DIR}")

# Try to enable NCZarr zip support
option(NETCDF_ENABLE_NCZARR_ZIP "Enable NCZarr ZIP support." ${NETCDF_ENABLE_NCZARR})

include(CMakeDependentOption)

# libdl is always available; built-in in Windows and OSX
cmake_dependent_option(NETCDF_ENABLE_PLUGINS "Enable dynamically loaded plugins (default on)."
  ON "BUILD_SHARED_LIBS" OFF)
MESSAGE(STATUS "NETCDF_ENABLE_PLUGINS: ${NETCDF_ENABLE_PLUGINS}")

if(NOT WIN32)
  if(HAVE_DLFCN_H)
    include_directories("dlfcn.h")
  endif()
endif()

if(NETCDF_ENABLE_PLUGINS)
  set(USEPLUGINS yes)
endif()

# Enable some developer-only tests
option(NETCDF_ENABLE_EXTRA_TESTS "Enable Extra tests. Some may not work because of known issues. Developers only." OFF)
if(NETCDF_ENABLE_EXTRA_TESTS)
  set(EXTRA_TESTS ON)
endif()

# Option to use bundled XGetopt in place of getopt(). This is mostly useful
# for MSVC builds. If not building utilities or some tests,
# getopt() isn't required at all.
if(MSVC)
  option(NETCDF_ENABLE_XGETOPT "Enable bundled XGetOpt instead of external getopt()." ON)
  if(NETCDF_ENABLE_XGETOPT)
    set(USE_X_GETOPT ON CACHE BOOL "")
  endif()
endif()

set(MATH "")
if(NOT WIN32)

  # STDIO instead of posixio.
  option(NETCDF_ENABLE_STDIO "If true, use stdio instead of posixio (ex. on the Cray)" OFF)
  if(NETCDF_ENABLE_STDIO)
    set(USE_STDIO ON CACHE BOOL "")
  endif()

  # FFIO insteaad of PosixIO
  option(NETCDF_ENABLE_FFIO "If true, use ffio instead of posixio" OFF)
  if(NETCDF_ENABLE_FFIO)
    set(USE_FFIO ON CACHE BOOL "")
  endif()
endif()

# Options for S3 Support
#option(NETCDF_ENABLE_S3 "Enable S3 support." OFF)
option(NETCDF_ENABLE_S3_AWS "Enable S3 support via AWS-CPP-SDK" OFF)
option(NETCDF_ENABLE_S3_INTERNAL "Enable S3 Internal support." OFF)

cmake_dependent_option(NETCDF_ENABLE_S3 "Enable S3 Support" ON "NETCDF_ENABLE_S3_AWS OR NETCDF_ENABLE_S3_INTERNAL" OFF)

option(NETCDF_ENABLE_NCZARR_S3 "Enable NCZarr S3 support; Deprecated in favor of NETCDF_ENABLE_S3" ${NETCDF_ENABLE_S3})

if(NOT NETCDF_ENABLE_REMOTE_FUNCTIONALITY)
  set(NETCDF_ENABLE_S3 OFF CACHE BOOL "" FORCE)
  set(NETCDF_ENABLE_S3_INTERNAL OFF CACHE BOOL "" FORCE)
  set(NETCDF_ENABLE_S3_AWS OFF CACHE BOOL "" FORCE)
  set(NETCDF_ENABLE_NCZARR_S3 OFF CACHE BOOL "" FORCE)
endif()

# Control S3 Testing: Multi-valued option
set(WITH_S3_TESTING OFF CACHE STRING "Control S3 Testing: ON (i.e. all) OFF (i.e. none)  PUBLIC")
    SET_PROPERTY(CACHE WITH_S3_TESTING PROPERTY STRINGS ON OFF PUBLIC)  #
if(WITH_S3_TESTING STREQUAL "")
  set(WITH_S3_TESTING OFF CACHE STRING "") # Default
endif()

if(WITH_S3_TESTING)
message(WARNING "**** DO NOT USE WITH_S3_TESTING=ON UNLESS YOU HAVE ACCESS TO THE UNIDATA S3 BUCKET! ***")
endif()

# NETCDF_ENABLE_NCZARR_S3 is now an alias for NETCDF_ENABLE_S3 (but...)
if (NOT NETCDF_ENABLE_S3 AND NETCDF_ENABLE_NCZARR_S3)
  set(NETCDF_ENABLE_S3 ON CACHE BOOL "NCARR S3" FORCE) # For back compatibility
endif()
unset(NETCDF_ENABLE_NCZARR_S3)

if(NOT NETCDF_ENABLE_REMOTE_FUNCTIONALITY)
  message(WARNING "NETCDF_ENABLE_REMOTE_FUNCTIONALITY=NO => disable all s3 functionality")
  set(NETCDF_ENABLE_S3 OFF CACHE BOOL "" FORCE)
  set(NETCDF_ENABLE_S3_INTERNAL OFF CACHE BOOL "" FORCE)
  set(NETCDF_ENABLE_NCZARR_S3 OFF CACHE BOOL "" FORCE)
  set(NETCDF_ENABLE_HDF5_ROS3 OFF CACHE BOOL "Use ROS3" FORCE)
  set(WITH_S3_TESTING OFF CACHE STRING "" FORCE)
endif()


if(NETCDF_ENABLE_S3)
  if(NOT NETCDF_ENABLE_S3_AWS AND NOT NETCDF_ENABLE_S3_INTERNAL)
    message(FATAL_ERROR "S3 support library not found; please specify option -DNETCDF_ENABLE_S3=NO")
    set(NETCDF_ENABLE_S3 OFF CACHE BOOL "S3 support" FORCE)        
  endif()
  if(NETCDF_ENABLE_S3_AWS AND NETCDF_ENABLE_S3_INTERNAL)
    message(WARNING "Both aws-sdk-cpp and s3-internal enabled => use s3-internal")
    set(NETCDF_ENABLE_S3_AWS OFF CACHE BOOL "S3 AWS" FORCE)        
  endif()
endif()

if(NOT NETCDF_ENABLE_S3)
  if(WITH_S3_TESTING STREQUAL "PUBLIC" OR WITH_S3_TESTING)
    message(WARNING "S3 support is disabled => WITH_S3_TESTING=OFF")
    set(WITH_S3_TESTING OFF CACHE STRING "" FORCE)
  endif()
endif()

option(NETCDF_ENABLE_LIBXML2 "Link against libxml2 if it is available, use the packaged tinyxml2 parser otherwise." ON)
set(XMLPARSER "tinyxml2 (bundled)")

if(NOT NETCDF_ENABLE_BYTERANGE AND NETCDF_ENABLE_HDF5_ROS3)
    message(WARNING "ROS3 support requires NETCDF_ENABLE_BYTERANGE=TRUE; disabling ROS3 support")
    set(NETCDF_ENABLE_HDF5_ROS3 OFF CACHE BOOL "ROS3 support" FORCE)
endif()

##
# Enable Tests
##
option(NETCDF_ENABLE_TESTS "Enable basic tests, run with 'make test'." ON)
if(NETCDF_ENABLE_TESTS)
  set(BUILD_TESTSETS ON CACHE BOOL "")
  # Options for CTest-based tests, dashboards.
  set(NC_CTEST_PROJECT_NAME "netcdf-c" CACHE STRING "Project Name for CTest-based testing purposes.")
  set(NC_CTEST_DROP_SITE "cdash.unidata.ucar.edu:443" CACHE STRING "Dashboard location for CTest-based testing purposes.")
  set(NC_CTEST_DROP_LOC_PREFIX "" CACHE STRING "Prefix for Dashboard location on remote server when using CTest-based testing.")
  set(SUBMIT_URL "https://cdash.unidata.ucar.edu:443")
  if("${host}" STREQUAL "")
    find_program(HOSTNAME_CMD NAMES hostname)
    if(NOT WIN32)
      set(HOSTNAME_ARG "-s")
    endif()
    if(HOSTNAME_CMD)
      execute_process(COMMAND ${HOSTNAME_CMD} "${HOSTNAME_ARG}" OUTPUT_VARIABLE HOSTNAME OUTPUT_STRIP_TRAILING_WHITESPACE)
      set(NC_CTEST_SITE "${HOSTNAME}" CACHE STRING "Hostname of test machine.")
    endif()
  else()
    set(NC_CTEST_SITE "${host}" CACHE STRING "Hostname of test machine.")
  endif()

  if(NC_CTEST_SITE)
    set(SITE "${NC_CTEST_SITE}" CACHE STRING "")
  endif()

  ###
  # This option dictates whether or not to turn on
  # tests which are known to fail.  This is not the
  # same thing as an 'expected failure'. Rather, these
  # are tests that will need to be fixed eventually.
  #
  # By placing them here, we can occasionally turn this
  # flag on and see if any known failures have been
  # fixed in the course of code improvement/other bug
  # fixes.
  #
  # To use this, simply add as a fencepost around tests
  # which are known to fail.
  ###

  option(NETCDF_ENABLE_FAILING_TESTS "Run tests which are known to fail, check to see if any have been fixed." OFF)

  ###
  # Option to turn on unit testing. See
  # https://github.com/Unidata/netcdf-c/pull/1472 for more information.
  ###
  option(NETCDF_ENABLE_UNIT_TESTS "Run Unit Tests." ON)

  ###
  # Option to turn on performance testing.
  # See https://github.com/Unidata/netcdf-c/issues/2627 for more information.
  ###
  option(NETCDF_ENABLE_BENCHMARKS "Run benchmark Tests." OFF)

  set(BUILD_BENCHMARKS ${NETCDF_ENABLE_BENCHMARKS} CACHE BOOL "alias for NETCDF_ENABLE_BENCHMARKS")

  ###
  # End known-failures.
  ###
  MARK_AS_ADVANCED(NETCDF_ENABLE_FAILING_TESTS)
endif()

###
# Option to enable extreme numbers during testing.
###
option(NETCDF_ENABLE_EXTREME_NUMBERS "Enable extreme numbers during testing, such as MAX_INT-1" ON)
if(NETCDF_ENABLE_EXTREME_NUMBERS)
  set(USE_EXTREME_NUMBERS ON)
endif()

# Enable Large file tests
if(NETCDF_ENABLE_LARGE_FILE_TESTS)
  set(LARGE_FILE_TESTS ON)
endif()

option(NETCDF_ENABLE_METADATA_PERF_TESTS "Enable test of metadata performance." OFF)
if(NETCDF_ENABLE_METADATA_PERF_TESTS)
  set(ENABLE_METADATA_PERF ON)
endif()

# Location for large file tests.
set(TEMP_LARGE "." CACHE STRING "Location to store large file tests.")

option(NETCDF_ENABLE_FSYNC "Enable experimental fsync code." OFF)
if(NETCDF_ENABLE_FSYNC)
  set(USE_FSYNC ON)
endif()

# Linux specific large file support flags.
# Modelled after check in CMakeLists.txt for hdf5.
option(NETCDF_ENABLE_LARGE_FILE_SUPPORT "Enable large file support." ON)
if(NETCDF_ENABLE_LARGE_FILE_SUPPORT)
  if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /LARGEADDRESSAWARE")
  else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64")
  endif()
endif()

option(NETCDF_ENABLE_EXAMPLE_TESTS "Run extra example tests.  Requires GNU Sed. Ignored if HDF5 is not Enabled" OFF)
if(NOT NETCDF_ENABLE_HDF5 AND NETCDF_ENABLE_EXAMPLE_TESTS)
  set(NETCDF_ENABLE_EXAMPLE_TESTS OFF)
endif()

##################################
# Dependencies
##################################

include(cmake/dependencies.cmake)

################################
# End Dependencies
################################

# Enable Parallel IO with netCDF-4/HDF5 files using HDF5 parallel I/O.
set(STATUS_PARALLEL "OFF")
set(IMPORT_MPI "")
option(NETCDF_ENABLE_PARALLEL4 "Build netCDF-4 with parallel IO" "${HDF5_PARALLEL}")
option(NETCDF_MPIEXEC "Command to use instead of mpiexec to launch parallel I/O tests" OFF)
if(NETCDF_ENABLE_PARALLEL4 AND NETCDF_ENABLE_HDF5)
  if(NOT HDF5_PARALLEL)
    set(USE_PARALLEL OFF CACHE BOOL "")
    message(STATUS "Cannot find HDF5 library built with parallel support. Disabling parallel build.")
  else()
    set(HDF5_PARALLEL ON CACHE BOOL "")
    set(USE_PARALLEL ON CACHE BOOL "")
    set(USE_PARALLEL4 ON CACHE BOOL "")
    set(STATUS_PARALLEL "ON")
    if(NETCDF_MPIEXEC)
      set(MPIEXEC "${NETCDF_MPIEXEC}")
    endif()
    message(STATUS "MPIEXEC command will be ${MPIEXEC}")
    configure_file("${netCDF_SOURCE_DIR}/nc_test4/run_par_test.sh.in"
      "${netCDF_BINARY_DIR}/tmp/run_par_test.sh" @ONLY NEWLINE_STYLE LF)
    file(COPY "${netCDF_BINARY_DIR}/tmp/run_par_test.sh"
      DESTINATION ${netCDF_BINARY_DIR}/nc_test4
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    configure_file("${netCDF_SOURCE_DIR}/nc_test4/run_par_warn_test.sh.in"
      "${netCDF_BINARY_DIR}/tmp/run_par_warn_test.sh" @ONLY NEWLINE_STYLE LF)
    file(COPY "${netCDF_BINARY_DIR}/tmp/run_par_warn_test.sh"
      DESTINATION ${netCDF_BINARY_DIR}/nc_test4
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    configure_file("${netCDF_SOURCE_DIR}/h5_test/run_par_tests.sh.in"
      "${netCDF_BINARY_DIR}/tmp/run_par_tests.sh" @ONLY NEWLINE_STYLE LF)
    file(COPY "${netCDF_BINARY_DIR}/tmp/run_par_tests.sh"
      DESTINATION ${netCDF_BINARY_DIR}/h5_test
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    set(IMPORT_MPI "include(CMakeFindDependencyMacro)\nfind_dependency(MPI COMPONENTS C)")
  endif()
endif()

# Options to enable parallel IO for classic formats with PnetCDF library.
set(STATUS_PNETCDF ${NETCDF_ENABLE_PNETCDF})
set(USE_PNETCDF ${NETCDF_ENABLE_PNETCDF})
if(NETCDF_ENABLE_PNETCDF)
  set(STATUS_PARALLEL ON)
  set(USE_PARALLEL ON CACHE BOOL "")
  ### 
  # Generate pnetcdf test
  ###
  configure_file("${netCDF_SOURCE_DIR}/nc_test/run_pnetcdf_tests.sh.in"
    "${netCDF_BINARY_DIR}/nc_test/run_pnetcdf_tests.sh")
endif()

# Options to enable use of fill values for elements causing NC_ERANGE
set(NETCDF_ENABLE_ERANGE_FILL AUTO CACHE STRING "AUTO")
option(NETCDF_ENABLE_ERANGE_FILL "Enable use of fill value when out-of-range type conversion causes NC_ERANGE error." OFF)
if(NETCDF_ENABLE_ERANGE_FILL) # enable or auto
   string(TOUPPER ${NETCDF_ENABLE_ERANGE_FILL} NETCDF_ENABLE_ERANGE_FILL)
   if(NETCDF_ENABLE_ERANGE_FILL AND NOT NETCDF_ENABLE_ERANGE_FILL STREQUAL "AUTO")
      # explicitly enabled
      set(NETCDF_ENABLE_ERANGE_FILL ON)
   else()
      if(NOT NETCDF_ENABLE_ERANGE_FILL STREQUAL "AUTO")
         set(NETCDF_ENABLE_ERANGE_FILL OFF)
      endif()
   endif()
endif(NETCDF_ENABLE_ERANGE_FILL)
# Now NETCDF_ENABLE_ERANGE_FILL is either AUTO, ON, or OFF

# More relaxed coordinate check is now mandatory for all builds.
set(ENABLE_ZERO_LENGTH_COORD_BOUND ON)

# check and conform with PnetCDF settings on ERANGE_FILL and RELAX_COORD_BOUND
if(USE_PNETCDF)
  if(NETCDF_ENABLE_ERANGE_FILL STREQUAL "AUTO") # not set on command line
     set(NETCDF_ENABLE_ERANGE_FILL "${PNETCDF_HAS_ERANGE_FILL}")
  else()
     # user explicitly set this option on command line
     if(NOT NETCDF_ENABLE_ERANGE_FILL STREQUAL "${PNETCDF_HAS_ERANGE_FILL}")
        if(NETCDF_ENABLE_ERANGE_FILL)
           message(FATAL_ERROR "Enabling erange-fill conflicts with PnetCDF setting")
        else()
           message(FATAL_ERROR "Disabling erange-fill conflicts with PnetCDF setting")
        endif()
     endif()
  endif()
endif()

if(NETCDF_ENABLE_ERANGE_FILL)
  message(STATUS "Enabling use of fill value when NC_ERANGE")
  set(M4FLAGS "-DERANGE_FILL" CACHE STRING "")
endif()

if(ENABLE_ZERO_LENGTH_COORD_BOUND)
  message(STATUS "Enabling a more relaxed check for NC_EINVALCOORDS")
  target_compile_definitions(netcdf PRIVATE RELAX_COORD_BOUND)
endif()

# Enable Parallel Tests.
option(NETCDF_ENABLE_PARALLEL_TESTS "Enable Parallel IO Tests. Requires HDF5/NetCDF4 with parallel I/O Support." "${HDF5_PARALLEL}")
if(NETCDF_ENABLE_PARALLEL_TESTS AND USE_PARALLEL)
  set(TEST_PARALLEL ON CACHE BOOL "")
  if(USE_HDF5)
    set(TEST_PARALLEL4 ON CACHE BOOL "")
  endif()
endif()

IF (NETCDF_ENABLE_PARALLEL_TESTS AND NOT USE_PARALLEL)
  message(FATAL_ERROR "Parallel tests requested, but no parallel HDF5 installation detected.")
endif()

# Enable special filter test; experimental when using cmake.
option(NETCDF_ENABLE_FILTER_TESTING "Enable filter testing. Ignored if shared libraries or netCDF4 are not enabled" ${NETCDF_ENABLE_PLUGINS})

if(NETCDF_ENABLE_FILTER_TESTING)
  if(NOT NETCDF_ENABLE_HDF5 AND NOT NETCDF_ENABLE_NCZARR)
    message(WARNING "NETCDF_ENABLE_FILTER_TESTING requires HDF5 and/or NCZarr. Disabling.")
    set(NETCDF_ENABLE_FILTER_TESTING OFF CACHE BOOL "Enable Filter Testing" FORCE)
  endif()
endif()

if(NOT BUILD_SHARED_LIBS)
  message(WARNING "NETCDF_ENABLE_FILTER_TESTING requires shared libraries. Disabling.")
  set(NETCDF_ENABLE_FILTER_TESTING OFF)
endif()

option(NETCDF_ENABLE_NCZARR_FILTERS "Enable NCZarr filters" ${NETCDF_ENABLE_PLUGINS})

# Constraints
if (NOT NETCDF_ENABLE_PLUGINS AND NETCDF_ENABLE_NCZARR_FILTERS)
  message(WARNING "NETCDF_ENABLE_NCZARR_FILTERS requires NETCDF_ENABLE_PLUGINS. Disabling.")
  set(NETCDF_ENABLE_NCZARR_FILTERS OFF CACHE BOOL "Enable NCZarr Filters." FORCE)
endif()

IF (NOT NETCDF_ENABLE_NCZARR AND NETCDF_ENABLE_NCZARR_FILTERS) 
    message(WARNING "NETCDF_ENABLE_NCZARR==NO => NETCDF_ENABLE_NCZARR_FILTERS==NO")
  set(NETCDF_ENABLE_NCZARR_FILTERS OFF CACHE BOOL "Disable NCZARR_FILTERS" FORCE)
endif()

# Determine whether or not to generate documentation.
option(NETCDF_ENABLE_DOXYGEN "Enable generation of doxygen-based documentation." OFF)
if(NETCDF_ENABLE_DOXYGEN)
  # Offer the option to build internal documentation.
  option(NETCDF_ENABLE_INTERNAL_DOCS "Build internal documentation. This is of interest to developers only." OFF)
  if(NETCDF_ENABLE_INTERNAL_DOCS)
    set(BUILD_INTERNAL_DOCS yes CACHE STRING "")
  else()
    set(BUILD_INTERNAL_DOCS no CACHE STRING "")
  endif()

  ###
  #
  # If we are building release documentation, we need to set some
  # variables that will be used in the Doxygen.in template.
  ###
  option(NETCDF_ENABLE_DOXYGEN_BUILD_RELEASE_DOCS "Build release documentation.  This is of interest only to the netCDF developers." OFF)
  if(NETCDF_ENABLE_DOXYGEN_BUILD_RELEASE_DOCS)
    set(DOXYGEN_CSS_FILE "${CMAKE_SOURCE_DIR}/docs/release.css" CACHE STRING "")
    set(DOXYGEN_HEADER_FILE "${CMAKE_SOURCE_DIR}/docs/release_header.html" CACHE STRING "")
    set(DOXYGEN_SEARCHENGINE "NO" CACHE STRING "")
    set(NETCDF_ENABLE_DOXYGEN_SERVER_BASED_SEARCH NO CACHE STRING "")
  else()
    set(DOXYGEN_CSS_FILE "" CACHE STRING "")
    set(DOXYGEN_HEADER_FILE "" CACHE STRING "")
    set(DOXYGEN_SEARCHENGINE "YES" CACHE STRING "")

    # If not using release document configuration,
    # provide an option for server-based search.
    option(NETCDF_ENABLE_DOXYGEN_SERVER_SIDE_SEARCH "Configure Doxygen with server-based search." OFF)
    if(NETCDF_ENABLE_DOXYGEN_SERVER_SIDE_SEARCH)
      set(DOXYGEN_SERVER_BASED_SEARCH "YES" CACHE STRING "")
    else()
      set(DOXYGEN_SERVER_BASED_SEARCH "NO" CACHE STRING "")
    endif(NETCDF_ENABLE_DOXYGEN_SERVER_SIDE_SEARCH)


  endif(NETCDF_ENABLE_DOXYGEN_BUILD_RELEASE_DOCS)
  # Option to turn on the TODO list in the doxygen-generated documentation.
  option(NETCDF_DOXYGEN_ENABLE_TASKS "Turn on test, todo, bug lists in documentation. This is of interest to developers only." OFF)
  if(NETCDF_DOXYGEN_ENABLE_TASKS)
    set(SHOW_DOXYGEN_TAG_LIST YES CACHE STRING "")
  else(NETCDF_DOXYGEN_ENABLE_TASKS)
    set(SHOW_DOXYGEN_TODO_LIST NO CACHE STRING "")
  endif(NETCDF_DOXYGEN_ENABLE_TASKS)

  option(NETCDF_ENABLE_DOXYGEN_PDF_OUTPUT "[EXPERIMENTAL] Turn on PDF output for Doxygen-generated documentation." OFF)

  if(NETCDF_ENABLE_DOXYGEN_PDF_OUTPUT)
    set(NC_NETCDF_ENABLE_DOXYGEN_PDF_OUTPUT "YES" CACHE STRING "")
  else()
    set(NC_NETCDF_ENABLE_DOXYGEN_PDF_OUTPUT "NO" CACHE STRING "")
  endif()

  find_program(NC_DOT NAMES dot)
  # Specify whether or not 'dot' was found on the system path.
  if(NC_DOT)
    set(HAVE_DOT YES CACHE STRING "")
  else(NC_DOT)
    set(HAVE_DOT NO CACHE STRING "")
  endif(NC_DOT)
endif()

# Always enable DISKLESS
option(NETCDF_ENABLE_DISKLESS "Enable in-memory files" ON)

# Always enable quantization.
option(NETCDF_ENABLE_QUANTIZE "Enable variable quantization" ON)

# By default, MSVC has a stack size of 1000000.
# Allow a user to override this.
if(MSVC)
  set(NC_MSVC_STACK_SIZE 40000000 CACHE STRING "Default stack size for MSVC-based projects.")
  # By default, CMake sets the stack to 1000000.
  # Remove this limitation.
  # See here for more details:
  # http://www.cmake.org/pipermail/cmake/2009-April/028710.html
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:${NC_MSVC_STACK_SIZE}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /STACK:${NC_MSVC_STACK_SIZE}")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /STACK:${NC_MSVC_STACK_SIZE}")
endif()

# Set some of the options as advanced.
MARK_AS_ADVANCED(NETCDF_ENABLE_INTERNAL_DOCS VALGRIND_TESTS NETCDF_ENABLE_COVERAGE_TESTS )
MARK_AS_ADVANCED(NETCDF_ENABLE_DAP_REMOTE_TESTS NETCDF_ENABLE_DAP_LONG_TESTS USE_REMOTE_CDASH NETCDF_ENABLE_EXTERNAL_SERVER_TESTS)
MARK_AS_ADVANCED(NETCDF_ENABLE_DOXYGEN_BUILD_RELEASE_DOCS NETCDF_DOXYGEN_ENABLE_TASKS NETCDF_ENABLE_DOXYGEN_SERVER_SIDE_SEARCH)
MARK_AS_ADVANCED(NETCDF_ENABLE_SHARED_LIBRARY_VERSION)

################################
# Option checks
################################

# Library include checks
CHECK_INCLUDE_file("math.h"      HAVE_MATH_H)
CHECK_INCLUDE_file("unistd.h"  HAVE_UNISTD_H)
# Solve a compatibility issue in ncgen/, which checks
# for NO_UNISTD_H
if(NOT HAVE_UNISTD_H)
  set(YY_NO_UNISTD_H TRUE)
endif()

CHECK_INCLUDE_file("alloca.h"  HAVE_ALLOCA_H)
CHECK_INCLUDE_file("malloc.h"    HAVE_MALLOC_H)
CHECK_INCLUDE_file("fcntl.h"   HAVE_FCNTL_H)
CHECK_INCLUDE_file("getopt.h"  HAVE_GETOPT_H)
CHECK_INCLUDE_file("locale.h"  HAVE_LOCALE_H)
CHECK_INCLUDE_file("stdint.h"  HAVE_STDINT_H)
CHECK_INCLUDE_file("stdio.h"   HAVE_STDIO_H)
if(MSVC)
CHECK_INCLUDE_file("io.h"      HAVE_IO_H)
endif(MSVC)
CHECK_INCLUDE_file("stdlib.h"  HAVE_STDLIB_H)
CHECK_INCLUDE_file("ctype.h"   HAVE_CTYPE_H)
CHECK_INCLUDE_file("sys/xattr_h" HAVE_SYS_XATTR_H)
CHECK_INCLUDE_file("stdarg.h"    HAVE_STDARG_H)
CHECK_INCLUDE_file("strings.h"   HAVE_STRINGS_H)
CHECK_INCLUDE_file("signal.h"    HAVE_SIGNAL_H)
CHECK_INCLUDE_file("sys/param.h" HAVE_SYS_PARAM_H)
CHECK_INCLUDE_file("sys/stat.h"  HAVE_SYS_STAT_H)
CHECK_INCLUDE_file("sys/time.h"  HAVE_SYS_TIME_H)
CHECK_INCLUDE_file("sys/types.h" HAVE_SYS_TYPES_H)
CHECK_INCLUDE_file("sys/mman.h"  HAVE_SYS_MMAN_H)
CHECK_INCLUDE_file("sys/resource.h" HAVE_SYS_RESOURCE_H)
CHECK_INCLUDE_file("fcntl.h"  HAVE_FCNTL_H)
CHECK_INCLUDE_file("inttypes.h"  HAVE_INTTYPES_H)
CHECK_INCLUDE_file("pstdint.h"  HAVE_PSTDINT_H)
CHECK_INCLUDE_file("endian.h" HAVE_ENDIAN_H)
CHECK_INCLUDE_file("BaseTsd.h"  HAVE_BASETSD_H)
CHECK_INCLUDE_file("stddef.h"   HAVE_STDDEF_H)
CHECK_INCLUDE_file("string.h"  HAVE_STRING_H)
CHECK_INCLUDE_file("winsock2.h" HAVE_WINSOCK2_H)
CHECK_INCLUDE_file("ftw.h"  HAVE_FTW_H)
CHECK_INCLUDE_file("libgen.h" HAVE_LIBGEN_H)
CHECK_INCLUDE_file("execinfo.h" HAVE_EXECINFO_H)
CHECK_INCLUDE_file("dirent.h" HAVE_DIRENT_H)
CHECK_INCLUDE_file("time.h" HAVE_TIME_H)
CHECK_INCLUDE_file("dlfcn.h" HAVE_DLFCN_H)

# Symbol Exists
CHECK_SYMBOL_EXISTS(isfinite "math.h" HAVE_DECL_ISFINITE)
CHECK_SYMBOL_EXISTS(isnan "math.h" HAVE_DECL_ISNAN)
CHECK_SYMBOL_EXISTS(isinf "math.h" HAVE_DECL_ISINF)
CHECK_SYMBOL_EXISTS(st_blksize "sys/stat.h" HAVE_STRUCT_STAT_ST_BLKSIZE)
CHECK_SYMBOL_EXISTS(alloca "alloca.h" HAVE_ALLOCA)
CHECK_SYMBOL_EXISTS(snprintf "stdio.h" HAVE_SNPRINTF)

# Type checks
# Aliases for automake consistency
set(SIZEOF_VOIDSTAR ${CMAKE_SIZEOF_VOID_P})
set(SIZEOF_VOIDP ${SIZEOF_VOIDSTAR})
CHECK_TYPE_SIZE("char"      SIZEOF_CHAR)
CHECK_TYPE_SIZE("double"    SIZEOF_DOUBLE)
CHECK_TYPE_SIZE("float"     SIZEOF_FLOAT)
CHECK_TYPE_SIZE("int"       SIZEOF_INT)
CHECK_TYPE_SIZE("uint"      SIZEOF_UINT)
if(SIZEOF_UINT)
  set(HAVE_UINT TRUE)
endif(SIZEOF_UINT)

CHECK_TYPE_SIZE("schar"      SIZEOF_SCHAR)
if(SIZEOF_SCHAR)
  set(HAVE_SCHAR TRUE)
endif(SIZEOF_SCHAR)

CHECK_TYPE_SIZE("long"      SIZEOF_LONG)
CHECK_TYPE_SIZE("long long" SIZEOF_LONG_LONG)
if(SIZEOF_LONG_LONG)
  set(HAVE_LONG_LONG_INT TRUE)
endif(SIZEOF_LONG_LONG)

CHECK_TYPE_SIZE("unsigned long long" SIZEOF_UNSIGNED_LONG_LONG)

CHECK_TYPE_SIZE("off_t"     SIZEOF_OFF_T)
CHECK_TYPE_SIZE("off64_t"   SIZEOF_OFF64_T)
CHECK_TYPE_SIZE("short"     SIZEOF_SHORT)
CHECK_TYPE_SIZE("ushort"    SIZEOF_USHORT)
if(SIZEOF_USHORT)
  set(HAVE_USHORT TRUE)
endif(SIZEOF_USHORT)

CHECK_TYPE_SIZE("_Bool"     SIZEOF__BOOL)

CHECK_TYPE_SIZE("size_t"    SIZEOF_SIZE_T)

# Check whether to turn on or off CDF5 support.
if(SIZEOF_SIZE_T EQUAL 4)
   if(NETCDF_ENABLE_CDF5) # enable or auto
      string(TOUPPER ${NETCDF_ENABLE_CDF5} NETCDF_ENABLE_CDF5)
      if(NETCDF_ENABLE_CDF5 AND NOT NETCDF_ENABLE_CDF5 STREQUAL "AUTO") # explicitly enabled
         message(FATAL_ERROR "Unable to support CDF5 feature because size_t is less than 8 bytes")
      endif(NETCDF_ENABLE_CDF5 AND NOT NETCDF_ENABLE_CDF5 STREQUAL "AUTO")
      set(NETCDF_ENABLE_CDF5 OFF) # cannot support CDF5
      set(USE_CDF5 OFF CACHE BOOL "") # cannot support CDF5
   endif(NETCDF_ENABLE_CDF5)
else(SIZEOF_SIZE_T EQUAL 4)
   if(NETCDF_ENABLE_CDF5) # explicitly set by user or not set
      set(USE_CDF5 ON CACHE BOOL "")
   else(NETCDF_ENABLE_CDF5) # explicitly disabled by user
      set(USE_CDF5 OFF CACHE BOOL "")
   endif(NETCDF_ENABLE_CDF5)
endif(SIZEOF_SIZE_T EQUAL 4)

CHECK_TYPE_SIZE("ssize_t"   SIZEOF_SSIZE_T)
if(SIZEOF_SSIZE_T)
  set(HAVE_SSIZE_T TRUE)
endif(SIZEOF_SSIZE_T)
CHECK_TYPE_SIZE("ptrdiff_t" SIZEOF_PTRDIFF_T)
if(SIZEOF_PTRDIFF_T)
  set(HAVE_PTRDIFF_T TRUE)
endif(SIZEOF_PTRDIFF_T)
CHECK_TYPE_SIZE("uintptr_t" SIZEOF_UINTPTR_T)
if(SIZEOF_UINTPTR_T)
  set(HAVE_UINTPTR_T TRUE)
endif(SIZEOF_UINTPTR_T)
CHECK_TYPE_SIZE("mode_t" SIZEOF_MODE_T)
if(SIZEOF_MODE_T)
  set(HAVE_MODE_T TRUE)
endif(SIZEOF_MODE_T)

# __int64 is used on Windows for large file support.
CHECK_TYPE_SIZE("__int64"   SIZEOF___INT_64)
CHECK_TYPE_SIZE("int64_t"   SIZEOF_INT64_T)
CHECK_TYPE_SIZE("uint64"  SIZEOF_UINT64)
CHECK_TYPE_SIZE("unsigned char"      SIZEOF_UCHAR)
CHECK_TYPE_SIZE("unsigned short int" SIZEOF_UNSIGNED_SHORT_INT)
CHECK_TYPE_SIZE("unsigned int"       SIZEOF_UNSIGNED_INT)
CHECK_TYPE_SIZE("long long"          SIZEOF_LONGLONG)
CHECK_TYPE_SIZE("unsigned long long" SIZEOF_ULONGLONG)

CHECK_TYPE_SIZE("uint64_t"  SIZEOF_UINT64_T)
if(SIZEOF_UINT64_T)
  set(HAVE_UINT64_T TRUE)
endif(SIZEOF_UINT64_T)

# On windows systems, we redefine off_t as __int64
# to enable LFS. This is true on 32 and 64 bit system.s
# We must redefine SIZEOF_OFF_T to match.
if(MSVC AND SIZEOF___INT_64)
  set(SIZEOF_OFF_T  ${SIZEOF___INT_64})
endif()

# Check for various functions.
CHECK_FUNCTION_EXISTS(fsync HAVE_FSYNC)
CHECK_FUNCTION_EXISTS(strlcat   HAVE_STRLCAT)
CHECK_FUNCTION_EXISTS(strlcpy  HAVE_STRLCPY)
CHECK_FUNCTION_EXISTS(strdup  HAVE_STRDUP)
CHECK_FUNCTION_EXISTS(strndup HAVE_STRNDUP)
CHECK_FUNCTION_EXISTS(strlen  HAVE_STRLEN)
CHECK_FUNCTION_EXISTS(strtoll HAVE_STRTOLL)
CHECK_FUNCTION_EXISTS(strcasecmp  HAVE_STRCASECMP)
CHECK_FUNCTION_EXISTS(strtoull  HAVE_STRTOULL)
CHECK_FUNCTION_EXISTS(mkstemp HAVE_MKSTEMP)
CHECK_FUNCTION_EXISTS(mktemp HAVE_MKTEMP)
CHECK_FUNCTION_EXISTS(random HAVE_RANDOM)
CHECK_FUNCTION_EXISTS(gettimeofday  HAVE_GETTIMEOFDAY)
CHECK_FUNCTION_EXISTS(MPI_Comm_f2c  HAVE_MPI_COMM_F2C)
CHECK_FUNCTION_EXISTS(MPI_Info_f2c  HAVE_MPI_INFO_F2C)
CHECK_FUNCTION_EXISTS(memmove HAVE_MEMMOVE)
CHECK_FUNCTION_EXISTS(getpagesize HAVE_GETPAGESIZE)
CHECK_FUNCTION_EXISTS(sysconf HAVE_SYSCONF)
CHECK_FUNCTION_EXISTS(getrlimit HAVE_GETRLIMIT)
CHECK_FUNCTION_EXISTS(_filelengthi64 HAVE_FILE_LENGTH_I64)
CHECK_FUNCTION_EXISTS(mmap HAVE_MMAP)
CHECK_FUNCTION_EXISTS(mremap HAVE_MREMAP)
CHECK_FUNCTION_EXISTS(fileno HAVE_FILENO)
CHECK_FUNCTION_EXISTS(H5Literate2 HAVE_H5LITERATE2)

CHECK_FUNCTION_EXISTS(clock_gettime  HAVE_CLOCK_GETTIME)
CHECK_SYMBOL_EXISTS("struct timespec" "time.h" HAVE_STRUCT_TIMESPEC)
CHECK_FUNCTION_EXISTS(atexit HAVE_ATEXIT)

# Control invoking nc_finalize at exit
option(NETCDF_ENABLE_ATEXIT_FINALIZE "Invoke nc_finalize at exit." ON)
if(NOT HAVE_ATEXIT)
if(NETCDF_ENABLE_ATEXIT_FINALIZE AND NOT HAVE_ATEXIT)
  set(NETCDF_ENABLE_ATEXIT_FINALIZE OFF CACHE BOOL "Enable ATEXIT" FORCE)
  message(WARNING "NETCDF_ENABLE_ATEXIT_FINALIZE set but atexit() function not defined")
endif()
endif()

# Check to see if MAP_ANONYMOUS is defined.
if(MSVC)
  message(WARNING "mmap not supported under visual studio: disabling MMAP support.")
  set(NETCDF_ENABLE_MMAP OFF)
else()
  CHECK_C_SOURCE_COMPILES("
  #include <sys/mman.h>
  int main() {int x = MAP_ANONYMOUS;}" HAVE_MAPANON)
  if(NOT HAVE_MMAP OR NOT HAVE_MAPANON)
    message(WARNING "mmap or MAP_ANONYMOUS not found: disabling MMAP support.")
    set(NETCDF_ENABLE_MMAP OFF)
  endif()
endif()

if(NETCDF_ENABLE_MMAP)
  # Aliases
  set(BUILD_MMAP ON)
  set(USE_MMAP ON)
endif(NETCDF_ENABLE_MMAP)

#CHECK_FUNCTION_EXISTS(alloca HAVE_ALLOCA)

# Used in the `configure_file` calls below
set(ISCMAKE "yes")
if(MSVC)
  set(ISMSVC ON CACHE BOOL "" FORCE)
  set(REGEDIT ON CACHE BOOL "" FORCE)
  # Get windows major version and build number
  execute_process(COMMAND "systeminfo" OUTPUT_VARIABLE WININFO)
  if(WININFO STREQUAL "")
    set(WVM 0)
    set(WVB 0)
  else()
    string(REGEX MATCH "\nOS Version:[ \t]+[0-9.]+" WINVERLINE "${WININFO}")
    string(REGEX REPLACE "[^0-9]*([0-9]+)[.]([0-9])+[.]([0-9]+)" "\\1" WVM "${WINVERLINE}")
    string(REGEX REPLACE "[^0-9]*([0-9]+)[.]([0-9])+[.]([0-9]+)" "\\3" WVB "${WINVERLINE}")
  endif()
  set(WINVERMAJOR ${WVM} CACHE STRING "" FORCE)
  set(WINVERBUILD ${WVB} CACHE STRING "" FORCE)
endif()

#####
# End system inspection checks.
#####


# A basic script used to convert m4 files
find_program(NC_M4 NAMES m4 m4.exe)
if(NC_M4)
  message(STATUS "Found m4: ${NC_M4}")
  set(HAVE_M4 TRUE)
else()
  message(STATUS "m4 not found.")
  set(HAVE_M4 FALSE)
endif()

##specific
# Shell script Macro
##
# Determine if 'bash' is on the system.
##

option(NETCDF_ENABLE_BASH_SCRIPT_TESTING "Detection is typically automatic, but this option can be used to force enable/disable bash-script based tests." ON)

if(NETCDF_ENABLE_BASH_SCRIPT_TESTING)
  find_program(HAVE_BASH bash)
  if(HAVE_BASH)
    string(COMPARE EQUAL "${HAVE_BASH}" "C:/Windows/System32/bash.exe" IS_BASH_EXE)
    if(NOT IS_BASH_EXE)
      message(STATUS "Found bash: ${HAVE_BASH}")
    else()
      message(STATUS "Ignoring ${HAVE_BASH}")
      set(HAVE_BASH "")
    endif()
  else()
    message(STATUS "Bash shell not found; disabling shell script tests.")
  endif()
else(NETCDF_ENABLE_BASH_SCRIPT_TESTING)
  set(HAVE_BASH "")
endif(NETCDF_ENABLE_BASH_SCRIPT_TESTING)

# Create config.h file.
configure_file("${netCDF_SOURCE_DIR}/config.h.cmake.in"
  "${netCDF_BINARY_DIR}/config.h")

add_definitions(-DHAVE_CONFIG_H)

include_directories(${netCDF_BINARY_DIR})
# End autotools-style checks for config.h

#####
# Set the true names of all the libraries, if customized by external project
#####
# Recurse into other subdirectories.
add_subdirectory("include")
add_subdirectory(libdispatch)
add_subdirectory(libsrc)

if(USE_PNETCDF)
  add_subdirectory(libsrcp)
endif(USE_PNETCDF)

if(USE_NETCDF4)
  add_subdirectory(libsrc4)
endif()

if(USE_HDF5)
  add_subdirectory(libhdf5)
endif(USE_HDF5)

if(USE_HDF4)
  add_subdirectory(libhdf4)
endif(USE_HDF4)

if(NETCDF_ENABLE_DAP2)
  add_subdirectory(oc2)
  add_subdirectory(libdap2)
endif()

if(NETCDF_ENABLE_DAP4)
  add_subdirectory(libdap4)
  add_subdirectory(libncxml)
else()
  if(NETCDF_ENABLE_S3_INTERNAL)
    add_subdirectory(libncxml)
  endif()
endif()

if(NETCDF_ENABLE_PLUGINS)
  add_subdirectory(libncpoco)
endif()

if(NETCDF_ENABLE_NCZARR)
  add_subdirectory(libnczarr)
  file(COPY ${netCDF_SOURCE_DIR}/unit_test/timer_utils.h
       DESTINATION ${netCDF_BINARY_DIR}/nczarr_test/)
  file(COPY ${netCDF_SOURCE_DIR}/unit_test/timer_utils.c
       DESTINATION ${netCDF_BINARY_DIR}/nczarr_test/)
  file(COPY ${netCDF_SOURCE_DIR}/nc_test4/test_filter.c
       DESTINATION ${netCDF_BINARY_DIR}/nczarr_test/)
  file(COPY ${netCDF_SOURCE_DIR}/nc_test4/test_filter_misc.c
       DESTINATION ${netCDF_BINARY_DIR}/nczarr_test/)
  file(COPY ${netCDF_SOURCE_DIR}/nc_test4/test_filter_repeat.c
       DESTINATION ${netCDF_BINARY_DIR}/nczarr_test/)
  file(COPY ${netCDF_SOURCE_DIR}/nc_test4/test_filter_order.c
       DESTINATION ${netCDF_BINARY_DIR}/nczarr_test/)
  file(COPY ${netCDF_SOURCE_DIR}/nc_test4/tst_multifilter.c
       DESTINATION ${netCDF_BINARY_DIR}/nczarr_test/)
endif()

# Tests and files which depend on libnetcdf must be included
# *after* this line.
add_subdirectory(liblib)

if(NETCDF_ENABLE_PLUGINS)
  add_subdirectory(plugins)
endif()

# Enable Utilities.
if(NETCDF_BUILD_UTILITIES)
  include_directories(ncdump)
  add_subdirectory(ncgen)
  add_subdirectory(ncgen3)
  add_subdirectory(ncdump)
endif()

# Enable tests
if(NETCDF_ENABLE_TESTS)
  if(NETCDF_ENABLE_V2_API)
    add_subdirectory(nctest)
  endif()
  add_subdirectory(nc_test)
  if(USE_HDF4)
    add_subdirectory(hdf4_test)
  endif()
  if(USE_HDF5)
    include_directories(h5_test)
    add_subdirectory(nc_test4)
    add_subdirectory(h5_test)
  endif()
  if(NETCDF_ENABLE_DAP2)
    add_subdirectory(ncdap_test)
  endif()
  if(NETCDF_ENABLE_DAP4)
    add_subdirectory(dap4_test)
  endif()
  if(NETCDF_ENABLE_EXAMPLES)
    add_subdirectory(examples)
  endif()
  if(NETCDF_ENABLE_BENCHMARKS)
    add_subdirectory(nc_perf)
  endif(NETCDF_ENABLE_BENCHMARKS)
  if(NETCDF_ENABLE_UNIT_TESTS)
    add_subdirectory(unit_test)
  endif(NETCDF_ENABLE_UNIT_TESTS)
  if(NETCDF_ENABLE_NCZARR)
    include_directories(nczarr_test)
    add_subdirectory(nczarr_test)
  endif()
endif()

# Code to generate an export header
#GENERATE_EXPORT_HEADER(netcdf
# BASE_NAME netcdf
# EXPORT_MACRO_NAME netcdf_EXPORT
# EXPORT_FILE_NAME netcdf_Export.h
# STATIC_DEFINE netcdf_BUILT_AS_STATIC
#)



##
# Brute force, grab all of the dlls from the dependency directory,
# install them in the binary dir. Grab all of the .libs, put them
# in the libdir.
##
if(MSVC)
  foreach(CPP ${CMAKE_PREFIX_PATH})
    file(GLOB COPY_FILES ${CPP}/lib/*.lib)
  endforeach()
  install(FILES ${COPY_FILES}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  COMPONENT dependencies)

  foreach(CPP ${CMAKE_PREFIX_PATH})
    file(GLOB COPY_FILES ${CPP}/bin/*.dll)
  endforeach()
  string(REGEX REPLACE "msv[.*].dll" "" COPY_FILES "${COPY_FILES}")
  install(FILES ${COPY_FILES}
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT dependencies)

endif()

# Subdirectory CMakeLists.txt files should specify their own
# 'install' files.
# Including 'CPack' kicks everything off.
include(InstallRequiredSystemLibraries)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
  @ONLY
  )

###
# Create pkgconfig files.
###
if(NOT DEFINED CMAKE_INSTALL_MANDIR)
  set(CMAKE_INSTALL_MANDIR "${CMAKE_INSTALL_PREFIX}/share/man/")
endif()

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR lib)
endif(NOT DEFINED CMAKE_INSTALL_LIBDIR)

# Set
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX})
set(libdir ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
set(includedir ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
set(CC ${CMAKE_C_COMPILER})

# Process all dependency libraries and create a string
# used when parsing netcdf.pc.in

set(NC_LIBS "")

foreach(_LIB ${ALL_TLL_LIBS})
  get_filename_component(_LIB_NAME ${_LIB} NAME_WE)
  string(REGEX REPLACE "^lib" "" _NAME ${_LIB_NAME})
  list(APPEND NC_LIBS "-l${_NAME}")
  get_filename_component(_LIB_DIR ${_LIB} PATH)
  list(APPEND LINKFLAGS "-L${_LIB_DIR}")
endforeach()

if(NC_LIBS)
  string(REPLACE ";" " " NC_LIBS "${NC_LIBS}")
endif()

string(REPLACE ";" " " LINKFLAGS "${LINKFLAGS}")

list(REMOVE_DUPLICATES NC_LIBS)
list(REMOVE_DUPLICATES LINKFLAGS)

set(LIBS ${NC_LIBS})
set(NC_LIBS "-lnetcdf")

configure_file(
  ${netCDF_SOURCE_DIR}/netcdf.pc.in
  ${netCDF_BINARY_DIR}/netcdf.pc @ONLY)


if(NOT IS_DIRECTORY ${netCDF_BINARY_DIR}/tmp)
  file(MAKE_DIRECTORY ${netCDF_BINARY_DIR}/tmp)
endif()

install(FILES ${netCDF_BINARY_DIR}/netcdf.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  COMPONENT utilities)

###
# End pkgconfig file creation.
###

##
# Print the configuration summary
##
print_conf_summary()

# Enable Makedist files.
ADD_MAKEDIST()
ENABLE_MAKEDIST(README.md COPYRIGHT RELEASE_NOTES.md INSTALL INSTALL.cmake test_prog.c lib_flags.am cmake CMakeLists.txt COMPILE.cmake.txt config.h.cmake.in cmake_uninstall.cmake.in netcdf-config-version.cmake.in netcdf-config.cmake.in FixBundle.cmake.in nc-config.cmake.in configure configure.ac install-sh config.h.in config.sub CTestConfig.cmake.in)

#####
# Configure and print the libnetcdf.settings file.
#####

# Set variables to mirror those used by autoconf.
# This way we don't need to maintain two separate template
# files.
set(host_cpu "${cpu}")
set(host_vendor "${osname}")
set(host_os "${osrel}")
string(RANDOM LENGTH 3 ALPHABET "0123456789" PLATFORMUID)
math(EXPR PLATFORMUID "${PLATFORMUID} + 1" OUTPUT_FORMAT DECIMAL)
       
set(CC_VERSION "${CMAKE_C_COMPILER}")

# Build *FLAGS for libnetcdf.settings.
set(CFLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}")
set(CPPFLAGS "${CMAKE_CPP_FLAGS} ${CMAKE_CPP_FLAGS_${CMAKE_BUILD_TYPE}}")
set(LDFLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_${CMAKE_BUILD_TYPE}}")

is_disabled(BUILD_SHARED_LIBS enable_static)
is_enabled(BUILD_SHARED_LIBS enable_shared)

is_enabled(NETCDF_ENABLE_V2_API HAS_NC2)
is_enabled(NETCDF_ENABLE_NETCDF4 HAS_NC4)
is_enabled(NETCDF_ENABLE_HDF4 HAS_HDF4)
is_enabled(USE_HDF4 HAS_HDF4)
is_enabled(USE_HDF5 HAS_HDF5)
is_enabled(OFF HAS_BENCHMARKS)
is_enabled(STATUS_PNETCDF HAS_PNETCDF)
is_enabled(STATUS_PARALLEL HAS_PARALLEL)
is_enabled(NETCDF_ENABLE_PARALLEL4 HAS_PARALLEL4)
is_enabled(NETCDF_ENABLE_DAP HAS_DAP)
is_enabled(NETCDF_ENABLE_DAP2 HAS_DAP2)
is_enabled(NETCDF_ENABLE_DAP4 HAS_DAP4)
is_enabled(NETCDF_ENABLE_BYTERANGE HAS_BYTERANGE)
is_enabled(NETCDF_ENABLE_DISKLESS HAS_DISKLESS)
is_enabled(USE_MMAP HAS_MMAP)
is_enabled(ENABLE_ZERO_LENGTH_COORD_BOUND RELAX_COORD_BOUND)
is_enabled(USE_CDF5 HAS_CDF5)
is_enabled(NETCDF_ENABLE_ERANGE_FILL HAS_ERANGE_FILL)
is_enabled(HDF5_HAS_PAR_FILTERS HAS_PAR_FILTERS)
is_enabled(NETCDF_ENABLE_S3 HAS_S3)
is_enabled(NETCDF_ENABLE_S3_AWS HAS_S3_AWS)
is_enabled(NETCDF_ENABLE_S3_INTERNAL HAS_S3_INTERNAL)
is_enabled(HAS_HDF5_ROS3 HAS_HDF5_ROS3)
is_enabled(NETCDF_ENABLE_NCZARR HAS_NCZARR)
is_enabled(NETCDF_ENABLE_NCZARR_ZIP HAS_NCZARR_ZIP)
is_enabled(NETCDF_ENABLE_PLUGINS HAS_PLUGINS)
is_enabled(NETCDF_ENABLE_QUANTIZE HAS_QUANTIZE)
is_enabled(NETCDF_ENABLE_LOGGING HAS_LOGGING)
is_enabled(NETCDF_ENABLE_FILTER_TESTING DO_FILTER_TESTS)
is_enabled(HAVE_SZ HAS_SZIP)
is_enabled(HAVE_SZ HAS_SZLIB_WRITE)
is_enabled(HAVE_ZSTD HAS_ZSTD)
is_enabled(HAVE_BLOSC HAS_BLOSC)
is_enabled(HAVE_BZ2 HAS_BZ2)
is_enabled(NETCDF_ENABLE_REMOTE_FUNCTIONALITY DO_REMOTE_FUNCTIONALITY)

if(NETCDF_ENABLE_S3_INTERNAL)
  set(WHICH_S3_SDK "internal")
  set(NC_WHICH_S3_SDK "internal")
elseif(NETCDF_ENABLE_S3_AWS)
  set(WHICH_S3_SDK "aws-sdk-cpp")
  set(NC_WHICH_S3_SDK "aws-sdk-cpp")
else()
  set(WHICH_S3_SDK "none")
  set(NC_WHICH_S3_SDK "none")
endif()

if(WITH_S3_TESTING STREQUAL PUBLIC)
  set(NETCDF_ENABLE_S3_TESTING "public")
elseif(WITH_S3_TESTING)
  set(NETCDF_ENABLE_S3_TESTING "yes")
  set(NETCDF_ENABLE_S3_TESTALL "yes")
elseif(NOT WITH_S3_TESTING)
  set(NETCDF_ENABLE_S3_TESTING "no")
else()
  set(NETCDF_ENABLE_S3_TESTING "no")
endif()

# The Unidata testing S3 bucket
# WARNING: this must match the value in configure.ac
set(S3TESTBUCKET "unidata-zarr-test-data" CACHE STRING "S3 test bucket")

# The working S3 path tree within the Unidata bucket.
# WARNING: this must match the value in configure.ac
set(S3TESTSUBTREE "netcdf-c" CACHE STRING "Working S3 path.")
# Build a unique id based on the date
string(TIMESTAMP TESTUID "%s")
if(NETCDF_ENABLE_S3_TESTING)
file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/s3cleanup_${PLATFORMUID}.uids" "${TESTUID}\n")
endif()

# Copy the CTest customization file into binary directory, as required.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake")

message(STATUS "STD_FILTERS: ${STD_FILTERS}")

# Generate file from template.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libnetcdf.settings.in"
  "${CMAKE_CURRENT_BINARY_DIR}/libnetcdf.settings"
  @ONLY)

if(NETCDF_ENABLE_PARALLEL4 AND NETCDF_ENABLE_HDF5)
  if(HDF5_PARALLEL)
    configure_file("${netCDF_SOURCE_DIR}/nc_test4/run_par_test.sh.in"
      "${netCDF_BINARY_DIR}/tmp/run_par_test.sh" @ONLY NEWLINE_STYLE LF)
    file(COPY "${netCDF_BINARY_DIR}/tmp/run_par_test.sh"
      DESTINATION ${netCDF_BINARY_DIR}/nc_test4
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    configure_file("${netCDF_SOURCE_DIR}/h5_test/run_par_tests.sh.in"
      "${netCDF_BINARY_DIR}/tmp/run_par_tests.sh" @ONLY NEWLINE_STYLE LF)
    file(COPY "${netCDF_BINARY_DIR}/tmp/run_par_tests.sh"
      DESTINATION ${netCDF_BINARY_DIR}/h5_test
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
  endif()
endif()

# Read in settings file, print out.
# Avoid using system-specific calls so that this
# might also work on Windows.
file(READ "${CMAKE_CURRENT_BINARY_DIR}/libnetcdf.settings"
  LIBNETCDF_SETTINGS)
message(STATUS ${LIBNETCDF_SETTINGS})

### 
# Report deprecated options detected by
# cmake/deprecated.cmake. Added in support of
# https://github.com/Unidata/netcdf-c/pull/2895
###

if(DEFINED DEPR_OPT)
  message(WARNING "Warning! Deprecated Options used. Please migrate your build system as follows:\n${DEPR_OPT}" )
endif()
  # Install libnetcdf.settings file into same location
# as the libraries.
install(FILES "${netCDF_BINARY_DIR}/libnetcdf.settings"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  COMPONENT libraries)

#####
# End libnetcdf.settings section.
#####

#####
# Create 'nc-config' file.
#####
configure_file("${netCDF_SOURCE_DIR}/nc-config.cmake.in"
  "${netCDF_BINARY_DIR}/tmp/nc-config" @ONLY
  NEWLINE_STYLE LF)
file(COPY "${netCDF_BINARY_DIR}/tmp/nc-config"
  DESTINATION ${netCDF_BINARY_DIR}/
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

install(PROGRAMS ${netCDF_BINARY_DIR}/nc-config
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT utilities)

#####
# Create 'netcdf_meta.h' include file.
#####
configure_file(
  ${netCDF_SOURCE_DIR}/include/netcdf_meta.h.in
  ${netCDF_BINARY_DIR}/include/netcdf_meta.h @ONLY)

#####
# Create 'netcdf_dispatch.h' include file.
#####
configure_file(
  ${netCDF_SOURCE_DIR}/include/netcdf_dispatch.h.in
  ${netCDF_BINARY_DIR}/include/netcdf_dispatch.h @ONLY NEWLINE_STYLE LF)

####
# Build test_common.sh
#####
set(EXTRA_DIST ${EXTRA_DIST} ${CMAKE_CURRENT_SOURCE_DIR}/test_common.in)
set(TOPSRCDIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(TOPBUILDDIR "${CMAKE_CURRENT_BINARY_DIR}")
set(abs_top_builddir "${CMAKE_CURRENT_BINARY_DIR}")
set(abs_top_srcdir "${CMAKE_CURRENT_SOURCE_DIR}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_common.in ${CMAKE_CURRENT_BINARY_DIR}/test_common.sh @ONLY NEWLINE_STYLE LF)

#####
# Build doxygen documentation, if need be.
# This must come after setting top_builddir, etc.
#####
add_subdirectory(docs)

####
# Build s3cleanup.sh and s3gc.sh
#####
set(EXTRA_DIST ${EXTRA_DIST} ${CMAKE_CURRENT_SOURCE_DIR}/s3cleanup.in ${CMAKE_CURRENT_SOURCE_DIR}/s3gc.in)
set(TOPSRCDIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(TOPBUILDDIR "${CMAKE_CURRENT_BINARY_DIR}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/s3cleanup.in ${CMAKE_CURRENT_BINARY_DIR}/s3cleanup.sh @ONLY NEWLINE_STYLE LF)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/s3gc.in ${CMAKE_CURRENT_BINARY_DIR}/s3gc.sh @ONLY NEWLINE_STYLE LF)

#####
# Build and copy nc_test4/findplugin.sh to various places
#####
foreach(CC nc_test4 nczarr_test v3_nczarr_test plugins h5_test examples/C)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/plugins/findplugin.in ${CMAKE_CURRENT_BINARY_DIR}/${CC}/findplugin.sh @ONLY NEWLINE_STYLE LF)
endforeach()

if(NETCDF_ENABLE_BENCHMARKS)
  if(NETCDF_ENABLE_PARALLEL4)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/nc_perf/run_par_bm_test.sh.in ${CMAKE_CURRENT_BINARY_DIR}/nc_perf/run_par_bm_test.sh @ONLY NEWLINE_STYLE LF)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/nc_perf/run_gfs_test.sh.in ${CMAKE_CURRENT_BINARY_DIR}/nc_perf/run_gfs_test.sh @ONLY NEWLINE_STYLE LF)
  endif(NETCDF_ENABLE_PARALLEL4)
endif(NETCDF_ENABLE_BENCHMARKS)

if(NETCDF_ENABLE_TESTS)
    #####
    # Build ncdap_test|dap4_test/findtestserver[4].c
    #####
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ncdap_test/findtestserver.c.in ${CMAKE_CURRENT_BINARY_DIR}/ncdap_test/findtestserver.c @ONLY NEWLINE_STYLE LF)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ncdap_test/findtestserver.c.in ${CMAKE_CURRENT_BINARY_DIR}/dap4_test/findtestserver4.c @ONLY NEWLINE_STYLE LF)

    #####
    # Build dap4_test/pingurl4.c
    #####
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ncdap_test/pingurl.c ${CMAKE_CURRENT_BINARY_DIR}/dap4_test/pingurl4.c @ONLY NEWLINE_STYLE LF)

    #####
    # Build CTestCustom.cmake to cleanup S3 after tests are done.
    #####
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake NEWLINE_STYLE LF)
   

endif()

if(DEFINED ENV{LIB_FUZZING_ENGINE})
  add_subdirectory(fuzz)
endif(DEFINED ENV{LIB_FUZZING_ENGINE})

####
# Export files
####

# Create CMake package configuration files. With these, other packages using
# cmake should be able to find netcdf using find_package and find_library.
# The EXPORT call is paired with one in liblib.
set(ConfigPackageLocation ${CMAKE_INSTALL_LIBDIR}/cmake/netCDF)

install(EXPORT netCDFTargets
  DESTINATION ${ConfigPackageLocation}
  COMPONENT headers
  NAMESPACE netCDF::
  )

export(EXPORT netCDFTargets FILE netCDFTargets.cmake NAMESPACE netCDF::)

include(CMakePackageConfigHelpers)
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/netCDFConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/netCDFConfig.cmake"
  INSTALL_DESTINATION "${ConfigPackageLocation}"
)

target_include_directories(netcdf
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Create export configuration
write_basic_package_version_file(
  netCDFConfigVersion.cmake
  VERSION ${netCDF_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
  "${CMAKE_CURRENT_BINARY_DIR}/netCDFConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/netCDFConfigVersion.cmake"
  DESTINATION ${ConfigPackageLocation}
  COMPONENT headers
)

####
# End export files
####

# CPack inclusion must come last.
option(NETCDF_PACKAGE "Create netCDF-C package " ${NETCDF_IS_TOP_LEVEL})

if (NETCDF_PACKAGE)
  include(CMakeInstallation.cmake)
endif()
