#
# - Find bzip2
# Find the native BZIP2 includes and library
#
#  BZIP2_INCLUDE_DIRS  - where to find bzlib.h
#  BZIP2_LIBRARIES     - List of libraries when using bzip2.
#  BZIP2_FOUND         - True if bzip2 found.
#

include(SelectLibraryConfigurations)
include(FindPackageHandleStandardArgs)

find_path( BZIP2_INCLUDE_DIRS bzlib.h
  ENV
  BZIP2_ROOT
  PATH_SUFFIXES include Include
  )
mark_as_advanced( BZIP2_INCLUDE_DIRS )

find_library ( BZIP2_LIBRARIES
  NAMES bz2
  ENV BZIP2_ROOT
  PATH_SUFFIXES lib Lib
  )
mark_as_advanced ( BZIP2_LIBRARIES )

find_package_handle_standard_args( BZIP2 DEFAULT_MSG
  BZIP2_LIBRARIES BZIP2_INCLUDE_DIRS )

