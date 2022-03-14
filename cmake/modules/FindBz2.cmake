# Searches for an installation of the bz2 library. On success, it sets the following variables:
#
#   Bz2_FOUND              Set to true to indicate the bz2 library was found
#   Bz2_INCLUDE_DIRS       The directory containing the header file bz2/bz2.h
#   Bz2_LIBRARIES          The libraries needed to use the bz2 library
#
# To specify an additional directory to search, set Bz2_ROOT.
#
# Author: Siddhartha Chaudhuri, 2009
#

# Look for the header, first in the user-specified location and then in the system locations
SET(Bz2_INCLUDE_DOC "The directory containing the header file bz2.h")
FIND_PATH(Bz2_INCLUDE_DIRS NAMES bz2.h bz2/bz2.h PATHS ${Bz2_ROOT} ${Bz2_ROOT}/include DOC ${Bz2_INCLUDE_DOC} NO_DEFAULT_PATH)
IF(NOT Bz2_INCLUDE_DIRS)  # now look in system locations
  FIND_PATH(Bz2_INCLUDE_DIRS NAMES bzlib.h DOC ${Bz2_INCLUDE_DOC})
ENDIF(NOT Bz2_INCLUDE_DIRS)

SET(Bz2_FOUND FALSE)

IF(Bz2_INCLUDE_DIRS)
  SET(Bz2_LIBRARY_DIRS ${Bz2_INCLUDE_DIRS})

  IF("${Bz2_LIBRARY_DIRS}" MATCHES "/include$")
    # Strip off the trailing "/include" in the path.
    GET_FILENAME_COMPONENT(Bz2_LIBRARY_DIRS ${Bz2_LIBRARY_DIRS} PATH)
  ENDIF("${Bz2_LIBRARY_DIRS}" MATCHES "/include$")

  IF(EXISTS "${Bz2_LIBRARY_DIRS}/lib")
    SET(Bz2_LIBRARY_DIRS ${Bz2_LIBRARY_DIRS}/lib)
  ENDIF(EXISTS "${Bz2_LIBRARY_DIRS}/lib")

  # Find Bz2 libraries
  FIND_LIBRARY(Bz2_DEBUG_LIBRARY NAMES bz2d bz2_d libbz2d libbz2_d libbz2
               PATH_SUFFIXES Debug ${CMAKE_LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_ARCHITECTURE}/Debug
               PATHS ${Bz2_LIBRARY_DIRS} NO_DEFAULT_PATH)
  FIND_LIBRARY(Bz2_RELEASE_LIBRARY NAMES bz2 libbz2
               PATH_SUFFIXES Release ${CMAKE_LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_ARCHITECTURE}/Release
               PATHS ${Bz2_LIBRARY_DIRS} NO_DEFAULT_PATH)

  SET(Bz2_LIBRARIES )
  IF(Bz2_DEBUG_LIBRARY AND Bz2_RELEASE_LIBRARY)
    SET(Bz2_LIBRARIES debug ${Bz2_DEBUG_LIBRARY} optimized ${Bz2_RELEASE_LIBRARY})
  ELSEIF(Bz2_DEBUG_LIBRARY)
    SET(Bz2_LIBRARIES ${Bz2_DEBUG_LIBRARY})
  ELSEIF(Bz2_RELEASE_LIBRARY)
    SET(Bz2_LIBRARIES ${Bz2_RELEASE_LIBRARY})
  ENDIF(Bz2_DEBUG_LIBRARY AND Bz2_RELEASE_LIBRARY)

  IF(Bz2_LIBRARIES)
    SET(Bz2_FOUND TRUE)
  ENDIF(Bz2_LIBRARIES)
ENDIF(Bz2_INCLUDE_DIRS)

IF(Bz2_FOUND)
#  IF(NOT Bz2_FIND_QUIETLY)
    MESSAGE(STATUS "Found Bz2: headers at ${Bz2_INCLUDE_DIRS}, libraries at ${Bz2_LIBRARY_DIRS}")
    MESSAGE(STATUS "	library is ${Bz2_LIBRARIES}")    
#  ENDIF(NOT Bz2_FIND_QUIETLY)
ELSE(Bz2_FOUND)
  IF(Bz2_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Bz2 library not found")
  ENDIF(Bz2_FIND_REQUIRED)
ENDIF(Bz2_FOUND)
