#define XSTRINGIFY(s) #s
#define STRINGIFY(s) XSTRINGIFY(s)

static const char*
gettopsrcdir(void)
{
    const char* topsrcdir;
#ifdef TOPSRCDIR
    topsrcdir = STRINGIFY(TOPSRCDIR);
#else
    static char wd[8192];
    getwd(wd,sizeof(wd));
    strcat(wd,SEP);
    strcat(wd,"..");
    topsrcdir = wd;
#endif
    if(topsrcdir == NULL) {
        fprintf(stderr,"*** FAIL: $abs_top_srcdir not defined\n");
        exit(1);
    }    
    fprintf(stderr,"topsrcdir=%s\n",topsrcdir);
    return topsrcdir;
}
