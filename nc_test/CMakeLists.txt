SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")
SET (nc_test_m4_SOURCES test_get test_put)
FOREACH (F ${nc_test_m4_SOURCES})
	GEN_m4(${F})
ENDFOREACH()


SET (nc_test_SRC nc_test.c error.c test_get.c test_put.c test_read.c test_write.c util.c)
ADD_EXECUTABLE(nc_test ${nc_test_SRC})
TARGET_LINK_LIBRARIES(nc_test netcdf)

# Some extra tests
SET(EXTRA_TSTS t_nc tst_small tst_misc tst_norm tst_names tst_nofill tst_nofill2 tst_nofill3)

IF(NOT MSVC)
	SET(EXTRA_TSTS ${EXTRA_TSTS} tst_atts3)
ENDIF()

IF(USE_NETCDF4)
  SET(EXTRA_TSTS ${EXTRA_TSTS} tst_atts)
ENDIF()

IF(LARGE_FILE_TESTS)
  SET(EXTRA_TSTS ${EXTRA_TSTS} quick_large_files tst_big_var6 tst_big_var2 tst_big_rvar tst_big_var tst_large large_files)
ENDIF()

IF(BUILD_DISKLESS)
  SET(EXTRA_TSTS ${EXTRA_TSTS} tst_diskless tst_diskless3 tst_diskless4)
  IF(USE_NETCDF4)
    SET(EXTRA_TSTS ${EXTRA_TSTS} tst_diskless2)
  ENDIF()
ENDIF()


FOREACH(CTEST ${EXTRA_TSTS})
	add_bin_test(nc_test ${CTEST})
	#ADD_EXECUTABLE(${CTEST} ${CTEST}.c)
	#TARGET_LINK_LIBRARIES(${CTEST} netcdf)
ENDFOREACH()





## Tests
ADD_TEST(nc_test ${EXECUTABLE_OUTPUT_PATH}/nc_test)

IF(BUILD_DISKLESS)
	add_sh_test(nc_test run_diskless)  	
	#ADD_TEST(run_diskless ${CMAKE_CURRENT_SOURCE_DIR}/run_diskless.sh ${CMAKE_CFG_INTDIR}/run_diskless)
ENDIF()
