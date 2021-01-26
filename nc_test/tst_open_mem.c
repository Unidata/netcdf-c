/*! \file

Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
2015, 2016, 2017, 2018
University Corporation for Atmospheric Research/Unidata.

See \ref copyright file for more info.

*/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "netcdf.h"
#include "netcdf_mem.h"
#include "ncbytes.h"
#include "ncpathmgr.h"
#include "nc_tests.h"

#ifdef USE_HDF5
#include <hdf5.h>
extern int H5Eprint1(FILE * stream);
#endif

static int
readfile(const char* path, NC_memio* memio)
{
    int status = NC_NOERR;
    int fd = -1;
    off_t filesize = 0;
    off_t count = 0;
    char* memory = NULL;
    char* p = NULL;
    int oflags = O_RDONLY;
#ifdef O_BINARY
    oflags |= O_BINARY;
#endif

    /* Open the file to get its size*/
    fd = NCopen2(path,oflags);
    if(fd < 0) {
	fprintf(stderr,"cannot open file: %s\n",path);
	status = errno;
	goto done;
    }
    /* get current filesize */
    filesize = lseek(fd,0,SEEK_END);
    if(filesize < 0)
	{status = errno; goto done;}
    count = lseek(fd,0,SEEK_SET); /* rewind */
    /* allocate memory */
    memory = malloc((size_t)filesize);
    if(memory == NULL)
	{status = NC_ENOMEM; goto done;}
    count = filesize;
    p = memory;
    while(count > 0) {
        ssize_t actual;
        actual = read(fd,p,count);
	if(actual <= 0)
	    {status = NC_EIO; goto done;}
	count -= actual;
	p += actual;
    }
    if(memio) {
	memio->size = (size_t)filesize;
	memio->memory = memory;
	memory = NULL;
    }
done:
    if(status != NC_NOERR && memory != NULL)
	free(memory);
    if(fd  >= 0) close(fd);
    return status;
}

int
main(int argc, char** argv)
{
    int retval = NC_NOERR;
    int ncid;
    NC_memio mem;
    char* path = "f03tst_open_mem.nc";

    if(argc > 1)
	path = argv[1];
    if((retval=readfile(path,&mem)))
	goto exit;
    if((retval = nc_open_mem("mem", NC_INMEMORY|NC_NETCDF4, mem.size, mem.memory, &ncid)))
	goto exit;
    if((retval = nc_close(ncid)))
	goto exit;
    if(mem.memory)
        free(mem.memory);
    return 0;
exit:
    fprintf(stderr,"retval=%d\n",retval);
    return 1;
}
