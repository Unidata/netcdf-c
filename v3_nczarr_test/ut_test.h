/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifndef ZTEST_H
#define ZTEST_H

#include "nclist.h"

typedef struct Dimdef {
    char* name;
    size64_t size;
    int isunlimited;
} Dimdef;

typedef struct Vardef  {
    char* name;
    nc_type typeid;
    size_t typesize;
    size_t rank;
    Dimdef* dimrefs[NC_MAX_VAR_DIMS];
    size64_t dimsizes[NC_MAX_VAR_DIMS];
    size64_t chunksizes[NC_MAX_VAR_DIMS];
} Vardef;

/* Expose functions for unit tests */
typedef struct NCZ_UT_PRINTER {
    int printsort;
    void (*printer)(struct NCZ_UT_PRINTER*);
    /* Union of all fields */
    size_t rank;
    size64_t count;
    size64_t offset;
    size64_t* indices;
    size64_t* vector;
    void** pvector;
    NCZOdometer* odom;
    void* output;
    size_t used;
} NCZ_UT_PRINTER;

/* Arguments from command line */
struct UTOptions {
    int debug;
    char** cmds;
    char* file;
    char* output;
    char* kind;
    char* profile;
    NCZChunkRange ranges[NC_MAX_VAR_DIMS];
    int nslices;
    NCZSlice slices[NC_MAX_VAR_DIMS];
    NClist* dimdefs; /*List<struct Dimdef*> */
    NClist* vardefs; /*List<struct Vardef*> */
    int* idata;
};

struct Test {
    char* cmd;
    int (*test)(void);
};

EXTERNL struct UTOptions utoptions;

#define NCCHECK(expr) nccheck((expr),__LINE__)

EXTERNL void usage(int err);
EXTERNL int ut_init(int argc, char** argv, struct UTOptions* test);
EXTERNL void ut_final(void);

EXTERNL void nccheck(int stat, int line);
EXTERNL char* makeurl(const char* file, NCZM_IMPL, struct UTOptions*);
//extern int setup(int argc, char** argv);
EXTERNL struct Test* findtest(const char* cmd, struct Test* tests);
EXTERNL int runtests(const char** cmds, struct Test* tests);

#endif /*ZTEST_H*/
