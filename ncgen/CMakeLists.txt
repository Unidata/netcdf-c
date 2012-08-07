SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

SET(ncgen_FILES generate.c main.c cdata.c bindata.c genchar.c cvt.c data.c debug.c escapes.c genc.c genbin.c generr.c genlib.c getfill.c odom.c offsets.c semantics.c ncgentab.c dump.c util.c bytebuffer.c list.c genf77.c f77data.c genj.c jdata.c nciter.c ConvertUTF.c  )

ADD_EXECUTABLE(ncgen ${ncgen_FILES})

TARGET_LINK_LIBRARIES(ncgen netcdf)

INSTALL(TARGETS ncgen DESTINATION bin)

IF(ENABLE_TESTS)
	FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
	FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE) 

	ADD_TEST(run_tests ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh ${CMAKE_CFG_INTDIR}/run_tests)
	IF(USE_NETCDF4)
		ADD_TEST(run_nc4_tests ${CMAKE_CURRENT_BINARY_DIR}/run_nc4_tests.sh ${CMAKE_CFG_INTDIR}/run_nc4_tests)
	ENDIF()
ENDIF()
## Tests
#ADD_TEST(run_tests.sh ${EXECUTABLE_OUTPUT_PATH}/run_tests.sh)

