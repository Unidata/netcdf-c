SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")
# For some reason, these appear to need to be set in each folder.
IF(ENABLE_COVERAGE_TESTS)
        SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -W -Wshadow -Wunused-variable -Wun    used-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecate    d -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
        SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
        SET(CMAKE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
ENDIF()

SET(oc_SOURCES occurlfunctions.c dapparse.c daplex.c ocdata.c ocuri.c ochttp.c ocbytes.c oc.c ocdebug.c ocdump.c ocinternal.c occompile.c oclist.c oclog.c ocnode.c ocutil.c occlientparams.c ocrc.c ocread.c xxdr.c daptab.c)

add_library(oc2 OBJECT ${oc_SOURCES}) 



