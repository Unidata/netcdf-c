# Searches for an installation of the zstd library. On success, it sets the following variables:
#
#   Zstd_FOUND              Set to true to indicate the zstd library was found
#   Zstd_INCLUDE_DIRS       The directory containing the header file zstd/zstd.h
#   Zstd_LIBRARIES          The libraries needed to use the zstd library
#
# To specify an additional directory to search, set Zstd_ROOT.
#
# Author: Siddhartha Chaudhuri, 2009
#

# Look for the header, first in the user-specified location and then in the system locations
SET(Zstd_INCLUDE_DOC "The directory containing the header file zstd.h")
FIND_PATH(Zstd_INCLUDE_DIRS NAMES zstd.h zstd/zstd.h PATHS ${Zstd_ROOT} ${Zstd_ROOT}/include DOC ${Zstd_INCLUDE_DOC} NO_DEFAULT_PATH)
IF(NOT Zstd_INCLUDE_DIRS)  # now look in system locations
  FIND_PATH(Zstd_INCLUDE_DIRS NAMES zstd.h zstd/zstd.h DOC ${Zstd_INCLUDE_DOC})
ENDIF(NOT Zstd_INCLUDE_DIRS)

SET(Zstd_FOUND FALSE)

IF(Zstd_INCLUDE_DIRS)
  SET(Zstd_LIBRARY_DIRS ${Zstd_INCLUDE_DIRS})

  IF("${Zstd_LIBRARY_DIRS}" MATCHES "/include$")
    # Strip off the trailing "/include" in the path.
    GET_FILENAME_COMPONENT(Zstd_LIBRARY_DIRS ${Zstd_LIBRARY_DIRS} PATH)
  ENDIF("${Zstd_LIBRARY_DIRS}" MATCHES "/include$")

  IF(EXISTS "${Zstd_LIBRARY_DIRS}/lib")
    SET(Zstd_LIBRARY_DIRS ${Zstd_LIBRARY_DIRS}/lib)
  ENDIF(EXISTS "${Zstd_LIBRARY_DIRS}/lib")

  # Find Zstd libraries
  FIND_LIBRARY(Zstd_DEBUG_LIBRARY NAMES zstdd zstd_d libzstdd libzstd_d libzstd
               PATH_SUFFIXES Debug ${CMAKE_LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_ARCHITECTURE}/Debug
               PATHS ${Zstd_LIBRARY_DIRS} NO_DEFAULT_PATH)
  FIND_LIBRARY(Zstd_RELEASE_LIBRARY NAMES zstd libzstd
               PATH_SUFFIXES Release ${CMAKE_LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_ARCHITECTURE}/Release
               PATHS ${Zstd_LIBRARY_DIRS} NO_DEFAULT_PATH)

  SET(Zstd_LIBRARIES )
  IF(Zstd_DEBUG_LIBRARY AND Zstd_RELEASE_LIBRARY)
    SET(Zstd_LIBRARIES debug ${Zstd_DEBUG_LIBRARY} optimized ${Zstd_RELEASE_LIBRARY})
  ELSEIF(Zstd_DEBUG_LIBRARY)
    SET(Zstd_LIBRARIES ${Zstd_DEBUG_LIBRARY})
  ELSEIF(Zstd_RELEASE_LIBRARY)
    SET(Zstd_LIBRARIES ${Zstd_RELEASE_LIBRARY})
  ENDIF(Zstd_DEBUG_LIBRARY AND Zstd_RELEASE_LIBRARY)

  IF(Zstd_LIBRARIES)
    SET(Zstd_FOUND TRUE)
  ENDIF(Zstd_LIBRARIES)
ENDIF(Zstd_INCLUDE_DIRS)

IF(Zstd_FOUND)
#  IF(NOT Zstd_FIND_QUIETLY)
    MESSAGE(STATUS "Found Zstd: headers at ${Zstd_INCLUDE_DIRS}, libraries at ${Zstd_LIBRARY_DIRS}")
    MESSAGE(STATUS "	library is ${Zstd_LIBRARIES}")    
#  ENDIF(NOT Zstd_FIND_QUIETLY)
ELSE(Zstd_FOUND)
  IF(Zstd_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Zstd library not found")
  ENDIF(Zstd_FIND_REQUIRED)
ENDIF(Zstd_FOUND)
