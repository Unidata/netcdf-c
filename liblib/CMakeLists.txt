SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")
# For some reason, these appear to need to be set in each folder.
IF(ENABLE_COVERAGE_TESTS)
        SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -W -Wshadow -Wunused-variable -Wun    used-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecate    d -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
        SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
        SET(CMAKE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
ENDIF()

SET(liblib_LIBS dispatch netcdf3)

#####
# Add target objects/modules based on options.
#####

IF(USE_HDF5 OR USE_NETCDF4) 
	SET(liblib_LIBS ${liblib_LIBS} netcdf4)
ENDIF()

IF(USE_DAP)
	SET(liblib_LIBS ${liblib_LIBS} oc2 dap2)
ENDIF()

IF(BUILD_CDMREMOTE)
	SET(liblib_LIBS ${liblib_LIBS} cdmr)
ENDIF()


FOREACH(LIBS ${liblib_LIBS})
	SET(LARGS ${LARGS} $<TARGET_OBJECTS:${LIBS}>)
ENDFOREACH()

ADD_LIBRARY(netcdf stub.c ${LARGS} )

#####
# Add dependencies required for linking.
#####

TARGET_LINK_LIBRARIES(netcdf ${MMATH})

IF(USE_DAP)
	TARGET_LINK_LIBRARIES(netcdf ${CURL_LIBRARIES})
ENDIF()

IF(USE_HDF5 OR USE_NETCDF4)
	TARGET_LINK_LIBRARIES(netcdf ${HDF5_LIBRARIES} ${HDF5_HL_LIBRARY} ${SZIP_LIBRARY} ${ZLIB_LIBRARY})
ENDIF()

IF(ENABLE_PNETCDF AND PNETCDF)
	TARGET_LINK_LIBRARIES(netcdf ${PNETCDF})
ENDIF()

INSTALL(TARGETS netcdf DESTINATION lib)
#include(CPack)
