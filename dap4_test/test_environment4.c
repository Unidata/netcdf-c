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
    char arg[8192];
    char* p;
    const char* result = "";

    if(argc < 2) goto done; /* return nothing */
    strncpy(arg,argv[1],sizeof(arg));
    for(p=arg;*p;p++) { /* convert to lower case */
	int c = *p;
        if(c >= 'A' && c <= 'Z') c = ((c - 'A') + 'a');
	*p = (char)c;
    }    
    do {
#ifdef TOPSRCDIR
    if(strcmp(arg,"topsrcdir") == 0) {
        result = STRINGIFY(TOPSRCDIR);
	break;
    }
#endif
#ifdef TOPBINDIR
    if(strcmp(arg,"topbindir") == 0) {
        result = STRINGIFY(TOPBINDIR);
	break;
    }
#endif
    } while(0);
done:
    printf("%s",result);
    return 0;
}
