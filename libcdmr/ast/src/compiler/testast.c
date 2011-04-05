#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
 
#include <ast_runtime.h>
#include <ast_byteio.h>

/* Consistency */
typedef int32_t sint32_t;
typedef int64_t sint64_t;
typedef uint32_t fixed32_t;
typedef uint64_t fixed64_t;
typedef int32_t sfixed32_t;
typedef int64_t sfixed64_t;
typedef float float_t;
typedef double double_t;


static char* n_int32 = "int32";
static char* n_int64 = "int64";
static char* n_uint32 = "uint32";
static char* n_uint64 = "uint64";
static char* n_sint32 = "sint32";
static char* n_sint64 = "sint64";
static char* n_fixed32 = "fixed32";
static char* n_fixed64 = "fixed64";
static char* n_sfixed32 = "sfixed32";
static char* n_sfixed64 = "sfixed64";
static char* n_float = "float";
static char* n_double = "double";
static char* n_string = "string";
static char* n_bytes = "bytes";

#include "Primtests.h"
#include "Bytetests.h"
#include "Enumtests.h"
#include "Msgtests.h"

static char* testname = NULL;

#include "testprim1.c"
#include "testprim2.c"
#include "testprim3.c"
#include "testprim4.c"
#include "testenum1.c"
#include "testmsg1.c"

/* getopt flags */
static int ignoreerrors = 0;

int
main(int argn, char** argv)
{
    int status = AST_NOERR;
    int c;
    opterr = 0;
 
    while ((c = getopt (argn, argv, "i")) != -1)
    switch (c) {
    case 'i':
        ignoreerrors = 1;
	break;
    case '?':
        if (optopt == 'c')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
            fprintf (stderr,
                     "Unknown option character `\\x%x'.\n",
                     optopt);
          return 1;
    default:
        abort ();
    }

    status = testprim1();
    status = testprim2();
    status = testprim3();
    status = testprim4();
    status = testmsg1();
    status = testenum1();
    exit(0);
}
