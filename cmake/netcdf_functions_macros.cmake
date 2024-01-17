################################
# Macros
################################

macro(set_std_filter filter)
# Upper case the filter name
string(TOUPPER "${filter}" upfilter)
string(TOLOWER "${filter}" downfilter)
if(ENABLE_FILTER_${upfilter})
# Define a test flag for filter
  IF(${filter}_FOUND)
    INCLUDE_DIRECTORIES(${${filter}_INCLUDE_DIRS})
    SET(ENABLE_${upfilter} TRUE)
    SET(HAVE_${upfilter} ON)
    SET(STD_FILTERS "${STD_FILTERS} ${downfilter}")
    MESSAGE(">>> Standard Filter: ${downfilter}")
  ELSE()
    SET(ENABLE_${upfilter} FALSE)
    SET(HAVE_${upfilter} OFF)
  ENDIF()
ELSE()
  SET(HAVE_${upfilter} OFF)
ENDIF()
endmacro(set_std_filter)

macro(getuname name flag)
execute_process(COMMAND "${UNAME}" "${flag}" OUTPUT_VARIABLE "${name}" OUTPUT_STRIP_TRAILING_WHITESPACE)
endmacro(getuname)

# A macro to check if a C linker supports a particular flag.
MACRO(CHECK_C_LINKER_FLAG M_FLAG M_RESULT)
  SET(T_REQ_FLAG "${CMAKE_REQUIRED_FLAGS}")
  SET(CMAKE_REQUIRED_FLAGS "${M_FLAG}")
  CHECK_C_SOURCE_COMPILES("int main() {return 0;}" ${M_RESULT})
  SET(CMAKE_REQUIRED_FLAGS "${T_REQ_FLAG}")
ENDMACRO()

# Macro for replacing '/MD' with '/MT'.
# Used only on Windows, /MD tells VS to use the shared
# CRT libs, MT tells VS to use the static CRT libs.
#
# Taken From:
#   http://www.cmake.org/Wiki/CMake_FAQ#How_can_I_build_my_MSVC_application_with_a_static_runtime.3F
#
MACRO(specify_static_crt_flag)
  SET(vars
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_C_FLAGS_RELWITHDEBINFO
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL
    CMAKE_CXX_FLAGS_RELWITHDEBINFO)

  FOREACH(flag_var ${vars})
    IF(${flag_var} MATCHES "/MD")
      STRING(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    ENDIF()
  ENDFOREACH()

  FOREACH(flag_var ${vars})
    MESSAGE(STATUS " '${flag_var}': ${${flag_var}}")
  ENDFOREACH()
  MESSAGE(STATUS "")
ENDMACRO()


################################
# Define Utility Macros
################################

# Macro to append files to the EXTRA_DIST files.
# Note: can only be used in subdirectories because of the use of PARENT_SCOPE
SET(EXTRA_DIST "")
MACRO(ADD_EXTRA_DIST files)
  FOREACH(F ${files})
    SET(EXTRA_DIST ${EXTRA_DIST} ${CMAKE_CURRENT_SOURCE_DIR}/${F})
    SET(EXTRA_DIST ${EXTRA_DIST} PARENT_SCOPE)
  ENDFOREACH()
ENDMACRO()


MACRO(GEN_m4 filename)

  set(fallbackdest "${CMAKE_CURRENT_SOURCE_DIR}/${filename}.c")
  set(dest "${CMAKE_CURRENT_BINARY_DIR}/${filename}.c")

  # If m4 isn't present, and the generated file doesn't exist,
  # it cannot be generated and an error should be thrown.
  IF(NOT HAVE_M4)
    IF(NOT EXISTS ${fallbackdest})
      MESSAGE(FATAL_ERROR "m4 is required to generate ${filename}.c. Please install m4 so that it is on the PATH and try again.")
    ELSE()
      SET(dest ${fallbackdest})
    ENDIF()
  ELSE()
    ADD_CUSTOM_COMMAND(
      OUTPUT ${dest}
      COMMAND ${NC_M4}
      ARGS ${M4FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/${filename}.m4 > ${dest}
      VERBATIM
      )

  ENDIF()
ENDMACRO(GEN_m4)

# Binary tests, but ones which depend on value of 'TEMP_LARGE' being defined.
MACRO(add_bin_env_temp_large_test prefix F)
  ADD_EXECUTABLE(${prefix}_${F} ${F}.c)
  TARGET_LINK_LIBRARIES(${prefix}_${F} netcdf)
  IF(MSVC)
    SET_TARGET_PROPERTIES(${prefix}_${F}
      PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
      )
  ENDIF()

  ADD_TEST(${prefix}_${F} bash "-c" "TEMP_LARGE=${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${prefix}_${F}")
  IF(MSVC)
    SET_PROPERTY(TARGET ${prefix}_${F} PROPERTY FOLDER "tests")
    SET_TARGET_PROPERTIES(${prefix}_${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(${prefix}_${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(${prefix}_${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
      ${CMAKE_CURRENT_BINARY_DIR})
   SET_TARGET_PROPERTIES(${prefix}_${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
      ${CMAKE_CURRENT_BINARY_DIR})
  ENDIF()
ENDMACRO()


# Tests which are binary, but depend on a particular environmental variable.
MACRO(add_bin_env_test prefix F)
  ADD_EXECUTABLE(${prefix}_${F} ${F}.c)
  TARGET_LINK_LIBRARIES(${prefix}_${F} netcdf)
  IF(MSVC)
    SET_TARGET_PROPERTIES(${prefix}_${F}
      PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
      )
  ENDIF()

  ADD_TEST(${prefix}_${F} bash "-c" "TOPSRCDIR=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${prefix}_${F}")
  IF(MSVC)
    SET_PROPERTY(TARGET ${prefix}_${F} PROPERTY FOLDER "tests")
  ENDIF()
ENDMACRO()

# Build a binary used by a script, but don't make a test out of it.
MACRO(build_bin_test F)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${F}.c")
    ADD_EXECUTABLE(${F} "${CMAKE_CURRENT_SOURCE_DIR}/${F}.c" ${ARGN})
  else()
    # File should have been copied to the binary directory
    ADD_EXECUTABLE(${F} "${CMAKE_CURRENT_BINARY_DIR}/${F}.c" ${ARGN})
  endif()
  TARGET_LINK_LIBRARIES(${F} netcdf ${ALL_TLL_LIBS})
  IF(MSVC)
    SET_TARGET_PROPERTIES(${F}
      PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
      )
    SET_TARGET_PROPERTIES(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
      ${CMAKE_CURRENT_BINARY_DIR})
  ENDIF()
ENDMACRO()

# Binary tests which are used by a script looking for a specific name.
MACRO(add_bin_test_no_prefix F)
  build_bin_test(${F} ${ARGN})
  ADD_TEST(${F} ${EXECUTABLE_OUTPUT_PATH}/${F})
  IF(MSVC)
    SET_PROPERTY(TEST ${F} PROPERTY FOLDER "tests/")
    SET_TARGET_PROPERTIES(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
      ${CMAKE_CURRENT_BINARY_DIR})
  ENDIF()
ENDMACRO()

# Binary tests which are used by a script looking for a specific name.
MACRO(build_bin_test_no_prefix F)
  build_bin_test(${F})
  IF(MSVC)
    #SET_PROPERTY(TEST ${F} PROPERTY FOLDER "tests/")
    SET_TARGET_PROPERTIES(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
      ${CMAKE_CURRENT_BINARY_DIR})
  ENDIF()
ENDMACRO()

# Build a test and add it to the test list.
MACRO(add_bin_test prefix F)
  ADD_EXECUTABLE(${prefix}_${F} ${F}.c ${ARGN})
  TARGET_LINK_LIBRARIES(${prefix}_${F}
    ${ALL_TLL_LIBS}
    netcdf
    )
  IF(MSVC)
    SET_TARGET_PROPERTIES(${prefix}_${F}
      PROPERTIES LINK_FLAGS_DEBUG " /NODEFAULTLIB:MSVCRT"
      )
  ENDIF()
  ADD_TEST(${prefix}_${F}
           ${EXECUTABLE_OUTPUT_PATH}/${prefix}_${F}
           )
  IF(MSVC)
    SET_PROPERTY(TEST ${prefix}_${F} PROPERTY FOLDER "tests/")
    SET_TARGET_PROPERTIES(${prefix}_${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(${prefix}_${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG
      ${CMAKE_CURRENT_BINARY_DIR})
    SET_TARGET_PROPERTIES(${prefix}_${F} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE
      ${CMAKE_CURRENT_BINARY_DIR})
  ENDIF()
ENDMACRO()

# A cmake script to print out information at the end of the configuration step.
MACRO(print_conf_summary)
  MESSAGE("")
  MESSAGE("")
  MESSAGE("Configuration Summary:")
  MESSAGE("")
  MESSAGE(STATUS "Building Shared Libraries:     ${BUILD_SHARED_LIBS}")
  MESSAGE(STATUS "Building netCDF-4:             ${ENABLE_NETCDF_4}")
  MESSAGE(STATUS "Building DAP2 Support:         ${ENABLE_DAP2}")
  MESSAGE(STATUS "Building DAP4 Support:         ${ENABLE_DAP4}")
  MESSAGE(STATUS "Building Byte-range Support:   ${ENABLE_BYTERANGE}")
  MESSAGE(STATUS "Building Utilities:            ${BUILD_UTILITIES}")
  IF(CMAKE_PREFIX_PATH)
    MESSAGE(STATUS "CMake Prefix Path:             ${CMAKE_PREFIX_PATH}")
  ENDIF()
  MESSAGE("")

  IF(${STATUS_PNETCDF} OR ${STATUS_PARALLEL})
    MESSAGE("Building Parallel NetCDF")
    MESSAGE(STATUS "Using PnetCDF:       ${STATUS_PNETCDF}")
    MESSAGE(STATUS "Using Parallel IO:   ${STATUS_PARALLEL}")
    MESSAGE("")
  ENDIF()

  MESSAGE("Tests Enabled:              ${ENABLE_TESTS}")
  IF(ENABLE_TESTS)
    MESSAGE(STATUS "DAP Remote Tests:  ${ENABLE_DAP_REMOTE_TESTS}")
    MESSAGE(STATUS "Extra Tests:       ${ENABLE_EXTRA_TESTS}")
    MESSAGE(STATUS "Coverage Tests:    ${ENABLE_COVERAGE_TESTS}")
    MESSAGE(STATUS "Parallel Tests:    ${ENABLE_PARALLEL_TESTS}")
    MESSAGE(STATUS "Large File Tests:  ${ENABLE_LARGE_FILE_TESTS}")
    MESSAGE(STATUS "Extreme Numbers:   ${ENABLE_EXTREME_NUMBERS}")
    MESSAGE(STATUS "Unit Tests:        ${ENABLE_UNIT_TESTS}")
  ENDIF()

  MESSAGE("")
  MESSAGE("Compiler:")
  MESSAGE("")
  MESSAGE(STATUS "Build Type:           ${CMAKE_BUILD_TYPE}")
  MESSAGE(STATUS "CMAKE_C_COMPILER:     ${CMAKE_C_COMPILER}")
  MESSAGE(STATUS "CMAKE_C_FLAGS:        ${CMAKE_C_FLAGS}")
  IF("${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    MESSAGE(STATUS "CMAKE_C_FLAGS_DEBUG:  ${CMAKE_C_FLAGS_DEBUG}")
  ENDIF()
  IF("${CMAKE_BUILD_TYPE}" STREQUAL "RELEASE")
    MESSAGE(STATUS "CMAKE_C_FLAGS_RELEASE:   ${CMAKE_C_FLAGS_RELEASE}")
  ENDIF()

  MESSAGE(STATUS "Linking against:      ${ALL_TLL_LIBS}")

  MESSAGE("")
ENDMACRO()

MACRO(add_sh_test prefix F)
  IF(HAVE_BASH)
    ADD_TEST(${prefix}_${F} bash "-c" "export srcdir=${CMAKE_CURRENT_SOURCE_DIR};export TOPSRCDIR=${CMAKE_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}/${F}.sh ${ARGN}")
  ENDIF()
ENDMACRO()


################################
# Functions
################################

function(booleanize VALUE RETVAR)
  # force case
  STRING(TOLOWER "${VALUE}" LCVALUE)
  # Now do all the comparisons
  if(LCVALUE IN_LIST TRUELIST OR LCVALUE GREATER 0)
    SET(${RETVAR} TRUE PARENT_SCOPE)
  elseif(LCVALUE IN_LIST FALSELIST OR LCVALUE MATCHES ".*-notfound" OR LCVALUE STREQUAL "")
    SET(${RETVAR} FALSE PARENT_SCOPE)
   else()
     SET(${RETVAR} NOTFOUND PARENT_SCOPE)
  endif()
endfunction()


# A function used to create autotools-style 'yes/no' definitions.
# If a variable is set, it 'yes' is returned. Otherwise, 'no' is
# returned.
#
# Also creates a version of the ret_val prepended with 'NC',
# when feature is true, which is used to generate netcdf_meta.h.
FUNCTION(is_enabled feature ret_val)
  IF(${feature})
    SET(${ret_val} "yes" PARENT_SCOPE)
    SET("NC_${ret_val}" 1 PARENT_SCOPE)
  ELSE()
    SET(${ret_val} "no" PARENT_SCOPE)
    SET("NC_${ret_val}" 0 PARENT_SCOPE)
  ENDIF(${feature})
ENDFUNCTION()

# A function used to create autotools-style 'yes/no' definitions.
# If a variable is set, it 'yes' is returned. Otherwise, 'no' is
# returned.
#
# Also creates a version of the ret_val prepended with 'NC',
# when feature is true, which is used to generate netcdf_meta.h.
FUNCTION(is_disabled feature ret_val)
  IF(${feature})
    SET(${ret_val} "no" PARENT_SCOPE)
  ELSE()
    SET(${ret_val} "yes" PARENT_SCOPE)
    SET("NC_${ret_val}" 1 PARENT_SCOPE)
  ENDIF(${feature})
ENDFUNCTION()