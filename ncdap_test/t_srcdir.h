#define XSTRINGIFY(s) #s
#define STRINGIFY(s) XSTRINGIFY(s)

static const char*
gettopsrcdir(void)
{
    const char* topsrcdir;
    topsrcdir = STRINGIFY(TOPSRCDIR);
    if(topsrcdir == NULL) {
        fprintf(stderr,"*** FAIL: $abs_top_srcdir not defined\n");
        exit(1);
    }    
    fprintf(stderr,"topsrcdir=%s\n",topsrcdir);
    return topsrcdir;
}
