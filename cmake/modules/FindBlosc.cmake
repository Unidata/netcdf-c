# Searches for an installation of the blosc library. On success, it sets the following variables:
#
#   Blosc_FOUND              Set to true to indicate the blosc library was found
#   Blosc_INCLUDE_DIRS       The directory containing the header file blosc/blosc.h
#   Blosc_LIBRARIES          The libraries needed to use the blosc library
#
# To specify an additional directory to search, set Blosc_ROOT.
#
# Author: Siddhartha Chaudhuri, 2009
#

# Look for the header, first in the user-specified location and then in the system locations
SET(Blosc_INCLUDE_DOC "The directory containing the header file blosc.h")
FIND_PATH(Blosc_INCLUDE_DIRS NAMES blosc.h blosc/blosc.h PATHS ${Blosc_ROOT} ${Blosc_ROOT}/include DOC ${Blosc_INCLUDE_DOC} NO_DEFAULT_PATH)
IF(NOT Blosc_INCLUDE_DIRS)  # now look in system locations
  FIND_PATH(Blosc_INCLUDE_DIRS NAMES blosc.h blosc/blosc.h DOC ${Blosc_INCLUDE_DOC})
ENDIF(NOT Blosc_INCLUDE_DIRS)

SET(Blosc_FOUND FALSE)

IF(Blosc_INCLUDE_DIRS)
  SET(Blosc_LIBRARY_DIRS ${Blosc_INCLUDE_DIRS})

  IF("${Blosc_LIBRARY_DIRS}" MATCHES "/include$")
    # Strip off the trailing "/include" in the path.
    GET_FILENAME_COMPONENT(Blosc_LIBRARY_DIRS ${Blosc_LIBRARY_DIRS} PATH)
  ENDIF("${Blosc_LIBRARY_DIRS}" MATCHES "/include$")

  IF(EXISTS "${Blosc_LIBRARY_DIRS}/lib")
    SET(Blosc_LIBRARY_DIRS ${Blosc_LIBRARY_DIRS}/lib)
  ENDIF(EXISTS "${Blosc_LIBRARY_DIRS}/lib")

  # Find Blosc libraries
  FIND_LIBRARY(Blosc_DEBUG_LIBRARY NAMES bloscd blosc_d libbloscd libblosc_d libblosc
               PATH_SUFFIXES Debug ${CMAKE_LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_ARCHITECTURE}/Debug
               PATHS ${Blosc_LIBRARY_DIRS} NO_DEFAULT_PATH)
  FIND_LIBRARY(Blosc_RELEASE_LIBRARY NAMES blosc libblosc
               PATH_SUFFIXES Release ${CMAKE_LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_ARCHITECTURE}/Release
               PATHS ${Blosc_LIBRARY_DIRS} NO_DEFAULT_PATH)

  SET(Blosc_LIBRARIES )
  IF(Blosc_DEBUG_LIBRARY AND Blosc_RELEASE_LIBRARY)
    SET(Blosc_LIBRARIES debug ${Blosc_DEBUG_LIBRARY} optimized ${Blosc_RELEASE_LIBRARY})
  ELSEIF(Blosc_DEBUG_LIBRARY)
    SET(Blosc_LIBRARIES ${Blosc_DEBUG_LIBRARY})
  ELSEIF(Blosc_RELEASE_LIBRARY)
    SET(Blosc_LIBRARIES ${Blosc_RELEASE_LIBRARY})
  ENDIF(Blosc_DEBUG_LIBRARY AND Blosc_RELEASE_LIBRARY)

  IF(Blosc_LIBRARIES)
    SET(Blosc_FOUND TRUE)
  ENDIF(Blosc_LIBRARIES)
ENDIF(Blosc_INCLUDE_DIRS)

IF(Blosc_FOUND)
#  IF(NOT Blosc_FIND_QUIETLY)
    MESSAGE(STATUS "Found Blosc: headers at ${Blosc_INCLUDE_DIRS}, libraries at ${Blosc_LIBRARY_DIRS}")
    MESSAGE(STATUS "	library is ${Blosc_LIBRARIES}")    
#  ENDIF(NOT Blosc_FIND_QUIETLY)
ELSE(Blosc_FOUND)
  IF(Blosc_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Blosc library not found")
  ENDIF(Blosc_FIND_REQUIRED)
ENDIF(Blosc_FOUND)
