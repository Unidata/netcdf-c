/*
Wei-keng Liao's (wkliao@eecs.northwestern.edu)
netcdf-3 validator program
(https://github.com/Parallel-NetCDF/PnetCDF/blob/master/src/utils/ncvalidator/ncvalidator.c)
*/

/*
Copyright (c) 2003 Northwestern University and Argonne National Laboratory
All rights reserved.

Portions of this software were developed by the Unidata Program at the
University Corporation for Atmospheric Research.

Access and use of this software shall impose the following obligations and
understandings on the user. The user is granted the right, without any
fee or
cost, to use, copy, modify, alter, enhance and distribute this
software, and
any derivative works thereof, and its supporting documentation for any
purpose
whatsoever, provided that this entire notice appears in all copies of the
software, derivative works and supporting documentation. Further,
Northwestern
University and Argonne National Laboratory request that the user credit
Northwestern University and Argonne National Laboratory in any
publications
that result from the use of this software or in any product that
includes this
software. The names Northwestern University and Argonne National
Laboratory,
however, may not be used in any advertising or publicity to endorse or
promote
any products or commercial entity unless specific written permission is
obtained from Northwestern University and Argonne National Laboratory.
The user
also understands that Northwestern University and Argonne National
Laboratory
are not obligated to provide the user with any support, consulting,
training or
assistance of any kind with regard to the use, operation and
performance of
this software nor to provide the user with any updates, revisions, new
versions
or "bug fixes."

THIS SOFTWARE IS PROVIDED BY NORTHWESTERN UNIVERSITY AND ARGONNE NATIONAL
LABORATORY "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL NORTHWESTERN
UNIVERSITY
AND ARGONNE NATIONAL LABORATORY BE LIABLE FOR ANY SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
USE,
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS
ACTION, ARISING OUT OF OR IN CONNECTION WITH THE ACCESS, USE OR
PERFORMANCE OF
THIS SOFTWARE.
*/

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  /* open() */
#include <sys/stat.h>   /* open() */
#include <fcntl.h>      /* open() */
#include <string.h>     /* strcpy(), strncpy() */
#include <inttypes.h>   /* check for Endianness, uint32_t*/
#include <assert.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>     /* read() getopt() */
#endif

#ifdef _WIN32
#include <io.h>
#define snprintf _snprintf
#include "XGetopt.h"
int opterr;
int optind;
#endif

#define X_ALIGN         4
#define X_INT_MAX       2147483647
#define X_UINT_MAX      4294967295U
#define X_INT64_MAX     9223372036854775807LL

#ifndef EXIT_FAILURE
#ifndef vms
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#else
/* In OpenVMS, success is indicated by odd values and failure by even values. */
#define EXIT_SUCCESS 1
#define EXIT_FAILURE 0
#endif
#endif

static int verbose, trace;
static int repair;
static const char nada[4] = {0, 0, 0, 0};

#ifndef MAX
#define MAX(mm,nn) (((mm) > (nn)) ? (mm) : (nn))
#endif
#ifndef MIN
#define MIN(mm,nn) (((mm) < (nn)) ? (mm) : (nn))
#endif

/* useful for aligning memory */
#define _RNDUP(x, unit) ((((x) + (unit) - 1) / (unit)) * (unit))

#define ERR_ADDR (((size_t) gbp->pos - (size_t) gbp->base) + gbp->offset - gbp->size)

#define IS_RECVAR(vp) ((vp)->shape != NULL ? (*(vp)->shape == NC_UNLIMITED) : 0 )

#ifdef PNETCDF_DEBUG
#define DEBUG_RETURN_ERROR(err) {                               \
    if (verbose) printf("\t(Error %s at line %d in file %s)\n", \
                 #err,__LINE__,__FILE__);                       \
    return err;                                                 \
}
#define DEBUG_ASSIGN_ERROR(status, err) {                       \
    if (verbose) printf("\t(Error %s at line %d in file %s)\n", \
                 #err,__LINE__,__FILE__);                       \
    status = err;                                               \
}
#else
#define DEBUG_RETURN_ERROR(err) return err;
#define DEBUG_ASSIGN_ERROR(status, err) { status = err; }
#endif

#define NC_UNLIMITED    0L
#define NC_ARRAY_GROWBY 64

#define NC_MAX_INT      2147483647
#define NC_MAX_DIMS     NC_MAX_INT
#define NC_MAX_ATTRS    NC_MAX_INT
#define NC_MAX_VARS     NC_MAX_INT
#define NC_MAX_VAR_DIMS NC_MAX_INT  /* max per-variable dimensions */

#define NC_NAT          0       /**< Not A Type */
#define NC_BYTE         1       /**< signed 1 byte integer */
#define NC_CHAR         2       /**< ISO/ASCII character */
#define NC_SHORT        3       /**< signed 2 byte integer */
#define NC_INT          4       /**< signed 4 byte integer */
#define NC_LONG         NC_INT
#define NC_FLOAT        5       /**< single precision floating point number */
#define NC_DOUBLE       6       /**< double precision floating point number */
#define NC_UBYTE        7       /**< unsigned 1 byte int */
#define NC_USHORT       8       /**< unsigned 2-byte int */
#define NC_UINT         9       /**< unsigned 4-byte int */
#define NC_INT64        10      /**< signed 8-byte int */
#define NC_UINT64       11      /**< unsigned 8-byte int */

typedef int nc_type;

#define MIN_NC_XSZ 32
#define NC_DEFAULT_CHUNKSIZE 1048576

typedef enum {
    NC_INVALID     = -1,  /* invalid */
    NC_UNSPECIFIED =  0,  /* ABSENT */
    NC_DIMENSION   = 10,  /* \x00 \x00 \x00 \x0A */
    NC_VARIABLE    = 11,  /* \x00 \x00 \x00 \x0B */
    NC_ATTRIBUTE   = 12   /* \x00 \x00 \x00 \x0C */
} NC_tag;

typedef struct {
    char      *name;
    size_t     name_len;
    long long  size;
} NC_dim;

typedef struct NC_dimarray {
    int      ndefined;     /* number of defined dimensions */
    int      unlimited_id; /* ID of unlimited dimension */
    NC_dim **value;
} NC_dimarray;

typedef struct {
    long long  xsz;      /* amount of space at xvalue (4-byte aligned) */
    char      *name;     /* name of the attributes */
    size_t     name_len;
    nc_type    xtype;    /* the discriminant */
    long long  nelems;   /* number of attribute elements */
    void      *xvalue;   /* the actual data, in external representation */
} NC_attr;

typedef struct NC_attrarray {
    int       ndefined;  /* number of defined attributes */
    NC_attr **value;
} NC_attrarray;

typedef struct {
    int           xsz;    /* byte size of 1 array element */
    long long    *shape;  /* dim->size of each dim */
    long long    *dsizes; /* the right to left product of shape */
    char         *name;   /* name of the variable */
    size_t        name_len;
    int           ndims;  /* number of dimensions */
    int          *dimids; /* array of dimension IDs */
    NC_attrarray  attrs;  /* attribute array */
    nc_type       xtype;  /* variable's data type */
    long long     len;    /* this is the "vsize" defined in header format, the
                             total size in bytes of the array variable.
                             For record variable, this is the record size */
    long long     begin;  /* starting file offset of this variable */
} NC_var;

typedef struct NC_vararray {
    int      ndefined;    /* number of defined variables */
    int      num_rec_vars;/* number of defined record variables */
    NC_var **value;
} NC_vararray;

typedef struct NC {
    int           format;
    char         *path;
    long long     xsz;      /* external size of this header, <= var[0].begin */
    long long     begin_var;/* file offset of the first (non-record) var */
    long long     begin_rec;/* file offset of the first 'record' */

    long long     recsize;  /* length of 'record': sum of single record sizes
                               of all the record variables */
    long long     numrecs;  /* number of 'records' allocated */
    NC_dimarray   dims;     /* dimensions defined */
    NC_attrarray  attrs;    /* global attributes defined */
    NC_vararray   vars;     /* variables defined */
} NC;

typedef struct bufferinfo {
    int        is_little_endian;
    int        fd;
    off_t      offset;   /* current read/write offset in the file */
    int        version;  /* 1, 2, and 5 for CDF-1, 2, and 5 respectively */
    void      *base;     /* beginning of read/write buffer */
    void      *pos;      /* current position in buffer */
    size_t     size;     /* size of the buffer */
} bufferinfo;

#define NC_NOERR        0       /**< No Error */
#define NC_EMAXDIMS     (-41)   /**< NC_MAX_DIMS or NC_MAX_VAR_DIMS exceeds */
#define NC_EMAXATTS     (-44)   /**< NC_MAX_ATTRS exceeded */
#define NC_EBADTYPE     (-45)   /**< Not a netcdf data type */
#define NC_EBADDIM      (-46)   /**< Invalid dimension id or name */
#define NC_EUNLIMPOS    (-47)   /**< NC_UNLIMITED in the wrong index */
#define NC_EMAXVARS     (-48)
#define NC_ENOTNC       (-51)   /**< Not a netcdf file (file format violates CDF specification) */
#define NC_EUNLIMIT     (-54)   /**< NC_UNLIMITED size already in use */
#define NC_ENOMEM       (-61)   /**< Memory allocation (malloc) failure */
#define NC_EVARSIZE     (-62)   /**< One or more variable sizes violate format constraints */
#define NC_EFILE        (-204)  /**< Unknown error in file operation */
#define NC_ENOTSUPPORT  (-214)  /**< Feature is not yet supported */
#define NC_ENULLPAD     (-134)  /**< Header Bytes not Null-Byte padded */

/*
 * "magic number" at beginning of file: 0x43444601 (big Endian)
 */
static const char ncmagic[] = {'C', 'D', 'F', 0x01};

#define ABSENT 0

#define SWAP4B(a) ( ((a) << 24) | \
                   (((a) <<  8) & 0x00ff0000) | \
                   (((a) >>  8) & 0x0000ff00) | \
                   (((a) >> 24) & 0x000000ff) )

#define SWAP8B(a) ( (((a) & 0x00000000000000FFULL) << 56) | \
                    (((a) & 0x000000000000FF00ULL) << 40) | \
                    (((a) & 0x0000000000FF0000ULL) << 24) | \
                    (((a) & 0x00000000FF000000ULL) <<  8) | \
                    (((a) & 0x000000FF00000000ULL) >>  8) | \
                    (((a) & 0x0000FF0000000000ULL) >> 24) | \
                    (((a) & 0x00FF000000000000ULL) >> 40) | \
                    (((a) & 0xFF00000000000000ULL) >> 56) )

static int check_little_endian(void)
{
    /* return 0 for big endian, 1 for little endian. */
    volatile uint32_t i=0x01234567;
    return (*((uint8_t*)(&i))) == 0x67;
}

static void
swap4b(void *val)
{
    uint32_t *op = (uint32_t*)val;
    *op = SWAP4B(*op);
}

static void
swap8b(unsigned long long *val)
{
    uint64_t *op = (uint64_t*)val;
    *op = SWAP8B(*op);
}

static unsigned long long
get_uint64(bufferinfo *gbp)
{
    /* retrieve a 64bit unsigned integer and return it as unsigned long long */
    unsigned long long tmp;
    memcpy(&tmp, gbp->pos, 8);
    if (gbp->is_little_endian) swap8b(&tmp);
    gbp->pos = (char*)gbp->pos + 8;  /* advance gbp->pos 8 bytes */
    return tmp;
}

static unsigned int
get_uint32(bufferinfo *gbp)
{
    /* retrieve a 32bit unsigned integer and return it as unsigned int */
    unsigned int tmp;
    memcpy(&tmp, gbp->pos, 4);
    if (gbp->is_little_endian) swap4b(&tmp);
    gbp->pos = (char*)gbp->pos + 4;  /* advance gbp->pos 4 bytes */
    return tmp;
}

static void
free_NC_dim(NC_dim *dimp)
{
    if (dimp == NULL) return;
    free(dimp->name);
    free(dimp);
}

static void
free_NC_dimarray(NC_dimarray *ncap)
{
    int i;

    assert(ncap != NULL);
    if (ncap->value == NULL) return;

    for (i=0; i<ncap->ndefined; i++)
        if (ncap->value[i] != NULL)
            free_NC_dim(ncap->value[i]);

    free(ncap->value);
    ncap->value    = NULL;
    ncap->ndefined = 0;
}

static void
free_NC_attr(NC_attr *attrp)
{
    if (attrp == NULL) return;
    free(attrp->name);
    if (attrp->xvalue != NULL) free(attrp->xvalue);
    free(attrp);
}

static void
free_NC_attrarray(NC_attrarray *ncap)
{
    int i;

    assert(ncap != NULL);
    if (ncap->value == NULL) return;

    for (i=0; i<ncap->ndefined; i++)
        free_NC_attr(ncap->value[i]);

    free(ncap->value);
    ncap->value    = NULL;
    ncap->ndefined = 0;
}

static void
free_NC_var(NC_var *varp)
{
    if (varp == NULL) return;
    free_NC_attrarray(&varp->attrs);
    free(varp->name);
    free(varp->shape);
    free(varp->dsizes);
    free(varp->dimids);
    free(varp);
}

static void
free_NC_vararray(NC_vararray *ncap)
{
    int i;

    assert(ncap != NULL);
    if (ncap->value == NULL) return;

    for (i=0; i<ncap->ndefined; i++) {
        if (ncap->value[i] != NULL)
            free_NC_var(ncap->value[i]);
    }

    free(ncap->value);
    ncap->value    = NULL;
    ncap->ndefined = 0;
}

/*
 * To compute how much space will the xdr'd header take
 */

/*----< hdr_len_NC_name() >--------------------------------------------------*/
static long long
hdr_len_NC_name(size_t nchars,
                int    sizeof_t)     /* NON_NEG */
{
    /* netCDF file format:
     * name       = nelems  namestring
     * nelems     = NON_NEG
     * namestring = ID1 [IDN ...] padding
     * ID1        = alphanumeric | '_'
     * IDN        = alphanumeric | special1 | special2
     * padding    = <0, 1, 2, or 3 bytes to next 4-byte boundary>
     * NON_NEG    = <non-negative INT> |  // CDF-1 and CDF-2
     *              <non-negative INT64>  // CDF-5
     */
    long long sz = sizeof_t; /* nelems */

    if (nchars != 0)  /* namestring */
        sz += _RNDUP(nchars, X_ALIGN);

    return sz;
}

/*----< hdr_len_NC_dim() >---------------------------------------------------*/
static long long
hdr_len_NC_dim(const NC_dim *dimp,
               int           sizeof_t)     /* NON_NEG */
{
    /* netCDF file format:
     *  ...
     * dim        = name  dim_length
     * dim_length = NON_NEG
     * NON_NEG    = <non-negative INT> |  // CDF-1 and CDF-2
     *              <non-negative INT64>  // CDF-5
     */
    long long sz;

    assert(dimp != NULL);

    sz = hdr_len_NC_name(dimp->name_len, sizeof_t); /* name */
    sz += sizeof_t;                                 /* dim_length */

    return sz;
}

/*----< hdr_len_NC_dimarray() >----------------------------------------------*/
static long long
hdr_len_NC_dimarray(const NC_dimarray *ncap,
                    int                sizeof_t)     /* NON_NEG */
{
    /* netCDF file format:
     *  ...
     * dim_list     = ABSENT | NC_DIMENSION  nelems  [dim ...]
     * ABSENT       = ZERO  ZERO |  // list is not present for CDF-1 and 2
     *                ZERO  ZERO64  // for CDF-5
     * ZERO         = \x00 \x00 \x00 \x00                      // 32-bit zero
     * ZERO64       = \x00 \x00 \x00 \x00 \x00 \x00 \x00 \x00  // 64-bit zero
     * NC_DIMENSION = \x00 \x00 \x00 \x0A         // tag for list of dimensions
     * nelems       = NON_NEG       // number of elements in following sequence
     * NON_NEG      = <non-negative INT> |        // CDF-1 and CDF-2
     *                <non-negative INT64>        // CDF-5
     */
    int i;
    long long xlen;

    xlen = 4;           /* NC_DIMENSION */
    xlen += sizeof_t;   /* nelems */

    if (ncap == NULL) /* ABSENT: no dimension is defined */
        return xlen;

    /* [dim ...] */
    for (i=0; i<ncap->ndefined; i++)
        xlen += hdr_len_NC_dim(ncap->value[i], sizeof_t);

    return xlen;
}

/*----< hdr_len_NC_attr() >--------------------------------------------------*/
static long long
hdr_len_NC_attr(const NC_attr *attrp,
                int            sizeof_t)     /* NON_NEG */
{
    /* netCDF file format:
     *  ...
     * attr    = name  nc_type  nelems  [values ...]
     * nc_type = NC_BYTE | NC_CHAR | NC_SHORT | ...
     * nelems  = NON_NEG       // number of elements in following sequence
     * values  = bytes | chars | shorts | ints | floats | doubles
     * bytes   = [BYTE ...]  padding
     * chars   = [CHAR ...]  padding
     * shorts  = [SHORT ...]  padding
     * ints    = [INT ...]
     * floats  = [FLOAT ...]
     * doubles = [DOUBLE ...]
     * padding = <0, 1, 2, or 3 bytes to next 4-byte boundary>
     * NON_NEG = <non-negative INT> |  // CDF-1 and CDF-2
     *           <non-negative INT64>  // CDF-5
     */
    long long sz;

    assert(attrp != NULL);

    sz  = hdr_len_NC_name(attrp->name_len, sizeof_t); /* name */
    sz += 4;                                          /* nc_type */
    sz += sizeof_t;                                   /* nelems */
    sz += attrp->xsz;                                 /* [values ...] */

    return sz;
}

/*----< hdr_len_NC_attrarray() >---------------------------------------------*/
static long long
hdr_len_NC_attrarray(const NC_attrarray *ncap,
                     int                 sizeof_t)     /* NON_NEG */
{
    /* netCDF file format:
     *  ...
     * att_list     = ABSENT | NC_ATTRIBUTE  nelems  [attr ...]
     * ABSENT       = ZERO  ZERO |  // list is not present for CDF-1 and 2
     *                ZERO  ZERO64  // for CDF-5
     * ZERO         = \x00 \x00 \x00 \x00                      // 32-bit zero
     * ZERO64       = \x00 \x00 \x00 \x00 \x00 \x00 \x00 \x00  // 64-bit zero
     * NC_ATTRIBUTE = \x00 \x00 \x00 \x0C         // tag for list of attributes
     * nelems       = NON_NEG       // number of elements in following sequence
     * NON_NEG      = <non-negative INT> |        // CDF-1 and CDF-2
     *                <non-negative INT64>        // CDF-5
     */
    int i;
    long long xlen;

    xlen = 4;          /* NC_ATTRIBUTE */
    xlen += sizeof_t;  /* nelems */

    if (ncap == NULL) /* ABSENT: no attribute is defined */
        return xlen;

    for (i=0; i<ncap->ndefined; i++) /* [attr ...] */
        xlen += hdr_len_NC_attr(ncap->value[i], sizeof_t);

    return xlen;
}

/*----< hdr_len_NC_var() >---------------------------------------------------*/
static long long
hdr_len_NC_var(const NC_var *varp,
               int           sizeof_off_t, /* OFFSET */
               int           sizeof_t)     /* NON_NEG */
{
    /* netCDF file format:
     * netcdf_file = header data
     * header      = magic numrecs dim_list gatt_list var_list
     *  ...
     * var         = name nelems [dimid ...] vatt_list nc_type vsize begin
     * nelems      = NON_NEG
     * dimid       = NON_NEG
     * vatt_list   = att_list
     * nc_type     = NC_BYTE | NC_CHAR | NC_SHORT | ...
     * vsize       = NON_NEG
     * begin       = OFFSET        // Variable start location.
     * OFFSET      = <non-negative INT> |  // CDF-1
     *               <non-negative INT64>  // CDF-2 and CDF-5
     * NON_NEG     = <non-negative INT> |  // CDF-1 and CDF-2
     *               <non-negative INT64>  // CDF-5
     */
    long long sz;

    assert(varp != NULL);

    /* for CDF-1, sizeof_off_t == 4 && sizeof_t == 4
     * for CDF-2, sizeof_off_t == 8 && sizeof_t == 4
     * for CDF-5, sizeof_off_t == 8 && sizeof_t == 8
     */
    sz = hdr_len_NC_name(varp->name_len, sizeof_t);     /* name */
    sz += sizeof_t;                                     /* nelems */
    sz += ((long long)sizeof_t) * varp->ndims;          /* [dimid ...] */
    sz += hdr_len_NC_attrarray(&varp->attrs, sizeof_t); /* vatt_list */
    sz += 4;                                            /* nc_type */
    sz += sizeof_t;                                     /* vsize */
    sz += sizeof_off_t;                                 /* begin */

    return sz;
}

/*----< hdr_len_NC_vararray() >----------------------------------------------*/
static long long
hdr_len_NC_vararray(const NC_vararray *ncap,
                    int                sizeof_t,     /* NON_NEG */
                    int                sizeof_off_t) /* OFFSET */
{
    /* netCDF file format:
     * netcdf_file = header  data
     * header      = magic  numrecs  dim_list  gatt_list  var_list
     *  ...
     * var_list    = ABSENT | NC_VARIABLE   nelems  [var ...]
     * ABSENT      = ZERO  ZERO |  // list is not present for CDF-1 and 2
     *               ZERO  ZERO64  // for CDF-5
     * ZERO        = \x00 \x00 \x00 \x00                      // 32-bit zero
     * ZERO64      = \x00 \x00 \x00 \x00 \x00 \x00 \x00 \x00  // 64-bit zero
     * NC_VARIABLE = \x00 \x00 \x00 \x0B         // tag for list of variables
     * nelems      = NON_NEG       // number of elements in following sequence
     * NON_NEG     = <non-negative INT> |        // CDF-1 and CDF-2
     *               <non-negative INT64>        // CDF-5
     */
    int i;
    long long xlen;

    xlen = 4;           /* NC_VARIABLE */
    xlen += sizeof_t;   /* nelems */

    if (ncap == NULL) /* ABSENT: no variable is defined */
        return xlen;

    /* for CDF-1, sizeof_off_t == 4 && sizeof_t == 4
     * for CDF-2, sizeof_off_t == 8 && sizeof_t == 4
     * for CDF-5, sizeof_off_t == 8 && sizeof_t == 8
     */
    for (i=0; i<ncap->ndefined; i++)  /* [var ...] */
        xlen += hdr_len_NC_var(ncap->value[i], sizeof_off_t, sizeof_t);

    return xlen;
}

/*----< hdr_len_NC() >-------------------------------------------------------*/
static long long
hdr_len_NC(const NC *ncp)
{
    /* netCDF file format:
     * netcdf_file = header  data
     * header      = magic  numrecs  dim_list  gatt_list  var_list
     *  ...
     * numrecs     = NON_NEG | STREAMING   // length of record dimension
     * NON_NEG     = <non-negative INT> |  // CDF-1 and CDF-2
     *               <non-negative INT64>  // CDF-5
     */

    int sizeof_t, sizeof_off_t;
    long long xlen;

    assert(ncp != NULL);

    if (ncp->format == 5) {        /* CDF-5 */
        sizeof_t     = 8; /* 8-byte integer for all integers */
        sizeof_off_t = 8; /* 8-byte integer for var begin */
    }
    else if (ncp->format == 2) { /* CDF-2 */
        sizeof_t     = 4; /* 4-byte integer in CDF-1 */
        sizeof_off_t = 8; /* 8-byte integer for var begin */
    }
    else { /* CDF-1 */
        sizeof_t     = 4; /* 4-byte integer in CDF-1 */
        sizeof_off_t = 4; /* 4-byte integer in CDF-1 */
    }

    xlen  = sizeof(ncmagic);                                           /* magic */
    xlen += sizeof_t;                                                  /* numrecs */
    xlen += hdr_len_NC_dimarray(&ncp->dims,   sizeof_t);               /* dim_list */
    xlen += hdr_len_NC_attrarray(&ncp->attrs, sizeof_t);               /* gatt_list */
    xlen += hdr_len_NC_vararray(&ncp->vars,   sizeof_t, sizeof_off_t); /* var_list */

    return xlen; /* return the header size (not yet aligned) */
}

/*----< ncmpio_xlen_nc_type() >----------------------------------------------*/
/* return the length of external NC data type */
static int
xlen_nc_type(nc_type xtype) {
    switch(xtype) {
        case NC_BYTE:
        case NC_CHAR:
        case NC_UBYTE:  return 1;
        case NC_SHORT:
        case NC_USHORT: return 2;
        case NC_INT:
        case NC_UINT:
        case NC_FLOAT:  return 4;
        case NC_DOUBLE:
        case NC_INT64:
        case NC_UINT64: return 8;
        default: DEBUG_RETURN_ERROR(NC_EBADTYPE)
    }
}

static NC_dim *
elem_NC_dimarray(const NC_dimarray *ncap,
                 int                dimid)
{
    /* returns the dimension ID defined earlier */
    assert(ncap != NULL);

    if (dimid < 0 || ncap->ndefined == 0 || dimid >= ncap->ndefined)
        return NULL;

    assert(ncap->value != NULL);

    return ncap->value[dimid];
}

static int
var_shape64(NC_var            *varp,
            const NC_dimarray *dims,
            const char        *loc)
{
    int i;
    long long product = 1;

    /* set the size of 1 element */
    varp->xsz = xlen_nc_type(varp->xtype);

    if (varp->ndims == 0) goto out;

    /*
     * use the user supplied dimension indices to determine the shape
     */
    for (i=0; i<varp->ndims; i++) {
        const NC_dim *dimp;

        if (varp->dimids[i] < 0) {
            if (verbose) printf("Error:\n");
            if (verbose) printf("\t%s: dimension ID [%d] invalid (%d)\n",loc,i,varp->dimids[i]);
            DEBUG_RETURN_ERROR(NC_EBADDIM);
        }

        if (varp->dimids[i] >= ((dims != NULL) ? dims->ndefined : 1)) {
            if (verbose) printf("Error:\n");
            if (verbose) printf("\t%s: dimension ID [%d] (%d) larger than defined (%d)\n",loc,i,varp->dimids[i], ((dims != NULL) ? dims->ndefined : 1));
            DEBUG_RETURN_ERROR(NC_EBADDIM);
        }

        /* get the pointer to the dim object */
        dimp = elem_NC_dimarray(dims, varp->dimids[i]);
        varp->shape[i] = dimp->size;

        /* check for record variable, only the highest dimension can
         * be unlimited */
        if (varp->shape[i] == NC_UNLIMITED && i != 0) {
            if (verbose) printf("Error:\n");
            if (verbose) printf("\t%s: dimension ID [%d] is NC_UNLIMITED in the wrong index\n",loc,i);
            DEBUG_RETURN_ERROR(NC_EUNLIMPOS);
        }
    }

    /*
     * compute the dsizes, the right to left product of shape
     */
    product = 1;
    if (varp->ndims == 1) {
        if (varp->shape[0] == NC_UNLIMITED)
            varp->dsizes[0] = 1;
        else {
            varp->dsizes[0] = varp->shape[0];
            product = varp->shape[0];
        }
    }
    else { /* varp->ndims > 1 */
        varp->dsizes[varp->ndims-1] = varp->shape[varp->ndims-1];
        product = varp->shape[varp->ndims-1];
        for (i=varp->ndims-2; i>=0; i--) {
            if (varp->shape[i] != NC_UNLIMITED)
                product *= varp->shape[i];
            varp->dsizes[i] = product;
        }
    }

out :
    /*
     * For CDF-1 and CDF-2 formats, the total number of array elements
     * cannot exceed 2^32, unless this variable is the last fixed-size
     * variable, there is no record variable, and the file starting
     * offset of this variable is less than 2GiB.
     * We will check this in ncmpi_enddef() which calls ncmpii_NC_enddef()
     * which calls ncmpii_NC_check_vlens()
    if (ncp->format != 5 && product >= X_UINT_MAX)
        DEBUG_RETURN_ERROR(NC_EVARSIZE);
     */

    /*
     * align variable size to 4 byte boundary, required by all netcdf file
     * formats
     */
    varp->len = product * varp->xsz;
    if (varp->len % 4 > 0)
        varp->len += 4 - varp->len % 4; /* round up */

    return NC_NOERR;
}

/* calculate the following
 *   ncp->begin_var           first variable's offset, file header extent
 *   ncp->begin_rec           first record variable's offset
 *   ncp->recsize             sum of all single record size of all variables
 *   ncp->vars.value[*]->len  individual variable size (record size)
 */
static int
compute_var_shape(NC *ncp)
{
    int i, j, err;
    char xloc[1024];
    NC_var *first_var = NULL;       /* first "non-record" var */
    NC_var *first_rec = NULL;       /* first "record" var */

    if (ncp->vars.ndefined == 0) return NC_NOERR;

    ncp->begin_var = ncp->xsz;
    ncp->begin_rec = ncp->xsz;
    ncp->recsize   = 0;

    for (i=0; i<ncp->vars.ndefined; i++) {
        sprintf(xloc,"var %s:",ncp->vars.value[i]->name);
        /* check if dimids are valid */
        for (j=0; j<ncp->vars.value[i]->ndims; j++) {
            if (ncp->vars.value[i]->dimids[j] < 0) {
                if (verbose) printf("Error:\n");
                if (verbose) printf("\t%s: dimension ID [%d] invalid (%d)\n",xloc,i,ncp->vars.value[i]->dimids[i]);
                DEBUG_RETURN_ERROR(NC_EBADDIM) /* dimid is not defined */
            }
            else if (ncp->vars.value[i]->dimids[j] >= ncp->dims.ndefined) {
                if (verbose) printf("Error:\n");
                if (verbose) printf("\t%s: dimension ID [%d] (%d) larger than defined (%d)\n",xloc,i,ncp->vars.value[i]->dimids[i], ncp->dims.ndefined);
                DEBUG_RETURN_ERROR(NC_EBADDIM);
            }
        }
        /* ncp->vars.value[i]->len will be recomputed from dimensions in
         * var_shape64() */
        err = var_shape64(ncp->vars.value[i], &ncp->dims, xloc);
        if (err != NC_NOERR) return err;

        if (IS_RECVAR(ncp->vars.value[i])) {
            if (first_rec == NULL) first_rec = ncp->vars.value[i];
            ncp->recsize += ncp->vars.value[i]->len;
        }
        else { /* fixed-size variable */
            if (first_var == NULL) first_var = ncp->vars.value[i];
            ncp->begin_rec = ncp->vars.value[i]->begin
                           + ncp->vars.value[i]->len;
        }
    }

    if (first_rec != NULL) {
        if (ncp->begin_rec > first_rec->begin) {
            if (verbose) printf("Error:\n");
            if (verbose) printf("\tbegin of record section (%lld) greater than the begin of first record (%lld)\n",ncp->begin_rec, first_rec->begin);
            DEBUG_RETURN_ERROR(NC_ENOTNC) /* not a netCDF file or corrupted */
        }

        ncp->begin_rec = first_rec->begin;
        /*
         * for special case of exactly one record variable, pack value
         */
        if (ncp->recsize == first_rec->len)
            ncp->recsize = *first_rec->dsizes * first_rec->xsz;
    }

    if (first_var != NULL)
        ncp->begin_var = first_var->begin;
    else
        ncp->begin_var = ncp->begin_rec;

    if (ncp->begin_var <= 0) {
        if (verbose) printf("Error:\n");
        if (verbose) printf("\tbegin of variable section (%lld) is negative\n",ncp->begin_var);
        DEBUG_RETURN_ERROR(NC_ENOTNC) /* not a netCDF file or corrupted */
    }
    else if (ncp->xsz > ncp->begin_var) {
        if (verbose) printf("Error:\n");
        if (verbose) printf("\tfile header size (%lld) is larger than the begin of data section (%lld)\n",ncp->xsz, ncp->begin_var);
        DEBUG_RETURN_ERROR(NC_ENOTNC) /* not a netCDF file or corrupted */
    }
    else if (ncp->begin_rec <= 0) {
        if (verbose) printf("Error:\n");
        if (verbose) printf("\tbegin of record section (%lld) is zero or negative\n",ncp->begin_rec);
        DEBUG_RETURN_ERROR(NC_ENOTNC) /* not a netCDF file or corrupted */
    }
    else if (ncp->begin_var > ncp->begin_rec) {
        if (verbose) printf("Error:\n");
        if (verbose) printf("\tbegin of data section (%lld) is larger than record section (%lld)\n",ncp->begin_var, ncp->begin_rec);
        DEBUG_RETURN_ERROR(NC_ENOTNC) /* not a netCDF file or corrupted */
    }

    return NC_NOERR;
}

/*
 * repair file contents
 */
static int
val_repair(int fd, off_t offset, size_t len, void *buf)
{
    ssize_t nn;

    if (-1 == lseek(fd, offset, SEEK_SET)) {
        if (verbose)
            printf("Error at line %d: lseek %s\n",__LINE__,strerror(errno));
        return -1;
    }
    nn = write(fd, buf, len);
    if (nn == -1) {
        if (verbose)
            printf("Error at line %d: write %s\n",__LINE__,strerror(errno));
        return -1;
    }
    if (nn != len) {
        if (verbose)
            printf("Error at line %d: writing %zd bytes but only %zd written\n",
                   __LINE__,len, nn);
        return -1;
    }

    return NC_NOERR;
}

/*
 * Fetch the next header chunk.
 */
static int
val_fetch(int fd, bufferinfo *gbp) {
    ssize_t nn = 0;
    long long slack;        /* any leftover data in the buffer */
    size_t pos_addr, base_addr;

    assert(gbp->base != NULL);

    pos_addr = (size_t) gbp->pos;
    base_addr = (size_t) gbp->base;

    slack = gbp->size - (pos_addr - base_addr);
    /* if gbp->pos and gbp->base are the same, there is no leftover buffer data
     * to worry about.
     * In the other extreme, where gbp->size == (gbp->pos - gbp->base), then all
     * data in the buffer has been consumed */
    if (slack == gbp->size) slack = 0;

    memset(gbp->base, 0, gbp->size);
    gbp->pos = gbp->base;

    if (-1 == lseek(fd, gbp->offset-slack, SEEK_SET)) {
        fprintf(stderr,"Error at line %d: lseek %s\n",__LINE__,strerror(errno));
        return -1;
    }
    nn = read(fd, gbp->base, gbp->size);
    if (nn == -1) {
        fprintf(stderr,"Error at line %d: read %s\n",__LINE__,strerror(errno));
        return -1;
    }
    gbp->offset += (gbp->size - slack);

    return NC_NOERR;
}

/*
 * Ensure that 'nextread' bytes are available.
 */
static int
val_check_buffer(int         fd,
                 bufferinfo *gbp,
                 long long  nextread)
{
    size_t pos_addr, base_addr;

    pos_addr = (size_t) gbp->pos;
    base_addr = (size_t) gbp->base;

    if (pos_addr + nextread <= base_addr + gbp->size)
        return NC_NOERR;

    return val_fetch(fd, gbp);
}

static int
val_get_NC_tag(int fd, bufferinfo *gbp, NC_tag *tagp, const char *loc)
{
    int status;
    size_t err_addr;
    unsigned int tag;

    err_addr = ERR_ADDR;
    status = val_check_buffer(fd, gbp, (gbp->version < 5) ? 4 : 8);
    if (status != NC_NOERR) goto fn_exit;

    tag = get_uint32(gbp);
    switch(tag) {
        case  0: *tagp = NC_UNSPECIFIED; break;
        case 10: *tagp = NC_DIMENSION;   break;
        case 11: *tagp = NC_VARIABLE;    break;
        case 12: *tagp = NC_ATTRIBUTE;   break;
        default:
            *tagp = NC_INVALID;
            if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
            if (verbose) printf("\tInvalid NC component tag (%d)\n",tag);
            return NC_ENOTNC;
    }
    return NC_NOERR;

fn_exit:
    if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
    if (verbose) printf("\t%s: Fail to read NC component tag\n",loc);
    return status;
}

static int
hdr_get_NON_NEG(int fd, bufferinfo *gbp, long long *sp)
{
    /* netCDF file format:
     *  ...
     * NON_NEG    = <non-negative INT> |  // CDF-1 and CDF-2
     *              <non-negative INT64>  // CDF-5
     */
    int sizeof_NON_NEG, status;

    sizeof_NON_NEG = (gbp->version < 5) ? 4 : 8;
    status = val_check_buffer(fd, gbp, sizeof_NON_NEG);
    if (status != NC_NOERR) {
        if (verbose) printf("%d-byte size is expected for ", sizeof_NON_NEG);
        return status;
    }
    if (gbp->version < 5)
        *sp = (long long) get_uint32(gbp);
    else
        *sp = (long long) get_uint64(gbp);

    return status;
}

static int
hdr_get_name(int          fd,
             bufferinfo  *gbp,
             char       **namep,
             const char  *loc)
{
    /* netCDF file format:
     *  ...
     * name       = nelems  namestring
     * nelems     = NON_NEG
     * namestring = ID1 [IDN ...] padding
     * ID1        = alphanumeric | '_'
     * IDN        = alphanumeric | special1 | special2
     * padding    = <0, 1, 2, or 3 bytes to next 4-byte boundary>
     * NON_NEG    = <non-negative INT> |  // CDF-1 and CDF-2
     *              <non-negative INT64>  // CDF-5
     */
    int err=NC_NOERR;
    char *cpos, pad[X_ALIGN-1];
    long long nchars=0, padding, bufremain, strcount;
    size_t err_addr, pos_addr, base_addr;

    *namep = NULL;

    /* read nelems, string length */
    err_addr = ERR_ADDR;
    err = hdr_get_NON_NEG(fd, gbp, &nchars);
    if (err != NC_NOERR) {
        if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
        if (verbose) printf("\t%s: Failed to read name string length\n", loc);
        return err;
    }

    *namep = (char*) malloc((size_t)nchars + 1);
    if (*namep == NULL) DEBUG_RETURN_ERROR(NC_ENOMEM)
    (*namep)[nchars] = '\0'; /* add terminal character */

    padding   = _RNDUP(nchars, X_ALIGN) - nchars;
    pos_addr  = (size_t) gbp->pos;
    base_addr = (size_t) gbp->base;
    bufremain = gbp->size - (pos_addr - base_addr);
    cpos = *namep;

    while (nchars > 0) {
        if (bufremain > 0) {
            strcount = MIN(bufremain, nchars);
            (void) memcpy(cpos, gbp->pos, strcount);
            nchars -= strcount;
            gbp->pos = (void *)((char *)gbp->pos + strcount);
            cpos += strcount;
            bufremain -= strcount;
        } else {
            err_addr = ERR_ADDR;
            err = val_fetch(fd, gbp);
            if (err != NC_NOERR) {
                if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
                if (verbose) printf("\t%s - fetching name string\n", loc);
                free(*namep);
                *namep = NULL;
                return err;
            }
            bufremain = gbp->size;
        }
    }

    if (padding > 0) {
        err_addr = ERR_ADDR;
        err = val_check_buffer(fd, gbp, padding);
        if (err != NC_NOERR) {
            if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
            if (verbose) printf("\t%s - fetching name string padding\n", loc);
            return err;
        }
        memset(pad, 0, X_ALIGN-1);
        if (memcmp(gbp->pos, pad, padding) != 0) {
            /* This is considered not a fatal error, we continue to validate */
            if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
            if (verbose) printf("\t%s \"%s\": name padding is non-null byte\n", loc, *namep);
            /* free(*namep);
               *namep = NULL; */
            DEBUG_ASSIGN_ERROR(err, NC_ENULLPAD)
            if (repair) {
                val_repair(fd, err_addr, (size_t)padding, (void*)nada);
                if (verbose)
                    printf("\t%s \"%s\": name padding error has been **repaired**\n",loc,*namep);
            }
        }
        gbp->pos = (void *)((char *)gbp->pos + padding);
    }

    return err;
}

static int
val_get_NC_dim(int fd, bufferinfo *gbp, NC_dim **dimpp, NC_dimarray *ncap) {
    int err, status=NC_NOERR;
    char *name=NULL;
    size_t err_addr;
    long long dim_length;
    NC_dim *dimp;

    *dimpp = NULL;

    status = hdr_get_name(fd, gbp, &name, "Dimension");
    if (status != NC_NOERR && status != NC_ENULLPAD) {
        if (name != NULL) free(name);
        return status;
    }

    /* read dimension length */
    err_addr = ERR_ADDR;
    err = hdr_get_NON_NEG(fd, gbp, &dim_length);
    if (err != NC_NOERR) { /* frees dimp */
        if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
        if (verbose) printf("\tDimension \"%s\": Failed to read dimension size\n",name);
        free(name);
        return err;
    }

    /* check if unlimited_id already set */
    if (ncap->unlimited_id != -1 && dim_length == 0) {
        if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
        if (verbose) printf("\tDimension \"%s\": NC_UNLIMITED dimension already found (\"%s\")\n",name,ncap->value[ncap->unlimited_id]->name);
        free(name);
        return NC_EUNLIMIT;
    }

    dimp = (NC_dim*) malloc(sizeof(NC_dim));
    if (dimp == NULL) {
        free(name);
        DEBUG_RETURN_ERROR(NC_ENOMEM)
    }
    dimp->name     = name;
    dimp->name_len = strlen(name);
    dimp->size     = dim_length;

    *dimpp = dimp;

    return status;
}

static int
val_get_NC_dimarray(int fd, bufferinfo *gbp, NC_dimarray *ncap, long long numrecs)
{
    /* netCDF file format:
     *  ...
     * dim_list     = ABSENT | NC_DIMENSION  nelems  [dim ...]
     * ABSENT       = ZERO  ZERO |  // list is not present for CDF-1 and 2
     *                ZERO  ZERO64  // for CDF-5
     * ZERO         = \x00 \x00 \x00 \x00                      // 32-bit zero
     * ZERO64       = \x00 \x00 \x00 \x00 \x00 \x00 \x00 \x00  // 64-bit zero
     * NC_DIMENSION = \x00 \x00 \x00 \x0A         // tag for list of dimensions
     * nelems       = NON_NEG       // number of elements in following sequence
     * NON_NEG      = <non-negative INT> |        // CDF-1 and CDF-2
     *                <non-negative INT64>        // CDF-5
     */
    int dim, err, status=NC_NOERR;
    NC_tag tag = NC_UNSPECIFIED;
    size_t tag_err_addr, nelems_err_addr;
    long long tmp;

    assert(gbp != NULL && gbp->pos != NULL);
    assert(ncap != NULL);
    assert(ncap->value == NULL);

    /* read NC_tag (NC_DIMENSION or ZERO) from gbp buffer */
    tag_err_addr = ERR_ADDR;
    err = val_get_NC_tag(fd, gbp, &tag, "tag NC_DIMENSION");
    if (err != NC_NOERR) return err;

    /* read nelems (number of dimensions) from gbp buffer */
    nelems_err_addr = ERR_ADDR;
    err = hdr_get_NON_NEG(fd, gbp, &tmp);
    if (err != NC_NOERR) {
        if (verbose) printf("Error @ [0x%8.8zx]:\n", nelems_err_addr);
        if (verbose) printf("\tFailed to read tag NC_DIMENSION\n");
        return err;
    }
    if (tmp > NC_MAX_DIMS) {
        /* number of allowable defined dimensions NC_MAX_DIMS */
        if (verbose) printf("Error @ [0x%8.8zx]:\n", nelems_err_addr);
        if (verbose) printf("\tNumber of dimensions (%lld) defined in file exceeds NC_MAX_DIMS (%d)\n",tmp,NC_MAX_DIMS);
        DEBUG_RETURN_ERROR(NC_EMAXDIMS)
    }
    ncap->ndefined = (int)tmp;
    if (trace) printf("\ndim_list (dimension list):\n");

    ncap->unlimited_id = -1;

    if (ncap->ndefined == 0) {
        /* no dimension defined */
        /* From the CDF file format specification, the tag is either
         * NC_DIMENSION or ABSENT (ZERO), but we follow NetCDF library to skip
         * checking the tag when ndefined is zero.
         */
        if (trace) printf("\ttag = ABSENT (no dimension defined)\n");
        return NC_NOERR;
#if 0
        if (tag != ABSENT) {
            if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
            if (verbose) printf("\tInvalid NC component tag, while ABSENT is expected for ");
            DEBUG_RETURN_ERROR(NC_ENOTNC)
        }
#endif
    } else {
        if (tag != NC_DIMENSION) {
            if (verbose) printf("Error @ [0x%8.8zx]:\n", tag_err_addr);
            if (verbose) printf("\tInvalid NC component tag (%d), expecting NC_DIMENSION (%d)\n",tag,NC_DIMENSION);
            DEBUG_RETURN_ERROR(NC_ENOTNC)
        }
        if (trace) {
            printf("\ttag = NC_DIMENSION\n");
            printf("\tnumber of dimensions defined = %d\n", ncap->ndefined);
        }

        /* check each dimension */
        size_t alloc_size = (size_t)ncap->ndefined + NC_ARRAY_GROWBY;
        ncap->value = (NC_dim **) calloc(alloc_size, sizeof(NC_dim *));
        if (ncap->value == NULL) DEBUG_RETURN_ERROR(NC_ENOMEM)

        for (dim=0; dim<ncap->ndefined; dim++) {
            err = val_get_NC_dim(fd, gbp, &ncap->value[dim], ncap);
            if (err != NC_NOERR && err != NC_ENULLPAD) {
                ncap->ndefined = dim;
                free_NC_dimarray(ncap);
                return err;
            }
            if (status == NC_NOERR) status = err;
            if (ncap->value[dim]->size == NC_UNLIMITED)
                ncap->unlimited_id = dim; /* ID of unlimited dimension */

            if (trace) {
                if (ncap->unlimited_id == dim)
                    printf("\tdimension ID %2d: name \"%s\", length = UNLIMITED (%lld currently)\n",
                           dim,ncap->value[dim]->name, numrecs);
                else
                    printf("\tdimension ID %2d: name \"%s\", length = %lld\n",
                           dim,ncap->value[dim]->name,ncap->value[dim]->size);
            }
        }
    }

    return status;
}

static int
val_get_nc_type(int         fd,
                bufferinfo *gbp,
                nc_type    *xtypep,
                const char *loc)
{
    /* nc_type is 4-byte integer */
    int status;
    size_t err_addr;
    unsigned int xtype = 0;

    err_addr = ERR_ADDR;
    status = val_check_buffer(fd, gbp, 4);
    if (status != NC_NOERR) goto read_err_exit;

    /* get a 4-byte integer */
    xtype = get_uint32(gbp);

    if (xtype < NC_BYTE) goto err_exit;

    if (gbp->version < 5) {
        if (xtype > NC_DOUBLE) goto err_exit;
    }
    else if (xtype > NC_UINT64) goto err_exit;

    *xtypep = (nc_type) xtype;

    return NC_NOERR;

read_err_exit:
    if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
    if (verbose) printf("\t%s: Failed to read NC data type\n",loc);
    return status;

err_exit:
    if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
    if (verbose) printf("\t%s: Unknown NC data type (%u)\n",loc, xtype);
    DEBUG_RETURN_ERROR(NC_EBADTYPE)
}

/*
 * Get the values of an attribute
 */
static int
val_get_NC_attrV(int         fd,
                 bufferinfo *gbp,
                 NC_attr    *attrp,
                 const char *loc)
{
    int status=NC_NOERR;
    void *value = attrp->xvalue;
    char pad[X_ALIGN-1];
    size_t err_addr;
    long long nvalues, padding, bufremain, attcount;
    size_t pos_addr, base_addr;

    nvalues = attrp->nelems * xlen_nc_type(attrp->xtype);
    padding = attrp->xsz - nvalues;
    pos_addr  = (size_t) gbp->pos;
    base_addr = (size_t) gbp->base;
    bufremain = gbp->size - (pos_addr - base_addr);

    while (nvalues > 0) {
        if (bufremain > 0) {
            attcount = MIN(bufremain, nvalues);
            (void) memcpy(value, gbp->pos, attcount);
            nvalues -= attcount;
            gbp->pos = (void *)((char *)gbp->pos + attcount);
            value = (void *)((char *)value + attcount);
            bufremain -= attcount;
        } else {
            err_addr = ERR_ADDR;
            status = val_fetch(fd, gbp);
            if (status != NC_NOERR) {
                if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
                if (verbose) printf("\t%s: Failed to fetch next chunk into a buffer\n", loc);
                return status;
            }
            bufremain = gbp->size;
        }
    }

    if (padding > 0) {
        memset(pad, 0, X_ALIGN-1);
        if (memcmp(gbp->pos, pad, padding) != 0) {
            /* This is considered not a fatal error, we continue to validate */
            if (verbose) printf("Error @ [0x%8.8zx]:\n", (size_t)ERR_ADDR);
            if (verbose) printf("\t%s: value padding is non-null byte\n", loc);
            DEBUG_ASSIGN_ERROR(status, NC_ENULLPAD)
            if (repair) {
                val_repair(fd, ERR_ADDR, (size_t)padding, (void*)nada);
                if (verbose)
                    printf("\t%s: value padding has been **repaired**\n",loc);
            }
        }
        gbp->pos = (void *)((char *)gbp->pos + padding);
    }

    return status;
}

static char*
str_NC_type(nc_type xtype)
{
    switch(xtype) {
        case NC_BYTE:   return "NC_BYTE";
        case NC_CHAR:   return "NC_CHAR";
        case NC_UBYTE:  return "NC_UBYTE";
        case NC_SHORT:  return "NC_SHORT";
        case NC_USHORT: return "NC_USHORT";
        case NC_INT:    return "NC_INT";
        case NC_UINT:   return "NC_UINT";
        case NC_FLOAT:  return "NC_FLOAT";
        case NC_DOUBLE: return "NC_DOUBLE";
        case NC_INT64:  return "NC_INT64";
        case NC_UINT64: return "NC_UINT64";
        default: return "";
    }
}

static long long
x_len_NC_attrV(nc_type    xtype,
               long long nelems)
{
    switch(xtype) {
        case NC_BYTE:
        case NC_CHAR:
        case NC_UBYTE:  return _RNDUP(nelems, 4);
        case NC_SHORT:
        case NC_USHORT: return ((nelems + (nelems)%2) * 2);
        case NC_INT:    return (nelems * 4);
        case NC_UINT:   return (nelems * 4);
        case NC_FLOAT:  return (nelems * 4);
        case NC_DOUBLE: return (nelems * 8);
        case NC_INT64:  return (nelems * 8);
        case NC_UINT64: return (nelems * 8);
        default: if (verbose) fprintf(stderr, "Error: bad xtype(%d) in %s\n",xtype,__func__);
    }
    return 0;
}

static int
new_NC_attr(char        *name,
            nc_type      xtype,
            long long   nelems,
            NC_attr    **attrp)
{
    *attrp = (NC_attr*) malloc(sizeof(NC_attr));
    if (*attrp == NULL ) DEBUG_RETURN_ERROR(NC_ENOMEM)

    (*attrp)->xtype    = xtype;
    (*attrp)->xsz      = 0;
    (*attrp)->nelems   = nelems;
    (*attrp)->xvalue   = NULL;
    (*attrp)->name     = name;
    (*attrp)->name_len = strlen(name);

    if (nelems > 0) {
        long long xsz = x_len_NC_attrV(xtype, nelems);
        (*attrp)->xsz    = xsz;
        (*attrp)->xvalue = malloc((size_t)xsz);
        if ((*attrp)->xvalue == NULL) {
            free(*attrp);
            *attrp = NULL;
            DEBUG_RETURN_ERROR(NC_ENOMEM)
        }
    }
    return NC_NOERR;
}

static int
val_get_NC_attr(int          fd,
                bufferinfo  *gbp,
                NC_attr    **attrpp,
                const char  *loc)
{
    char *name=NULL, xloc[1024];
    int err, status=NC_NOERR;
    size_t err_addr;
    nc_type xtype;
    long long nelems;
    NC_attr *attrp;

    status = hdr_get_name(fd, gbp, &name, loc);
    if (status != NC_NOERR && status != NC_ENULLPAD) {
        if (name != NULL) free(name);
        return status;
    }

    sprintf(xloc,"%s \"%s\"",loc,name);
    err = val_get_nc_type(fd, gbp, &xtype, xloc);
    if (err != NC_NOERR) {
        if (name != NULL) free(name);
        return err;
    }

    err_addr = ERR_ADDR;
    err = hdr_get_NON_NEG(fd, gbp, &nelems);
    if (err != NC_NOERR) {
        if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
        if (verbose) printf("\t%s: Failed to read attribute length\n",xloc);
        if (name != NULL) free(name);
        return err;
    }

    err = new_NC_attr(name, xtype, nelems, &attrp);
    if(err != NC_NOERR) {
        if (name != NULL) free(name);
        return err;
    }

    err = val_get_NC_attrV(fd, gbp, attrp, xloc);
    if (err != NC_NOERR && err != NC_ENULLPAD) {
        free(attrp->name);
        free(attrp->xvalue);
        free(attrp);
        return err;
    }
    if (status == NC_NOERR) status = err;

    *attrpp = attrp;

    return status;
}

static int
val_get_NC_attrarray(int           fd,
                     bufferinfo   *gbp,
                     NC_attrarray *ncap,
                     const char   *loc)
{
    /* netCDF file format:
     *  ...
     * att_list     = ABSENT | NC_ATTRIBUTE  nelems  [attr ...]
     * ABSENT       = ZERO  ZERO |  // list is not present for CDF-1 and 2
     *                ZERO  ZERO64  // for CDF-5
     * ZERO         = \x00 \x00 \x00 \x00                      // 32-bit zero
     * ZERO64       = \x00 \x00 \x00 \x00 \x00 \x00 \x00 \x00  // 64-bit zero
     * NC_ATTRIBUTE = \x00 \x00 \x00 \x0C         // tag for list of attributes
     * nelems       = NON_NEG       // number of elements in following sequence
     * NON_NEG      = <non-negative INT> |        // CDF-1 and CDF-2
     *                <non-negative INT64>        // CDF-5
     */
    char xloc[1024];
    int i, err, status=NC_NOERR;
    NC_tag tag = NC_UNSPECIFIED;
    long long tmp;
    size_t tag_err_addr, nelems_err_addr;

    assert(gbp != NULL && gbp->pos != NULL);
    assert(ncap != NULL);
    assert(ncap->value == NULL);

    /* read NC_tag (NC_ATTRIBUTE or ZERO) from gbp buffer */
    tag_err_addr = ERR_ADDR;
    err = val_get_NC_tag(fd, gbp, &tag, "tag NC_ATTRIBUTE");
    if (err != NC_NOERR) return err;

    /* read nelems (number of attributes) from gbp buffer */
    nelems_err_addr = ERR_ADDR;
    err = hdr_get_NON_NEG(fd, gbp, &tmp);
    if (err != NC_NOERR) {
        if (verbose) printf("Error @ [0x%8.8zx]:\n", nelems_err_addr);
        if (verbose) printf("\tFailed to read tag NC_ATTRIBUTE\n");
        return err;
    }
    if (tmp > NC_MAX_ATTRS) {
        /* number of allowable defined attributes NC_MAX_ATTRS */
        if (verbose) printf("Error @ [0x%8.8zx]:\n", nelems_err_addr);
        if (verbose) printf("\t%s attributes: number of attributes (%lld) exceeds NC_MAX_ATTRS (%d)\n",loc,tmp,NC_MAX_ATTRS);
        return NC_EMAXATTS;
    }
    ncap->ndefined = (int)tmp;

    if (trace) {
        if (!strcmp(loc, "Global"))
            printf("\ngatt_list (global attribute list):\n");
        else
            printf("\t\tvatt_list (variable attribute list):\n");
    }

    if (ncap->ndefined == 0) {
        /* no attribute defined */
        /* From the CDF file format specification, the tag is either
         * NC_ATTRIBUTE or ABSENT (ZERO), but we follow NetCDF library to skip
         * checking the tag when ndefined is zero.
         */
        if (trace) {
            if (strcmp(loc, "Global")) printf("\t\t");
            printf("\ttag = ABSENT (no attribute defined)\n");
        }
        return NC_NOERR;
#if 0
        if (tag != ABSENT) {
            if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
            if (verbose) printf("\tInvalid NC component tag, while ABSENT is expected for ");
            DEBUG_RETURN_ERROR(NC_ENOTNC)
        }
#endif
    } else {
        sprintf(xloc, "%s attribute", loc);
        if (tag != NC_ATTRIBUTE) {
            if (verbose) printf("Error @ [0x%8.8zx]:\n", tag_err_addr);
            if (verbose) printf("\t%s: Invalid NC component tag (%d), expecting NC_ATTRIBUTE (%d)\n",xloc,tag,NC_ATTRIBUTE);
            DEBUG_RETURN_ERROR(NC_ENOTNC)
        }
        if (trace) {
            if (strcmp(loc, "Global")) printf("\t\t");
            printf("\ttag = NC_ATTRIBUTE\n");
            if (strcmp(loc, "Global")) printf("\t\t");
            printf("\tnumber of attributes = %d\n", ncap->ndefined);
        }

        size_t alloc_size = (size_t)ncap->ndefined + NC_ARRAY_GROWBY;
        ncap->value = (NC_attr **) calloc(alloc_size, sizeof(NC_attr *));
        if (ncap->value == NULL) DEBUG_RETURN_ERROR(NC_ENOMEM)

        for (i=0; i<ncap->ndefined; i++) {
            err = val_get_NC_attr(fd, gbp, &ncap->value[i], xloc);
            if (err != NC_NOERR && err != NC_ENULLPAD) {
                ncap->ndefined = i;
                free_NC_attrarray(ncap);
                return err;
            }
            if (status == NC_NOERR) status = err;
            if (trace) {
                if (strcmp(loc, "Global")) printf("\t\t");
                printf("\tattribute name \"%s\", type = %s, length = %lld\n",
                       ncap->value[i]->name, str_NC_type(ncap->value[i]->xtype),
                       ncap->value[i]->nelems);
            }
        }
    }

    return status;
}

/*----< ncmpio_new_NC_var() >------------------------------------------------*/
static NC_var *
val_new_NC_var(char *name, int ndims)
{
    NC_var *varp;

    varp = (NC_var *) calloc(1, sizeof(NC_var));
    if (varp == NULL) return NULL;

    if (ndims > 0) {
        varp->shape  = (long long*)calloc(ndims, sizeof(long long));
        varp->dsizes = (long long*)calloc(ndims, sizeof(long long));
        varp->dimids = (int *)     calloc(ndims, sizeof(int));
    }

    varp->name     = name;
    varp->name_len = strlen(name);
    varp->ndims    = ndims;
    varp->xsz      = 0;
    varp->len      = 0;
    varp->begin    = 0;

    return varp;
}

/*----< len_nctype() >-------------------------------------------------------*/
static int
len_nctype(nc_type type) {
    switch(type) {
        case NC_BYTE:
        case NC_CHAR:
        case NC_UBYTE:  return 1;
        case NC_SHORT:
        case NC_USHORT: return 2;
        case NC_INT:
        case NC_UINT:
        case NC_FLOAT:  return 4;
        case NC_DOUBLE:
        case NC_INT64:
        case NC_UINT64: return 8;
        default: assert("len_nctype bad type" == 0);
    }
    return 0;
}

/*----< val_get_NC_var() >---------------------------------------------------*/
static int
val_get_NC_var(int          fd,
               bufferinfo  *gbp,
               NC_var     **varpp,
               int          f_ndims, /* no. dimensions defined in file */
               const char  *loc)
{
    /* netCDF file format:
     * netcdf_file = header data
     * header      = magic numrecs dim_list gatt_list var_list
     *  ...
     * var         = name nelems [dimid ...] vatt_list nc_type vsize begin
     * nelems      = NON_NEG
     * dimid       = NON_NEG
     * vatt_list   = att_list
     * nc_type     = NC_BYTE | NC_CHAR | NC_SHORT | ...
     * vsize       = NON_NEG
     * begin       = OFFSET        // Variable start location.
     * OFFSET      = <non-negative INT> |  // CDF-1
     *               <non-negative INT64>  // CDF-2 and CDF-5
     * NON_NEG     = <non-negative INT> |  // CDF-1 and CDF-2
     *               <non-negative INT64>  // CDF-5
     */
    char *name=NULL, xloc[1024];
    int dim, dimid, err, status=NC_NOERR;
    size_t err_addr;
    long long ndims;
    NC_var *varp;

    /* read variable name */
    err = hdr_get_name(fd, gbp, &name, loc);
    if (err != NC_NOERR && err != NC_ENULLPAD) {
        if (name != NULL) free(name);
        return err;
    }
    status = err;

    if (trace) printf("\t\tname = \"%s\"\n", name);

    /* read number of dimensions */
    sprintf(xloc,"%s \"%s\"",loc,name);
    err_addr = ERR_ADDR;
    err = hdr_get_NON_NEG(fd, gbp, &ndims);
    if (err != NC_NOERR) {
        if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
        if (verbose) printf("\t%s: Failed to read number of dimensions\n",xloc);
        if (name != NULL) free(name);
        return err;
    }
    if (status == NC_NOERR) status = err;

    /* cannot be more than NC_MAX_VAR_DIMS */
    if (ndims > NC_MAX_VAR_DIMS) {
        if (verbose) printf("Error:\n");
        if (verbose) printf("\t%s: number of dimensions (%lld) larger than NC_MAX_VAR_DIMS (%d)\n",xloc,ndims,NC_MAX_VAR_DIMS);
        if (name != NULL) free(name);
        DEBUG_RETURN_ERROR(NC_EMAXDIMS)
    }

    if (trace) printf("\t\tnumber of dimensions = %lld\n", ndims);

    /* allocate variable object */
    varp = val_new_NC_var(name, ndims);
    if (varp == NULL) {
        if (name != NULL) free(name);
        DEBUG_RETURN_ERROR(NC_ENOMEM)
    }

    if (trace && ndims > 0) printf("\t\tdimension IDs:");

    /* read dimension IDs and check dimensions */
    for (dim=0; dim<ndims; dim++) {
        err = val_check_buffer(fd, gbp, (gbp->version < 5 ? 4 : 8));
        if (err != NC_NOERR) {
            if (trace) printf("\n");
            if (verbose) printf("Error @ [0x%8.8zx]:\n", (size_t)ERR_ADDR);
            if (verbose) printf("\t%s: Fail to read dimid[%d]\n",xloc,dim);
            free_NC_var(varp);
            return err;
        }
        err_addr = ERR_ADDR;
        if (gbp->version < 5)
            dimid = (int) get_uint32(gbp);
        else
            dimid = (int) get_uint64(gbp);

        if (trace) printf(" %d", dimid);

        /* dimid should be < f_ndims (num of dimensions defined in file) */
        if (dimid >= f_ndims) {
            if (trace) printf("\n");
            if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
            if (verbose) printf("\t%s \"%s\": dimid[%d]=%d is larger than the number of dimensions defined in file (%d)\n",loc,name,dim,dimid,f_ndims);
            free_NC_var(varp);
            DEBUG_RETURN_ERROR(NC_EBADDIM)
        }
        varp->dimids[dim] = dimid;
    }
    if (trace && ndims > 0) printf("\n");

    /* var = name nelems [dimid ...] vatt_list nc_type vsize begin
     *                               ^^^^^^^^^                     */
    sprintf(xloc,"%s \"%s\"",loc,name);
    err = val_get_NC_attrarray(fd, gbp, &varp->attrs, xloc);
    if (err != NC_NOERR && err != NC_ENULLPAD) {
        free_NC_var(varp);
        return err;
    }
    if (status == NC_NOERR) status = err;

    /* var = name nelems [dimid ...] vatt_list nc_type vsize begin
     *                                         ^^^^^^^             */
    err = val_get_nc_type(fd, gbp, &varp->xtype, xloc);
    if (err != NC_NOERR) {
        free_NC_var(varp);
        return err;
    }
    if (trace) printf("\t\tdata type: %s\n", str_NC_type(varp->xtype));

    varp->xsz = len_nctype(varp->xtype);

    /* var = name nelems [dimid ...] vatt_list nc_type vsize begin
     *                                                 ^^^^^
     * instead of use vsize from file, we recalculate it in
     * compute_var_shape() */
    err_addr = ERR_ADDR;
    err = hdr_get_NON_NEG(fd, gbp, &varp->len);
    if (err != NC_NOERR) {
        if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
        if (verbose) printf("\t%s: Failed to read vsize\n",xloc);
        free_NC_var(varp);
        return err;
    }
    if (trace) printf("\t\tvariable size (vsize): %lld\n", varp->len);

    err = val_check_buffer(fd, gbp, (gbp->version == 1 ? 4 : 8));
    if (err != NC_NOERR) {
        if (verbose) printf("Error @ [0x%8.8zx]:\n", (size_t)ERR_ADDR);
        if (verbose) printf("\t%s: Fail to read begin\n",xloc);
        free_NC_var(varp);
        return err;
    }
    /* var = name nelems [dimid ...] vatt_list nc_type vsize begin
     *                                                       ^^^^^ */
    if (gbp->version == 1)
        varp->begin = (long long) get_uint32(gbp);
    else
        varp->begin = (long long) get_uint64(gbp);

    if (trace) printf("\t\tstarting file offset (begin): %lld\n", varp->begin);

    *varpp = varp;
    return status;
}

static int
val_get_NC_vararray(int          fd,
                    bufferinfo  *gbp,
                    NC_vararray *ncap,
                    int          f_ndims) /* no. dimensions defined in file */

{
    /* netCDF file format:
     * netcdf_file = header  data
     * header      = magic  numrecs  dim_list  gatt_list  var_list
     *  ...
     * var_list    = ABSENT | NC_VARIABLE   nelems  [var ...]
     * ABSENT      = ZERO  ZERO |  // list is not present for CDF-1 and 2
     *               ZERO  ZERO64  // for CDF-5
     * ZERO        = \x00 \x00 \x00 \x00                      // 32-bit zero
     * ZERO64      = \x00 \x00 \x00 \x00 \x00 \x00 \x00 \x00  // 64-bit zero
     * NC_VARIABLE = \x00 \x00 \x00 \x0B         // tag for list of variables
     * nelems      = NON_NEG       // number of elements in following sequence
     * NON_NEG     = <non-negative INT> |        // CDF-1 and CDF-2
     *               <non-negative INT64>        // CDF-5
     */
    int var, err, status=NC_NOERR;
    NC_tag tag = NC_UNSPECIFIED;
    long long tmp;
    size_t tag_err_addr, nelems_err_addr;

    assert(gbp != NULL && gbp->pos != NULL);
    assert(ncap != NULL);
    assert(ncap->value == NULL);

    /* read NC_tag (NC_VARIABLE or ZERO) from gbp buffer */
    tag_err_addr = ERR_ADDR;
    err = val_get_NC_tag(fd, gbp, &tag, "tag NC_VARIABLE");
    if (err != NC_NOERR) return err;

    /* read nelems (number of variables) from gbp buffer */
    nelems_err_addr = ERR_ADDR;
    err = hdr_get_NON_NEG(fd, gbp, &tmp);
    if (err != NC_NOERR) {
        if (verbose) printf("Error @ [0x%8.8zx]:\n", nelems_err_addr);
        if (verbose) printf("\tFailed to read tag NC_VARIABLE\n");
        return err;
    }
    if (tmp > NC_MAX_VARS) {
        /* number of allowable defined variables NC_MAX_VARS */
        if (verbose) printf("Error @ [0x%8.8zx]:\n", nelems_err_addr);
        if (verbose) printf("\tNumber of variables (%lld) exceeds NC_MAX_VARS (%d)\n",tmp,NC_MAX_VARS);
        DEBUG_RETURN_ERROR(NC_EMAXVARS);
    }
    ncap->ndefined = (int)tmp;

    if (trace) printf("\nvar_list (variable list):\n");

    if (ncap->ndefined == 0) {
        /* From the CDF file format specification, the tag is either
         * NC_VARIABLE or ABSENT (ZERO), but we follow NetCDF library to skip
         * checking the tag when ndefined is zero.
         */
        if (trace) printf("\ntag = ABSENT (no variable defined)\n");
        return NC_NOERR;
#if 0
        if (tag != ABSENT) {
            if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
            if (verbose) printf("\tInvalid NC component tag, while ABSENT is expected for ");
            DEBUG_RETURN_ERROR(NC_ENOTNC)
        }
#endif
    } else {
        if (tag != NC_VARIABLE) {
            if (verbose) printf("Error @ [0x%8.8zx]:\n", tag_err_addr);
            if (verbose) printf("\tInvalid NC component tag (%d), expecting NC_VARIABLE (%d)\n",tag,NC_VARIABLE);
            DEBUG_RETURN_ERROR(NC_ENOTNC)
        }
        if (trace) printf("\ttag = NC_VARIABLE\n");
        if (trace) printf("\tnumber of variables = %d\n", ncap->ndefined);

        size_t alloc_size = (size_t)ncap->ndefined + NC_ARRAY_GROWBY;
        ncap->value = (NC_var **) calloc(alloc_size, sizeof(NC_var *));
        if (ncap->value == NULL) DEBUG_RETURN_ERROR(NC_ENOMEM)

        for (var=0; var<ncap->ndefined; var++) {
            if (trace) printf("\n\tvariable ID = %d\n", var);
            err = val_get_NC_var(fd, gbp, &ncap->value[var], f_ndims, "Variable");
            if (err != NC_NOERR && err != NC_ENULLPAD) {
                ncap->ndefined = var;
                free_NC_vararray(ncap);
                return err;
            }
            if (status == NC_NOERR) status = err;
        }
    }

    return status;
}

/*----< NC_check_vlen() >----------------------------------------------------*/
/* Check whether variable size is less than or equal to vlen_max,
 * without overflowing in arithmetic calculations.  If OK, return 1,
 * else, return 0.  For CDF1 format or for CDF2 format on non-LFS
 * platforms, vlen_max should be 2^31 - 4, but for CDF2 format on
 * systems with LFS it should be 2^32 - 4.
 */
static int
NC_check_vlen(NC_var     *varp,
              long long  vlen_max)
{
    int i;
    long long prod=varp->xsz;     /* product of xsz and dimensions so far */

    for (i = IS_RECVAR(varp) ? 1 : 0; i < varp->ndims; i++) {
        if (varp->shape[i] > vlen_max / prod) {
            return 0;           /* size in bytes won't fit in a 32-bit int */
        }
        prod *= varp->shape[i];
    }
    return 1;
}

/*
 * Given a valid ncp, check all variables for their sizes against the maximal
 * allowable sizes. Different CDF formation versions have different maximal
 * sizes. This function returns NC_EVARSIZE if any variable has a bad len
 * (product of non-rec dim sizes too large), else return NC_NOERR.
 */
static int
val_NC_check_vlens(NC *ncp)
{
    NC_var **vpp;
    /* maximum permitted variable size (or size of one record's worth
       of a record variable) in bytes.  This is different for format 1
       and format 2. */
    long long ii, vlen_max, rec_vars_count;
    long long large_fix_vars_count, large_rec_vars_count;
    int last = 0;

    if (ncp->vars.ndefined == 0)
        return NC_NOERR;

    if (ncp->format >= 5) /* CDF-5 */
        vlen_max = X_INT64_MAX - 3; /* "- 3" handles rounded-up size */
    else if (ncp->format == 2) /* CDF2 format */
        vlen_max = X_UINT_MAX  - 3; /* "- 3" handles rounded-up size */
    else
        vlen_max = X_INT_MAX   - 3; /* CDF1 format */

    /* Loop through vars, first pass is for non-record variables */
    large_fix_vars_count = 0;
    rec_vars_count = 0;
    vpp = ncp->vars.value;
    for (ii = 0; ii < ncp->vars.ndefined; ii++, vpp++) {
        if (!IS_RECVAR(*vpp)) {
            last = 0;
            if (NC_check_vlen(*vpp, vlen_max) == 0) {
                /* check this variable's shape product against vlen_max */
                if (ncp->format >= 5) { /* CDF-5 */
                    if (verbose) printf("Error:\n");
                    if (verbose) printf("\tvar %s: variable size greater than max (%lld) allowable by CDF-%d\n",(*vpp)->name,vlen_max,ncp->format);
                    DEBUG_RETURN_ERROR(NC_EVARSIZE)
                }
                large_fix_vars_count++;
                last = 1;
            }
        } else {
            rec_vars_count++;
        }
    }
    /* OK if last non-record variable size too large, since not used to
       compute an offset */
    if (large_fix_vars_count > 1) {  /* only one "too-large" variable allowed */
        if (verbose) printf("Error:\n");
        if (verbose) printf("\tCDF-%d format allows only one large fixed-size variable\n",ncp->format);
        DEBUG_RETURN_ERROR(NC_EVARSIZE)
    }

    /* The only "too-large" variable must be the last one defined */
    if (large_fix_vars_count == 1 && last == 0) {
        if (verbose) printf("Error:\n");
        if (verbose) printf("\tCDF-%d format allows only one large fixed-size variable\n",ncp->format);
        DEBUG_RETURN_ERROR(NC_EVARSIZE)
    }

    if (rec_vars_count == 0) return NC_NOERR;

    /* if there is a "too-large" fixed-size variable, no record variable is
     * allowed */
    if (large_fix_vars_count == 1) {
        if (verbose) printf("Error:\n");
        if (verbose) printf("\tCDF-%d format allows only one large fixed-size variable when there is no record variable defined\n",ncp->format);
        DEBUG_RETURN_ERROR(NC_EVARSIZE)
    }

    /* Loop through vars, second pass is for record variables.   */
    large_rec_vars_count = 0;
    vpp = ncp->vars.value;
    for (ii = 0; ii < ncp->vars.ndefined; ii++, vpp++) {
        if (IS_RECVAR(*vpp)) {
            last = 0;
            if (NC_check_vlen(*vpp, vlen_max) == 0) {
                /* check this variable's shape product against vlen_max */
                if (ncp->format >= 5) { /* CDF-5 */
                    if (verbose) printf("Error:\n");
                    if (verbose) printf("\tvar %s: variable size greater than max (%lld) allowable by CDF-%d\n",(*vpp)->name,vlen_max,ncp->format);
                    DEBUG_RETURN_ERROR(NC_EVARSIZE)
                }
                large_rec_vars_count++;
                last = 1;
            }
        }
    }

    /* For CDF-2, no record variable can require more than 2^32 - 4 bytes of
     * storage for each record's worth of data, unless it is the last record
     * variable. See
     * http://www.unidata.ucar.edu/software/netcdf/docs/file_structure_and_performance.html#offset_format_limitations
     */
    if (large_rec_vars_count > 1) { /* only one "too-large" variable allowed */
        if (verbose) printf("Error:\n");
        if (verbose) printf("\tCDF-%d format allows only one large record variable\n",ncp->format);
        DEBUG_RETURN_ERROR(NC_EVARSIZE)
    }

    /* and it has to be the last one */
    if (large_rec_vars_count == 1 && last == 0) {
        if (verbose) printf("Error:\n");
        if (verbose) printf("\tCDF-%d format allows only one large record variable and it must be the last one defined\n",ncp->format);
        DEBUG_RETURN_ERROR(NC_EVARSIZE)
    }

    return NC_NOERR;
}

/*
 * Given a valid ncp, check all variables for their begins whether in an
 * increasing order.
 */
static int
val_NC_check_voff(NC *ncp)
{
    int nerrs=0, status=NC_NOERR;
    NC_var *varp;
    long long i, prev, prev_off;

    if (ncp->vars.ndefined == 0) return NC_NOERR;

    /* Loop through vars, first pass is for non-record variables */
    prev_off = ncp->begin_var;
    prev     = 0;
    for (i=0; i<ncp->vars.ndefined; i++) {
        varp = ncp->vars.value[i];
        if (IS_RECVAR(varp)) continue;

        if (varp->begin < prev_off) {
            if (verbose) {
                printf("Error - variable begin offset orders:\n");
                if (i == 0)
                    printf("\tvar \"%s\" begin offset (%lld) is less than header extent (%lld)\n", varp->name, varp->begin, prev_off);
                else
                    printf("\tvar \"%s\" begin offset (%lld) is less than previous variable \"%s\" end offset (%lld)\n", varp->name, varp->begin, ncp->vars.value[prev]->name, prev_off);
            }
            nerrs++;
            DEBUG_ASSIGN_ERROR(status, NC_ENOTNC)
        }
        prev_off = varp->begin + varp->len;
        prev = i;
    }

    if (ncp->begin_rec < prev_off) {
        if (verbose) printf("Error:\n");
        if (verbose) printf("\tRecord variable section begin offset (%lld) is less than fixed-size variable section end offset (%lld)\n", varp->begin, prev_off);
        nerrs++;
        DEBUG_ASSIGN_ERROR(status, NC_ENOTNC)
    }

    /* Loop through vars, second pass is for record variables */
    prev_off = ncp->begin_rec;
    prev     = 0;
    for (i=0; i<ncp->vars.ndefined; i++) {
        varp = ncp->vars.value[i];
        if (!IS_RECVAR(varp)) continue;

        if (varp->begin < prev_off) {
            if (verbose) {
                printf("Error:\n");
                if (i == 0)
                    printf("Variable \"%s\" begin offset (%lld) is less than record variable section begin offset (%lld)\n", varp->name, varp->begin, prev_off);
                else
                    printf("Variable \"%s\" begin offset (%lld) is less than previous variable \"%s\" end offset (%lld)\n", varp->name, varp->begin, ncp->vars.value[prev]->name, prev_off);
            }
            nerrs++;
            DEBUG_ASSIGN_ERROR(status, NC_ENOTNC)
        }
        prev_off = varp->begin + varp->len;
        prev = i;
    }

    return status;
}

static int
val_get_NC(int fd, NC *ncp)
{
    int err, status=NC_NOERR;
    bufferinfo getbuf;
    char magic[5];
    size_t err_addr, pos_addr, base_addr;
    const char *hdf5_signature="\211HDF\r\n\032\n";

    /* find Endianness of the running machine */
    getbuf.is_little_endian = check_little_endian();

    /* Initialize the get buffer that stores the header read from the file */
    getbuf.offset = 0;     /* read from start of the file */

    /* CDF-5's minimum header size is 4 bytes more than CDF-1 and CDF-2's */
    getbuf.size = NC_DEFAULT_CHUNKSIZE;
    getbuf.pos  = getbuf.base = (void *)malloc(getbuf.size);

    /* Fetch the next header chunk. The chunk is 'gbp->size' bytes big
     * netcdf_file = header data
     * header      = magic numrecs dim_list gatt_list var_list
     */
    status = val_fetch(fd, &getbuf);
    if (status != NC_NOERR) goto fn_exit;

    /* Check HDF file signature */
    if (memcmp(getbuf.base, hdf5_signature, 8) == 0) {
        if (verbose) {
            printf("Error: Input file is in HDF format\n");
            printf("       ncvalidator only validates NetCDF classic files\n");
        }
        status = NC_ENOTSUPPORT;
        goto fn_exit;
    }

    /* Check classic CDF file signature */
    magic[4] = '\0';
    memcpy(magic, getbuf.base, 4);
    getbuf.pos = (char*)getbuf.pos + 4;

    if (memcmp(magic, ncmagic, 3) != 0) {
        if (verbose) printf("Error: Unknown file signature\n");
        if (verbose) printf("\tExpecting \"CDF1\", \"CDF2\", or \"CDF5\", but got \"%4s\"\n",magic);
        status = NC_ENOTNC;
        goto fn_exit;
    }

    /* check version number in last byte of magic */
    if (magic[3] == 0x1) {
        getbuf.version = 1;
        ncp->format = 1;
    } else if (magic[3] == 0x2) {
        getbuf.version = 2;
        ncp->format = 2;
    } else if (magic[3] == 0x5) {
        getbuf.version = 5;
        ncp->format = 5;
    } else {
        if (verbose) printf("Error: Unknown file signature\n");
        if (verbose) printf("\tExpecting \"CDF1\", \"CDF2\", or \"CDF5\", but got \"%4s\"\n",magic);
        status = NC_ENOTNC;
        goto fn_exit;
    }
    if (trace) printf("magic = %d (file format: CDF-%d)\n", ncp->format,ncp->format);

    /* header = magic numrecs dim_list gatt_list var_list
     * Check numrecs
     */
    err_addr = 4;
    status = val_check_buffer(fd, &getbuf, (getbuf.version < 5) ? 4 : 8);
    if (status != NC_NOERR) {
        if (verbose) printf("Error @ [0x%8.8zx]:\n", err_addr);
        if (verbose) printf("\tFailed to read the number of records\n");
        status = NC_ENOTNC;
        goto fn_exit;
    }

    /* get numrecs from getbuf into ncp */
    if (getbuf.version < 5)
        ncp->numrecs = (long long) get_uint32(&getbuf);
    else
        ncp->numrecs = (long long) get_uint64(&getbuf);

    if (trace) printf("\nnumrecs = %lld (number of records)\n", ncp->numrecs);

    pos_addr  = (size_t) getbuf.pos;
    base_addr = (size_t) getbuf.base;
    assert(pos_addr < base_addr + getbuf.size);

    /* header = magic numrecs dim_list gatt_list var_list
     * dim_list = ABSENT | NC_DIMENSION  nelems  [dim ...]
     * Check dim_list
     */
    err = val_get_NC_dimarray(fd, &getbuf, &ncp->dims, ncp->numrecs);
    if (err != NC_NOERR && err != NC_ENULLPAD) {
        status = err;
        goto fn_exit;
    }
    if (status == NC_NOERR) status = err;

    /* header = magic numrecs dim_list gatt_list var_list
     * att_list = ABSENT | NC_ATTRIBUTE  nelems  [attr ...]
     * Check att_list
     */
    err = val_get_NC_attrarray(fd, &getbuf, &ncp->attrs, "Global");
    if (err != NC_NOERR && err != NC_ENULLPAD) {
        status = err;
        goto fn_exit;
    }
    if (status == NC_NOERR) status = err;

    /* header = magic numrecs dim_list gatt_list var_list
     * var_list    = ABSENT | NC_VARIABLE   nelems  [var ...]
     * Check var_list
     */
    err = val_get_NC_vararray(fd, &getbuf, &ncp->vars, ncp->dims.ndefined);
    if (err != NC_NOERR && err != NC_ENULLPAD) {
        status = err;
        goto fn_exit;
    }
    if (status == NC_NOERR) status = err;

    if (trace) printf("\n");

    /* get file header size */
    ncp->xsz = hdr_len_NC(ncp);

    /* Recompute the shapes of all variables */
    err = compute_var_shape(ncp);
    if (err != NC_NOERR) {
        status = err;
        goto fn_exit;
    }

    err = val_NC_check_vlens(ncp);
    if (err != NC_NOERR) {
        status = err;
        goto fn_exit;
    }

    err = val_NC_check_voff(ncp);
    if (err != NC_NOERR) {
        status = err;
        goto fn_exit;
    }

fn_exit:
    free(getbuf.base);

    return status;
}

/* End Of get NC */

static void
usage(char *argv0)
{
    char *help =
    "Usage: %s [-h] | [-t] [-x] [-q] file\n"
    "       [-h] Print help\n"
    "       [-t] Turn on tracing mode, printing progress of validation\n"
    "       [-x] Repair in-place the null-byte padding in file header.\n"
    "       [-q] Quiet mode (exit 1 when fail, 0 success)\n"
    "       file: Input netCDF file name\n"
    "*PnetCDF library version PNETCDF_RELEASE_VERSION of PNETCDF_RELEASE_DATE\n";
    fprintf(stderr, help, argv0);
}

int main(int argc, char **argv)
{
    char filename[512], *path;
    int i, omode, fd, status=NC_NOERR;
    NC *ncp=NULL;
    struct stat ncfilestat;

    /* get command-line arguments */
    verbose = 1;
    trace = 0;
    repair  = 0;
    while ((i = getopt(argc, argv, "xthq")) != EOF)
        switch(i) {
            case 'x': repair = 1;
                      break;
            case 't': trace = 1;
                      break;
            case 'q': verbose = 0;
                      break;
            case 'h':
            default:  usage(argv[0]);
                      return 1;
        }

    if (argv[optind] == NULL) { /* input file name is mandatory */
        usage(argv[0]);
        return 1;
    }

    snprintf(filename, 512, "%s", argv[optind]);

    /* remove the file system type prefix name if there is any.
     * For example, when filename = "lustre:/home/foo/testfile.nc", remove
     * "lustre:" to make path = "/home/foo/testfile.nc" in open() below
     */
    path = strchr(filename, ':');
    if (path == NULL) path = filename; /* no prefix */
    else              path++;

    if (repair) omode = O_RDWR;
    else        omode = O_RDONLY;

    fd = open(path, omode);
    if (fd == -1) {
        fprintf(stderr, "Error on open file %s (%s)\n",
                filename,strerror(errno));
        return 1;
    }

    /* Allocate NC object */
    ncp = (NC*) calloc(1, sizeof(NC));
    if (ncp == NULL) {
        status = NC_ENOMEM;
        if (verbose) printf("Error at line %d when calling calloc()\n",__LINE__);
        goto prog_exit;
    }

    /* read and validate the header */
    status = val_get_NC(fd, ncp);
    if (status != NC_NOERR && status != NC_ENULLPAD && status != -1)
        goto prog_exit;

    /* check data size */
    if (-1 == fstat(fd, &ncfilestat)) {
        if (verbose) printf("Error at line %d fstat (%s)\n",__LINE__,strerror(errno));
        status = NC_EFILE;
        goto prog_exit;
    }
    if (ncp->numrecs > 0) {
        long long expect_fsize;
        expect_fsize = ncp->begin_rec + ncp->recsize * ncp->numrecs;
        if (expect_fsize < ncfilestat.st_size) {
            if (verbose) printf("Error: file size (%lld) is larger than expected (%lld)!\n",(long long)ncfilestat.st_size, expect_fsize);
            if (verbose) printf("\tbegin_rec=%lld recsize=%lld numrecs=%lld ncfilestat.st_size=%lld\n",ncp->begin_rec, ncp->recsize, ncp->numrecs, (long long) ncfilestat.st_size);
            status = NC_EFILE;
            goto prog_exit;
        }
        else if (expect_fsize > ncfilestat.st_size) {
            /* if file header are valid and the only error is the file size
             * less than expected, then this is due to partial data written
             * to the variable while the file is in no fill mode */
            if (verbose) {
                printf("Warning:\n");
                printf("\tfile size (%lld) is less than expected (%lld)!\n",(long long)ncfilestat.st_size, expect_fsize);
            }
        }
    }
    else {
        long long expect_fsize;
        if (ncp->vars.ndefined == 0)
            expect_fsize = ncp->xsz;
        else
            /* find the size of last fixed-size variable */
            expect_fsize = ncp->vars.value[ncp->vars.ndefined-1]->begin +
                           ncp->vars.value[ncp->vars.ndefined-1]->len;
        if (expect_fsize < ncfilestat.st_size) {
            if (verbose) printf("Error:\n");
            if (verbose) printf("\tfile size (%lld) is larger than expected (%lld)!\n",(long long)ncfilestat.st_size, expect_fsize);
            status = NC_EFILE;
            goto prog_exit;
        }
        else if (expect_fsize > ncfilestat.st_size) {
            /* if file header are valid and the only error is the file size
             * less than expected, then this is due to partial data written
             * to the variable while the file is in no fill mode */
            if (verbose) printf("Warning:\n");
            if (verbose) printf("\tfile size (%lld) is less than expected (%lld)!\n",(long long)ncfilestat.st_size, expect_fsize);
        }
    }

prog_exit:
    if (ncp != NULL) {
        free_NC_dimarray(&ncp->dims);
        free_NC_attrarray(&ncp->attrs);
        free_NC_vararray(&ncp->vars);
        free(ncp);
    }
    close(fd);

    if (verbose) {
        if (status == NC_NOERR)
            printf("File \"%s\" is a valid NetCDF classic file.\n",filename);
        else {
            printf("File \"%s\" fails to conform with classic CDF file format specifications\n",filename);
            if (repair) {
                printf("and it has been repaired in place to remove the errors.\n");
                printf("Please run \"%s %s\" to validate again.\n",argv[0],filename);
            }
        }
    }

    exit((status == NC_NOERR) ? EXIT_SUCCESS : EXIT_FAILURE);
}
