SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

remove_definitions(-DDLL_EXPORT)

SET(ncdump_FILES ncdump.c utils.c vardata.c dumplib.c indent.c nctime0.c )
SET(nccopy_FILES nccopy.c utils.c nciter.c chunkspec.c utils.c dimmap.c)

# Base tests
SET(ncDumpChecks rewrite-scalar tst_utf8)

SET(ncDumpTests "")

IF(USE_NETCDF4)
	SET(ncDumpTests ${ncDumpTests} tst_create_files tst_group_data tst_enum_data tst_opaque_data tst_string_data tst_vlen_data tst_comp tst_comp2 tst_nans tst_special_atts tst_h_rdc0 tst_unicode tst_fillbug tst_compress tst_chunking)
ENDIF()

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl ${CMAKE_CURRENT_SOURCE_DIR}/*.sh)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

ADD_EXECUTABLE(ncdump ${ncdump_FILES})
ADD_EXECUTABLE(nccopy ${nccopy_FILES})

TARGET_LINK_LIBRARIES(ncdump netcdf)
TARGET_LINK_LIBRARIES(nccopy netcdf)

IF(ENABLE_TESTS)
	FOREACH(CTEST ${ncDumpChecks})
		
		ADD_EXECUTABLE(${CTEST}_ncdump ${CTEST}.c)
		TARGET_LINK_LIBRARIES(${CTEST}_ncdump netcdf)
	ENDFOREACH()
	
	FOREACH(CTEST ${ncDumpTests})
  		ADD_EXECUTABLE(${CTEST}_ncdump ${CTEST}.c)
 		TARGET_LINK_LIBRARIES(${CTEST}_ncdump netcdf)
  		ADD_TEST(${CTEST} ${EXECUTABLE_OUTPUT_PATH}/${CTEST}_ncdump)
	ENDFOREACH()
	
	# Now, add shell script tests.
	ADD_TEST(run_ncdump_tests ${CMAKE_CURRENT_BINARY_DIR}/run_tests.sh ${CMAKE_CFG_INTDIR}/run_ncdump_tests)
ENDIF()

INSTALL(TARGETS ncdump DESTINATION bin)
INSTALL(TARGETS nccopy DESTINATION bin)
