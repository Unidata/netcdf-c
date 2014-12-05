#####
# Contains variables and settings used
# by the CMake build system in order to
# build binary installers.
#####

SET(CPACK_PACKAGE_VENDOR "Unidata")

##
# Declare exclusions list used when building a source file.
# NOTE!! This list uses regular expressions, NOT wildcards!!
##
SET(CPACK_SOURCE_IGNORE_FILES "${CPACK_SOURCE_IGNORE_FILES}"
  "/expecttds3/"
  "/nocacheremote3/"
  "/nocacheremote4/"
  "/special3/"
  "${CMAKE_BINARY_DIR}/*"
  "/myhtml/*"
  "/.svn/"
  "my.*\\\\.sh"
  "/.deps/"
  "/.libs"
  "/html/"
  ".*\\\\.jar"
  ".*\\\\.jdl"
  ".*\\\\.sed"
  ".*\\\\.proto"
  ".*\\\\.texi"
  ".*\\\\.example"
  "Make0"
  "/obsolete/"
  "/unknown/"
  ".*~"
  )

###
# Set options specific to the
# Nullsoft Installation System (NSIS)
###

IF(WIN32)
  SET(CPACK_NSIS_MODIFY_PATH ON)
  SET(CPACK_NSIS_DISPLAY_NAME "NetCDF ${netCDF_VERSION}")
  SET(CPACK_NSIS_PACKAGE_NAME "NetCDF ${netCDF_VERSION}")
  SET(CPACK_NSIS_HELP_LINK "http://www.unidata.ucar.edu/netcdf")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http://www.unidata.ucar.edu/netcdf")
  SET(CPACK_NSIS_CONTACT "support-netcdf@unidata.ucar.edu")
  SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
  SET(CPACK_NSIS_MENU_LINKS
	"http://www.unidata.ucar.edu/netcdf" "Unidata Website"
	"http://www.unidata.ucar.edu/netcdf/docs" "NetCDF Stable Documentation"
	"http://www.unidata.ucar.edu/netcdf/docs_rc" "NetCDF Unstable Documentation")

ENDIF()


##
# Set Copyright, License info for CPack.
##
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/COPYRIGHT
  ${CMAKE_CURRENT_BINARY_DIR}/COPYRIGHT.txt
  @ONLY
  )

SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/COPYRIGHT.txt")
IF(NOT CPACK_PACK_VERSION)
  SET(CPACK_PACKAGE_VERSION ${VERSION})
ENDIF()

IF(APPLE)
  SET(CPACK_SOURCE_GENERATOR "TGZ")
  SET(CPACK_GENERATOR "PackageMaker" "STGZ" "TBZ2" "TGZ" "ZIP")
ENDIF()

##
# Create an 'uninstall' target.
##
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)


ADD_CUSTOM_TARGET(uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

##
# Customize some of the package component descriptions
##

set(CPACK_COMPONENT_UTILITIES_DESCRIPTION
  "The NetCDF-C Utilities")
set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION
  "The NetCDF-C Libraries")
set(CPACK_COMPONENT_HEADERS_DESCRIPTION
  "Header files for use with NetCDF-C")
set(CPACK_COMPONENT_DEPENDENCIES_DESCRIPTION
  "Dependencies for this build of NetCDF-C")
set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION
  "The NetCDF-C user documentation.")

INCLUDE(CPack)