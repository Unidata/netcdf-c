#####
# Contains variables and settings used
# by the CMake build system in order to 
# build binary installers.
#####


###
# Set options specific to the 
# Nullsoft Installation System (NSIS)
###

IF(WIN32)
  SET(CPACK_NSIS_MODIFY_PATH ON)
  SET(CPACK_NSIS_HELP_LINK "http://www.unidata.ucar.edu/netcdf")
  SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
ENDIF()

INCLUDE(CPACK)