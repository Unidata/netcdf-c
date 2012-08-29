#Minimum required CMake Version
cmake_minimum_required(VERSION 2.8.8)

#Project Name
project(NetCDF C)
SET(NetCDF_VERSION 4.2.1)
ENABLE_TESTING()
INCLUDE(CTest)

IF(MSVC)
	SET(USE_FOLDERS ON)
ENDIF()

# CTest/CDash configuration
SET (CTEST_DROP_METHOD http CACHE STRING "")
SET (CTEST_DROP_SITE "yakov.unidata.ucar.edu/~wfisher/cdash/" CACHE STRING "")
SET (CTEST_DROP_LOCATION "/submit.php?project=WinNETCDF" CACHE STRING "")
SET (CTEST_DROP_SITE_CDASH TRUE CACHE BOOL "")
SET (CTEST_MEMORYCHECK_COMMAND valgrind CACHE STRING "")

ADD_DEFINITIONS()

### Verbose make, for debugging.
#SET(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

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
INCLUDE (${CMAKE_ROOT}/Modules/GetPrerequisites.cmake)


#####
# A basic script used to convert m4 files
#####
MACRO(GEN_m4 filename)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.c
		COMMAND m4 
		ARGS ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.m4 > ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.c
		VERBATIM
	)
ENDMACRO(GEN_m4)

MACRO(add_bin_test prefix F)
	ADD_EXECUTABLE(${prefix}_${F} ${F}.c)
	TARGET_LINK_LIBRARIES(${prefix}_${F} netcdf)
	ADD_TEST(${prefix}_${F} ${EXECUTABLE_OUTPUT_PATH}/${prefix}_${F})
	
ENDMACRO()
	
# Shell script Macro
MACRO(add_sh_test prefix F)
	ADD_TEST(${prefix}_${F} sh ${CMAKE_CURRENT_BINARY_DIR}/${F}.sh)
ENDMACRO()


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

#IF(NOT BUILD_SHARED_LIBS)
#	SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
#	SET(CMAKE_FIND_PACKAGE_SUFFIXES .a ${CMAKE_FIND_PACKAGE_SUFFIXES})
#ENDIF()

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

# Option to use examples.
OPTION (ENABLE_EXAMPLES "Build Examples" ON)


# Option to use Diskless
OPTION (ENABLE_DISKLESS "Build Diskless." OFF)
IF(ENABLE_DISKLESS)
	SET(BUILD_DISKLESS ON)
	SET(USE_DISKLESS ON)
ENDIF()
# Option Logging, only valid for netcdf4. 
# For some reason, these appear to need to be set in each folder.
IF(ENABLE_COVERAGE_TESTS)
	SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
        #SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -W -Wshadow -Wunused-variable -Wun    used-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecate    d -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
        SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
        SET(CMAKE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
ENDIF()
OPTION (ENABLE_LOGGING "Enable Logging." OFF)

# Option to use HDF4
OPTION (USE_HDF4 "Build NetCDF-4 with HDF5 read capability (HDF4, HDF5 and Zlib required." OFF)
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
OPTION (ENABLE_DLL "Build a Windows DLL." OFF)
IF (ENABLE_DLL)
  SET(BUILD_DLL ON CACHE BOOL "")
  ADD_DEFINITIONS(-DDLL_NETCDF)
  ADD_DEFINITIONS(-DDLL_EXPORT)
ENDIF ()

# Did the user specify a default minimum blocksize for posixio?
SET (NCIO_MINBLOCKSIZE 256 CACHE STRING "Minimum I/O Blocksize for netCDF classic and 64-bit offset format files.")

# Build NetCDF4
OPTION (ENABLE_NETCDF_4 "Enable NetCDF-4" ON)
SET(USE_NETCDF4 OFF CACHE BOOL "")
IF(ENABLE_NETCDF_4)
	SET(USE_NETCDF4 ON CACHE BOOL "")
	SET(ENABLE_NETCDF_4 ON CACHE BOOL "")
	SET(ENABLE_NETCDF4 ON CACHE BOOL "")
ENDIF()

# Option for cdmremote support.
OPTION (ENABLE_CDMREMOTE OFF "Build with cdmremote client support.")

# Option for building RPC
OPTION (ENABLE_RPC OFF "ENable RPC Client and Server.")
IF(ENABLE_RPC)
	SET(BUILD_RPC ON CACHE BOOL "")
ENDIF()

# Option to Enable HDF5
OPTION (USE_HDF5 "Use HDF5." ${ENABLE_NETCDF_4})
IF (USE_HDF5 OR ENABLE_NETCDF_4)
  SET(USE_HDF5 ON)
  SET(USE_NETCDF4 ON)
 
  FIND_PACKAGE(HDF5 COMPONENTS C HL REQUIRED)
  #FIND_LIBRARY(HDF5_LIBRARY NAMES hdf5)
  #FIND_LIBRARY(HDF5_HL_LIBRARY NAMES hdf5_hl)
  
  #IF(NOT HDF5_LIBRARIES OR NOT HDF5_HL_LIBRARY) 
  #  MESSAGE(FATAL_ERROR "HDF5 Not Found.")
  #ENDIF() 
  IF(NOT HDF5_FOUND)
	MESSAGE(FATAL_ERROR "HDF5 Not Found.")
  ELSE()
	#MESSAGE(STATUS "HDF5_LIBRARY: ${HDF5_LIBRARY}")
	#MESSAGE(STATUS "HDF5_LIBRARIES: ${HDF5_LIBRARIES}") 
	#MESSAGE(STATUS "HDF5_INCLUDE_DIRS: ${HDF5_INCLUDE_DIRS}") 
  	#MESSAGE(STATUS "HDF5_HL_LIBRARIES: ${HDF5_HL_LIBRARIES}")
  ENDIF()

  #MESSAGE(STATUS "Found HDF5: ${HDF5_LIBRARIES}")
  #MESSAGE(STATUS "Found HDF5_HL: ${HDF5_HL_LIBRARY}")
  #SET(HDF5_LIBRARIES ${HDF5_LIBRARY} ${HDF5_HL_LIBRARY})
  
  INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIR})

  #CHECK_INCLUDE_FILE("hdf5.h" HAVE_HDF5_H)
  #IF(NOT HAVE_HDF5_H)
#	MESSAGE(FATAL_ERROR "HDF5 Support specified, cannot find hdf5.h")
  #ENDIF(NOT HAVE_HDF5_H)
   
  # Check to see if H5Z_SZIP exists in HDF5_Libraries. If so, we must use szip.
  FOREACH(LIB ${HDF5_LIBRARIES})
	
	IF(NOT USE_SZIP)
		CHECK_LIBRARY_EXISTS(${LIB} H5Z_SZIP "" USE_SZIP)
	ENDIF()
	MESSAGE(STATUS "LIB: ${LIB}: USE_SZIP: ${USE_SZIP}") 
  ENDFOREACH()
  
  IF(USE_SZIP)
    	FIND_PACKAGE(SZIP)
	#FIND_LIBRARY(SZIP_LIBRARY NAMES szip sz)
  	IF(NOT SZIP_LIBRARY)
		MESSAGE(FATAL_ERROR "HDF5 compiled with szip, but cannot find local szip libraries.")
  	ELSE()
		MESSAGE(STATUS "Found SZip: ${SZIP_LIBRARY}")
  	ENDIF()
  ENDIF()

  FIND_PACKAGE(ZLIB)
  #FIND_LIBRARY(ZLIB_LIBRARY NAMES zlibwapi zlibstat zlib zlib1 z)
  IF(NOT ZLIB_LIBRARY)
	MESSAGE(FATAL_ERROR "HDF5 Support specified, cannot find ZLib.")
  ENDIF()
  SET(USE_ZLIB ON)	
  #ELSE()
  #	MESSAGE(STATUS "Found ZLib: ${ZLIB_LIBRARY}")
  #ENDIF()
  INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIRS})
  SET(H5_USE_16_API 1)
   

ENDIF ()


# Option to Build DAP Client
OPTION (ENABLE_DAP "Enable DAP Client." ON)
IF (ENABLE_DAP)
	SET(USE_DAP ON)
	FIND_PACKAGE(CURL)
	IF(NOT CURL_FOUND)
		MESSAGE(FATAL_ERROR "DAP Support specified, CURL libraries are not found.")
	ENDIF()
	ADD_DEFINITIONS(-DCURL_STATICLIB=1)
	INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIRS})
ENDIF()

# Option to Enable DAP long tests, remote tests.
OPTION(ENABLE_DAP_LONG_TESTS "Enable DAP long tests." OFF)
OPTION(ENABLE_DAP_REMOTE_TESTS "Enable DAP remote tests." ON)

# If NetCDF4 and DAP, Option for DAP groups.
IF(ENABLE_NETCDF4 AND USE_DAP)		
		  OPTION(ENABLE_DAP_GROUPS "Whether netcdf4 group names should be enabled." ON)
ELSE()
	SET(ENABLE_DAP_GROUPS OFF CACHE BOOL "Whether netcdf4 group names should be enabled.")
ENDIF()



# Does the user want to turn on PNETCDF read ability?
OPTION (ENABLE_PNETCDF "Enable parallel I/O for classic and 64-bit offset files using parallel-netcdf." OFF)
IF(ENABLE_PNETCDF)
	FIND_LIBRARY(PNETCDF NAMES pnetcdf)
	IF(NOT PNETCDF)
		MESSAGE(FATAL_ERROR "Cannot find pnetcdf library, yet pnetcdf support was requested.")
	ENDIF()
	SET(USE_PNETCDF ON CACHE BOOL "")
ENDIF()

SET(MATH "")
IF(NOT WIN32)
	# FFIO insteaad of PosixIO
	OPTION(ENABLE_FFIO "If true, use ffio instead of posixio" OFF)
	IF(ENABLE_FFIO)
		SET(USE_FFIO ON CACHE BOOL "")
	ENDIF()
ENDIF()

# Enable Tests
#IF(NOT MSVC)
	OPTION (ENABLE_TESTS "Enable basic tests, run with 'make test'." ON)
	IF(ENABLE_TESTS) 
		SET(BUILD_TESTSETS ON CACHE BOOL "")
	ENDIF()
#ELSE()
#	SET(ENABLE_TESTS OFF CACHE BOOL "")
#ENDIF()

# Enable Large file tests
OPTION (ENABLE_LARGE_FILE_TESTS "Enable large file tests." OFF)
IF(ENABLE_LARGE_FILE_TESTS)
	SET(LARGE_FILE_TESTS ON)
ENDIF()

OPTION (ENABLE_INTERNAL_DOCS "Enable documentation of library internals. This is of interest only to those developing the netCDF library." OFF)
IF(ENABLE_INTERNAL_DOCS)
	SET(BUILD_INTERNAL_DOCS ON)
ENDIF()

OPTION (ENABLE_FSYNC	"Enable experimental fsync code." OFF)
IF(ENABLE_FSYNC)
	SET(USE_FSYNC ON)
ENDIF()

# Provide the option to perform coverage tests.
OPTION (ENABLE_COVERAGE_TESTS "Enable compiler flags needed to perform coverage tests." OFF)
IF(ENABLE_COVERAGE_TESTS)
	#SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
	SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -W -profile-arcs -ftest-coverage")
	SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
	SET(CMAKE_EXE_LINKER_FLAGSx "-fprofile-arcs -ftest-coverage")
	MESSAGE(STATUS "Coverage Tests: On.")
ENDIF()

OPTION (ENABLE_EXAMPLE_TESTS "Run extra example tests.  Requires GNU Sed. Ignored if NetCDF-4 is not Enabled" OFF)
IF(NOT ENABLE_NETCDF_4 AND ENABLE_EXAMPLE_TESTS)
	SET (ENABLE_EXAMPLE_TESTS OFF)
ENDIF()

OPTION (ENABLE_PARALLEL_TESTS "Enable Parallel IO Tests. Ignored if NetCDF4 is not enabled, or if there is no parallel I/O Support." OFF)

OPTION (ENABLE_DOXYGEN "Enable generation of doxygen." OFF)
IF(ENABLE_DOXYGEN)
	SET(BUILD_DOCS ON CACHE BOOL "")
	OPTION(ENABLE_INTERNAL_DOCS "Build internal documentation. This is of interest to developers only." OFF)
	IF(ENABLE_INTERNAL_DOCS)
		SET(BUILD_INTERNAL_DOCS YES CACHE STRING "")
	ELSE()
		SET(BUILD_INTERNAL_DOCS NO CACHE STRING "")
	ENDIF()
ENDIF()



# Set some of the options as advanced.
MARK_AS_ADVANCED(ENABLE_INTERNAL_DOCS VALGRIND_TESTS ENABLE_PNETCDF BUILD_CDMREMOTE ENABLE_COVERAGE_TESTS)
MARK_AS_ADVANCED(ENABLE_DAP_REMOTE_TESTS ENABLE_DAP_LONG_TESTS)
#####
# End option checks.
#####

#####
# System inspection checks
#####

# Library include checks
CHECK_INCLUDE_FILE("math.h"      HAVE_MATH_H)
CHECK_INCLUDE_FILE("unistd.h"	 HAVE_UNISTD_H)
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
CHECK_FUNCTION_EXISTS(getrlimit	HAVE_GETRLIMIT)

#####
# End system inspection checks.
#####


# Check for the math library so it can be explicitely linked.
IF(NOT WIN32)
       FIND_LIBRARY(HAVE_LIBM NAMES math m libm)
       MESSAGE(STATUS "Found Math library: ${HAVE_LIBM}")
       IF(NOT HAVE_LIBM)
       	      MESSAGE(FATAL_ERROR "Unable to find the math library.")
       ENDIF()
ENDIF()


# Create config.h file
configure_file("${NetCDF_SOURCE_DIR}/cmake_config.h.in"
	"${NetCDF_BINARY_DIR}/config.h")
configure_file("${NetCDF_SOURCE_DIR}/cmake_nc-config.in"
  "${NetCDF_SOURCE_DIR}/nc-config")
FILE(COPY "${NetCDF_SOURCE_DIR}/nc-config"
  DESTINATION "${NetCDF_BINARY_DIR}"
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
EXECUTE_PROCESS(COMMAND "chmod 755 ${NetCDF_BINARY_DIR}/nc-config")

INSTALL(PROGRAMS ${NetCDF_BINARY_DIR}/nc-config DESTINATION bin)
INCLUDE_DIRECTORIES(${NetCDF_BINARY_DIR})
# End autotools-style checs for config.h



#####
# Set core names of the libraries.
#####
SET (NetCDF_LIB_CORENAME	"netcdf")

#####
# Set the true names of all the libraries, if customized by external project
#####
INCLUDE_DIRECTORIES(${NetCDF_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${NetCDF_SOURCE_DIR}/oc2)

# Recurse into other subdirectories.
#add_subdirectory(h5_test)
#add_subdirectory(man4)
#add_subdirectory(man4/images)
add_subdirectory("include")
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
MESSAGE(STATUS "Linking with: ${ALL_TLL_LIBS}") 

# Enable Utilities.

IF (BUILD_UTILITIES)
	ADD_SUBDIRECTORY(ncgen)
	ADD_SUBDIRECTORY(ncgen3)
	ADD_SUBDIRECTORY(ncdump)
ENDIF()

# Enable tests.
IF(ENABLE_TESTS)
	ADD_SUBDIRECTORY(nctest)
	ADD_SUBDIRECTORY(nc_test)
	IF(USE_NETCDF4)
  		ADD_SUBDIRECTORY(nc_test4)
		ADD_SUBDIRECTORY(h5_test)
	ENDIF()
	IF(USE_DAP)
		ADD_SUBDIRECTORY(ncdap_test)
	ENDIF()
	
	IF(ENABLE_EXAMPLES)
		ADD_SUBDIRECTORY(examples)
	ENDIF()

ENDIF()

#####
# Build doxygen documentation, if need be.
#####
IF(BUILD_DOCS)
	ADD_SUBDIRECTORY(man4)
ENDIF()
#####
# Moving on to CPack, install packages.
#####
INSTALL(FILES ${NetCDF_BINARY_DIR}/config.h
	DESTINATION include)
#INSTALL(FILES ${NetCDF_SOURCE_DIR}/include/netcdf.h
#  DESTINATION include)
INSTALL(DIRECTORY include DESTINATION ".")
IF(ENABLE_DOXYGEN)	
	INSTALL(DIRECTORY man4 DESTINATION ".")
ENDIF()
#INSTALL(FILES ${ALL_TLL_LIBS} DESTINATION deps)

# Subdirectory CMakeLists.txt files should specify their own
# 'install' files.
# Including 'CPack' kicks everything off.
INCLUDE(InstallRequiredSystemLibraries)
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/FixBundle.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake
  @ONLY
)
#INSTALL(SCRIPT ${CMAKE_CURRENT_BINARY_DIR}/FixBundle.cmake)

#####
# Various options for CPACK
#####

##
# Declare exclusions list used when building a source file.
# NOTE!! This list uses regular expressions, NOT wildcards!! 
## 
SET (CPACK_SOURCE_IGNORE_FILES "${CPACK_SOURCE_IGNORE_FILES}"
	"Makefile\\\\.in$"
	"${CMAKE_BINARY_DIR}/*"
	"/myhtml/*"
	"/.svn/"
	"my.*sh"
	".*~" 
)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/COPYRIGHT
	${CMAKE_CURRENT_BINARY_DIR}/COPYRIGHT.txt
	@ONLY
)

SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/COPYRIGHT.txt")
SET(CPACK_PACKAGE_VERSION "4.3")

IF(APPLE)
  SET(CPACK_SOURCE_GENERATOR "TBZ2")
  SET(CPACK_GENERATOR "PackageMaker" "STGZ" "TBZ2" "TGZ" "ZIP")
ENDIF()


MESSAGE(STATUS "CPACK_SOURCE_IGNORE_FILES: ${CPACK_SOURCE_IGNORE_FILES}")
# CPack inclusion must come last.
INCLUDE(CPack)




