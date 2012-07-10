#Minimum required CMake Version
cmake_minimum_required(VERSION 2.8.6)

#Project Name
project(NETCDF C)
ADD_DEFINITIONS()

#Add custom CMake Modules
SET (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/"
    CACHE INTERNAL "Location of our custom CMake modules.")

# Configure type checks
INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckTypeSize.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckFunctionExists.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckCXXSourceCompiles.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/CheckSymbolExists.cmake)


SET(CMAKE_INCLUDE_CURRENT_DIR ON)
INCLUDE_DIRECTORIES(${NETCDF_SOURCE_DIR}/include)

# Option to Build DLL
OPTION (DLL_NETCDF "Build a Windows DLL." OFF)

# Option to Build DAP Client
OPTION (ENABLE_DAP "Enable DAP Client." OFF)

# Option to Enable HDF4
OPTION (USE_HDF4 "Use HDF4." OFF)
IF (USE_HDF4) 
   FIND_PACKAGE(HDF4)
ENDIF (USE_HDF4)

# Packages to include

FIND_PACKAGE (CURL)
FIND_PACKAGE (HDF5)
FIND_PACKAGE (ZLIB)

## Beging autotools-style checks for config.h

CHECK_INCLUDE_FILE("alloca.h"	 HAVE_ALLOCA_H)
CHECK_INCLUDE_FILE("ctype.h"	 HAVE_CTYPE_H)
CHECK_INCLUDE_FILE("dirent.h"	 HAVE_DIRENT_H)
CHECK_INCLUDE_FILE("dlfcn.h"	 HAVE_DLFCN_H)
CHECK_INCLUDE_FILE("errno.h"	 HAVE_ERRNO_H)
CHECK_INCLUDE_FILE("fcntl.h"	 HAVE_FCNTL_H)
CHECK_INCLUDE_FILE("getopt.h"	 HAVE_GETOPT_H)
CHECK_INCLUDE_FILE("hdf5.h"	 HAVE_HDF5_H)
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

CHECK_TYPE_SIZE("double"	 SIZEOF_DOUBLE)
CHECK_TYPE_SIZE("float"		 SIZEOF_FLOAT)
CHECK_TYPE_SIZE("int"		 SIZEOF_INT)
CHECK_TYPE_SIZE("long"		 SIZEOF_LONG)
CHECK_TYPE_SIZE("long long"	 SIZEOF_LONG_LONG)
CHECK_TYPE_SIZE("off_t"		 SIZEOF_OFF_T)
CHECK_TYPE_SIZE("short"		 SIZEOF_SHORT)
CHECK_TYPE_SIZE("size_t"	 SIZEOF_SIZE_T)
CHECK_TYPE_SIZE("uchar"		 SIZEOF_UCHAR)



# Create config.h file
configure_file("${NETCDF_SOURCE_DIR}/cmake_config.h.in"
	"${NETCDF_BINARY_DIR}/config.h")
INCLUDE_DIRECTORIES(${NETCDF_BINARY_DIR})
## End autotools-style checs for config.h



#----
# Set core names of the libraries.
#---
SET (NETCDF_LIB_CORENAME	"netcdf")

#---
# Set the true names of all the libraries, if customized by external project
#---

# Recurse into other subdirectories.
#add_subdirectory(include)
#add_subdirectory(h5_test)
#add_subdirectory(man4)
#add_subdirectory(man4/images)
add_subdirectory(libsrc)
#add_subdirectory(libsrc4)
#add_subdirectory(nc_test4)
#add_subdirectory(nc_test)
#add_subdirectory(ncdump)
#add_subdirectory(ncgen3)
#add_subdirectory(ncgen)
#add_subdirectory(examples)
#add_subdirectory(oc)
#add_subdirectory(libdap2)
#add_subdirectory(libcdmr)
#add_subdirectory(librpc)
#add_subdirectory(libdispatch)
#add_subdirectory(liblib)
#add_subdirectory(ncdump)
#add_subdirectory(ncdap_test) 
