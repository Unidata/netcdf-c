####
# Export files
####

# Create CMake package configuration files. With these, other packages using
# cmake should be able to find netcdf using find_package and find_library.
# The EXPORT call is paired with one in liblib.
set(ConfigPackageLocation ${CMAKE_INSTALL_LIBDIR}/cmake/netCDF)

install(EXPORT netCDFTargets
  DESTINATION ${ConfigPackageLocation}
  COMPONENT headers
  NAMESPACE netCDF::
  )

include(CMakePackageConfigHelpers)
CONFIGURE_PACKAGE_CONFIG_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/netCDFConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/netCDFConfig.cmake"
  INSTALL_DESTINATION "${ConfigPackageLocation}"
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  PATH_VARS
  CMAKE_INSTALL_PREFIX
  CMAKE_INSTALL_INCLUDEDIR
  CMAKE_INSTALL_LIBDIR
  )

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/netCDFConfig.cmake"
  DESTINATION "${ConfigPackageLocation}"
  COMPONENT headers
  )

target_include_directories(netcdf
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Create export configuration
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/netCDF/netCDFConfigVersion.cmake"
  VERSION ${netCDF_VERSION}
  COMPATIBILITY SameMajorVersion
  )

install(
  FILES
  "${CMAKE_CURRENT_BINARY_DIR}/netCDF/netCDFConfigVersion.cmake"
  DESTINATION ${ConfigPackageLocation}
  COMPONENT headers
  )

####
# End export files
####

# CPack inclusion must come last.
option(NETCDF_PACKAGE "Create netCDF-C package " ${NETCDF_IS_TOP_LEVEL})

if (NETCDF_PACKAGE)
  include(CMakeInstallation.cmake)
endif()