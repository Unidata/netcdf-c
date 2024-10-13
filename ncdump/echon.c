/*
 * Copyright 2018, University Corporation for Atmospheric Research
 * See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && ! defined(__MINGW32__)
#include "XGetopt.h"
#endif

extern int NCstdbinary(void);

int
main(int argc, char** argv)
{
    int i;
    int len;
    char* input = NULL;
    char* output = NULL;
    int noeol = 1; /* Default to -n vs -N */
    int escape = 1; /* Ditto */
    char* p = NULL;
    char* q = NULL;
    char c;

    NCstdbinary(); /* avoid \r\n for windows */

    opterr = 1;
    while ((c = getopt(argc, argv,"enxNX")) != EOF) {
	switch(c) {
	case 'e': goto nomoreargs;
	case 'x': escape = 1; break;
	case 'n': noeol = 1; break;
	case 'X': escape = 0; break;
	case 'N': noeol = 0; break;
	case '?': default: break; /* ignore */
	}
    }    

nomoreargs:
    /* leave non-flag args */
    argc -= optind;
    argv += optind;

    /* Compute the max possible length of output */
    for(len=0,i=0;i<argc;i++) len += (int)strlen(argv[i]);
    len += (argc - 1); /* inter-arg spacing */
    input = (char*)calloc(1,(size_t)len+1+1); /* +1 for nul term +1 for '\n' */
    assert(input != NULL);
    input[0] = '\0';
    /* Concat all the arguments with  ' ' between them */   
    for(i=0;i<argc;i++) {
	if(i > 0) strcat(input," ");
	strcat(input,argv[i]);
    }
    output = (char*)calloc(1,(size_t)len+1+1); /* +1 for nul term +1 for '\n' */
    assert(output != NULL);
    output[0] = '\0';
    /* Optionally de-escape the input */
    for(p=input,q=output;*p;) {
        if(escape) {
	    switch (*p) {
	    default: *q++ = *p; break;
	    case '\\':
		p++;
		switch(*p) {
		case 'a': *q++ = '\a'; break;
		case 'b': *q++ = '\b'; break;
		case 'f': *q++ = '\f'; break;
		case 'r': *q++ = '\r'; break;
		case 'n': *q++ = '\n'; break;
		case 't': *q++ = '\t'; break;
		case 'v': *q++ = '\v'; break;
		default: *q++ = *p; break;
		}		 
		break;
	    }
	    p++;
        } else
	    *q++ = *p++;
    }
    if(!noeol) *p++ = '\n';
    *p = '\0';
    fputs(output,stdout);
    fflush(stdout);
    free(input);
    free(output);
    return 0;
}
