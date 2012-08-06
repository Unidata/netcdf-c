#Minimum required CMake Version
cmake_minimum_required(VERSION 2.8.8)

#Project Name
project(NetCDF C)
SET(NetCDF_VERSION 4.2.1)
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYRIGHT")
SET(CPACK_PACKAGE_VERSION "4.3")
ENABLE_TESTING()
INCLUDE(CTest)
# CTest/CDash configuration
SET (CTEST_DROP_METHOD http CACHE STRING "")
SET (CTEST_DROP_SITE "yakov.unidata.ucar.edu/~wfisher/cdash/" CACHE STRING "")
SET (CTEST_DROP_LOCATION "/submit.php?project=WinNETCDF" CACHE STRING "")
SET (CTEST_DROP_SITE_CDASH TRUE CACHE BOOL "")


ADD_DEFINITIONS()

### Verbose make, for debugging.
#SET(CMAKE_VERBOSE_MAKEFILE ON)

#Add custom CMake Module
SET (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/"
    CACHE INTERNAL "Location of our custom CMake modules.")

# Configure-type checks
INCLUDE (${CMAKE_ROOT}/Modules/CheckLibraryExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckCXXSourceCompiles.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindPkgConfig.cmake)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)
IF(APPLE)
  SET(CPACK_BINARY_DRAGNDROP ON CACHE BOOL "")
  SET(CPACK_BINARY_PACKAGEMAKER OFF CACHE BOOL "")
ENDIF()



#####
# Option checks
#####

# HDF5 cache variables.
SET (DEFAULT_CHUNK_SIZE 4194304 CACHE STRING "Default Chunk Cache Size.")
SET (DEFAULT_CHUNKS_IN_CACHE 10 CACHE STRING "Default number of chunks in cache.")
SET (CHUNK_CACHE_SIZE 4194304 CACHE STRING "Default Chunk Cache Size.")
SET (CHUNK_CACHE_NELEMS 1009 CACHE STRING "Default maximum number of elements in cache.")
SET (CHUNK_CACHE_PREEMPTION 0.75 CACHE STRING "Default file chunk cache preemption policy for HDf5 files (a number between 0 and 1, inclusive.")
SET (MAX_DEFAULT_CACHE_SIZE 67108864 CACHE STRING "Default maximum cache size.")


# Set the appropriate compiler/architecture for universal OSX binaries.
IF(${CMAKE_SYSTEM_NAME} EQUAL "Darwin")
	SET(CMAKE_OSX_ARCHITECTURES i386;x86_64)
ENDIF(${CMAKE_SYSTEM_NAME} EQUAL "Darwin")

# Option to build NetCDF Version 2
OPTION (BUILD_V2 "Build NetCDF Version 2." ON)

# Option to build utilities
OPTION (BUILD_UTILITIES "Build ncgen, ncgen3, ncdump." ON)
	
# Option to use MMAP
OPTION (BUILD_MMAP "Use MMAP." OFF)

# Option to use Diskless
OPTION (BUILD_DISKLESS "Build Diskless." OFF)

# Option Logging, only valid for netcdf4. 
OPTION (LOGGING "Enable Logging." OFF)

# Option to use HDF4
OPTION (USE_HDF4 "Use HDF4." OFF)
IF (USE_HDF4) 
   FIND_PACKAGE(HDF4)
   IF(NOT HDF4_FOUND)
	MESSAGE(FATAL_ERROR "HDF4 Support specified, but HDF4 libraries were not found.")
   ENDIF(NOT HDF4_FOUND)
   CHECK_INCLUDE_FILE("mfhdf.h"	HAVE_MFHDF_H)
   IF(NOT HAVE_MFHDF_H)
	MESSAGE(FATAL_ERROR "HDF4 Support specified, cannot find file mfhdf.h")
   ENDIF(NOT HAVE_MFHDF_H) 
	

ENDIF (USE_HDF4)



# Option to Build DLL
OPTION (BUILD_DLL "Build a Windows DLL." OFF)
IF (BUILD_DLL)
  ADD_DEFINITIONS(-DDLL_NETCDF)
  ADD_DEFINITIONS(-DDLL_EXPORT)
ENDIF ()



OPTION (ENABLE_NETCDF_4 "Enable NetCDF-4" ${USE_HDF5})
IF(ENABLE_NETCDF_4)
	SET(USE_NETCDF4 ON CACHE BOOL "")
	SET(ENABLE_NETCDF4 ON CACHE BOOL "")
ENDIF()

OPTION (ENABLE_CDMREMOTE OFF "Build with cdmremote client support.")

# Option to Enable HDF5
OPTION (USE_HDF5 "Use HDF5." ${ENABLE_NETCDF_4})

IF (USE_HDF5 OR ENABLE_NETCDF_4)
  SET(USE_HDF5 ON)
  SET(USE_NETCDF4 ON)
  
  FIND_PACKAGE(HDF5)
  IF(NOT HDF5_FOUND)
    MESSAGE(FATAL_ERROR "HDF5 Not Found")
   ENDIF(NOT HDF5_FOUND)
   CHECK_INCLUDE_FILE("hdf5.h" HAVE_HDF5_H)
   IF(NOT HAVE_HDF5_H)
	MESSAGE(FATAL_ERROR "HDF5 Support specified, cannot find hdf5.h")
   ENDIF(NOT HAVE_HDF5_H)
 
   FIND_LIBRARY(HDF5_HL_LIBRARY NAMES hdf5_hl libhdf5_hl hdf5_hldll)  
   IF(NOT HDF5_HL_LIBRARY)
	MESSAGE(FATAL_ERROR "HDF5 Support specified, cannot find hdf5_hl")
   ENDIF()	
     
   SET(H5_USE_16_API 1)

ENDIF ()


# Option to Build DAP Client
OPTION (USE_DAP "Enable DAP Client." ON)
IF (USE_DAP)
	FIND_PACKAGE(CURL)
	#FIND_LIBRARY(CURL NAMES curl libcurl)
	IF(NOT CURL_FOUND)
		MESSAGE(FATAL_ERROR "DAP Support specified, CURL libraries are not found.")
	ENDIF(NOT CURL_FOUND)
	#MESSAGE("Looking for libcurl: ${CURL}")

ENDIF(USE_DAP)

IF(ENABLE_NETCDF4 AND USE_DAP)		
	OPTION(ENABLE_DAP_GROUPS "Whether netcdf4 group names should be enabled." ON)
ELSE()
	SET(ENABLE_DAP_GROUPS OFF CACHE BOOL "Whether netcdf4 group names should be enabled.")
ENDIF()

OPTION (ENABLE_DOXYGEN "Enable generation of documentation" OFF)
OPTION (ENABLE_INTERNAL_DOCS "Enable documentation of library internals. This is of interest only to those developing the netCDF library." OFF)
OPTION (USE_FSYNC	"Enable experimental fsync code." OFF)
OPTION (VALGRIND_TESTS  "Build with valgrind tests (valgrind is required, static builds only." OFF)

# Does the user want to turn on PNETCDF read ability?
OPTION (ENABLE_PNETCDF "Enable parallel I/O for classic and 64-bit offset files using parallel-netcdf." OFF)
IF(ENABLE_PNETCDF)
	FIND_LIBRARY(PNETCDF NAMES pnetcdf)
	IF(NOT PNETCDF)
		MESSAGE(FATAL_ERROR "Cannot find pnetcdf library, yet pnetcdf support was requested.")
	ENDIF()
	SET(USE_PNETCDF ON CACHE BOOL "")
ENDIF()


FIND_LIBRARY(MATH NAMES math m)
MESSAGE(STATUS "Math library: ${MATH}")
IF(NOT MATH)
  MESSAGE(FATAL_ERROR "Unable to find the math library.")
ENDIF()


OPTION(BUILD_CDMREMOTE "Enabled CDMRemote" OFF)
OPTION(USE_FFIO "If true, use ffio instead of posixio" OFF)


MARK_AS_ADVANCED(ENABLE_DOXYGEN ENABLE_INTERNAL_DOCS VALGRIND_TESTS ENABLE_PNETCDF BUILD_CDMREMOTE)

#####
# End option checks.
#####


# Library checks
CHECK_INCLUDE_FILE("math.h"      HAVE_MATH_H)
CHECK_INCLUDE_FILE("alloca.h"	 HAVE_ALLOCA_H)
CHECK_INCLUDE_FILE("ctype.h"	 HAVE_CTYPE_H)
CHECK_INCLUDE_FILE("dirent.h"	 HAVE_DIRENT_H)
CHECK_INCLUDE_FILE("dlfcn.h"	 HAVE_DLFCN_H)
CHECK_INCLUDE_FILE("errno.h"	 HAVE_ERRNO_H)
CHECK_INCLUDE_FILE("fcntl.h"	 HAVE_FCNTL_H)
CHECK_INCLUDE_FILE("getopt.h"	 HAVE_GETOPT_H)
CHECK_INCLUDE_FILE("stdbool.h"	 HAVE_STDBOOL_H)
CHECK_INCLUDE_FILE("locale.h"	 HAVE_LOCAL_H)
CHECK_INCLUDE_FILE("stdint.h"	 HAVE_STDINT_H)
CHECK_INCLUDE_FILE("stdio.h"	 HAVE_STDIO_H)
CHECK_INCLUDE_FILE("stdlib.h"	 HAVE_STDLIB_H)
CHECK_INCLUDE_FILE("strings.h"	 HAVE_STRINGS_H)
CHECK_INCLUDE_FILE("sys/dir.h"	 HAVE_SYS_DIR_H)
CHECK_INCLUDE_FILE("sys/ndir.h"	 HAVE_SYS_NDIR_H)
CHECK_INCLUDE_FILE("sys/param.h" HAVE_SYS_PARAM_H)
CHECK_INCLUDE_FILE("sys/stat.h"	 HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE("sys/time.h"	 HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILE("sys/types.h" HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE("sys/wait.h"	 HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILE("sys/resource.h" HAVE_SYS_RESOURCE_H)
CHECK_INCLUDE_FILE("fcntl.h"	HAVE_FCNTL_H)
CHECK_INCLUDE_FILE("inttypes.h"  HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE("curl.h"	HAVE_CURL_H)
CHECK_INCLUDE_FILE("pstdint.h"	HAVE_PSTDINT_H)
CHECK_INCLUDE_FILE("endian.h"	HAVE_ENDIAN_H)

# Type checks
CHECK_TYPE_SIZE("double"	 SIZEOF_DOUBLE)
CHECK_TYPE_SIZE("float"		 SIZEOF_FLOAT)
CHECK_TYPE_SIZE("int"		 SIZEOF_INT)
CHECK_TYPE_SIZE("long"		 SIZEOF_LONG)
CHECK_TYPE_SIZE("long long"	 SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE("off_t"		 SIZEOF_OFF_T)
CHECK_TYPE_SIZE("short"		 SIZEOF_SHORT)
CHECK_TYPE_SIZE("size_t"	 SIZEOF_SIZE_T)
CHECK_TYPE_SIZE("uchar"		 SIZEOF_UCHAR)

# Check for various functions. 
CHECK_FUNCTION_EXISTS(fsync	HAVE_FSYNC)
CHECK_FUNCTION_EXISTS(strdup	HAVE_STRDUP)
CHECK_FUNCTION_EXISTS(strlcat 	HAVE_STRLCAT)
CHECK_FUNCTION_EXISTS(strerror	HAVE_STRERROR)
CHECK_FUNCTION_EXISTS(snprintf	HAVE_SNPRINTF)
CHECK_FUNCTION_EXISTS(strchr	HAVE_STRCHR)
CHECK_FUNCTION_EXISTS(strrchr	HAVE_STRRCHR)
CHECK_FUNCTION_EXISTS(strcat	HAVE_STRCAT)
CHECK_FUNCTION_EXISTS(strcpy	HAVE_STRCPY)
CHECK_FUNCTION_EXISTS(strdup	HAVE_STRDUP)
CHECK_FUNCTION_EXISTS(strcasecmp	HAVE_STRCASECMP)
CHECK_FUNCTION_EXISTS(strtod	HAVE_STRTOD)
CHECK_FUNCTION_EXISTS(strtoll	HAVE_STRTOLL)
CHECK_FUNCTION_EXISTS(strtoull	HAVE_STROULL)
CHECK_FUNCTION_EXISTS(strstr	HAVE_STRSTR)
CHECK_FUNCTION_EXISTS(mkstemp	HAVE_MKSTEMP)
CHECK_FUNCTION_EXISTS(rand	HAVE_RAND)
CHECK_FUNCTION_EXISTS(getrlimit	HAVE_GETRLIMIT)
CHECK_FUNCTION_EXISTS(gettimeofday	HAVE_GETTIMEOFDAY)
CHECK_FUNCTION_EXISTS(fsync	HAVE_FSYNC)
CHECK_FUNCTION_EXISTS(MPI_Comm_f2C	HAVE_MPI_COMM_F2C)
CHECK_FUNCTION_EXISTS(memmove	HAVE_MEMMOVE)
CHECK_FUNCTION_EXISTS(getpagesize	HAVE_GETPAGESIZE)
CHECK_FUNCTION_EXISTS(sysconf	HAVE_SYSCONF)
CHECK_FUNCTION_EXISTS(mremap	HAVE_MREMAP)

###
# Options related to the man page generation.
###




# The C API man page.
###

set(man_MANS netcdf.3)

set(ARGS_MANPAGE -DAPI=C)

IF (USE_NETCDF4)
	SET(ARGS_MANPAGE ${ARGS_MANPAGE} -DNETCDF4=TRUE)
ENDIF(USE_NETCDF4)

IF (BUILD_DAP)
	SET(ARGS_MANPAGE ${ARGS_MANPAGE} -DDAP=TRUE)
ENDIF(BUILD_DAP)

IF (BUILD_PARALLEL)
	SET(ARGS_MANPAGE ${ARGS_MANPAGE} -DPARALLEL_IO=TRUE)
ENDIF(BUILD_PARALLEL)

CHECK_FUNCTION_EXISTS(getrlimit	HAVE_GETRLIMIT)

# Check existence of various libraries.


# Create config.h file
configure_file("${NetCDF_SOURCE_DIR}/cmake_config.h.in"
	"${NetCDF_BINARY_DIR}/config.h")
configure_file("${NetCDF_SOURCE_DIR}/cmake_nc-config.in"
	"${NetCDF_BINARY_DIR}/nc-config")
IF(APPLE)
  EXECUTE_PROCESS(COMMAND "chmod 755 ${NetCDF_BINARY_DIR}/nc-config")
ENDIF()
INSTALL(PROGRAMS ${NetCDF_BINARY_DIR}/nc-config DESTINATION bin)
INCLUDE_DIRECTORIES(${NetCDF_BINARY_DIR})
## End autotools-style checs for config.h



#----
# Set core names of the libraries.
#---
SET (NetCDF_LIB_CORENAME	"netcdf")

#---
# Set the true names of all the libraries, if customized by external project
#---
INCLUDE_DIRECTORIES(${NetCDF_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${NetCDF_SOURCE_DIR}/oc2)

# Recurse into other subdirectories.
#add_subdirectory(h5_test)
#add_subdirectory(man4)
#add_subdirectory(man4/images)
add_subdirectory(libdispatch)
add_subdirectory(libsrc)

IF (USE_HDF5)
add_subdirectory(libsrc4)
ENDIF (USE_HDF5)

IF (USE_DAP)
	ADD_SUBDIRECTORY(oc2)
	ADD_SUBDIRECTORY(libdap2)
ENDIF()

IF (BUILD_CDMREMOTE)
   ADD_SUBDIRECTORY(libcdmr)
ENDIF()

add_subdirectory(liblib)

IF (BUILD_UTILITIES)
	ADD_SUBDIRECTORY(ncdump)
	ADD_SUBDIRECTORY(ncgen)
	ADD_SUBDIRECTORY(ncgen3)
ENDIF()

ADD_SUBDIRECTORY(nctest)
ADD_SUBDIRECTORY(nc_test)

#####
# Moving on to CPack, install packages.
#####
INSTALL(FILES ${NetCDF_BINARY_DIR}/config.h
	DESTINATION include)
INSTALL(FILES ${NetCDF_SOURCE_DIR}/include/netcdf.h
  DESTINATION include)

# Subdirectory CMakeLists.txt files should specify their own
# 'install' files.
# Including 'CPack' kicks everything off.
INCLUDE(InstallRequiredSystemLibraries)
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
  @ONLY
)

SET (CPACK_SOURCE_IGNORE_FILES "${CPACK_SOURCE_IGNORE_FILES};/${CMAKE_CURRENT_BINARY_DIR}/;~$")

# CPack inclusion must come last.
INCLUDE(CPack)




