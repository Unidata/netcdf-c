#ifndef TST_UTILS_H
#define TST_UTILS_H

#include "netcdf.h"

typedef enum Op {None=0, Create=1, Read=2, Write=3, Wholechunk=4, Odom=5, Extend=6} Op;

/* Bit mask of defined options; powers of 2*/
#define HAS_DIMLENS (1<<0)
#define HAS_CHUNKS (1<<1)
#define HAS_STRIDE (1<<2)
#define HAS_START (1<<3)
#define HAS_STOP (1<<4)
#define HAS_EDGES (1<<5)
#define HAS_MAX (1<<6)
#define HAS_DATA (1<<7)

#define MAX_DATA 4096

/* Options */

typedef struct Options {
    unsigned debug;
    int wdebug;
    int optimize;
    int wholechunk;
    Op create;
    Op op;
    int mode;
    int formatx;
    int rank;
    char file[1024];
    unsigned flags; 
    size_t dimlens[NC_MAX_VAR_DIMS];
    size_t chunks[NC_MAX_VAR_DIMS];
    size_t start[NC_MAX_VAR_DIMS];
    size_t edges[NC_MAX_VAR_DIMS];
    size_t stride[NC_MAX_VAR_DIMS];
    size_t stop[NC_MAX_VAR_DIMS];
    size_t max[NC_MAX_VAR_DIMS];
    int data;
} Options;

typedef struct Metadata {
    int ncid;
    int varid;
    int dimids[NC_MAX_VAR_DIMS];
    int fill;
} Metadata;

typedef struct Odometer {
  int rank; /*rank */
  size_t start[NC_MAX_VAR_DIMS];
  size_t edges[NC_MAX_VAR_DIMS];
  size_t stride[NC_MAX_VAR_DIMS];
  size_t stop[NC_MAX_VAR_DIMS];
  size_t max[NC_MAX_VAR_DIMS]; /* max size of ith index */
  size_t index[NC_MAX_VAR_DIMS]; /* current value of the odometer*/
} Odometer;

extern void usage(int);

EXTERNL Odometer* odom_new(int rank, const size_t* start, const size_t* stop, const size_t* stride, const size_t* max);
EXTERNL void odom_free(Odometer* odom);
EXTERNL int odom_more(Odometer* odom);
EXTERNL int odom_next(Odometer* odom);
EXTERNL size_t* odom_indices(Odometer* odom);
EXTERNL size_t odom_offset(Odometer* odom);
EXTERNL const char* odom_print1(Odometer* odom, int isshort);
EXTERNL const char* odom_print(Odometer* odom);
EXTERNL const char* odom_printshort(Odometer* odom);

EXTERNL int parsevector(const char* s0, size_t* vec);
EXTERNL int parsedata(const char* s0, int* data);
EXTERNL const char* filenamefor(const char* f0);
EXTERNL const char* printvector(int rank, const size_t* vec);
EXTERNL const char* printvector64(int rank, const size64_t* vec);

EXTERNL int getoptions(int* argcp, char*** argvp);
EXTERNL int verifyoptions(Options*);
EXTERNL int getmetadata(int create);
EXTERNL void cleanup(void);

EXTERNL nc_type gettype(const char* name);
EXTERNL size_t gettypesize(nc_type t);
EXTERNL const char* gettypename(nc_type t);

EXTERNL const char* ncz_gets3testurl(void);

EXTERNL Options* options;
EXTERNL Metadata* meta;

EXTERNL void ncz_report(int err, int line);

#define ERR(e) ncz_report(e,__LINE__)

#endif /*TST_UTILS_H*/
