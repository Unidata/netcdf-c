SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

# For some reason, these appear to need to be set in each folder.
IF(ENABLE_COVERAGE_TESTS)
        SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -W -Wshadow -Wunused-variable -Wun    used-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecate    d -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
        SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
        SET(CMAKE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
ENDIF()

SET(nctest_SRC add.c atttests.c cdftests.c dimtests.c driver.c emalloc.c error.c misctest.c rec.c slabs.c val.c vardef.c varget.c vargetg.c varput.c varputg.c vartests.c vputget.c vputgetg.c)

FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.nc)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

ADD_EXECUTABLE(nctest ${nctest_SRC})
ADD_EXECUTABLE(tst_rename tst_rename.c)

TARGET_LINK_LIBRARIES(nctest netcdf)
TARGET_LINK_LIBRARIES(tst_rename netcdf)

INSTALL(TARGETS nctest DESTINATION bin)
INSTALL(TARGETS tst_rename DESTINATION bin)
## Tests
#ADD_TEST(run_tests.sh ${CMAKE_SOURCE_PATH}/run_tests.sh)

ADD_TEST(nctest ${EXECUTABLE_OUTPUT_PATH}/nctest)
ADD_TEST(tst_rename ${EXECUTABLE_OUTPUT_PATH}/tst_rename)
