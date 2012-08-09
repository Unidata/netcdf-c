SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

# For some reason, these appear to need to be set in each folder.
IF(ENABLE_COVERAGE_TESTS)
        SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -W -Wshadow -Wunused-variable -Wun    used-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecate    d -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
        SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
        SET(CMAKE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
ENDIF()


SET(ncgen_FILES generate.c main.c cdata.c bindata.c genchar.c cvt.c data.c debug.c escapes.c genc.c genbin.c generr.c genlib.c getfill.c odom.c offsets.c semantics.c ncgentab.c dump.c util.c bytebuffer.c list.c genf77.c f77data.c genj.c jdata.c nciter.c ConvertUTF.c  )

ADD_EXECUTABLE(ncgen ${ncgen_FILES})

TARGET_LINK_LIBRARIES(ncgen netcdf)

INSTALL(TARGETS ncgen DESTINATION bin)

# Stuff to build if tests are enabled.
IF(ENABLE_TESTS)
	FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
	FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE) 

	ADD_TEST(run_ncgen_tests sh ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh)
	IF(USE_NETCDF4)
		ADD_TEST(run_ncgen_nc4_tests sh ${CMAKE_CURRENT_BINARY_DIR}/run_nc4_tests.sh)
	ENDIF()
ENDIF()

