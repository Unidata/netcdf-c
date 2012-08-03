SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

SET(nctest_SRC add.c atttests.c cdftests.c dimtests.c driver.c emalloc.c error.c misctest.c rec.c slabs.c val.c vardef.c varget.c vargetg.c varput.c varputg.c vartests.c vputget.c vputgetg.c)

ADD_EXECUTABLE(nctest ${nctest_SRC})

TARGET_LINK_LIBRARIES(nctest netcdf)

INSTALL(TARGETS nctest DESTINATION bin)

## Tests
#ADD_TEST(run_tests.sh ${EXECUTABLE_OUTPUT_PATH}/run_tests.sh)
ADD_TEST(nctest ${EXECUTABLE_OUTPUT_PATH}/nctest)
