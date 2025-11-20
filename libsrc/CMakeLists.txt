## This is a CMake file, part of Unidata's netCDF package.
# Copyright 2012-2018, see the COPYRIGHT file for more information.

set(libsrc_SOURCES v1hpg.c putget.c attr.c nc3dispatch.c
  nc3internal.c var.c dim.c ncx.c lookup3.c ncio.c)

## 
# Turn off inclusion of particular files when using the cmake-native
# option to turn on Unity Builds.  
#
# For more information, see:
#  * https://github.com/Unidata/netcdf-c/pull/2839/
#  * https://cmake.org/cmake/help/latest/prop_tgt/UNITY_BUILD.html
#  * https://cmake.org/cmake/help/latest/prop_tgt/UNITY_BUILD_MODE.html#prop_tgt:UNITY_BUILD_MODE
##
set_property(SOURCE httpio.c posixio.c mmapio.c
  PROPERTY
    SKIP_UNITY_BUILD_INCLUSION ON)

# Process these files with m4.
set(m4_SOURCES attr ncx putget)
foreach (f ${m4_SOURCES})
  GEN_m4(${f} dest)
  list(APPEND libsrc_SOURCES ${dest})
endforeach(f)

list(APPEND libsrc_SOURCES pstdint.h ncio.h ncx.h)

list(APPEND libsrc_SOURCES memio.c)

if (BUILD_MMAP)
  list(APPEND libsrc_SOURCES mmapio.c)
endif( BUILD_MMAP)

if (USE_FFIO)
  list(APPEND libsrc_SOURCES ffio.c)
elseif (USE_STDIO)
  list(APPEND libsrc_SOURCES ncstdio.c)
else (USE_FFIO)
  list(APPEND libsrc_SOURCES posixio.c)
ENDif (USE_FFIO)

if (NETCDF_ENABLE_BYTERANGE)
  list(APPEND libsrc_SOURCES httpio.c)

if (NETCDF_ENABLE_S3)
  list(APPEND libsrc_SOURCES s3io.c)
endif(NETCDF_ENABLE_S3)
endif(NETCDF_ENABLE_BYTERANGE)

add_library(netcdf3 OBJECT ${libsrc_SOURCES})

if (NETCDF_ENABLE_DLL)
  target_compile_definitions(netcdf3 PRIVATE DLL_NETCDF DLL_EXPORT)
endif()

if (NETCDF_ENABLE_BYTERANGE)
  target_include_directories(netcdf3 PUBLIC ${CURL_INCLUDE_DIRS})
  target_link_libraries(netcdf3 PUBLIC CURL::libcurl)
  target_compile_options(netcdf3
    PRIVATE
      -DCURL_STATICLIB=1
  )
endif (NETCDF_ENABLE_BYTERANGE)

if(STATUS_PARALLEL)
  target_link_libraries(netcdf3 PUBLIC MPI::MPI_C)
endif(STATUS_PARALLEL)

# The C API man page.

set(MAN_FILES netcdf.3)

set(ARGS_MANPAGE -DAPI=C)

if (USE_HDF5)
  set(ARGS_MANPAGE ${ARGS_MANPAGE} -DNETCDF4=TRUE)
endif(USE_HDF5)

if (BUILD_DAP)
  set(ARGS_MANPAGE ${ARGS_MANPAGE} -DDAP=TRUE)
endif(BUILD_DAP)

if (BUILD_PARALLEL)
  set(ARGS_MANPAGE ${ARGS_MANPAGE} -DPARALLEL_IO=TRUE)
endif(BUILD_PARALLEL)

# Generate the man page.
if(HAVE_M4)

  file(GLOB libsrc_MANPAGE ${CMAKE_CURRENT_SOURCE_DIR}/../docs/netcdf.m4)
  file(COPY ${libsrc_MANPAGE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

  if(NOT MSVC)
    add_custom_target(manpage ALL
      COMMAND ${NC_M4} ${ARGS_MANPAGE} "${CMAKE_CURRENT_BINARY_DIR}/netcdf.m4" > "${CMAKE_CURRENT_BINARY_DIR}/netcdf.3"
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
    
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/netcdf.3" DESTINATION "${CMAKE_INSTALL_MANDIR}/man3" COMPONENT documentation)
  endif(NOT MSVC)

endif()
