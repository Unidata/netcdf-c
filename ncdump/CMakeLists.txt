SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

IF(BUILD_SHARED_LIBS AND WIN32)
	remove_definitions(-DDLL_EXPORT)
	remove_definitions(-DDLL_NETCDF)
ENDIF()

FILE(GLOB COPY_FILES ${CMAKE_BINARY_DIR}/ncgen/*.nc ${CMAKE_BINARY_DIR}/nc_test4/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.ncml ${CMAKE_CURRENT_SOURCE_DIR}/*.nc ${CMAKE_CURRENT_SOURCE_DIR}/*.cdl ${CMAKE_CURRENT_SOURCE_DIR}/*.sh ${CMAKE_CURRENT_SOURCE_DIR}/cdl4 ${CMAKE_CURRENT_SOURCE_DIR}/expected4 )
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ FILE_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE)

#IF(EXTRA_TESTS)
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
#ENDIF()

SET(ncdump_FILES ncdump.c vardata.c dumplib.c indent.c nctime0.c utils.c nciter.c)
SET(nccopy_FILES nccopy.c nciter.c chunkspec.c utils.c dimmap.c)

IF(USE_X_GETOPT)
	SET(ncdump_FILES ${ncdump_FILES} XGetopt.c)
	SET(nccopy_FILES ${nccopy_FILES} XGetopt.c)
ENDIF()




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
	#MACRO(add_bin_test F)
	#	ADD_EXECUTABLE(ncdump_${F} ${F}.c)
	#	TARGET_LINK_LIBRARIES(ncdump_${F} netcdf)
	#	ADD_TEST(ncdump_${F} ${EXECUTABLE_OUTPUT_PATH}/ncdump_${F})
	
	#ENDMACRO()
	
	# Shell script Macro
	#MACRO(add_sh_test F)
	#	IF(NOT MSVC)
	#		ADD_TEST(ncdump_${F} sh ${CMAKE_CURRENT_BINARY_DIR}/${F}.sh)
	#	ENDIF()
	#ENDMACRO()
	
	## Start adding tests in the appropriate order
	add_sh_test(ncdump run_tests)
	add_sh_test(ncdump tst_64bit)
	add_bin_test(ncdump ctest)
	add_bin_test(ncdump ctest64)
	add_sh_test(ncdump tst_output)
	add_sh_test(ncdump tst_lengths)
	add_sh_test(ncdump tst_calendars)
	add_bin_test(ncdump tst_utf8)
	add_sh_test(ncdump run_utf8_tests)
	add_sh_test(ncdump tst_nccopy3)
	add_sh_test(ncdump tst_charfill)
	add_sh_test(ncdump tst_iter)

	IF(EXTRA_TESTS)
		add_sh_test(ncdump run_back_comp_tests)
	ENDIF()
	
	IF(USE_NETCDF4)
		add_bin_test(ncdump tst_create_files)
		add_bin_test(ncdump tst_group_data)
		add_bin_test(ncdump tst_enum_data)
		add_bin_test(ncdump tst_opaque_data)
		add_bin_test(ncdump tst_string_data)
		add_bin_test(ncdump tst_vlen_data)
		add_bin_test(ncdump tst_comp)
		add_bin_test(ncdump tst_comp2)
		add_bin_test(ncdump tst_nans)
		add_bin_test(ncdump tst_special_atts)
		#add_sh_test(tst_netcdf4)
		#add_bin_test(tst_h_rdc0)
		add_bin_test(ncdump tst_unicode)
		#add_bin_test(tst_fillbug)
		#add_sh_test(tst_fillbug)
		IF(NOT WIN32)	
			add_sh_test(ncdump tst_netcdf4_4)
	ENDIF()	
		add_bin_test(ncdump tst_compress)
		#add_sh_test(tst_nccopy4)
		add_sh_test(ncdump tst_grp_spec)
	ENDIF()
	
	#add_sh_test(tst_ncgen4_classic)
	IF(USE_NETCDF4)
		#add_sh_test(tst_ncgen4)
	ENDIF()


ENDIF()

INSTALL(TARGETS ncdump DESTINATION bin)
INSTALL(TARGETS nccopy DESTINATION bin)

SET(CLEANFILES test0.nc test1.cdl test1.nc test2.cdl ctest1.cdl ctest.c ctest64.c ctest0.nc  ctest0_64.nc c1.cdl ctest1_64.cdl c0.nc small.nc small2.nc c0tmp.nc c1.ncml utf8.cdl utf8_64.cdl utf8.nc utf8_64.nc tmp.cdl tst_vlen_data.nc tst_utf8.nc tst_special_atts.nc tst_unicode.nc tst_solar_2.nc tst_string_data.nc tst_calendars.nc tst_nans.nc tst_opaque_data.nc tst_solar_cmp.nc tst_enum_data.nc tst_solar_1.nc tst_mslp_64.nc tst_mslp.nc tst_comp2.nc tst_ncml.nc tst_fillbug.nc tst_group_data.nc tst_small.nc tst_comp.nc tst_unicode.cdl tst_group_data.cdl tst_compounds2.cdl tst_comp.cdl tst_enum_data.cdl tst_small.cdl tst_times.cdl tst_solar_2.cdl tst_string_data.cdl tst_fillbug.cdl tst_opaque_data.cdl tst_compounds4.cdl tst_utf8.cdl tst_compounds3.cdl tst_special_atts.cdl tst_nans.cdl tst_format_att_64.cdl tst_vlen_data.cdl tst_solar_1.cdl tst_format_att.cdl tst_inflated.nc tmp_subset.cdl tst_inflated4.nc tst_deflated.nc tst_chunking.nc tmp*.nc tst_charfill.nc tmp_tst_charfill.cdl iter.* tst_nc_test_netcdf4_4_0.cdl)

SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CLEANFILES}")

