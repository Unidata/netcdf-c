/*
Copyright (c) 1998-2017 University Corporation for Atmospheric Research/Unidata
See LICENSE.txt for license information.
*/

#include "includes.h"

extern char* ncclassname(nc_class);

#ifdef DEBUG
int debug = 1;
#else
int debug = 0;
#endif

void fdebug(const char *fmt, ...)
{
    va_list argv;
    if(debug == 0) return;
    va_start(argv,fmt);
    (void)vfprintf(stderr,fmt,argv) ;
}

/**************************************************/

/* Support debugging of memory*/

void
chkfree(void* memory)
{
    if(memory == NULL) {
	panic("free: null memory");
    }
    free(memory);
}

void*
chkcalloc(size_t size)
{
    void* memory = calloc(size,1); /* use calloc to zero memory*/
    if(memory == NULL) {
	panic("malloc:out of memory");
    }
    return memory;
}

void*
chkrealloc(void* ptr, size_t size)
{
    void* memory = realloc(ptr,size);
    if(memory == NULL) {
	panic("realloc:out of memory");
    }
    return memory;
}

char*
chkstrdup(const char* s)
{
    char* dup;
    if(s == NULL) {
	panic("strdup: null argument");
    }
    dup = strdup(s);
    if(dup == NULL) {
	panic("strdup: out of memory");
    }
    return dup;
}

int
panic(const char* fmt, ...)
{
    va_list args;
    if(fmt != NULL) {
      va_start(args, fmt);
      vfprintf(stderr, fmt, args);
      fprintf(stderr, "\n" );
      va_end( args );
    } else {
      fprintf(stderr, "panic" );
    }
    fprintf(stderr, "\n" );
    fflush(stderr);
    abort();
    return 0;
}
