IF(HAVE_LIBXML2)
SET(libncxml_SOURCES ncxml_xml2.c)
ELSE()
SET(libncxml_SOURCES ncxml_tinyxml2.cpp tinyxml2.cpp tinyxml2.h)
ENDIF()

add_library(ncxml OBJECT ${libncxml_SOURCES})
TARGET_INCLUDE_DIRECTORIES(ncxml PUBLIC ./include)

#IF(NOT MSVC)
#target_compile_features(ncxml PUBLIC cxx_std_11)
#ENDIF()

ADD_EXTRA_DIST(CMakeLists.txt license.txt)
