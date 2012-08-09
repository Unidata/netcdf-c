SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")

# Process these files with m4.

SET(oc_SOURCES occurlfunctions.c dapparse.c daplex.c ocuri.c ochttp.c ocbytes.c oc.c occontent.c ocdebug.c ocdrno.c ocdump.c ocinternal.c oclist.c oclog.c ocnode.c ocutil.c occlientparams.c ocrc.c ocread.c xxdr.c daptab.c)

add_library(oc OBJECT ${oc_SOURCES}) 


