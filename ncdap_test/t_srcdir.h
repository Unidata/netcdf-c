#define XSTRINGIFY(s) #s
#define STRINGIFY(s) XSTRINGIFY(s)

static const char*
gettopsrcdir(void)
{
    const char* topsrcdir;
    topsrcdir = getenv("TOPSRCDIR");
    if(topsrcdir == NULL) {
        topsrcdir = STRINGIFY(TOPSRCDIR);
        if(topsrcdir == NULL) {
            fprintf(stderr,"*** FAIL: Neither $abs_top_srcdir nor env var TOPSRCDIR defined\n");
            exit(1);
        }
    }
    fprintf(stderr,"topsrcdir=%s\n",topsrcdir);
    return topsrcdir;
}
