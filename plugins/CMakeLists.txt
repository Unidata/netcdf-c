# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
set(CMAKE_BUILD_TYPE "")

if(WIN32)
  set(PLUGINEXT "dll")
  set(PLUGINPRE "__nc")
elseif(CYGWIN)
  set(PLUGINEXT "dll")
  set(PLUGINPRE "cyg__nc")
else()
   set(PLUGINPRE "lib__nc")
   if(APPLE)
     set(PLUGINEXT "dylib")
   else()
     set(PLUGINEXT "so")
   endif()
endif()

file(READ H5Znoop.c NOOP_SOURCE)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/H5Znoop1.c "#define NOOP_INSTANCE 1\n")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/H5Znoop1.c "${NOOP_SOURCE}")

set(h5misc_SOURCES H5Zmisc.c H5Zutil.c h5misc.h)

set(h5noop_SOURCES H5Znoop.c H5Zutil.c h5noop.h)
set_source_files_properties(H5Znoop.c PROPERTIES COMPILE_OPTIONS -DNOOP_INSTANCE=0)

set(h5noop1_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/H5Znoop1.c H5Zutil.c h5noop.h)
set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/H5Znoop1.c PROPERTIES COMPILE_OPTIONS -DNOOP_INSTANCE=1)

set(h5unknown_SOURCES H5Zunknown.c)

set(h5shuffle_SOURCES H5Zshuffle.c)
set(h5fletcher32_SOURCES H5Zfletcher32.c H5checksum.c)
set(h5deflate_SOURCES H5Zdeflate.c)

set(nczmisc_SOURCES NCZmisc.c)
set(nczhdf5filters_SOURCES NCZhdf5filters.c)
set(nczstdfilters_SOURCES NCZstdfilters.c)

if(NETCDF_ENABLE_PLUGINS)
# LDFLAGS = -module -avoid-version -shared -export-dynamic -no-undefined

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}")

macro(buildplugin TARGET TARGETLIB)
    set(MANGLELIB "${PLUGINPRE}${TARGETLIB}")
    add_library(${TARGET} MODULE ${${TARGET}_SOURCES})
    set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_NAME ${MANGLELIB})
    set_target_properties(${TARGET} PROPERTIES ARCHIVE_OUTPUT_NAME ${MANGLELIB})
    set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_NAME ${MANGLELIB})
    set_target_properties(${TARGET} PROPERTIES PREFIX "") # Critical that this be set to ""
    set_target_properties(${TARGET} PROPERTIES SUFFIX ".${PLUGINEXT}")
    target_link_libraries(${TARGET} PUBLIC ${ALL_TLL_LIBS};${ARGN})
  if(MSVC)
    target_compile_options(${TARGET} PRIVATE /Zi)
    # Tell linker to include symbol data
    set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "/INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF")
    # Set file name & location
    set_target_properties(${TARGET} PROPERTIES COMPILE_PDB_NAME ${TARGET} COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR})
    if(MPI_C_INCLUDE_PATH)
      target_include_directories(${TARGET} PRIVATE ${MPI_C_INCLUDE_PATH})
    endif(MPI_C_INCLUDE_PATH)
    target_include_directories(${TARGET} PRIVATE ${ZLIB_INCLUDE_DIRS})
  endif()

  if(STATUS_PARALLEL)
    target_link_libraries(${TARGET} PUBLIC MPI::MPI_C)
  endif(STATUS_PARALLEL)

  if (USE_HDF5)
    target_link_libraries(${TARGET} PRIVATE HDF5::HDF5)
  endif(USE_HDF5)
endmacro()

buildplugin(h5misc "h5misc" netcdf)
buildplugin(h5noop "h5noop")
buildplugin(h5noop1 "h5noop1")
buildplugin(h5unknown "h5unknown")

buildplugin(h5shuffle "h5shuffle")
buildplugin(h5fletcher32 "h5fletcher32")
buildplugin(h5deflate "h5deflate")

buildplugin(nczmisc "zmisc")
buildplugin(nczhdf5filters "zhdf5filters" netcdf)

if(NETCDF_ENABLE_BLOSC)
  set(h5blosc_SOURCES H5Zblosc.c)
  buildplugin(h5blosc "h5blosc" ${Blosc_LIBRARIES})
  buildplugin(nczstdfilters "zstdfilters" netcdf;${Blosc_LIBRARIES})
else()
  buildplugin(nczstdfilters "zstdfilters" netcdf)
endif()

if(NETCDF_ENABLE_ZSTD)
  set(h5zstd_SOURCES H5Zzstd.c H5Zzstd.h)
  buildplugin(h5zstd "h5zstd" ${Zstd_LIBRARIES})
endif()

# Need our version of szip if libsz available and we are not using HDF5
if(HAVE_SZ)
  set(h5szip_SOURCES H5Zszip.c H5Zszip.h)
  buildplugin(h5szip "h5szip" ${Szip_LIBRARIES})
endif()

if(HAVE_LOCAL_BZ2)
  set(h5bzip2_SOURCES H5Zbzip2.c blocksort.c huffman.c crctable.c randtable.c compress.c decompress.c bzlib.c bzlib.h bzlib_private.h)
  buildplugin(h5bzip2 "h5bzip2")
else()
  set(h5bzip2_SOURCES H5Zbzip2.c)
  buildplugin(h5bzip2 "h5bzip2" ${Bzip2_LIBRARIES})
endif()

# Installation
if(ENABLE_PLUGIN_INSTALL)

macro(installplugin PLUG)
  set(INSTALLED_PLUGIN_LIB "${PLUGINPRE}${PLUG}.${PLUGINEXT}")
  message(STATUS "Installing: ${INSTALLED_PLUGIN_LIB} into ${NETCDF_PLUGIN_INSTALL_DIR}")
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${INSTALLED_PLUGIN_LIB} DESTINATION ${NETCDF_PLUGIN_INSTALL_DIR})
endmacro()

install(DIRECTORY DESTINATION ${NETCDF_PLUGIN_INSTALL_DIR})
if(Bzip2_FOUND OR Bz2_FOUND)
  installplugin(h5bzip2)
endif()
if(Zstd_FOUND)
  installplugin(h5zstd)
endif()
if(Blosc_FOUND)
  installplugin(h5blosc)
endif()

  installplugin(h5shuffle)
  installplugin(h5fletcher32)
  installplugin(h5deflate)

  installplugin(zhdf5filters)
  installplugin(zstdfilters)
  
  if(Szip_FOUND)
    installplugin(h5szip)
  endif()

# Copy some test files from current source dir to out-of-tree build dir.
file(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
if(MSVC)
  file(COPY ${COPY_FILES} DESTINATION ${RUNTIME_OUTPUT_DIRECTORY}/)
endif()

endif(ENABLE_PLUGIN_INSTALL)
endif(NETCDF_ENABLE_PLUGINS)
