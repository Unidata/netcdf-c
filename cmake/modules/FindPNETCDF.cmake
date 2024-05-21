# FindPNETCDF
# -----------
#
# Find parallel IO library for classic netCDF files
#
# This module will define the following variables:
#
# ::
#
#   PNETCDF_FOUND
#   PNETCDF_INCLUDE_DIRS
#   PNETCDF_LIBRARIES
#   PNETCDF_VERSION
#   PNETCDF_HAS_ERANGE_FILL  - True if PnetCDF was built with ``NC_ERANGE`` support`
#   PNETCDF_HAS_RELAXED_COORD_BOUND - True if PnetCDF was built with relaxed coordinate bounds
#
# The ``PNETCDF::pnetcdf`` target will also be exported

include(FindPackageHandleStandardArgs)

find_path(PNETCDF_INCLUDE_DIR NAMES pnetcdf.h)
find_library(PNETCDF_LIBRARY NAMES pnetcdf)

if (PNETCDF_INCLUDE_DIR)
  set(pnetcdf_h "${PNETCDF_INCLUDE_DIR}/pnetcdf.h" )
  message(DEBUG "PnetCDF include file ${pnetcdf_h} will be searched for version")
  file(STRINGS "${pnetcdf_h}" pnetcdf_major_string REGEX "^#define PNETCDF_VERSION_MAJOR")
  string(REGEX REPLACE "[^0-9]" "" pnetcdf_major "${pnetcdf_major_string}")
  file(STRINGS "${pnetcdf_h}" pnetcdf_minor_string REGEX "^#define PNETCDF_VERSION_MINOR")
  string(REGEX REPLACE "[^0-9]" "" pnetcdf_minor "${pnetcdf_minor_string}")
  file(STRINGS "${pnetcdf_h}" pnetcdf_sub_string REGEX "^#define PNETCDF_VERSION_SUB")
  string(REGEX REPLACE "[^0-9]" "" pnetcdf_sub "${pnetcdf_sub_string}")
  set(pnetcdf_version "${pnetcdf_major}.${pnetcdf_minor}.${pnetcdf_sub}")
  message(DEBUG "Found PnetCDF version ${pnetcdf_version}")
endif()

message(DEBUG "[ ${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} ]"
  " PNETCDF_ROOT = ${PNETCDF_ROOT}"
  " PNETCDF_INCLUDE_DIR = ${PNETCDF_INCLUDE_DIR}"
  " PNETCDF_LIBRARY = ${PNETCDF_LIBRARY}"
  " PNETCDF_VERSION = ${PNETCDF_VERSION}"
)

file(STRINGS "${pnetcdf_h}" enable_erange_fill_pnetcdf REGEX "^#define PNETCDF_ERANGE_FILL")
string(REGEX REPLACE "[^0-9]" "" erange_fill_pnetcdf "${enable_erange_fill_pnetcdf}")
if("${erange_fill_pnetcdf}" STREQUAL "1")
  set(PNETCDF_HAS_ERANGE_FILL ON CACHE BOOL "")
else()
  set(PNETCDF_HAS_ERANGE_FILL OFF CACHE BOOL "")
endif()

mark_as_advanced(PNETCDF_INCLUDE_DIR PNETCDF_LIBRARY PNETCDF_ERANGE_FILL)

file(STRINGS "${pnetcdf_h}" relax_coord_bound_pnetcdf REGEX "^#define PNETCDF_RELAX_COORD_BOUND")
string(REGEX REPLACE "[^0-9]" "" relax_coord_bound "${relax_coord_bound_pnetcdf}")
if ("${relax_coord_bound}" STREQUAL "1")
  set(PNETCDF_HAS_RELAXED_COORD_BOUND ON CACHE BOOL "")
else()
  set(PNETCDF_HAS_RELAXED_COORD_BOUND OFF CACHE BOOL "")
endif()

find_package_handle_standard_args(PNETCDF
  REQUIRED_VARS PNETCDF_LIBRARY PNETCDF_INCLUDE_DIR
  VERSION_VAR PNETCDF_VERSION
  )

if (PNETCDF_FOUND AND NOT TARGET PNETCDF::PNETCDF)
  find_package(MPI REQUIRED)
  add_library(PNETCDF::PNETCDF UNKNOWN IMPORTED)
  set_target_properties(PNETCDF::PNETCDF PROPERTIES
    IMPORTED_LINK_INTERFACE_LIBRARIES MPI::MPI_C
    IMPORTED_LOCATION "${PNETCDF_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${PNETCDF_INCLUDE_DIR}"
  )
  find_library(MATH_LIBRARY m)
  if (MATH_LIBRARY)
    set_target_properties(PNETCDF::PNETCDF PROPERTIES
      IMPORTED_LINK_INTERFACE_LIBRARIES ${MATH_LIBRARY})
  endif()
endif()
