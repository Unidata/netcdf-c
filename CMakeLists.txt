#Minimum required CMake Version
cmake_minimum_required(VERSION 2.8.6)

#Project Name
project(netCDF 4.2)

OPTION (NETCDF_USE_FOLDERS "ENable folder grouping of projects in IDEs." ON)
MARK_AS_ADVANCED (NETCDF_USE_FOLDERS)
IF (NETCDF_USE_FOLDERS)
   SET_PROPERTY (GLOBAL PROPERTY USE_FOLDERS ON)
ENDIF (NETCDF_USE_FOLDERS)

#----
# Set core names of the libraries.
#---
SET (NETCDF_LIB_CORENAME	"netcdf")

#---
# Set the true names of all the libraries, if customized by external project
#---
SET (NETCDF_LIB_NAME		"${NETCDF_

# Recurse into other subdirectories.
add_subdirectory(include)
add_subdirectory(h5_test)
add_subdirectory(man4)
add_subdirectory(man4/images)
add_subdirectory(libsrc)
add_subdirectory(libsrc4)
add_subdirectory(nc_test4)
add_subdirectory(nc_test)
add_subdirectory(ncdump)
add_subdirectory(ncgen3)
add_subdirectory(ncgen)
add_subdirectory(examples)
add_subdirectory(oc)
add_subdirectory(libdap2)
add_subdirectory(libcdmr)
add_subdirectory(librpc)
add_subdirectory(libdispatch)
add_subdirectory(liblib)
add_subdirectory(ncdump)
add_subdirectory(ncdap_test)
