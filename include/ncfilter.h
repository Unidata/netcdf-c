/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */
/**
 * @file
 * @internal This header file contains macros, types and prototypes
 * used to build and manipulate filters. It contains definitions
 * for multiple file formats.
 *
 * @author Dennis Heimbigner
 */

#ifndef NCFILTER_H
#define NCFILTER_H


/**************************************************/
/* Internal filter related structures */

/* Internal filter actions */
#define NCFILTER_DEF		1
#define NCFILTER_REMOVE  	2
#define NCFILTER_FILTERIDS      3
#define NCFILTER_INFO		4
#define NCFILTER_CLIENT_REG	10
#define NCFILTER_CLIENT_UNREG	11
#define NCFILTER_CLIENT_INQ	12

typedef enum NC_FILTER_UNION {
    NC_FILTER_UNION_SPEC=((int)1),
    NC_FILTER_UNION_IDS=((int)2),
    NC_FILTER_UNION_CLIENT=((int)3),
} NC_FILTER_UNION;

/**************************************************/
/* Provide structs to pass args to filter_actions function using strings */
/* Originally, this used the HDF5 model where ids and params were unsigned ints.
   To extend to zarr, we have converted so that ids and params are strings (char*).
   Additionally we now allow a string id to be either an integer (as before but as a string)
   or a name defined in the known_filters table in dfilter.c */

typedef struct NC_FILTERX_SPEC {
    int active;            /**< true iff underlying library was told to activate filter */
    char* filterid;	   /**< ID for arbitrary filter. */
    size_t nparams;
    char** params;  	   /**< Params for arbitrary filter. */
} NC_FILTERX_SPEC;

typedef struct NC_FILTERX_IDS {
    size_t nfilters;   /**< number of filters */
    char** filterids;  /**< Filter ids. */
} NC_FILTERX_IDS;

typedef struct NC_FILTERX_CLIENT {
    char* id;
    /* The filter info for x */
    /* Avoid needing hdf.h by using void* */
    void* info;
} NC_FILTERX_CLIENT;

typedef struct NC_FILTERX_OBJ {
    NC_FILTER_UNION usort; /* discriminate union */
    union {
        NC_FILTERX_SPEC spec;
        NC_FILTERX_IDS ids;
        NC_FILTERX_CLIENT client;
    } u;
} NC_FILTERX_OBJ;

struct NC_VAR_INFO;

EXTERNL int NC4_filterx_lookup(struct NC_VAR_INFO* var, const char* id, NC_FILTERX_SPEC** specp);
EXTERNL int NC4_filterx_add(struct NC_VAR_INFO* var, int active, const char* id, int nparams, const char** params);
EXTERNL int NC4_filterx_remove(struct NC_VAR_INFO* var, const char* xid);
EXTERNL int NC4_filterx_freelist(struct NC_VAR_INFO* var);
EXTERNL int NC4_filterx_free(NC_FILTERX_SPEC*);
EXTERNL int NC_cvtX2I_id(const char* xid, unsigned int* id);
EXTERNL int NC_cvtI2X_id(unsigned int id, char** xidp, int usename);
EXTERNL void NC_filterx_freestringvec(size_t n, char** vec);
EXTERNL int NC_filterx_copy(size_t n, const char** vec, char*** copyp);
/* As a rule, user provides space for output */
EXTERNL int NC_cvtI2X_idlist(int n, const unsigned int* ids, char** xid);
EXTERNL int NC_cvtI2X_params(int n, const unsigned int* iparams, char** xparams);
EXTERNL int NC_cvtX2I_params(size_t nparams, const char** xparamslist, unsigned int* params);
EXTERNL int NC_cvtX2I_idlist(size_t n, const char** xidlist, unsigned int* ids);

#endif /*NCFILTER_H*/
