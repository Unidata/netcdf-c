SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

SET (nc_test_SRC nc_test.c error.c test_get.c test_put.c test_read.c test_write.c util.c)

ADD_EXECUTABLE(nc_test ${nc_test_SRC})
TARGET_LINK_LIBRARIES(nc_test netcdf)

# Some extra tests
SET(EXTRA_TSTS t_nc tst_small tst_misc tst_norm tst_names tst_nofill tst_nofill2 tst_nofill3 tst_atts3)

IF(USE_NETCDF4)
  SET(EXTRA_TSTS ${EXTRA_TSTS} tst_atts)
ENDIF()

IF(BUILD_DISKLESS)
  SET(EXTRA_TSTS ${EXTRA_TSTS} tst_diskless tst_diskless3 tst_diskless4)
  IF(USE_NETCDF4)
    SET(EXTRA_TSTS ${EXTRA_TSTS} tst_diskless2)
  ENDIF()
ENDIF()


FOREACH(CTEST ${EXTRA_TSTS})
  ADD_EXECUTABLE(${CTEST} ${CTEST}.c)
  TARGET_LINK_LIBRARIES(${CTEST} netcdf)
ENDFOREACH()





## Tests
#ADD_TEST(run_tests.sh ${CMAKE_SOURCE_PATH}/run_tests.sh)

ADD_TEST(nc_test ${EXECUTABLE_OUTPUT_PATH}/nc_test)

IF(BUILD_DISKLESS)
  ADD_TEST(run_diskless ${CMAKE_CURRENT_SOURCE_DIR}/run_diskless.sh ${CMAKE_CFG_INTDIR}/run_diskless)
ENDIF()
