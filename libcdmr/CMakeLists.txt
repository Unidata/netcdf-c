SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")
# For some reason, these appear to need to be set in each folder.
IF(ENABLE_COVERAGE_TESTS)
        SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -W -Wshadow -Wunused-variable -Wun    used-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecate    d -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
        SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
        SET(CMAKE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
ENDIF()


SET(cdmr_SOURCES nccrcvt.c nccr.c nccrdispatch.c crmeta.c nccrproto.c nccrgetvarx.c crdebug.c crutil.c nccrdump.c ncStreamx.c curlwrap.c ast_byteio.c ast_curl.c ast_internal.c ast_runtime.c ast_util.c ast_debug.c cceparse.c cceconstraints.c)

add_library(cdmr OBJECT ${cdmr_SOURCES}) 






