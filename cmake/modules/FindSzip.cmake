# Searches for an installation of the szip library. On success, it sets the following variables:
#
#   Szip_FOUND              Set to true to indicate the szip library was found
#   Szip_INCLUDE_DIRS       The directory containing the header file szip/szip.h
#   Szip_LIBRARIES          The libraries needed to use the szip library
#
# To specify an additional directory to search, set Szip_ROOT.
#
# Author: Siddhartha Chaudhuri, 2009
#

# Look for the header, first in the user-specified location and then in the system locations
SET(Szip_INCLUDE_DOC "The directory containing the header file szip.h")
FIND_PATH(Szip_INCLUDE_DIRS NAMES szlib.h szip.h szip/szip.h PATHS ${Szip_ROOT} ${Szip_ROOT}/include DOC ${Szip_INCLUDE_DOC} NO_DEFAULT_PATH)
IF(NOT Szip_INCLUDE_DIRS)  # now look in system locations
  FIND_PATH(Szip_INCLUDE_DIRS NAMES szlib.h szip.h szip/szip.h DOC ${Szip_INCLUDE_DOC})
ENDIF(NOT Szip_INCLUDE_DIRS)

SET(Szip_FOUND FALSE)

IF(Szip_INCLUDE_DIRS)
  SET(Szip_LIBRARY_DIRS ${Szip_INCLUDE_DIRS})

  IF("${Szip_LIBRARY_DIRS}" MATCHES "/include$")
    # Strip off the trailing "/include" in the path.
    GET_FILENAME_COMPONENT(Szip_LIBRARY_DIRS ${Szip_LIBRARY_DIRS} PATH)
  ENDIF("${Szip_LIBRARY_DIRS}" MATCHES "/include$")

  IF(EXISTS "${Szip_LIBRARY_DIRS}/lib")
    SET(Szip_LIBRARY_DIRS ${Szip_LIBRARY_DIRS}/lib)
  ENDIF(EXISTS "${Szip_LIBRARY_DIRS}/lib")

  # Find Szip libraries
  FIND_LIBRARY(Szip_DEBUG_LIBRARY NAMES szipd szip_d libszipd libszip_d szip libszip sz2 libsz2
               PATH_SUFFIXES Debug ${CMAKE_LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_ARCHITECTURE}/Debug
               PATHS ${Szip_LIBRARY_DIRS} NO_DEFAULT_PATH)
  FIND_LIBRARY(Szip_RELEASE_LIBRARY NAMES szip libszip sz libsz sz2 libsz2
               PATH_SUFFIXES Release ${CMAKE_LIBRARY_ARCHITECTURE} ${CMAKE_LIBRARY_ARCHITECTURE}/Release
               PATHS ${Szip_LIBRARY_DIRS} NO_DEFAULT_PATH)

  SET(Szip_LIBRARIES )
  IF(Szip_DEBUG_LIBRARY AND Szip_RELEASE_LIBRARY)
    SET(Szip_LIBRARIES debug ${Szip_DEBUG_LIBRARY} optimized ${Szip_RELEASE_LIBRARY})
  ELSEIF(Szip_DEBUG_LIBRARY)
    SET(Szip_LIBRARIES ${Szip_DEBUG_LIBRARY})
  ELSEIF(Szip_RELEASE_LIBRARY)
    SET(Szip_LIBRARIES ${Szip_RELEASE_LIBRARY})
  ENDIF(Szip_DEBUG_LIBRARY AND Szip_RELEASE_LIBRARY)

  IF(Szip_LIBRARIES)
    SET(Szip_FOUND TRUE)
  ENDIF(Szip_LIBRARIES)
ENDIF(Szip_INCLUDE_DIRS)

IF(Szip_FOUND)
#  IF(NOT Szip_FIND_QUIETLY)
    MESSAGE(STATUS "Found Szip: headers at ${Szip_INCLUDE_DIRS}, libraries at ${Szip_LIBRARY_DIRS}")
    MESSAGE(STATUS "	library is ${Szip_LIBRARIES}")    
#  ENDIF(NOT Szip_FIND_QUIETLY)
ELSE(Szip_FOUND)
  IF(Szip_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Szip library not found")
  ENDIF(Szip_FIND_REQUIRED)
ENDIF(Szip_FOUND)
