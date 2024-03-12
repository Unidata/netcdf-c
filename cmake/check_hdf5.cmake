# Work out which HDF5 config header we can safely include
#
# We'd like to just use H5public.h, but if HDF5 was built against MPI, this
# might require us to have found MPI already. The next best file is H5pubconf.h,
# which actually has all the feature macros we want to check, but some
# distributions rename this for multiarch, so we've got to check some different
# names.
#
# HDF5_INCLUDE_DIR should already be set before calling this
function(check_hdf5_feature_header)
  if (_H5_FEATURE_HEADER)
    return()
  endif()

  include(CheckIncludeFile)

  set(CMAKE_REQUIRED_INCLUDES ${HDF5_INCLUDE_DIR})

  message(STATUS "Checking for HDF5 config header")
  foreach(_h5_header "H5public.h" "H5pubconf.h" "H5pubconf-64.h" "H5pubconf-32.h")
    check_include_file(${_h5_header} _can_include_h5_header${_h5_header})

    if (_can_include_h5_header${_h5_header})
      message(STATUS "Using ${_h5_header} to check for feature macros")
      set(_H5_FEATURE_HEADER ${_h5_header} CACHE INTERNAL "")
      return()
    endif()
  endforeach()

  message(FATAL_ERROR "Could not include any HDF5 config headers")
endfunction()


# Check for an HDF5 feature macro named FEATURE and store the result in VAR
#
# This just wraps `check_c_source_compiles` but ensures we use the correct header
function(check_hdf5_feature VAR FEATURE)
  if (NOT _H5_FEATURE_HEADER)
    check_hdf5_feature_header()
  endif()

  include(CheckCSourceCompiles)
  set(CMAKE_REQUIRED_INCLUDES ${HDF5_INCLUDE_DIR})

  message(STATUS "Checking for ${FEATURE}")
  check_c_source_compiles("
#include <${_H5_FEATURE_HEADER}>
#if !${FEATURE}
#error
#endif
int main() {}"
    _has_${FEATURE})

  set(${VAR} ${_has_${FEATURE}} PARENT_SCOPE)
endfunction()
