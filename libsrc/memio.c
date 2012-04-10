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

/* Turn off MMAP until we can figure out why it doesn't work */
#undef HAVE_MMAP

#ifdef HAVE_MMAP
#include <sys/mman.h>
#endif

/* !HAVE_MREMAP => !HAVE_MMAP */
#ifndef HAVE_MREMAP
#  undef HAVE_MMAP
#endif

#ifndef MAP_ANONYMOUS
#  ifdef MAP_ANON
#    define MAP_ANONYMOUS MAP_ANON
#  endif
#endif

/* !MAP_ANONYMOUS => !HAVE_MMAP */
#ifndef MAP_ANONYMOUS
#  undef HAVE_MMAP
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

#ifndef MEMIO_MAXBLOCKSIZE
#define MEMIO_MAXBLOCKSIZE 268435456 /* sanity check, about X_SIZE_T_MAX/8 */
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
#define DEFAULT_BLOCKSIZE (0x2000)

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
    int mmap; /* 1=>ok to use mmap if available */
#ifdef HAVE_MMAP
    int mapfd;
#endif
} NCMEMIO;

/* Forward */
static int memio_rel(ncio *const nciop, off_t offset, int rflags);
static int memio_get(ncio *const nciop, off_t offset, size_t extent, int rflags, void **const vpp);
static int memio_move(ncio *const nciop, off_t to, off_t from, size_t nbytes, int rflags);
static int memio_sync(ncio *const nciop);
static int memio_filesize(ncio* nciop, off_t* filesizep);
static int memio_pad_length(ncio* nciop, off_t length);
static int memio_close(ncio* nciop, int);

/* Mnemonic */
#define DOOPEN 1

/* Create a new ncio struct to hold info about the file. */
static int
memio_new(const char* path, int ioflags, off_t initialsize, ncio** nciopp, NCMEMIO** memiop)
{
    int status = NC_NOERR;
    ncio* nciop = NULL;
    NCMEMIO* memio = NULL;
    int openfd = -1;
#if defined HAVE_SYSCONF
    long pagesize = sysconf(_SC_PAGE_SIZE);
#elif defined HAVE_GETPAGESIZE
    long pagesize = getpagesize();
#else
    long pagesize = 4096; /* good guess */
#endif
    errno = 0;

    nciop = (ncio* )calloc(1,sizeof(ncio));
    if(nciop == NULL) {status = NC_ENOMEM; goto fail;}
    
    nciop->ioflags = ioflags;
    *((int*)&nciop->fd) = -1; /* caller will fix */

    *((char**)&nciop->path) = strdup(path);
    if(nciop->path == NULL) {status = NC_ENOMEM; goto fail;}

    *((ncio_relfunc**)&nciop->rel) = memio_rel;
    *((ncio_getfunc**)&nciop->get) = memio_get;
    *((ncio_movefunc**)&nciop->move) = memio_move;
    *((ncio_syncfunc**)&nciop->sync) = memio_sync;
    *((ncio_filesizefunc**)&nciop->filesize) = memio_filesize;
    *((ncio_pad_lengthfunc**)&nciop->pad_length) = memio_pad_length;
    *((ncio_closefunc**)&nciop->close) = memio_close;

    memio = (NCMEMIO*)calloc(1,sizeof(NCMEMIO));
    if(memio == NULL) {status = NC_ENOMEM; goto fail;}
    *((void* *)&nciop->pvt) = memio;

    /* See if ok to use mmap */
#ifdef HAVE_MMAP
    memio->mmap = (sizeof(void*) < 8 && fIsSet(ioflags,NC_64BIT_OFFSET)?0:1);
#else
    memio->mmap = 0;
#endif

    memio->memory = NULL;
    memio->alloc = initialsize;
    if(memio->alloc < pagesize)
	memio->alloc = pagesize;
#ifdef HAVE_MMAP
    if((memio->alloc % pagesize) != 0)
	memio->alloc += (pagesize - (memio->alloc % pagesize));
#endif
    memio->size = 0;
    memio->pos = 0;
    memio->persist = fIsSet(ioflags,NC_WRITE);
#ifdef HAVE_MMAP
    memio->mapfd = -1;
#endif

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
memio_create(const char* path, int ioflags,
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

    status = memio_new(path, ioflags, initialsz, &nciop, &memio);
    if(status != NC_NOERR)
        return status;

    if(!persist) { /* mmap => use anonymous mmap  */
        if(memio->mmap) { /* use mmap if available */
#ifdef HAVE_MMAP
	    /* force map at sbrk */
	    void* top = sbrk(0);
	    memio->mapfd = -1;
	    memio->memory = (char*)mmap(top,memio->alloc,
                                    PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_FIXED|MAP_POPULATE,
                                    memio->mapfd,0);
	    {/* test writing of the mmap'd memory */
	      memio->memory[0] = 0;}
#endif /*HAVE_MMAP*/
	} else {/*!memio->mmap; just malloc memory */
	    memio->memory = (char*)malloc(memio->alloc);
	    if(memio->memory == NULL) {status = NC_ENOMEM; goto unwind_open;}
	}

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

	if(memio->mmap) {
#ifdef HAVE_MMAP
	    /* force map at sbrk */
	    void* top = sbrk(0);
	    memio->mapfd = fd;
            memio->memory = (char*)mmap(top,memio->alloc,
                                    PROT_READ|PROT_WRITE,MAP_SHARED|MAP_FIXED,
                                    memio->mapfd,0);
	    {/* test reading of the mmap'd memory */
	      int tst = memio->memory[0];}
#endif /*HAVE_MMAP*/
	} else {/*memio->mmap*/
	    (void)close(fd); /* will reopen at nc_close */
	    /* malloc memory */
	    memio->memory = (char*)malloc(memio->alloc);
	    if(memio->memory == NULL) {status = NC_ENOMEM; goto unwind_open;}
	}
    } /*!persist*/

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
    ncio* nciop;
    int fd;
    int status;
    int persist = (fIsSet(ioflags,NC_WRITE)?1:0);
    int oflags;
    NCMEMIO* memio = NULL;
    size_t sizehint;
    off_t filesize;
    ssize_t count;

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

    status = memio_new(path, ioflags, filesize, &nciop, &memio);
    if(status != NC_NOERR)
	return status;

    if(memio->mmap) {
#ifdef HAVE_MMAP
        memio->mapfd = fd;
        memio->memory = (char*)mmap(NULL,memio->alloc,
                                    persist?(PROT_READ|PROT_WRITE):(PROT_READ),
				    MAP_SHARED,
                                    memio->mapfd,0);
#endif /*HAVE_MMAP*/
    } else {/*!memio->mmap*/
        memio->memory = (char*)malloc(memio->alloc);
        if(memio->memory == NULL) {status = NC_ENOMEM; goto unwind_open;}

        /* Read the file into the memio memory */
        count = read(fd, memio->memory, (size_t)filesize);
        if(count < filesize) {status = NC_ENOTNC; goto unwind_open;}
        (void)close(fd); /* until memio_close() */
    }

    memio->size = memio->alloc;

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
    memio_close(nciop,0);
    return status;
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
	off_t newsize;
	char* newmem;
	switch(TACTIC) {
	case TACTIC_DOUBLE:
	    newsize = (memio->alloc * 2);
	    break;
	case TACTIC_INCR:
	default:
	    newsize = length + (length % DEFAULT_BLOCKSIZE);
	    break;
	}

        if(memio->mmap) {
#ifdef HAVE_MMAP
	    newmem = (char*)mremap(memio->memory,memio->alloc,newsize,MREMAP_MAYMOVE);
	    if(newmem == NULL) return NC_ENOMEM;
#endif /*HAVE_MMAP*/
        } else {
            newmem = (char*)realloc(memio->memory,newsize);
	    if(newmem == NULL) return NC_ENOMEM;
	    /* zero out the extra memory */
            memset((void*)(newmem+memio->alloc),0,(newsize - memio->alloc));
	}
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
memio_close(ncio* nciop, int doUnlink)
{
    int status = NC_NOERR;
    NCMEMIO* memio;
    if(nciop == NULL || nciop->pvt == NULL) return NC_NOERR;

    memio = (NCMEMIO*)nciop->pvt;
    assert(memio != NULL);

    if(memio->mmap) {
#ifdef HAVE_MMAP
        status = munmap(memio->memory,memio->alloc);
        if(status) goto done;
        /* Do we need to close the file? */
        if(memio->mapfd >= 0)
	    (void)close(memio->mapfd);
#endif /*HAVE_MMAP*/
    } else {/*!memio->mmap*/
        /* See if the user wants the contents persisted to a file */
        if(memio->persist) {
	    /* Try to open the file for writing */
	    int fd = open(nciop->path, O_WRONLY|O_CREAT|O_TRUNC, OPENMODE);
	    if(fd >= 0) {
	        long count = write(fd, memio->memory, memio->size);
	        if(count < 0)
		    status = errno;
	        else if(count < memio->size)
		    {status = NC_EDISKLESS; goto done;}
	        (void)close(fd);		
	    } else
	        status = errno;
	}
         /* Free up things */
         if(memio->memory != NULL) free(memio->memory);
     }

done:
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
