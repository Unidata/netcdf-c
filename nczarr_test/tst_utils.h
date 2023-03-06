#ifndef TST_UTILS_H
#define TST_UTILS_H

#include "netcdf.h"

#define ERR(e) report(e,__LINE__)

typedef enum Op {None, Read, Write, Wholechunk, Odom} Op;

/* Bit mask of defined options; powers of 2*/
#define HAS_DIMLENS (1<<0)
#define HAS_CHUNKS (1<<1)
#define HAS_STRIDE (1<<2)
#define HAS_START (1<<3)
#define HAS_STOP (1<<4)
#define HAS_COUNT (1<<5)
#define HAS_MAX (1<<6)

/* Options */

typedef struct Options {
    unsigned debug;
    unsigned wdebug;
    int optimize;
    int wholechunk;
    Op op;
    int mode;
    int formatx;
    int rank;
    char file[1024];
    unsigned flags; 
    size_t dimlens[NC_MAX_VAR_DIMS];
    size_t chunks[NC_MAX_VAR_DIMS];
    size_t stride[NC_MAX_VAR_DIMS];
    size_t start[NC_MAX_VAR_DIMS];
    size_t stop[NC_MAX_VAR_DIMS];
    size_t count[NC_MAX_VAR_DIMS];
    size_t max[NC_MAX_VAR_DIMS];
} Options;

typedef struct Metadata {
    int ncid;
    int varid;
    int dimids[NC_MAX_VAR_DIMS];
    int fill;
} Metadata;

typedef struct Odometer {
  size_t rank; /*rank */
  size_t start[NC_MAX_VAR_DIMS];
  size_t stop[NC_MAX_VAR_DIMS];
  size_t stride[NC_MAX_VAR_DIMS];
  size_t max[NC_MAX_VAR_DIMS]; /* max size of ith index */
  size_t count[NC_MAX_VAR_DIMS];
  size_t index[NC_MAX_VAR_DIMS]; /* current value of the odometer*/
} Odometer;

EXTERNL Odometer* odom_new(size_t rank, const size_t* start, const size_t* stop, const size_t* stride, const size_t* max);
EXTERNL void odom_free(Odometer* odom);
EXTERNL int odom_more(Odometer* odom);
EXTERNL int odom_next(Odometer* odom);
EXTERNL size_t* odom_indices(Odometer* odom);
EXTERNL size_t odom_offset(Odometer* odom);
EXTERNL const char* odom_print1(Odometer* odom, int isshort);
EXTERNL const char* odom_print(Odometer* odom);
EXTERNL const char* odom_printshort(Odometer* odom);

EXTERNL int parsevector(const char* s0, size_t* vec);
EXTERNL const char* filenamefor(const char* f0);
EXTERNL const char* printvector(int rank, const size_t* vec);
EXTERNL const char* printvector64(int rank, const size64_t* vec);

EXTERNL int getoptions(int* argcp, char*** argvp);
EXTERNL int getmetadata(int create);
EXTERNL void cleanup(void);

EXTERNL int nc__testurl(const char*,char**);

EXTERNL const char* ncz_gets3testurl(void);

static void
report(int err, int lineno)
{
    fprintf(stderr,"Error: %d: %s\n", lineno, nc_strerror(err));
    exit(1);
}

EXTERNL Options* options;
EXTERNL Metadata* meta;
EXTERNL NClist* capture;

#endif /*TST_UTILS_H*/
