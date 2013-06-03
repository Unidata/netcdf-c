SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

SET(cdmr_SOURCES nccrcvt.c nccr.c nccrdispatch.c crmeta.c nccrproto.c nccrgetvarx.c crdebug.c crutil.c nccrdump.c ncStreamx.c curlwrap.c ast_byteio.c ast_curl.c ast_internal.c ast_runtime.c ast_util.c ast_debug.c cceparse.c cceconstraints.c)

add_library(cdmr OBJECT ${cdmr_SOURCES}) 

FILE(GLOB CUR_EXTRA_DIST RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
SET(CUR_EXTRA_DIST ${CUR_EXTRA_DIST} CMakeLists.txt Makefile.am cce.y ast.jar ncStream.proto extends.sed)
ADD_EXTRA_DIST("${CUR_EXTRA_DIST}")


