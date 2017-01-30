#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XSTRINGIFY(s) #s
#define STRINGIFY(s) XSTRINGIFY(s)

#ifdef _MSC_VER
static const char* SEP = "\\";
#else
static const char* SEP = "/";
#endif

int
main(int argc, char** argv)
{
#ifdef TOPSRCDIR
    const char* topsrcdir = STRINGIFY(TOPSRCDIR);
    printf("%s",topsrcdir);
#endif
    return 0;
}
