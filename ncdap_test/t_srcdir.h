/*! \file

Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
2015, 2016, 2017, 2018
University Corporation for Atmospheric Research/Unidata.

See \ref copyright file for more info.

*/
#define XSTRINGIFY(s) #s
#define STRINGIFY(s) XSTRINGIFY(s)

static const char*
gettopsrcdir(void)
{
    const char* topsrcdir = NULL;
#ifdef TOPSRCDIR
    topsrcdir = STRINGIFY(TOPSRCDIR);
#else
    static char tsd[4096];
    extern char *getcwd(char *buf, size_t size);
    tsd[0] = '\0';
    getcwd(tsd,sizeof(tsd));
    if(strlen(tsd) > 0) {
        strcat(tsd,"/..");
        topsrcdir = tsd;
    }
#endif
    if(topsrcdir == NULL) {
        fprintf(stderr,"*** FAIL: $abs_top_srcdir not defined\n");
        exit(1);
    }
    fprintf(stderr,"topsrcdir=%s\n",topsrcdir);
    return topsrcdir;
}
