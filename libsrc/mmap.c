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

#undef DEBUG

#ifdef DEBUG
#include <stdio.h>
#endif

#include <sys/mman.h>

#ifndef MAP_ANONYMOUS
#  ifdef MAP_ANON
#    define MAP_ANONYMOUS MAP_ANON
#  endif
#endif

/* !MAP_ANONYMOUS => !HAVE_MMAP */
#ifndef MAP_ANONYMOUS
#error mmap not fully implemented: missing MAP_ANONYMOUS
#endif

#ifdef HAVE_MMAP
  /* This is conditionalized by __USE_GNU ; why? */
  extern void *mremap(void*,size_t,size_t,int);
# ifndef MREMAP_MAYMOVE
#   define MREMAP_MAYMOVE 1
# endif
#endif /*HAVE_MMAP*/

#ifndef HAVE_SSIZE_T
#define ssize_t int
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

/* Define the mode flags for create: rw by owner, no access by anyone else */
#define OPENMODE 0600
#define OPENANYMODE 0666

#include "ncio.h"
#include "fbits.h"
#include "rnd.h"

/* #define INSTRUMENT 1 */
#if INSTRUMENT /* debugging */
#undef NDEBUG
#include <stdio.h>
#include "instr.h"
#endif

#ifndef MMAP_MAXBLOCKSIZE
#define MMAP_MAXBLOCKSIZE 268435456 /* sanity check, about X_SIZE_T_MAX/8 */
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

/* Private data for memio */

typedef struct NCMEMIO {
    int locked; /* => we cannot realloc */
    int persist; /* => save to a file; triggered by NC_WRITE */
    char* memory;
    off_t alloc;
    off_t size;
    off_t pos;
    int mapfd;
} NCMEMIO;

/* Forward */
static int mmap_rel(ncio *const nciop, off_t offset, int rflags);
static int mmap_get(ncio *const nciop, off_t offset, size_t extent, int rflags, void **const vpp);
static int mmap_move(ncio *const nciop, off_t to, off_t from, size_t nbytes, int rflags);
static int mmap_sync(ncio *const nciop);
static int mmap_filesize(ncio* nciop, off_t* filesizep);
static int mmap_pad_length(ncio* nciop, off_t length);
static int mmap_close(ncio* nciop, int);

/* Mnemonic */
#define DOOPEN 1

static long pagesize = 0;

/* Create a new ncio struct to hold info about the file. */
static int
mmap_new(const char* path, int ioflags, off_t initialsize, ncio** nciopp, NCMEMIO** memiop)
{
    int status = NC_NOERR;
    ncio* nciop = NULL;
    NCMEMIO* memio = NULL;
    int openfd = -1;

    if(pagesize == 0) {
#if defined HAVE_SYSCONF
        pagesize = sysconf(_SC_PAGE_SIZE);
#elif defined HAVE_GETPAGESIZE
        pagesize = getpagesize();
#else
        pagesize = 4096; /* good guess */
#endif
    }

    errno = 0;

    /* Always force the allocated size to be a multiple of pagesize */
    if(initialsize == 0) initialsize = pagesize;
    if((initialsize % pagesize) != 0)
	initialsize += (pagesize - (initialsize % pagesize));

    nciop = (ncio* )calloc(1,sizeof(ncio));
    if(nciop == NULL) {status = NC_ENOMEM; goto fail;}
    
    nciop->ioflags = ioflags;
    *((int*)&nciop->fd) = -1; /* caller will fix */

    *((char**)&nciop->path) = strdup(path);
    if(nciop->path == NULL) {status = NC_ENOMEM; goto fail;}

    *((ncio_relfunc**)&nciop->rel) = mmap_rel;
    *((ncio_getfunc**)&nciop->get) = mmap_get;
    *((ncio_movefunc**)&nciop->move) = mmap_move;
    *((ncio_syncfunc**)&nciop->sync) = mmap_sync;
    *((ncio_filesizefunc**)&nciop->filesize) = mmap_filesize;
    *((ncio_pad_lengthfunc**)&nciop->pad_length) = mmap_pad_length;
    *((ncio_closefunc**)&nciop->close) = mmap_close;

    memio = (NCMEMIO*)calloc(1,sizeof(NCMEMIO));
    if(memio == NULL) {status = NC_ENOMEM; goto fail;}
    *((void* *)&nciop->pvt) = memio;

    memio->alloc = initialsize;

    memio->memory = NULL;
    memio->size = 0;
    memio->pos = 0;
    memio->persist = fIsSet(ioflags,NC_WRITE);

    /* See if ok to use mmap */
    if(sizeof(void*) < 8 && fIsSet(ioflags,NC_64BIT_OFFSET))
	return NC_DISKLESS; /* cannot support */
    memio->mapfd = -1;

    if(nciopp) *nciopp = nciop;
    if(memiop) *memiop = memio;

done:
    if(openfd >= 0) close(openfd);
    return status;

fail:
    if(nciop != NULL) {
        if(nciop->path != NULL) free((char*)nciop->path);
    }
    goto done;
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
mmap_create(const char* path, int ioflags,
    size_t initialsz,
    off_t igeto, size_t igetsz, size_t* sizehintp,
    ncio* *nciopp, void** const mempp)
{
    ncio* nciop;
    int fd;
    int status;
    NCMEMIO* memio = NULL;
    int persist = (ioflags & NC_WRITE?1:0);
    int oflags;

    if(path == NULL ||* path == 0)
        return NC_EINVAL;

    /* For diskless open has, the file must be classic version 1 or 2.*/
    if(fIsSet(ioflags,NC_NETCDF4))
        return NC_EDISKLESS; /* violates constraints */

    status = mmap_new(path, ioflags, initialsz, &nciop, &memio);
    if(status != NC_NOERR)
        return status;
    memio->size = 0;

    if(!persist) {
        memio->mapfd = -1;
	memio->memory = (char*)mmap(NULL,memio->alloc,
                                    PROT_READ|PROT_WRITE,
				    MAP_PRIVATE|MAP_ANONYMOUS,
                                    memio->mapfd,0);
	{memio->memory[0] = 0;} /* test writing of the mmap'd memory */
    } else { /*persist */
        /* Open the file, but make sure we can write it if needed */
        oflags = (persist ? O_RDWR : O_RDONLY);    
#ifdef O_BINARY
        fSet(oflags, O_BINARY);
#endif
    	oflags |= (O_CREAT|O_TRUNC);
        if(fIsSet(ioflags,NC_NOCLOBBER))
	    oflags |= O_EXCL;
#ifdef vms
        fd = open(path, oflags, 0, "ctx=stm");
#else
        fd  = open(path, oflags, OPENMODE);
#endif
        if(fd < 0) {status = errno; goto unwind_open;}
	memio->mapfd = fd;
        memio->memory = (char*)mmap(NULL,memio->alloc,
                                    PROT_READ|PROT_WRITE,
				    MAP_SHARED,
                                    memio->mapfd,0);
	{int tst=memio->memory[0];} /* test reading of the mmap'd memory */
    } /*!persist*/

#ifdef DEBUG
fprintf(stderr,"mmap_create: initial memory: %lu/%lu\n",(unsigned long)memio->memory,(unsigned long)memio->alloc);
#endif

    fd = nc__pseudofd();
    *((int* )&nciop->fd) = fd; 

    fSet(nciop->ioflags, NC_WRITE);

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
    if(sizehintp) *sizehintp = pagesize;

    *nciopp = nciop;
    return NC_NOERR;

unwind_open:
    mmap_close(nciop,1);
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
mmap_open(const char* path,
    int ioflags,
    off_t igeto, size_t igetsz, size_t* sizehintp,
    ncio* *nciopp, void** const mempp)
{
    ncio* nciop;
    int fd;
    int status;
    int persist = (fIsSet(ioflags,NC_WRITE)?1:0);
    int oflags;
    NCMEMIO* memio = NULL;
    size_t sizehint;
    off_t filesize;

    if(path == NULL ||* path == 0)
        return EINVAL;

    assert(sizehintp != NULL);
    sizehint = *sizehintp;

    /* Open the file, but make sure we can write it if needed */
    oflags = (persist ? O_RDWR : O_RDONLY);    
#ifdef O_BINARY
    fSet(oflags, O_BINARY);
#endif
    oflags |= O_EXCL;
#ifdef vms
    fd = open(path, oflags, 0, "ctx=stm");
#else
    fd  = open(path, oflags, OPENMODE);
#endif
    if(fd < 0) {status = errno; goto unwind_open;}

    /* get current filesize  = max(|file|,initialize)*/
    filesize = lseek(fd,0,SEEK_END);
    if(filesize < 0) {status = errno; goto unwind_open;}
    /* move pointer back to beginning of file */
    (void)lseek(fd,0,SEEK_SET);
    if(filesize < (off_t)sizehint)
        filesize = (off_t)sizehint;

    status = mmap_new(path, ioflags, filesize, &nciop, &memio);
    if(status != NC_NOERR)
	return status;
    memio->size = filesize;

    memio->mapfd = fd;
    memio->memory = (char*)mmap(NULL,memio->alloc,
                                    persist?(PROT_READ|PROT_WRITE):(PROT_READ),
				    MAP_SHARED,
                                    memio->mapfd,0);
#ifdef DEBUG
fprintf(stderr,"mmap_open: initial memory: %lu/%lu\n",(unsigned long)memio->memory,(unsigned long)memio->alloc);
#endif

    /* Read the file into the memio memory */
    /* We need to do multiple reads because there is no
       guarantee that the amount read will be the full amount */
    {
	off_t red = memio->size;
	char* pos = memio->memory;
	while(red > 0) {
	    ssize_t count = read(fd, pos, red);
	    if(count < 0)
	        {close(fd); status = errno; goto unwind_open;}
	    if(count == 0)
	        {close(fd); status = NC_ENOTNC; goto unwind_open;}
	    red -= count;
	    pos += count;
	}
    }
    (void)close(fd); /* until mmap_close() */

    /* Use half the filesize as the blocksize */
    sizehint = filesize/2;

    fd = nc__pseudofd();
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

    *sizehintp = sizehint;
    *nciopp = nciop;
    return NC_NOERR;

unwind_open:
    mmap_close(nciop,0);
    return status;
}


/* 
 *  Get file size in bytes.
 */
static int
mmap_filesize(ncio* nciop, off_t* filesizep)
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
mmap_pad_length(ncio* nciop, off_t length)
{
    NCMEMIO* memio;
    if(nciop == NULL || nciop->pvt == NULL) return NC_EINVAL;
    memio = (NCMEMIO*)nciop->pvt;

    if(!fIsSet(nciop->ioflags, NC_WRITE))
        return EPERM; /* attempt to write readonly file*/

    if(memio->locked > 0)
	return NC_EDISKLESS;

    if(length > memio->alloc) {
        /* Realloc the allocated memory to a multiple of the pagesize*/
	off_t newsize = length;
	void* newmem = NULL;
	/* Round to a multiple of pagesize */
	if((newsize % pagesize) != 0)
	    newsize += (pagesize - (newsize % pagesize));

	newmem = (char*)mremap(memio->memory,memio->alloc,newsize,MREMAP_MAYMOVE);
	if(newmem == NULL) return NC_ENOMEM;

#ifdef DEBUG
fprintf(stderr,"realloc: %lu/%lu -> %lu/%lu\n",
(unsigned long)memio->memory,(unsigned long)memio->alloc,
(unsigned long)newmem,(unsigned long)newsize);
#endif
	memio->memory = newmem;
	memio->alloc = newsize;
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
mmap_close(ncio* nciop, int doUnlink)
{
    int status = NC_NOERR;
    NCMEMIO* memio;
    if(nciop == NULL || nciop->pvt == NULL) return NC_NOERR;

    memio = (NCMEMIO*)nciop->pvt;
    assert(memio != NULL);

    /* Since we are using mmap, persisting to a file should be automatic */
    status = munmap(memio->memory,memio->alloc);
    memio->memory = NULL; /* so we do not try to free it */

    /* Close file if it was open */
    if(memio->mapfd >= 0)
	close(memio->mapfd);

    /* do cleanup  */
    if(memio != NULL) free(memio);
    if(nciop->path != NULL) free((char*)nciop->path);
    free(nciop);
    return status;
}

static int
guarantee(ncio* nciop, off_t endpoint)
{
    NCMEMIO* memio = (NCMEMIO*)nciop->pvt;
    if(endpoint > memio->alloc) {
	/* extend the allocated memory and size */
	int status = mmap_pad_length(nciop,endpoint);
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
mmap_get(ncio* const nciop, off_t offset, size_t extent, int rflags, void** const vpp)
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
mmap_move(ncio* const nciop, off_t to, off_t from, size_t nbytes, int ignored)
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
mmap_rel(ncio* const nciop, off_t offset, int rflags)
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
mmap_sync(ncio* const nciop)
{
    return NC_NOERR; /* do nothing */
}
