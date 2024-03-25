if(HAVE_LIBXML2)
  set(libncxml_SOURCES ncxml_xml2.c)
else()
  set(libncxml_SOURCES ncxml_tinyxml2.cpp tinyxml2.cpp tinyxml2.h)
endif()

add_library(ncxml OBJECT ${libncxml_SOURCES})
if(HAVE_LIBXML2)
  target_include_directories(ncxml 
    PUBLIC 
      ./include
    PRIVATE
      ${LIBXML2_INCLUDE_DIRS}
)
else()
  target_include_directories(ncxml
  PUBLIC
    ./include
)
endif(HAVE_LIBXML2)

if (NETCDF_ENABLE_DLL)
  target_compile_definitions(ncxml PRIVATE DLL_NETCDF DLL_EXPORT)
endif()

add_extra_dist(CMakeLists.txt license.txt)
