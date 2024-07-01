################################
# Macros
################################

function(set_std_filter filter)
  # Upper case the filter name
  string(TOUPPER "${filter}" upfilter)
  string(TOLOWER "${filter}" downfilter)
  if(NETCDF_ENABLE_FILTER_${upfilter})
  # Define a test flag for filter
    if(${filter}_FOUND)
      include_directories(${${filter}_INCLUDE_DIRS})
      set(NETCDF_ENABLE_${upfilter} TRUE CACHE BOOL "Enable ${upfilter}")
      set(HAVE_${upfilter} ON CACHE BOOL "Have ${upfilter}")
      set(FOUND_STD_FILTERS "${FOUND_STD_FILTERS} ${downfilter}" PARENT_SCOPE)
    else()
      set(NETCDF_ENABLE_${upfilter} FALSE CACHE BOOL "Enable ${upfilter}" FORCE)
      set(HAVE_${upfilter} OFF CACHE BOOL "Have ${upfilter}" FORCE)
    endif()
  else()
    set(HAVE_${upfilter} OFF CACHE BOOL "Have ${upfilter}" FORCE)
  endif()
endfunction(set_std_filter)

macro(getuname name flag)
  execute_process(COMMAND "${UNAME}" "${flag}" OUTPUT_VARIABLE "${name}" OUTPUT_STRIP_TRAILING_WHITESPACE)
endmacro(getuname)

# A macro to check if a C linker supports a particular flag.
macro(CHECK_C_LINKER_FLAG M_FLAG M_RESULT)
  set(T_REQ_FLAG "${CMAKE_REQUIRED_FLAGS}")
  set(CMAKE_REQUIRED_FLAGS "${M_FLAG}")
  CHECK_C_SOURCE_COMPILES("int main() {return 0;}" ${M_RESULT})
  set(CMAKE_REQUIRED_FLAGS "${T_REQ_FLAG}")
endmacro()

# Macro for replacing '/MD' with '/MT'.
# Used only on Windows, /MD tells VS to use the shared
# CRT libs, MT tells VS to use the static CRT libs.
#
# Taken From:
#   http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_build_my_MSVC_application_with_a_static_runtime.3F
#
macro(specify_static_crt_flag)
  set(vars
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_C_FLAGS_RELWITHDEBINFO
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL
    CMAKE_CXX_FLAGS_RELWITHDEBINFO)

  foreach(flag_var ${vars})
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()

  foreach(flag_var ${vars})
    message(STATUS " '${flag_var}': ${${flag_var}}")
  endforeach()
  message(STATUS "")
endmacro()

# Macro to append files to the EXTRA_DIST files.
# Note: can only be used in subdirectories because of the use of PARENT_SCOPE
set(EXTRA_DIST "")
macro(ADD_EXTRA_DIST files)
  foreach(F ${files})
    set(EXTRA_DIST ${EXTRA_DIST} ${CMAKE_CURRENT_SOURCE_DIR}/${F})
    set(EXTRA_DIST ${EXTRA_DIST} PARENT_SCOPE)
  endforeach()
endmacro()

macro(GEN_m4 filename)

  set(fallbackdest "${CMAKE_CURRENT_SOURCE_DIR}/${filename}.c")
  set(dest "${CMAKE_CURRENT_BINARY_DIR}/${filename}.c")

  # If m4 isn't present, and the generated file doesn't exist,
  # it cannot be generated and an error should be thrown.
  if(NOT HAVE_M4)
    if(NOT EXISTS ${fallbackdest})
      message(FATAL_ERROR "m4 is required to generate ${filename}.c. Please install m4 so that it is on the PATH and try again.")
    else()
      set(dest ${fallbackdest})
    endif()
  else()
    add_custom_command(
      OUTPUT ${dest}
      COMMAND ${NC_M4}
      ARGS ${M4FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.m4 > ${dest}
      VERBATIM
      )

  endif()
endmacro(GEN_m4)

# Binary tests, but ones which depend on value of 'TEMP_LARGE' being defined.
macro(add_bin_env_temp_large_test prefix F)
  add_executable(${prefix}_${F} ${F}.c)
  target_link_libraries(${prefix}_${F} netcdf)
  IF(MSVC)
    set_target_properties(${prefix}_${F}
      PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
      )
  endif()

  add_test(${prefix}_${F} bash "-c" "TEMP_LARGE=${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${prefix}_${F}")
  if(WIN32)
    set_property(TARGET ${prefix}_${F} PROPERTY FOLDER "tests")
    set_target_properties(${prefix}_${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties(${prefix}_${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
      ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties(${prefix}_${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
      ${CMAKE_CURRENT_BINARY_DIR})
    set_target_properties(${prefix}_${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
      ${CMAKE_CURRENT_BINARY_DIR})
  endif()
endmacro()

# Tests which are binary, but depend on a particular environmental variable.
macro(add_bin_env_test prefix F)
  add_executable(${prefix}_${F} ${F}.c)
  target_link_libraries(${prefix}_${F} netcdf)
  if(MSVC)
    set_target_properties(${prefix}_${F}
      PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
      )
  endif()

  add_test(${prefix}_${F} bash "-c" "TOPSRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${prefix}_${F}")
  if(MSVC)
    set_property(TARGET ${prefix}_${F} PROPERTY FOLDER "tests")
  endif()
endmacro()

# Build a binary used by a script, but don't make a test out of it.
macro(build_bin_test F)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${F}.c")
    add_executable(${F} "${CMAKE_CURRENT_SOURCE_DIR}/${F}.c" ${ARGN})
  else()
    # File should have been copied to the binary directory
    add_executable(${F} "${CMAKE_CURRENT_BINARY_DIR}/${F}.c" ${ARGN})
  endif()
  target_link_libraries(${F} netcdf ${ALL_TLL_LIBS})

  if(MSVC)
    set_target_properties(${F}
      PROPERTIES 
        LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
    )
  endif()
endmacro()

# Binary tests which are used by a script looking for a specific name.
macro(add_bin_test_no_prefix F)
  build_bin_test(${F} ${ARGN})
  add_test(${F} ${EXECUTABLE_OUTPUT_PATH}/${F})
  if(WIN32)
    set_property(TEST ${F} PROPERTY FOLDER "tests/")
    set_target_properties(${F} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
    )
  endif()
endmacro()

# Binary tests which are used by a script looking for a specific name.
macro(build_bin_test_no_prefix F)
  build_bin_test(${F})


  if(WIN32)
    #SET_PROPERTY(TEST ${F} PROPERTY FOLDER "tests/")
    set_target_properties(${F} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
      RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
    )
  endif()
endmacro()

# Build a test and add it to the test list.
macro(add_bin_test prefix F)
  add_executable(${prefix}_${F} ${F}.c ${ARGN})
  target_link_libraries(${prefix}_${F}
    ${ALL_TLL_LIBS}
    netcdf
    )
  if(MSVC)
    set_target_properties(${prefix}_${F}
      PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
      )
  endif()
  add_test(${prefix}_${F}
           ${EXECUTABLE_OUTPUT_PATH}/${prefix}_${F}
           )
  if(WIN32)
    set_property(TEST ${prefix}_${F} PROPERTY FOLDER "tests/")
    set_target_properties(${prefix}_${F}
      PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}
    )
  endif()
endmacro()

# A cmake script to print out information at the end of the configuration step.
macro(print_conf_summary)
  message("")
  message("")
  message("Configuration Summary:")
  message("")
  message(STATUS "Building Shared Libraries:     ${BUILD_SHARED_LIBS}")
  message(STATUS "Building netCDF-4:             ${NETCDF_ENABLE_NETCDF_4}")
  message(STATUS "Building DAP2 Support:         ${NETCDF_ENABLE_DAP2}")
  message(STATUS "Building DAP4 Support:         ${NETCDF_ENABLE_DAP4}")
  message(STATUS "Building Byte-range Support:   ${NETCDF_ENABLE_BYTERANGE}")
  message(STATUS "Building Utilities:            ${NETCDF_BUILD_UTILITIES}")
  if(CMAKE_PREFIX_PATH)
    message(STATUS "CMake Prefix Path:             ${CMAKE_PREFIX_PATH}")
  endif()
  message("")

  if(${STATUS_PNETCDF} OR ${STATUS_PARALLEL})
    message("Building Parallel NetCDF")
    message(STATUS "Using PnetCDF:       ${STATUS_PNETCDF}")
    message(STATUS "Using Parallel IO:   ${STATUS_PARALLEL}")
    message("")
  endif()

  message("Tests Enabled:              ${NETCDF_ENABLE_TESTS}")
  if(NETCDF_ENABLE_TESTS)
    message(STATUS "DAP Remote Tests:  ${NETCDF_ENABLE_DAP_REMOTE_TESTS}")
    message(STATUS "Extra Tests:       ${NETCDF_ENABLE_EXTRA_TESTS}")
    message(STATUS "Coverage Tests:    ${NETCDF_ENABLE_COVERAGE_TESTS}")
    message(STATUS "Parallel Tests:    ${NETCDF_ENABLE_PARALLEL_TESTS}")
    message(STATUS "Large File Tests:  ${NETCDF_ENABLE_LARGE_FILE_TESTS}")
    message(STATUS "Extreme Numbers:   ${NETCDF_ENABLE_EXTREME_NUMBERS}")
    message(STATUS "Unit Tests:        ${NETCDF_ENABLE_UNIT_TESTS}")
  endif()

  message("")
  message("Compiler:")
  message("")
  message(STATUS "Build Type:           ${CMAKE_BUILD_TYPE}")
  message(STATUS "CMAKE_C_COMPILER:     ${CMAKE_C_COMPILER}")
  message(STATUS "CMAKE_C_FLAGS:        ${CMAKE_C_FLAGS}")
  if("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    message(STATUS "CMAKE_C_FLAGS_DEBUG:  ${CMAKE_C_FLAGS_DEBUG}")
  endif()
  if("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    message(STATUS "CMAKE_C_FLAGS_RELEASE:   ${CMAKE_C_FLAGS_RELEASE}")
  endif()

  message(STATUS "Linking against:      ${ALL_TLL_LIBS}")

  message("")
endmacro()

macro(add_sh_test prefix F)
  if(HAVE_BASH)
    add_test(${prefix}_${F} bash "-c" "export srcdir=${CMAKE_CURRENT_SOURCE_DIR};export TOPSRCDIR=${CMAKE_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}/${F}.sh ${ARGN}")
  endif()
endmacro()

macro(getdpkg_arch arch)
  execute_process(COMMAND "${NC_DPKG}" "--print-architecture" OUTPUT_VARIABLE "${arch}" OUTPUT_STRIP_TRAILING_WHITESPACE)
endmacro(getdpkg_arch)

################################
# Functions
################################

function(booleanize VALUE RETVAR)
  # force case
  string(TOLOWER "${VALUE}" LCVALUE)
  # Now do all the comparisons
  if(LCVALUE IN_LIST TRUELIST OR LCVALUE GREATER 0)
    set(${RETVAR} TRUE PARENT_SCOPE)
  elseif(LCVALUE IN_LIST FALSELIST OR LCVALUE MATCHES ".*-notfound" OR LCVALUE STREQUAL "")
    set(${RETVAR} FALSE PARENT_SCOPE)
   else()
     set(${RETVAR} NOTFOUND PARENT_SCOPE)
  endif()
endfunction()


# A function used to create autotools-style 'yes/no' definitions.
# If a variable is set, it 'yes' is returned. Otherwise, 'no' is
# returned.
#
# Also creates a version of the ret_val prepended with 'NC',
# when feature is true, which is used to generate netcdf_meta.h.
function(is_enabled feature ret_val)
  if(${feature})
    set(${ret_val} "yes" PARENT_SCOPE)
    set("NC_${ret_val}" 1 PARENT_SCOPE)
  else()
    set(${ret_val} "no" PARENT_SCOPE)
    set("NC_${ret_val}" 0 PARENT_SCOPE)
  endif(${feature})
endfunction()

# A function used to create autotools-style 'yes/no' definitions.
# If a variable is set, it 'yes' is returned. Otherwise, 'no' is
# returned.
#
# Also creates a version of the ret_val prepended with 'NC',
# when feature is true, which is used to generate netcdf_meta.h.
function(is_disabled feature ret_val)
  if(${feature})
    set(${ret_val} "no" PARENT_SCOPE)
  else()
    set(${ret_val} "yes" PARENT_SCOPE)
    set("NC_${ret_val}" 1 PARENT_SCOPE)
  endif(${feature})
endfunction()