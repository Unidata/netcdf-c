# Searches for an installation of the bzip2 library. On success, it sets the following variables:
#
#   Bzip2_FOUND              Set to true to indicate the bzip2 library was found
#   Bzip2_INCLUDE_DIRS       The directory containing the header file bzip2/bzip2.h
#   Bzip2_LIBRARIES          The libraries needed to use the bzip2 library
#
# To specify an additional directory to search, set Bzip2_ROOT.
#
# Author: Siddhartha Chaudhuri, 2009
#

# Look for the header, first in the user-specified location and then in the system locations
SET(Bzip2_INCLUDE_DOC "The directory containing the header file bzip2.h")
FIND_PATH(Bzip2_INCLUDE_DIRS NAMES bzip2.h bzip2/bzip2.h PATHS ${Bzip2_ROOT} ${Bzip2_ROOT}/include DOC ${Bzip2_INCLUDE_DOC} NO_DEFAULT_PATH)
IF(NOT Bzip2_INCLUDE_DIRS)  # now look in system locations
  FIND_PATH(Bzip2_INCLUDE_DIRS NAMES bzlib.h DOC ${Bzip2_INCLUDE_DOC})
ENDIF(NOT Bzip2_INCLUDE_DIRS)

SET(Bzip2_FOUND FALSE)

IF(Bzip2_INCLUDE_DIRS)
  SET(Bzip2_LIBRARY_DIRS ${Bzip2_INCLUDE_DIRS})

  IF("${Bzip2_LIBRARY_DIRS}" MATCHES "/include$")
    # Strip off the trailing "/include" in the path.
    GET_FILENAME_COMPONENT(Bzip2_LIBRARY_DIRS ${Bzip2_LIBRARY_DIRS} PATH)
  ENDIF("${Bzip2_LIBRARY_DIRS}" MATCHES "/include$")

  IF(EXISTS "${Bzip2_LIBRARY_DIRS}/lib")
    SET(Bzip2_LIBRARY_DIRS ${Bzip2_LIBRARY_DIRS}/lib)
  ENDIF(EXISTS "${Bzip2_LIBRARY_DIRS}/lib")

  # Find Bzip2 libraries
  FIND_LIBRARY(Bzip2_DEBUG_LIBRARY NAMES bzip2d bzip2_d libbzip2d libbzip2_d libbzip2
               PATH_SUFFIXES Debug ${CMAKE_LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_ARCHITECTURE}/Debug
               PATHS ${Bzip2_LIBRARY_DIRS} NO_DEFAULT_PATH)
  FIND_LIBRARY(Bzip2_RELEASE_LIBRARY NAMES bzip2 libbzip2
               PATH_SUFFIXES Release ${CMAKE_LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_ARCHITECTURE}/Release
               PATHS ${Bzip2_LIBRARY_DIRS} NO_DEFAULT_PATH)

  SET(Bzip2_LIBRARIES )
  IF(Bzip2_DEBUG_LIBRARY AND Bzip2_RELEASE_LIBRARY)
    SET(Bzip2_LIBRARIES debug ${Bzip2_DEBUG_LIBRARY} optimized ${Bzip2_RELEASE_LIBRARY})
  ELSEIF(Bzip2_DEBUG_LIBRARY)
    SET(Bzip2_LIBRARIES ${Bzip2_DEBUG_LIBRARY})
  ELSEIF(Bzip2_RELEASE_LIBRARY)
    SET(Bzip2_LIBRARIES ${Bzip2_RELEASE_LIBRARY})
  ENDIF(Bzip2_DEBUG_LIBRARY AND Bzip2_RELEASE_LIBRARY)

  IF(Bzip2_LIBRARIES)
    SET(Bzip2_FOUND TRUE)
  ENDIF(Bzip2_LIBRARIES)
ENDIF(Bzip2_INCLUDE_DIRS)

IF(Bzip2_FOUND)
#  IF(NOT Bzip2_FIND_QUIETLY)
    MESSAGE(STATUS "Found Bzip2: headers at ${Bzip2_INCLUDE_DIRS}, libraries at ${Bzip2_LIBRARY_DIRS}")
    MESSAGE(STATUS "	library is ${Bzip2_LIBRARIES}")    
#  ENDIF(NOT Bzip2_FIND_QUIETLY)
ELSE(Bzip2_FOUND)
  IF(Bzip2_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Bzip2 library not found")
  ENDIF(Bzip2_FIND_REQUIRED)
ENDIF(Bzip2_FOUND)
