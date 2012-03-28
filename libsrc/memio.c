/*
 *	Copyright 1996, University Corporation for Atmospheric Research
 *	See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "config.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#ifdef _MSC_VER /* Microsoft Compilers */
#include <io.h>
#else
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include "nc.h"

#ifndef HAVE_SSIZE_T
#define ssize_t int
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#include "ncio.h"
#include "fbits.h"
#include "rnd.h"

/* #define INSTRUMENT 1 */
#if INSTRUMENT /* debugging */
#undef NDEBUG
#include <stdio.h>
#include "instr.h"
#endif

#undef MIN  /* system may define MIN somewhere and complain */
#define MIN(mm,nn) (((mm) < (nn)) ? (mm) : (nn))

#if !defined(NDEBUG) && !defined(X_INT_MAX)
#define  X_INT_MAX 2147483647
#endif

#if 0 /* !defined(NDEBUG) && !defined(X_ALIGN) */
#define  X_ALIGN 4
#else
#undef X_ALIGN
#endif

/* Define the amount by which memory is incremented on realloc */
#define DEFAULT_BLOCKSIZE 4096

#define TACTIC_INCR 1
#define TACTIC_DOUBLE 2
#define TACTIC TACTIC_DOUBLE

/* Private data for memio */

typedef struct NCMEMIO {
    int locked; /* => we cannot realloc */
    int persist; /* => save to a file; triggered by NC_WRITE */
    char* memory;
    off_t alloc;
    off_t size;
    off_t pos;
} NCMEMIO;

/* Forward */
static int memio_rel(ncio *const nciop, off_t offset, int rflags);
static int memio_get(ncio *const nciop, off_t offset, size_t extent, int rflags, void **const vpp);
static int memio_move(ncio *const nciop, off_t to, off_t from, size_t nbytes, int rflags);
static int memio_sync(ncio *const nciop);
static int memio_filesize(ncio* nciop, off_t* filesizep);
static int memio_pad_length(ncio* nciop, off_t length);
static int memio_close(ncio* nciop, int);
static void memio_free(void *const pvt);

/* Create a new ncio struct to hold info about the file. */
static ncio* 
memio_new(const char* filepath, int ioflags, size_t initialsize, int persist)
{
    ncio* nciop = NULL;
    NCMEMIO* memio = NULL;

    nciop = (ncio* )calloc(1,sizeof(ncio));
    if(nciop == NULL)
	goto fail;
    
    nciop->ioflags = ioflags;
    *((int*)&nciop->fd) = -1;

    *((char**)&nciop->path) = strdup(filepath);
    if(nciop->path == NULL) goto fail;

    *((ncio_relfunc**)&nciop->rel) = memio_rel;
    *((ncio_getfunc**)&nciop->get) = memio_get;
    *((ncio_movefunc**)&nciop->move) = memio_move;
    *((ncio_syncfunc**)&nciop->sync) = memio_sync;
    *((ncio_filesizefunc**)&nciop->filesize) = memio_filesize;
    *((ncio_pad_lengthfunc**)&nciop->pad_length) = memio_pad_length;
    *((ncio_closefunc**)&nciop->close) = memio_close;

    memio = (NCMEMIO*)calloc(1,sizeof(NCMEMIO));
    if(memio == NULL) goto fail;
    *((void* *)&nciop->pvt) = memio;

    memio->alloc = initialsize;
    if(memio->alloc < DEFAULT_BLOCKSIZE)
	memio->alloc = DEFAULT_BLOCKSIZE;
    memio->size = 0;
    memio->pos = 0;
    memio->memory = (char*)malloc(memio->alloc);
    if(memio->memory == NULL) goto fail;
    memio->persist = persist;

    return nciop;
fail:
    if(nciop != NULL) {
        if(nciop->path != NULL) free((char*)nciop->path);
        if(memio != NULL) {
        }
    }
    return NULL;
}

/* Create a file, and the ncio struct to go with it. This function is
   only called from nc__create_mp.

   path - path of file to create.
   ioflags - flags from nc_create
   initialsz - From the netcdf man page: "The argument
   Iinitialsize sets the initial size of the file at creation time."
   igeto - 
   igetsz - 
   sizehintp - the size of a page of data for buffered reads and writes.
   nciopp - pointer to a pointer that will get location of newly
   created and inited ncio struct.
   mempp - pointer to pointer to the initial memory read.
*/
int
memio_create(const char* path, int ioflags,
    size_t initialsz,
    off_t igeto, size_t igetsz, size_t* sizehintp,
    ncio* *nciopp, void** const mempp)
{
    ncio* nciop;
    int fd;
    int status;
    int persist = (ioflags & NC_WRITE?1:0);

    if(path == NULL ||* path == 0)
        return NC_EINVAL;

    fSet(ioflags, NC_WRITE);

    nciop = memio_new(path, ioflags, initialsz, persist);
    if(nciop == NULL)
        return NC_ENOMEM;

    fd = nc__pseudofd();
    *((int* )&nciop->fd) = fd; 

    if(igetsz != 0)
    {
        status = nciop->get(nciop,
                igeto, igetsz,
                RGN_WRITE,
                mempp);
        if(status != NC_NOERR)
            goto unwind_open;
    }

    /* Pick a default sizehint */
    if(sizehintp) *sizehintp = DEFAULT_BLOCKSIZE;

    *nciopp = nciop;
    return NC_NOERR;

unwind_open:
    memio_close(nciop,1);
    return status;
}

/* This function opens the data file. It is only called from nc.c,
   from nc__open_mp and nc_delete_mp.

   path - path of data file.
   ioflags - flags passed into nc_open.
   igeto - looks like this function can do an initial page get, and
   igeto is going to be the offset for that. But it appears to be
   unused 
   igetsz - the size in bytes of initial page get (a.k.a. extent). Not
   ever used in the library.
   sizehintp - the size of a page of data for buffered reads and writes.
   nciopp - pointer to pointer that will get address of newly created
   and inited ncio struct.
   mempp - pointer to pointer to the initial memory read.
*/
int
memio_open(const char* path,
    int ioflags,
    off_t igeto, size_t igetsz, size_t* sizehintp,
    ncio* *nciopp, void** const mempp)
{
    /* Currrently, diskless open not supported */
#if 1
    return NC_EDISKLESS;
#else
    ncio* nciop;
    int fd;
    int status;

    if(path == NULL ||* path == 0)
        return EINVAL;

    nciop = memio_new(path, ioflags, 0);
    if(nciop == NULL)
        return ENOMEM;

    fd = --pseudofd;
    *((int* )&nciop->fd) = fd; 

    if(igetsz != 0)
    {
        status = nciop->get(nciop,
                igeto, igetsz,
                0,
                mempp);
        if(status != NC_NOERR)
            goto unwind_open;
    }

    /* Pick a default sizehint */
    if(sizehintp) *sizehintp = DEFAULT_BLOCKSIZE;

    *nciopp = nciop;
    return NC_NOERR;

unwind_open:
    memio_close(nciop,1);
    return status;
#endif
}


/* 
 *  Get file size in bytes.
 */
static int
memio_filesize(ncio* nciop, off_t* filesizep)
{
    NCMEMIO* memio;
    if(nciop == NULL || nciop->pvt == NULL) return NC_EINVAL;
    memio = (NCMEMIO*)nciop->pvt;
    if(filesizep != NULL) *filesizep = memio->size;
    return NC_NOERR;
}

/*
 *  Sync any changes to disk, then truncate or extend file so its size
 *  is length.  This is only intended to be called before close, if the
 *  file is open for writing and the actual size does not match the
 *  calculated size, perhaps as the result of having been previously
 *  written in NOFILL mode.
 */
static int
memio_pad_length(ncio* nciop, off_t length)
{
    NCMEMIO* memio;
    if(nciop == NULL || nciop->pvt == NULL) return NC_EINVAL;
    memio = (NCMEMIO*)nciop->pvt;

    if(!fIsSet(nciop->ioflags, NC_WRITE))
        return EPERM; /* attempt to write readonly file*/

    /* Realloc the allocated memory */
    if(memio->locked > 0)
	return NC_EDISKLESS;
    if(length > memio->alloc) {
	off_t actual;
	char* newmem;
	switch(TACTIC) {
	case TACTIC_DOUBLE:
	    actual = (memio->alloc * 2);
	    break;
	case TACTIC_INCR:
	default:
	    actual = length + (length % DEFAULT_BLOCKSIZE);
	    break;
	}
        newmem = (char*)realloc(memio->memory,actual);
	if(newmem == NULL) return NC_ENOMEM;
	memio->memory = newmem;
	/* zero out the extra memory */
        memset((void*)(newmem+memio->alloc),0,(actual - memio->alloc));
	memio->alloc = actual;
    }  
    memio->size = length;
    return NC_NOERR;
}

/* Write out any dirty buffers to disk and
   ensure that next read will get data from disk.
   Sync any changes, then close the open file associated with the ncio
   struct, and free its memory.
   nciop - pointer to ncio to close.
   doUnlink - if true, unlink file
*/

static int 
memio_close(ncio* nciop, int doUnlink)
{
    int status = NC_NOERR;
    NCMEMIO* memio;
    if(nciop == NULL || nciop->pvt == NULL) return NC_NOERR;
    /* See if the user wants the contents persisted to a file */
    memio = (NCMEMIO*)nciop->pvt;
    if(memio != NULL) {
        if(memio->persist) {
	    /* Try to open the file for writing */
	    int fd = open(nciop->path, O_WRONLY|O_CREAT|O_TRUNC, 0666);
	    if(fd >= 0) {
		long count = write(fd, memio->memory, memio->size);
		if(count < 0)
		    status = errno;
		else if(count < memio->size)
		    status = NC_EDISKLESS;
		(void)close(fd);		
	    } else
		status = errno;
	}
        /* Free up things */
        memio_free(memio);
	free(memio);
    }
    /* do cleanup of ncio structure */
    if(nciop->path != NULL) free((char*)nciop->path);
    free(nciop);
    return status;
}

static int
guarantee(ncio* nciop, off_t endpoint)
{
    NCMEMIO* memio = (NCMEMIO*)nciop->pvt;
    if(endpoint >= memio->alloc) {
	/* extend the allocated memory and size */
	int status = memio_pad_length(nciop,endpoint);
	if(status != NC_NOERR) return status;
    }
    if(memio->size < endpoint)
	memio->size = endpoint;
    return NC_NOERR;
}

/*
 * Request that the region (offset, extent)
 * be made available through *vpp.
 */
static int
memio_get(ncio* const nciop, off_t offset, size_t extent, int rflags, void** const vpp)
{
    int status = NC_NOERR;
    NCMEMIO* memio;
    if(nciop == NULL || nciop->pvt == NULL) return NC_EINVAL;
    memio = (NCMEMIO*)nciop->pvt;
    status = guarantee(nciop, offset+extent);
    memio->locked++;
    if(status != NC_NOERR) return status;
    if(vpp) *vpp = memio->memory+offset;
    return NC_NOERR;
}

/*
 * Like memmove(), safely move possibly overlapping data.
 */
static int
memio_move(ncio* const nciop, off_t to, off_t from, size_t nbytes, int ignored)
{
    int status = NC_NOERR;
    NCMEMIO* memio;

    if(nciop == NULL || nciop->pvt == NULL) return NC_EINVAL;
    memio = (NCMEMIO*)nciop->pvt;
    if(from < to) {
       /* extend if "to" is not currently allocated */
       status = guarantee(nciop,to+nbytes);
       if(status != NC_NOERR) return status;
    }
    /* check for overlap */
    if((to + nbytes) > from || (from + nbytes) > to) {
	/* Ranges overlap */
#ifdef HAVE_MEMMOVE
        memmove((void*)(memio->memory+to),(void*)(memio->memory+from),nbytes);
#else
        off_t overlap;
	off_t nbytes1;
        if((from + nbytes) > to) {
	    overlap = ((from + nbytes) - to); /* # bytes of overlap */
	    nbytes1 = (nbytes - overlap); /* # bytes of non-overlap */
	    /* move the non-overlapping part */
            memcpy((void*)(memio->memory+(to+overlap)),
                   (void*)(memio->memory+(from+overlap)),
		   nbytes1);
	    /* move the overlapping part */
	    memcpy((void*)(memio->memory+to),
                   (void*)(memio->memory+from),
		   overlap);
	} else { /*((to + nbytes) > from) */
	    overlap = ((to + nbytes) - from); /* # bytes of overlap */
	    nbytes1 = (nbytes - overlap); /* # bytes of non-overlap */
	    /* move the non-overlapping part */
            memcpy((void*)(memio->memory+to),
                   (void*)(memio->memory+from),
		   nbytes1);
	    /* move the overlapping part */
	    memcpy((void*)(memio->memory+(to+nbytes1)),
                   (void*)(memio->memory+(from+nbytes1)),
		   overlap);
	}
#endif
    } else {/* no overlap */
	memcpy((void*)(memio->memory+to),(void*)(memio->memory+from),nbytes);
    }
    return status;
}

static int
memio_rel(ncio* const nciop, off_t offset, int rflags)
{
    NCMEMIO* memio;
    if(nciop == NULL || nciop->pvt == NULL) return NC_EINVAL;
    memio = (NCMEMIO*)nciop->pvt;
    memio->locked--;
    return NC_NOERR; /* do nothing */
}

/*
 * Write out any dirty buffers to disk and
 * ensure that next read will get data from disk.
 */
static int
memio_sync(ncio* const nciop)
{
    return NC_NOERR; /* do nothing */
}

/*
 * free up anything hanging off pvt;
 */
static void
memio_free(void *const pvt)
{
    if(pvt != NULL) {
        NCMEMIO* memio;
        memio = (NCMEMIO*)pvt;
	if(memio->memory != NULL) free(memio->memory);
    }
}
