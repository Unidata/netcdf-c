/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifndef BM_UTILS_H
#define BM_UTILS_H

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "zincludes.h"

#undef DEBUG

/* Define the getopt tags */
#define OPT_UNKNOWN 0
#define OPT_TREEDEPTH 1
#define OPT_NGROUPS 2
#define OPT_NGROUPATTRS 3
#define OPT_NDIMS 4
#define OPT_NTYPES 5
#define OPT_NVARS 6
#define OPT_VARRANK 7
#define OPT_NVARATTRS 8
#define OPT_FORMAT 9
#define OPT_PATH 10
#define OPT_FILE 11
#define OPT_X 12
#define OPT_DEBUG 13
#define OPT_DIMS 14
#define OPT_CHUNKS 15
#define OPT_CACHEFACTOR 16
#define OPT_CACHESIZE 17
#define OPT_DEFLATELEVEL 18
#define OPT_WDEBUG 19
#define OPT_URL 20
#define OPT_READONLY 21
#define OPT_WRITEONLY 22

#define X_OPT_MATCH 1

extern struct BMOptions {
    int format;
    char* filename;
    char* url;
    char* path; /* as sent to e.g. nc_create() */
    char* pathtemplate;
    int xvalue;
    NCZM_IMPL impl;
    int debug;
    int wdebug;
    int readonly;
    int writeonly;
    struct BMMeta {
	int treedepth;
	int ngroups;
	int ngroupattrs;
	int ndims;
	int ntypes;
	int nvars;
	int varrank;
	int nvarattrs;
	size_t cachefactor;
	size_t cachesize;
	struct IntList {
	    int count;
   	    size_t* list;
	} dims;
	struct IntList chunks;
	int deflatelevel;
    } meta;
    struct X {
	int sync;
    } x;
} bmoptions;

#define NCCHECK(expr) nccheck((expr),__LINE__)

EXTERNL int bm_buildpath(struct BMOptions*);
EXTERNL int bm_getoptions(int* argcp, char*** argvp, struct BMOptions*);
EXTERNL void bm_clearoptions(struct BMOptions*);
EXTERNL const char* formatname(const struct BMOptions*);
EXTERNL void bm_reportoptions(struct BMOptions* o);
EXTERNL void bm_reportmetaoptions(struct BMMeta* o);
EXTERNL const char* bm_printvector(int rank, const size_t* vec);
EXTERNL const char* bm_printvectorp(int rank, const ptrdiff_t* vec);
EXTERNL const char* bm_varasprint(int rank, const size_t* start, const size_t* edges, const ptrdiff_t* stride);
EXTERNL int nc4_buildpath(struct BMOptions* o, char** pathp);
EXTERNL void reportoptions(struct BMOptions* o);
EXTERNL void reportmetaoptions(struct BMMeta* o);
EXTERNL void clearoptions(struct BMOptions*);

#include "ut_test.h"

#endif /*BM_UTILS_H*/
