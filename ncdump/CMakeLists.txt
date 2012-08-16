SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

remove_definitions(-DDLL_EXPORT)

FILE(GLOB COPY_FILES ${CMAKE_BINARY_DIR}/ncgen/*.nc ${CMAKE_BINARY_DIR}/nc_test4/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.ncml ${CMAKE_CURRENT_SOURCE_DIR}/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl ${CMAKE_CURRENT_SOURCE_DIR}/*.sh ${CMAKE_CURRENT_SOURCE_DIR}/cdl4 ${CMAKE_CURRENT_SOURCE_DIR}/expected4 )
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

IF(EXTRA_TESTS)
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ctest.c
	COMMAND cmake -E copy "${CMAKE_CURRENT_SOURCE_DIR}/ref_ctest.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/ctest.c"
	)
ADD_CUSTOM_COMMAND(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ctest64.c
	COMMAND cmake -E copy "${CMAKE_CURRENT_SOURCE_DIR}/ref_ctest64.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/ctest64.c"
	)
ENDIF()

SET(ncdump_FILES ncdump.c utils.c vardata.c dumplib.c indent.c nctime0.c )
SET(nccopy_FILES nccopy.c utils.c nciter.c chunkspec.c utils.c dimmap.c)



ADD_EXECUTABLE(ncdump ${ncdump_FILES})
ADD_EXECUTABLE(nccopy ${nccopy_FILES})

TARGET_LINK_LIBRARIES(ncdump netcdf ${ALL_TLL_LIBS})
TARGET_LINK_LIBRARIES(nccopy netcdf ${ALL_TLL_LIBS})

IF(ENABLE_TESTS)
	ADD_EXECUTABLE(rewrite-scalar rewrite-scalar.c)
	TARGET_LINK_LIBRARIES(rewrite-scalar netcdf)
	# Base tests
	# The tests are set up as a combination of shell scripts and executables that
	# must be run in a particular order. It is painful but will use macros to help
	# keep it from being too bad.
	# Binary Test Macro
	MACRO(add_bin_test F)
		ADD_EXECUTABLE(ncdump_${F} ${F}.c)
		TARGET_LINK_LIBRARIES(ncdump_${F} netcdf)
		ADD_TEST(ncdump_${F} ${EXECUTABLE_OUTPUT_PATH}/ncdump_${F})
	
	ENDMACRO()
	
	# Shell script Macro
	MACRO(add_sh_test F)
		ADD_TEST(ncdump_${F} sh ${CMAKE_CURRENT_BINARY_DIR}/${F}.sh)
	ENDMACRO()
	
	## Start adding tests in the appropriate order
	add_sh_test(run_tests)
	add_sh_test(tst_64bit)
	add_bin_test(ctest)
	add_bin_test(ctest64)
	add_sh_test(tst_output)
	add_sh_test(tst_lengths)
	add_sh_test(tst_calendars)
	add_bin_test(tst_utf8)
	add_sh_test(run_utf8_tests)
	add_sh_test(tst_nccopy3)
	add_sh_test(tst_charfill)
	add_sh_test(tst_iter)

	IF(EXTRA_TESTS)
		add_sh_test(run_back_comp_tests)
	ENDIF()
	
	IF(USE_NETCDF4)
		add_bin_test(tst_create_files)
		add_bin_test(tst_group_data)
		add_bin_test(tst_enum_data)
		add_bin_test(tst_opaque_data)
		add_bin_test(tst_string_data)
		add_bin_test(tst_vlen_data)
		add_bin_test(tst_comp)
		add_bin_test(tst_comp2)
		add_bin_test(tst_nans)
		add_bin_test(tst_special_atts)
		#add_sh_test(tst_netcdf4)
		#add_bin_test(tst_h_rdc0)
		add_bin_test(tst_unicode)
		#add_bin_test(tst_fillbug)
		#add_sh_test(tst_fillbug)
		add_sh_test(tst_netcdf4_4)
		add_bin_test(tst_compress)
		#add_sh_test(tst_nccopy4)
		add_sh_test(tst_grp_spec)
	ENDIF()
	
	#add_sh_test(tst_ncgen4_classic)
	IF(USE_NETCDF4)
		#add_sh_test(tst_ncgen4)
	ENDIF()


ENDIF()

INSTALL(TARGETS ncdump DESTINATION bin)
INSTALL(TARGETS nccopy DESTINATION bin)
