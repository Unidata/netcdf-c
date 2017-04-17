/** \file daux.c

This file contains the "auxilliary" api code.
This is code that calls functions in dapi.c
and functions in dexport.c. It is auxilliary in
that it implements api methods in netcdf.h, but
does not directly call dispatch table methods.
As a rule, if a function calls a dispatch table
method or calls NC_check_id, then it is not
an auxilliary function, but rather should be
in dapi.c

Copyright 2010 University Corporation for Atmospheric
Research/Unidata. See COPYRIGHT file for more info.
*/

#include "config.h"
#include <stdlib.h>
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include "netcdf.h"
#include "nc.h"
#include "ncdispatch.h"
#include "netcdf_mem.h"
#include "ncwinpath.h"

/**************************************************/
/* Type Declarations */

/*!
  \internal

*/
struct Odometer {
    int            rank;
    size_t         index[NC_MAX_VAR_DIMS];
    size_t         start[NC_MAX_VAR_DIMS];
    size_t         edges[NC_MAX_VAR_DIMS];
    ptrdiff_t      stride[NC_MAX_VAR_DIMS];
    size_t         stop[NC_MAX_VAR_DIMS];
};


/**************************************************/
/* Forward */

static int NC_get_var(int ncid, int varid, void *value, nc_type memtype);
static int NC_get_var1(int ncid, int varid, const size_t *coord, void* value, nc_type memtype);
static int NC_put_var(int ncid, int varid, const void *value, nc_type memtype);
static int NC_put_var1(int ncid, int varid, const size_t *coord, const void* value, nc_type memtype);

static void odom_init(struct Odometer* odom, int rank, const size_t* start, const size_t* edges, const ptrdiff_t* stride);
static int odom_more(struct Odometer* odom);
static int odom_next(struct Odometer* odom);

/**************************************************/
/* Originally in dfile.c */

/**  \ingroup datasets
Create a new netCDF file.

This function creates a new netCDF dataset, returning a netCDF ID that
can subsequently be used to refer to the netCDF dataset in other
netCDF function calls. The new netCDF dataset opened for write access
and placed in define mode, ready for you to add dimensions, variables,
and attributes.

\param path The file name of the new netCDF dataset.

\param cmode The creation mode flag. The following flags are available:
  NC_NOCLOBBER (do not overwrite existing file),
  NC_SHARE (limit write caching - netcdf classic files only),
  NC_64BIT_OFFSET (create 64-bit offset file),
  NC_64BIT_DATA (Alias NC_CDF5) (create CDF-5 file),
  NC_NETCDF4 (create netCDF-4/HDF5 file),
  NC_CLASSIC_MODEL (enforce netCDF classic mode on netCDF-4/HDF5 files),
  NC_DISKLESS (store data only in memory),
  NC_MMAP (use MMAP for NC_DISKLESS),
  and NC_WRITE.
  See discussion below.

\param ncidp Pointer to location where returned netCDF ID is to be
stored.

<h2>The cmode Flag</h2>

The cmode flag is used to control the type of file created, and some
aspects of how it may be used.

Setting NC_NOCLOBBER means you do not want to clobber (overwrite) an
existing dataset; an error (NC_EEXIST) is returned if the specified
dataset already exists.

The NC_SHARE flag is appropriate when one process may be writing the
dataset and one or more other processes reading the dataset
concurrently; it means that dataset accesses are not buffered and
caching is limited. Since the buffering scheme is optimized for
sequential access, programs that do not access data sequentially may
see some performance improvement by setting the NC_SHARE flag. This
flag is ignored for netCDF-4 files.

Setting NC_64BIT_OFFSET causes netCDF to create a 64-bit offset format
file, instead of a netCDF classic format file. The 64-bit offset
format imposes far fewer restrictions on very large (i.e. over 2 GB)
data files. See Large File Support.

Setting NC_64BIT_DATA (Alias NC_CDF5) causes netCDF to create a CDF-5
file format that supports large files (i.e. over 2GB) and large
variables (over 2B array elements.). See Large File Support.

Note that the flag NC_PNETCDF also exists as the combination of
NC_CDF5 or'd with NC_MPIIO to indicate that the pnetcdf library
should be used.

A zero value (defined for convenience as NC_CLOBBER) specifies the
default behavior: overwrite any existing dataset with the same file
name and buffer and cache accesses for efficiency. The dataset will be
in netCDF classic format. See NetCDF Classic Format Limitations.

Setting NC_NETCDF4 causes netCDF to create a HDF5/NetCDF-4 file.

Setting NC_CLASSIC_MODEL causes netCDF to enforce the classic data
model in this file. (This only has effect for netCDF-4/HDF5 files, as
classic and 64-bit offset files always use the classic model.) When
used with NC_NETCDF4, this flag ensures that the resulting
netCDF-4/HDF5 file may never contain any new constructs from the
enhanced data model. That is, it cannot contain groups, user defined
types, multiple unlimited dimensions, or new atomic types. The
advantage of this restriction is that such files are guaranteed to
work with existing netCDF software.

Setting NC_DISKLESS causes netCDF to create the file only in memory.
This allows for the use of files that have no long term purpose. Note that
with one exception, the in-memory file is destroyed upon calling
nc_close. If, however, the flag combination (NC_DISKLESS|NC_WRITE)
is used, then at close, the contents of the memory file will be
made persistent in the file path that was specified in the nc_create
call. If NC_DISKLESS is going to be used for creating a large classic file,
it behooves one to use either nc__create or nc_create_mp and specify
an appropriately large value of the initialsz parameter to avoid
to many extensions to the in-memory space for the file.
This flag applies to files in classic format and to file in extended
format (netcdf-4).

Normally, NC_DISKLESS allocates space in the heap for
storing the in-memory file. If, however, the ./configure
flags --enable-mmap is used, and the additional mode flag
NC_MMAP is specified, then the file will be created using
the operating system MMAP facility.
This flag only applies to files in classic format. Extended
format (netcdf-4) files will ignore the NC_MMAP flag.

Using NC_MMAP for nc_create is
only included for completeness vis-a-vis nc_open. The
ability to use MMAP is of limited use for nc_create because
nc_create is going to create the file in memory anyway.
Closing a MMAP'd file will be slightly faster, but not significantly.

Note that nc_create(path,cmode,ncidp) is equivalent to the invocation of
nc__create(path,cmode,NC_SIZEHINT_DEFAULT,NULL,ncidp).

\returns ::NC_NOERR No error.

\returns ::NC_ENOMEM System out of memory.

\returns ::NC_EHDFERR HDF5 error (netCDF-4 files only).

\returns ::NC_EFILEMETA Error writing netCDF-4 file-level metadata in
HDF5 file. (netCDF-4 files only).

\returns ::NC_EDISKLESS if there was an error in creating the
in-memory file.

\note When creating a netCDF-4 file HDF5 error reporting is turned
off, if it is on. This doesn't stop the HDF5 error stack from
recording the errors, it simply stops their display to the user
through stderr.

<h1>Examples</h1>

In this example we create a netCDF dataset named foo.nc; we want the
dataset to be created in the current directory only if a dataset with
that name does not already exist:

@code
     #include <netcdf.h>
        ...
     int status = NC_NOERR;
     int ncid;
        ...
     status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
     if (status != NC_NOERR) handle_error(status);
@endcode

In this example we create a netCDF dataset named foo_large.nc. It will
be in the 64-bit offset format.

@code
     #include <netcdf.h>
        ...
     int status = NC_NOERR;
     int ncid;
        ...
     status = nc_create("foo_large.nc", NC_NOCLOBBER|NC_64BIT_OFFSET, &ncid);
     if (status != NC_NOERR) handle_error(status);
@endcode

In this example we create a netCDF dataset named foo_HDF5.nc. It will
be in the HDF5 format.

@code
     #include <netcdf.h>
        ...
     int status = NC_NOERR;
     int ncid;
        ...
     status = nc_create("foo_HDF5.nc", NC_NOCLOBBER|NC_NETCDF4, &ncid);
     if (status != NC_NOERR) handle_error(status);
@endcode

In this example we create a netCDF dataset named
foo_HDF5_classic.nc. It will be in the HDF5 format, but will not allow
the use of any netCDF-4 advanced features. That is, it will conform to
the classic netCDF-3 data model.

@code
     #include <netcdf.h>
        ...
     int status = NC_NOERR;
     int ncid;
        ...
     status = nc_create("foo_HDF5_classic.nc", NC_NOCLOBBER|NC_NETCDF4|NC_CLASSIC_MODEL, &ncid);
     if (status != NC_NOERR) handle_error(status);
@endcode

In this example we create a in-memory netCDF classic dataset named
diskless.nc whose content will be lost when nc_close() is called.

@code
     #include <netcdf.h>
        ...
     int status = NC_NOERR;
     int ncid;
        ...
     status = nc_create("diskless.nc", NC_DISKLESS, &ncid);
     if (status != NC_NOERR) handle_error(status);
@endcode

In this example we create a in-memory netCDF classic dataset named
diskless.nc and specify that it should be made persistent
in a file named diskless.nc when nc_close() is called.

@code
     #include <netcdf.h>
        ...
     int status = NC_NOERR;
     int ncid;
        ...
     status = nc_create("diskless.nc", NC_DISKLESS|NC_WRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
@endcode

A variant of nc_create(), nc__create() (note the double underscore) allows
users to specify two tuning parameters for the file that it is
creating.  */
int
nc_create(const char *path, int cmode, int *ncidp)
{
   return nc__create(path,cmode,NC_SIZEHINT_DEFAULT,NULL,ncidp);
}

/*!
Create a netCDF file with some extra parameters controlling classic
file cacheing.

Like nc_create(), this function creates a netCDF file.

\param path The file name of the new netCDF dataset.

\param cmode The creation mode flag, the same as in nc_create().

\param initialsz On some systems, and with custom I/O layers, it may
be advantageous to set the size of the output file at creation
time. This parameter sets the initial size of the file at creation
time. This only applies to classic and 64-bit offset files.
The special value NC_SIZEHINT_DEFAULT (which is the value 0),
lets the netcdf library choose a suitable initial size.

\param chunksizehintp A pointer to the chunk size hint,
which controls a space versus time tradeoff, memory
allocated in the netcdf library versus number of system
calls. Because of internal requirements, the value may not
be set to exactly the value requested. The actual value
chosen is returned by reference. Using a NULL pointer or
having the pointer point to the value NC_SIZEHINT_DEFAULT
causes the library to choose a default. How the system
chooses the default depends on the system. On many systems,
the "preferred I/O block size" is available from the stat()
system call, struct stat member st_blksize. If this is
available it is used. Lacking that, twice the system
pagesize is used. Lacking a call to discover the system
pagesize, we just set default bufrsize to 8192. The bufrsize
is a property of a given open netcdf descriptor ncid, it is
not a persistent property of the netcdf dataset. This only
applies to classic and 64-bit offset files.

\param ncidp Pointer to location where returned netCDF ID is to be
stored.

\note This function uses the same return codes as the nc_create()
function.

<h1>Examples</h1>

In this example we create a netCDF dataset named foo_large.nc; we want
the dataset to be created in the current directory only if a dataset
with that name does not already exist. We also specify that bufrsize
and initial size for the file.

\code
#include <netcdf.h>
        ...
     int status = NC_NOERR;
     int ncid;
     int intialsz = 2048;
     int *bufrsize;
        ...
     *bufrsize = 1024;
     status = nc__create("foo.nc", NC_NOCLOBBER, initialsz, bufrsize, &ncid);
     if (status != NC_NOERR) handle_error(status);
\endcode

\ingroup datasets

*/
int
nc__create(const char *path, int cmode, size_t initialsz,
	   size_t *chunksizehintp, int *ncidp)
{
   return NC_create(path, cmode, initialsz, 0,
		    chunksizehintp, 0, NULL, ncidp);

}

/**
\internal

\deprecated This function was used in the old days with the Cray at
NCAR. The Cray is long gone, and this call is supported only for
backward compatibility.

 */
int
nc__create_mp(const char *path, int cmode, size_t initialsz,
	      int basepe, size_t *chunksizehintp, int *ncidp)
{
   return NC_create(path, cmode, initialsz, basepe,
		    chunksizehintp, 0, NULL, ncidp);
}

/** \ingroup datasets
Open an existing netCDF file.

This function opens an existing netCDF dataset for access. It
determines the underlying file format automatically. Use the same call
to open a netCDF classic, 64-bit offset, or netCDF-4 file.

\param path File name for netCDF dataset to be opened. When DAP
support is enabled, then the path may be an OPeNDAP URL rather than a
file path.

\param mode The mode flag may include NC_WRITE (for read/write
access) and NC_SHARE (see below) and NC_DISKLESS (see below).

\param ncidp Pointer to location where returned netCDF ID is to be
stored.

<h2>Open Mode</h2>

A zero value (or ::NC_NOWRITE) specifies the default behavior: open the
dataset with read-only access, buffering and caching accesses for
efficiency.

Otherwise, the open mode is ::NC_WRITE, ::NC_SHARE, or
::NC_WRITE|::NC_SHARE. Setting the ::NC_WRITE flag opens the dataset with
read-write access. ("Writing" means any kind of change to the dataset,
including appending or changing data, adding or renaming dimensions,
variables, and attributes, or deleting attributes.)

The NC_SHARE flag is only used for netCDF classic and 64-bit offset
files. It is appropriate when one process may be writing the dataset
and one or more other processes reading the dataset concurrently; it
means that dataset accesses are not buffered and caching is
limited. Since the buffering scheme is optimized for sequential
access, programs that do not access data sequentially may see some
performance improvement by setting the NC_SHARE flag.

This procedure may also be invoked with the NC_DISKLESS flag
set in the mode argument if the file to be opened is a
classic format file.  For nc_open(), this flag applies only
to files in classic format.  If the file is of type
NC_NETCDF4, then the NC_DISKLESS flag will be ignored.

If NC_DISKLESS is specified, then the whole file is read completely into
memory. In effect this creates an in-memory cache of the file.
If the mode flag also specifies NC_WRITE, then the in-memory cache
will be re-written to the disk file when nc_close() is called.
For some kinds of manipulations, having the in-memory cache can
speed up file processing. But in simple cases, non-cached
processing may actually be faster than using cached processing.
You will need to experiment to determine if the in-memory caching
is worthwhile for your application.

Normally, NC_DISKLESS allocates space in the heap for
storing the in-memory file. If, however, the ./configure
flags --enable-mmap is used, and the additional mode flag
NC_MMAP is specified, then the file will be opened using
the operating system MMAP facility.
This flag only applies to files in classic format. Extended
format (netcdf-4) files will ignore the NC_MMAP flag.

In most cases, using MMAP provides no advantage
for just NC_DISKLESS. The one case where using MMAP is an
advantage is when a file is to be opened and only a small portion
of its data is to be read and/or written.
In this scenario, MMAP will cause only the accessed data to be
retrieved from disk. Without MMAP, NC_DISKLESS will read the whole
file into memory on nc_open. Thus, MMAP will provide some performance
improvement in this case.

It is not necessary to pass any information about the format of the
file being opened. The file type will be detected automatically by the
netCDF library.

If a the path is a DAP URL, then the open mode is read-only.
Setting NC_WRITE will be ignored.

As of version 4.3.1.2, multiple calls to nc_open with the same
path will return the same ncid value.

\note When opening a netCDF-4 file HDF5 error reporting is turned off,
if it is on. This doesn't stop the HDF5 error stack from recording the
errors, it simply stops their display to the user through stderr.

nc_open()returns the value NC_NOERR if no errors occurred. Otherwise,
the returned status indicates an error. Possible causes of errors
include:

Note that nc_open(path,cmode,ncidp) is equivalent to the invocation of
nc__open(path,cmode,NC_SIZEHINT_DEFAULT,NULL,ncidp).

\returns ::NC_NOERR No error.

\returns ::NC_ENOMEM Out of memory.

\returns ::NC_EHDFERR HDF5 error. (NetCDF-4 files only.)

\returns ::NC_EDIMMETA Error in netCDF-4 dimension metadata. (NetCDF-4 files only.)

<h1>Examples</h1>

Here is an example using nc_open()to open an existing netCDF dataset
named foo.nc for read-only, non-shared access:

@code
#include <netcdf.h>
   ...
int status = NC_NOERR;
int ncid;
   ...
status = nc_open("foo.nc", 0, &ncid);
if (status != NC_NOERR) handle_error(status);
@endcode
*/
int
nc_open(const char *path, int mode, int *ncidp)
{
   return NC_open(path, mode, 0, NULL, 0, NULL, ncidp);
}

/** \ingroup datasets
Open a netCDF file with extra performance parameters for the classic
library.

\param path File name for netCDF dataset to be opened. When DAP
support is enabled, then the path may be an OPeNDAP URL rather than a
file path.

\param mode The mode flag may include NC_WRITE (for read/write
access) and NC_SHARE as in nc_open().

\param chunksizehintp A size hint for the classic library. Only
applies to classic and 64-bit offset files. See below for more
information.

\param ncidp Pointer to location where returned netCDF ID is to be
stored.

<h1>The chunksizehintp Parameter</h1>

The argument referenced by bufrsizehintp controls a space versus time
tradeoff, memory allocated in the netcdf library versus number of
system calls.

Because of internal requirements, the value may not be set to exactly
the value requested. The actual value chosen is returned by reference.

Using a NULL pointer or having the pointer point to the value
NC_SIZEHINT_DEFAULT causes the library to choose a default.
How the system chooses the default depends on the system. On
many systems, the "preferred I/O block size" is available from the
stat() system call, struct stat member st_blksize. If this is
available it is used. Lacking that, twice the system pagesize is used.

Lacking a call to discover the system pagesize, we just set default
bufrsize to 8192.

The bufrsize is a property of a given open netcdf descriptor ncid, it
is not a persistent property of the netcdf dataset.


\returns ::NC_NOERR No error.

\returns ::NC_ENOMEM Out of memory.

\returns ::NC_EHDFERR HDF5 error. (NetCDF-4 files only.)

\returns ::NC_EDIMMETA Error in netCDF-4 dimension metadata. (NetCDF-4
files only.)

*/
int
nc__open(const char *path, int mode,
	 size_t *chunksizehintp, int *ncidp)
{
   /* this API is for non-parallel access: TODO check for illegal cmode
    * flags, such as NC_PNETCDF, NC_MPIIO, or NC_MPIPOSIX, before entering
    * NC_open()? Note nc_open_par() also calls NC_open().
    */
   return NC_open(path, mode, 0, chunksizehintp, 0,
		  NULL, ncidp);
}

/** \ingroup datasets
Open a netCDF file with the contents taken from a block of memory.

\param path Must be non-null, but otherwise only used to set the dataset name.

\param mode the mode flags; Note that this procedure uses a limited set of flags because it forcibly sets NC_NOWRITE|NC_DISKLESS|NC_INMEMORY.

\param size The length of the block of memory being passed.

\param memory Pointer to the block of memory containing the contents
of a netcdf file.

\param ncidp Pointer to location where returned netCDF ID is to be
stored.

\returns ::NC_NOERR No error.

\returns ::NC_ENOMEM Out of memory.

\returns ::NC_EDISKLESS diskless io is not enabled for fails.

\returns ::NC_EINVAL, etc. other errors also returned by nc_open.

<h1>Examples</h1>

Here is an example using nc_open_mem() to open an existing netCDF dataset
named foo.nc for read-only, non-shared access. It differs from the nc_open()
example in that it assumes the contents of foo.nc have been read into memory.

@code
#include <netcdf.h>
#include <netcdf_mem.h>
   ...
int status = NC_NOERR;
int ncid;
size_t size;
void* memory;
   ...
size = <compute file size of foo.nc in bytes>;
memory = malloc(size);
   ...
status = nc_open_mem("foo.nc", 0, size, memory, &ncid);
if (status != NC_NOERR) handle_error(status);
@endcode
*/
int
nc_open_mem(const char* path, int mode, size_t size, void* memory, int* ncidp)
{
#ifdef USE_DISKLESS
    NC_MEM_INFO meminfo;

    /* Sanity checks */
    if(memory == NULL || size < MAGIC_NUMBER_LEN || path == NULL)
 	return NC_EINVAL;
    if(mode & (NC_WRITE|NC_MPIIO|NC_MPIPOSIX|NC_MMAP))
	return NC_EINVAL;
    mode |= (NC_INMEMORY|NC_DISKLESS);
    meminfo.size = size;
    meminfo.memory = memory;
    return NC_open(path, mode, 0, NULL, 0, &meminfo, ncidp);
#else
    return NC_EDISKLESS;
#endif
}

/**
\internal

\deprecated This function was used in the old days with the Cray at
NCAR. The Cray is long gone, and this call is supported only for
backward compatibility.

 */
int
nc__open_mp(const char *path, int mode, int basepe,
	    size_t *chunksizehintp, int *ncidp)
{
   return NC_open(path, mode, basepe, chunksizehintp,
		  0, NULL, ncidp);
}


/**************************************************/
/* Originally in dfile.c */

int
nc_inq_nvars(int ncid, int *nvarsp)
{
   return nc_inq(ncid, NULL, nvarsp, NULL, NULL);
}

/**************************************************/
/* Originally in dattinq.c */

/** \name Learning about Attributes

Functions to learn about the attributes in a file. */

/*! \{ */ /* All these functions are part of this named group... */

/**
\ingroup attributes
Find number of global or group attributes.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param nattsp Pointer where number of global or group attributes will be
written. \ref ignored_if_null.
*/
int
nc_inq_natts(int ncid, int *nattsp)
{
   if(nattsp == NULL) return NC_NOERR;
   return nc_inq(ncid, NULL, NULL, nattsp, NULL);
}

/**
\ingroup attributes
Find the type of an attribute.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID of the attribute's variable, or ::NC_GLOBAL
for a global or group attribute.

\param name Attribute \ref object_name. 

\param xtypep Pointer to location for returned attribute \ref data_type.
*/
int
nc_inq_atttype(int ncid, int varid, const char *name, nc_type *xtypep)
{
   return nc_inq_att(ncid, varid, name, xtypep, NULL);
}

/**
\ingroup attributes
Find the length of an attribute.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID of the attribute's variable, or ::NC_GLOBAL
for a global or group attribute.

\param name Attribute \ref object_name. 

\param lenp Pointer to location for returned number of values
currently stored in the attribute. Before using the value as a C
string, make sure it is null-terminated. \ref ignored_if_null.  
*/
int
nc_inq_attlen(int ncid, int varid, const char *name, size_t *lenp)
{
   return nc_inq_att(ncid, varid, name, NULL, lenp);
}

/*! \} */  /* End of named group ...*/

/**************************************************/
/* Originally in dattget.c */

/** \name Getting Attributes

Functions to get the values of attributes.
 */
/*! \{ */

/*!
\ingroup attributes
Get an attribute of any type.

The nc_get_att() functions works for any type of attribute, and must
be used to get attributes of user-defined type. We recommend that they
type safe versions of this function be used where possible.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID of the attribute's variable, or ::NC_GLOBAL
for a global attribute.

\param name Attribute \ref object_name.

\param value Pointer to location for returned attribute value(s). All
elements of the vector of attribute values are returned, so you must
allocate enough space to hold them. Before using the value as a C
string, make sure it is null-terminated. Call nc_inq_attlen() first to
find out the length of the attribute.

\note See documentation for nc_get_att_string() regarding a special case where memory must be explicitly released.

*/
int
nc_get_att(int ncid, int varid, const char *name, void *value)
{
   int stat = NC_NOERR;
   nc_type xtype;

   /* Need to get the type */
   if ((stat = nc_inq_atttype(ncid, varid, name, &xtype)))
      return stat;

   TRACE(nc_get_att);
   return NC_get_att(ncid, varid, name, value, xtype);
}

/*! \} */

/*!
\ingroup attributes
Get an attribute.

This function gets an attribute from the netCDF file. The nc_get_att()
function works with any type of data, including user defined types.

\note The netCDF library reads all attributes into memory when the
file is opened with nc_open(). Getting an attribute copies the value
from the in-memory store, and does not incure any file I/O penalties.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID of the attribute's variable, or ::NC_GLOBAL
for a global attribute.

\param name Attribute \ref object_name.

\param value Pointer to location for returned attribute value(s). All
elements of the vector of attribute values are returned, so you must
allocate enough space to hold them. If you don't know how much
space to reserve, call nc_inq_attlen() first to find out the length of
the attribute.

<h1>Example</h1>

Here is an example using nc_get_att_double() to determine the values
of a variable attribute named valid_range for a netCDF variable named
rh and using nc_get_att_text() to read a global attribute named title
in an existing netCDF dataset named foo.nc.

In this example, it is assumed that we don't know how many values will
be returned, but that we do know the types of the attributes. Hence,
to allocate enough space to store them, we must first inquire about
the length of the attributes.

\code
     #include <netcdf.h>
        ...
     int  status;
     int  ncid;
     int  rh_id;
     int  vr_len, t_len;
     double *vr_val;
     char *title;
     extern char *malloc()

        ...
     status = nc_open("foo.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_attlen (ncid, rh_id, "valid_range", &vr_len);
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_attlen (ncid, NC_GLOBAL, "title", &t_len);
     if (status != NC_NOERR) handle_error(status);

     vr_val = (double *) malloc(vr_len * sizeof(double));
     title = (char *) malloc(t_len + 1);

     status = nc_get_att_double(ncid, rh_id, "valid_range", vr_val);
     if (status != NC_NOERR) handle_error(status);
     status = nc_get_att_text(ncid, NC_GLOBAL, "title", title);
     if (status != NC_NOERR) handle_error(status);
     title[t_len] = '\0';
        ...
\endcode
*/
/*! \{ */

int
nc_get_att_text(int ncid, int varid, const char *name, char *value)
{
   TRACE(nc_get_att_text);
   return NC_get_att(ncid, varid, name, (void *)value, NC_CHAR);
}

int
nc_get_att_schar(int ncid, int varid, const char *name, signed char *value)
{
   TRACE(nc_get_att_schar);
   return NC_get_att(ncid, varid, name, (void *)value, NC_BYTE);
}

int
nc_get_att_uchar(int ncid, int varid, const char *name, unsigned char *value)
{
   TRACE(nc_get_att_uchar);
   return NC_get_att(ncid, varid, name, (void *)value, NC_UBYTE);
}

int
nc_get_att_short(int ncid, int varid, const char *name, short *value)
{
   TRACE(nc_get_att_short);
   return NC_get_att(ncid, varid, name, (void *)value, NC_SHORT);
}

int
nc_get_att_int(int ncid, int varid, const char *name, int *value)
{
   TRACE(nc_get_att_int);
   return NC_get_att(ncid, varid, name, (void *)value, NC_INT);
}

int
nc_get_att_long(int ncid, int varid, const char *name, long *value)
{
   TRACE(nc_get_att_long);
   return NC_get_att(ncid, varid, name, (void *)value, longtype);
}

int
nc_get_att_float(int ncid, int varid, const char *name, float *value)
{
   TRACE(nc_get_att_float);
   return NC_get_att(ncid, varid, name, (void *)value, NC_FLOAT);
}

int
nc_get_att_double(int ncid, int varid, const char *name, double *value)
{
   TRACE(nc_get_att_double);
   return NC_get_att(ncid, varid, name, (void *)value, NC_DOUBLE);
}

int
nc_get_att_ubyte(int ncid, int varid, const char *name, unsigned char *value)
{
   TRACE(nc_get_att_ubyte);
   return NC_get_att(ncid, varid, name, (void *)value, NC_UBYTE);
}

int
nc_get_att_ushort(int ncid, int varid, const char *name, unsigned short *value)
{
   TRACE(nc_get_att_ushort);
   return NC_get_att(ncid, varid, name, (void *)value, NC_USHORT);
}

int
nc_get_att_uint(int ncid, int varid, const char *name, unsigned int *value)
{
   TRACE(nc_get_att_uint);
   return NC_get_att(ncid, varid, name, (void *)value, NC_UINT);
}

int
nc_get_att_longlong(int ncid, int varid, const char *name, long long *value)
{
   TRACE(nc_get_att_longlong);
   return NC_get_att(ncid, varid, name, (void *)value, NC_INT64);
}

int
nc_get_att_ulonglong(int ncid, int varid, const char *name, unsigned long long *value)
{
   TRACE(nc_get_att_ulonglong);
   return NC_get_att(ncid, varid, name, (void *)value, NC_UINT64);
}
/*! \} */

/*!
\ingroup attributes
Get a variable-length string attribute.

This function gets an attribute from netCDF file. Thhe nc_get_att() function works with any type of data including user defined types, but this function will retrieve attributes which are of type variable-length string.

\note Note that unlike most other nc_get_att functions, nc_get_att_string() allocates a chunk of memory which is returned to the calling function.  This chunk of memory must be specifically deallocated with nc_free_string() to avoid any memory leaks.  Also note that you must still preallocate the memory needed for the array of pointers passed to nc_get_att_string().

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID of the attribute's variable, or ::NC_GLOBAL
for a global attribute.

\param name Attribute \ref object_name.

\param value Pointer to location for returned attribute value(s). All
elements of the vector of attribute values are returned, so you must
allocate enough space to hold them. If you don't know how much
space to reserve, call nc_inq_attlen() first to find out the length of
the attribute.

\section nc_get_att_string_example Example

\code{.c}
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netcdf.h>

void check(int stat) {
  if (stat != NC_NOERR) {
    printf("NetCDF error: %s\n", nc_strerror(stat));
    exit(1);
  }
}

int main(int argc, char ** argv) {
  int stat = 0;

  int ncid = 0;
  stat = nc_open("test.nc", NC_NOWRITE, &ncid); check(stat);

  int varid = 0;
  stat = nc_inq_varid(ncid, "variable", &varid); check(stat);

  size_t attlen = 0;
  stat = nc_inq_attlen(ncid, varid, "attribute", &attlen); check(stat);

  char **string_attr = (char**)malloc(attlen * sizeof(char*));
  memset(string_attr, 0, attlen * sizeof(char*));

  stat = nc_get_att_string(ncid, varid, "attribute", string_attr); check(stat);

  for (size_t k = 0; k < attlen; ++k) {
    printf("variable:attribute[%d] = %s\n", k, string_attr[k]);
  }

  stat = nc_free_string(attlen, string_attr); check(stat);

  free(string_attr);

  stat = nc_close(ncid); check(stat);

  return 0;
}
\endcode


*/

int
nc_get_att_string(int ncid, int varid, const char *name, char **value)
{
    TRACE(nc_get_att_string);
    return NC_get_att(ncid,varid,name,(void*)value, NC_STRING);
}
/*! \} */

/** \name Writing Attributes

Functions to write attributes. */
/*! \{ */

/*!
\ingroup attributes
Write a string attribute.

The function nc_put_att_string adds or changes a variable attribute or
global attribute of an open netCDF dataset. The string type is only
available in netCDF-4/HDF5 files, when ::NC_CLASSIC_MODEL has not been
used in nc_create().

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID of the variable to which the attribute will
be assigned or ::NC_GLOBAL for a global or group attribute.

\param name Attribute \ref object_name. \ref attribute_conventions may
apply.

\param len Number of values provided for the attribute.

\param value Pointer to one or more values.

\returns ::NC_NOERR No error.
\returns ::NC_EINVAL More than one value for _FillValue.
\returns ::NC_EGLOBAL Trying to set global _FillValue.
\returns ::NC_ENOTVAR Couldn't find varid.
\returns ::NC_EBADTYPE Fill value and var must be same type.
\returns ::NC_ENOMEM Out of memory
\returns ::NC_ELATEFILL Fill values must be written while the file
is still in initial define mode.
*/

int
nc_put_att_string(int ncid, int varid, const char *name,
		  size_t len, const char** value)
{
    return NC_put_att(ncid, varid, name, NC_STRING,len, (void*)value, NC_STRING);
}

/*!
\ingroup attributes
Write a text attribute.

Add or change a text attribute. If this attribute is new,
or if the space required to store the attribute is greater than
before, the netCDF dataset must be in define mode.

Although it's possible to create attributes of all types, text and
double attributes are adequate for most purposes.

Use the nc_put_att function to create attributes of any type,
including user-defined types. We recommend using the type safe
versions of this function whenever possible.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID of the variable to which the attribute will
be assigned or ::NC_GLOBAL for a global attribute.

\param name Attribute \ref object_name. \ref attribute_conventions may
apply.

\param len Number of values provided for the attribute.

\param value Pointer to one or more values.

\returns ::NC_NOERR No error.
\returns ::NC_EINVAL More than one value for _FillValue.
\returns ::NC_EGLOBAL Trying to set global _FillValue.
\returns ::NC_ENOTVAR Couldn't find varid.
\returns ::NC_EBADTYPE Fill value and var must be same type.
\returns ::NC_ENOMEM Out of memory
\returns ::NC_ELATEFILL Fill values must be written while the file
is still in initial define mode.

\note With netCDF-4 files, nc_put_att will notice if you are writing a
_Fill_Value_ attribute, and will tell the HDF5 layer to use the
specified fill value for that variable.

\section nc_put_att_text_example Example

Here is an example using nc_put_att_double() to add a variable
attribute named valid_range for a netCDF variable named rh and
nc_put_att_text() to add a global attribute named title to an existing
netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status;
     int  ncid;
     int  rh_id;
     static double rh_range[] = {0.0, 100.0};
     static char title[] = "example netCDF dataset";
        ...
     status = nc_open("foo.nc", NC_WRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_redef(ncid);
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_put_att_double (ncid, rh_id, "valid_range",
                                 NC_DOUBLE, 2, rh_range);
     if (status != NC_NOERR) handle_error(status);
     status = nc_put_att_text (ncid, NC_GLOBAL, "title",
                               strlen(title), title)
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_enddef(ncid);
     if (status != NC_NOERR) handle_error(status);
\endcode
*/


int nc_put_att_text(int ncid, int varid, const char *name,
		size_t len, const char *value)
{
   return NC_put_att(ncid, varid, name, NC_CHAR, len,(void *)value, NC_CHAR);
}

/*! \} */
/*!
\ingroup attributes
Write an attribute.

The function nc_put_att_ type adds or changes a variable attribute or
global attribute of an open netCDF dataset. If this attribute is new,
or if the space required to store the attribute is greater than
before, the netCDF dataset must be in define mode.

With netCDF-4 files, nc_put_att will notice if you are writing a
_FillValue attribute, and will tell the HDF5 layer to use the
specified fill value for that variable.  With either classic or
netCDF-4 files, a _FillValue attribute will be checked for validity,
to make sure it has only one value and that its type matches the type
of the associated variable.

Although it's possible to create attributes of all types, text and
double attributes are adequate for most purposes.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID of the variable to which the attribute will
be assigned or ::NC_GLOBAL for a global or group attribute.

\param name Attribute \ref object_name. \ref attribute_conventions may
apply.

\param xtype \ref data_type of the attribute.

\param len Number of values provided for the attribute.

\param value Pointer to one or more values.

\returns ::NC_NOERR No error.
\returns ::NC_EINVAL More than one value for _FillValue.
\returns ::NC_EGLOBAL Trying to set global _FillValue.
\returns ::NC_ENOTVAR Couldn't find varid.
\returns ::NC_EBADTYPE Fill value and var must be same type.
\returns ::NC_ENOMEM Out of memory
\returns ::NC_ELATEFILL Fill values must be written while the file
is still in initial define mode.

\section nc_put_att_double_example Example

Here is an example using nc_put_att_double() to add a variable
attribute named valid_range for a netCDF variable named rh and
nc_put_att_text() to add a global attribute named title to an existing
netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status;
     int  ncid;
     int  rh_id;
     static double rh_range[] = {0.0, 100.0};
     static char title[] = "example netCDF dataset";
        ...
     status = nc_open("foo.nc", NC_WRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_redef(ncid);
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_put_att_double (ncid, rh_id, "valid_range",
                                 NC_DOUBLE, 2, rh_range);
     if (status != NC_NOERR) handle_error(status);
     status = nc_put_att_text (ncid, NC_GLOBAL, "title",
                               strlen(title), title)
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_enddef(ncid);
     if (status != NC_NOERR) handle_error(status);
\endcode
*/
/*! \{*/
int
nc_put_att(int ncid, int varid, const char *name, nc_type xtype,
	   size_t len, const void *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, value, xtype);
}

int
nc_put_att_schar(int ncid, int varid, const char *name,
		 nc_type xtype, size_t len, const signed char *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, (void *)value, NC_BYTE);
}

int
nc_put_att_uchar(int ncid, int varid, const char *name,
		 nc_type xtype, size_t len, const unsigned char *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, (void *)value, NC_UBYTE);
}

int
nc_put_att_short(int ncid, int varid, const char *name,
		 nc_type xtype, size_t len, const short *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, (void *)value, NC_SHORT);
}

int
nc_put_att_int(int ncid, int varid, const char *name,
	       nc_type xtype, size_t len, const int *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, (void *)value, NC_INT);
}

int
nc_put_att_long(int ncid, int varid, const char *name,
		nc_type xtype, size_t len, const long *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, (void *)value, longtype);
}

int
nc_put_att_float(int ncid, int varid, const char *name,
		 nc_type xtype, size_t len, const float *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, (void *)value, NC_FLOAT);
}

int
nc_put_att_double(int ncid, int varid, const char *name,
		  nc_type xtype, size_t len, const double *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, (void *)value, NC_DOUBLE);
}

int
nc_put_att_ubyte(int ncid, int varid, const char *name,
		 nc_type xtype, size_t len, const unsigned char *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, (void *)value, NC_UBYTE);
}

int
nc_put_att_ushort(int ncid, int varid, const char *name,
		  nc_type xtype, size_t len, const unsigned short *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, (void *)value, NC_USHORT);
}

int
nc_put_att_uint(int ncid, int varid, const char *name,
		nc_type xtype, size_t len, const unsigned int *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, (void *)value, NC_UINT);
}

int
nc_put_att_longlong(int ncid, int varid, const char *name,
		    nc_type xtype, size_t len,
		    const long long *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, (void *)value, NC_INT64);
}

int
nc_put_att_ulonglong(int ncid, int varid, const char *name,
		     nc_type xtype, size_t len,
		     const unsigned long long *value)
{
   return NC_put_att(ncid, varid, name, xtype, len, (void *)value, NC_UINT64);
}

/**************************************************/
/* Originnally from ddim.c */

/*! \ingroup dimensions \{*/ /* All these functions are part of the above defgroup... */

/*!
Find out the name of a dimension.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param dimid Dimension ID, from a previous call to nc_inq_dimid() or
nc_def_dim().

\param name Returned dimension name. The caller must allocate space
for the returned name. The maximum possible length, in characters, of
a dimension name is given by the predefined constant
NC_MAX_NAME. (This doesn't include the null terminator, so declare
your array to be size NC_MAX_NAME+1). The returned character array
will be null-terminated. Ignored if NULL.

\returns ::NC_NOERR   No error.
\returns ::NC_EBADID  Not a valid ID.
\returns ::NC_EBADDIM Invalid dimension ID or name.

\section nc_inq_dim_example2 Example

Here is an example using nc_inq_dim() to determine the length of a
dimension named lat, and the name and current maximum length of the
unlimited dimension for an existing netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int status, ncid, latid, recid;
     size_t latlength, recs;
     char recname[NC_MAX_NAME+1];
        ...
     status = nc_open("foo.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_unlimdim(ncid, &recid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_dimid(ncid, "lat", &latid);
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_dimlen(ncid, latid, &latlength);
     if (status != NC_NOERR) handle_error(status);

     status = nc_inq_dim(ncid, recid, recname, &recs);
     if (status != NC_NOERR) handle_error(status);
\endcode

 */
int
nc_inq_dimname(int ncid, int dimid, char *name)
{
    if(name == NULL) return NC_NOERR;
    TRACE(nc_inq_dimname);
    return nc_inq_dim(ncid,dimid,name,NULL);
}

/*!
Find the length of a dimension.

The length for the unlimited dimension, if any, is the number of
records written so far.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param dimid Dimension ID, from a previous call to nc_inq_dimid() or
nc_def_dim().

\param lenp Pointer where the length will be stored.

\returns ::NC_NOERR   No error.
\returns ::NC_EBADID  Not a valid ID.
\returns ::NC_EBADDIM Invalid dimension ID or name.

\section nc_inq_dim_example3 Example

Here is an example using nc_inq_dim() to determine the length of a
dimension named lat, and the name and current maximum length of the
unlimited dimension for an existing netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int status, ncid, latid, recid;
     size_t latlength, recs;
     char recname[NC_MAX_NAME+1];
        ...
     status = nc_open("foo.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_unlimdim(ncid, &recid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_dimid(ncid, "lat", &latid);
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_dimlen(ncid, latid, &latlength);
     if (status != NC_NOERR) handle_error(status);

     status = nc_inq_dim(ncid, recid, recname, &recs);
     if (status != NC_NOERR) handle_error(status);
\endcode
 */
int
nc_inq_dimlen(int ncid, int dimid, size_t *lenp)
{
    if(lenp == NULL) return NC_NOERR;
    TRACE(nc_inq_dimlen);
    return nc_inq_dim(ncid,dimid,NULL,lenp);
}

/*! \} */  /* End of named group ...*/

/**************************************************/
/* Originally in dvarget.c*/

/** \name Reading Data from Variables

Functions to read data from variables. */
/*! \{ */ /* All these functions are part of this named group... */

/** \ingroup variables
Read an array of values from a variable.

The array to be read is specified by giving a corner and a vector of
edge lengths to \ref specify_hyperslab.

The data values are read into consecutive locations with the last
dimension varying fastest. The netCDF dataset must be in data mode
(for netCDF-4/HDF5 files, the switch to data mode will happen
automatically, unless the classic model is used).

The nc_get_vara() function will read a variable of any type,
including user defined type. For this function, the type of the data
in memory must match the type of the variable - no data conversion is
done.

Other nc_get_vara_ functions will convert data to the desired output
type as needed.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param startp Start vector with one element for each dimension to \ref
specify_hyperslab.

\param countp Count vector with one element for each dimension to \ref
specify_hyperslab.

\param ip Pointer where the data will be copied. Memory must be
allocated by the user before this function is called.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_EEDGE Start+count exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.

\section nc_get_vara_double_example Example

Here is an example using nc_get_vara_double() to read all the values of
the variable named rh from an existing netCDF dataset named
foo.nc. For simplicity in this example, we assume that we know that rh
is dimensioned with time, lat, and lon, and that there are three time
values, five lat values, and ten lon values.

\code
     #include <netcdf.h>
        ...
     #define TIMES 3
     #define LATS 5
     #define LONS 10
     int  status;
     int ncid;
     int rh_id;
     static size_t start[] = {0, 0, 0};
     static size_t count[] = {TIMES, LATS, LONS};
     double rh_vals[TIMES*LATS*LONS];
        ...
     status = nc_open("foo.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_get_vara_double(ncid, rh_id, start, count, rh_vals);
     if (status != NC_NOERR) handle_error(status);
\endcode
 */
/**@{*/
int
nc_get_vara(int ncid, int varid, const size_t *startp,
	    const size_t *countp, void *ip)
{
   nc_type xtype = NC_NAT;
   int stat = nc_inq_vartype(ncid, varid, &xtype);
   if(stat != NC_NOERR) return stat;
   return NC_get_vara(ncid, varid, startp, countp, ip, xtype);
}

int
nc_get_vara_text(int ncid, int varid, const size_t *startp,
		 const size_t *countp, char *ip)
{
   return NC_get_vara(ncid, varid, startp, countp, (void *)ip, NC_CHAR);
}

int
nc_get_vara_schar(int ncid, int varid, const size_t *startp,
		  const size_t *countp, signed char *ip)
{
   return NC_get_vara(ncid, varid, startp, countp, (void *)ip, NC_BYTE);
}

int
nc_get_vara_uchar(int ncid, int varid, const size_t *startp,
		  const size_t *countp, unsigned char *ip)
{
   return NC_get_vara(ncid, varid, startp, countp, (void *)ip, T_uchar);
}

int
nc_get_vara_short(int ncid, int varid, const size_t *startp,
		  const size_t *countp, short *ip)
{
   return NC_get_vara(ncid, varid, startp, countp, (void *)ip, NC_SHORT);
}

int
nc_get_vara_int(int ncid, int varid,
		const size_t *startp, const size_t *countp, int *ip)
{
   return NC_get_vara(ncid,varid,startp,countp, (void *)ip,NC_INT);
}

int
nc_get_vara_long(int ncid, int varid,
		 const size_t *startp, const size_t *countp, long *ip)
{
   return NC_get_vara(ncid,varid,startp,countp, (void *)ip,T_long);
}

int
nc_get_vara_float(int ncid, int varid,
		  const size_t *startp, const size_t *countp, float *ip)
{
   return NC_get_vara(ncid,varid,startp,countp, (void *)ip,T_float);
}


int
nc_get_vara_double(int ncid, int varid, const size_t *startp,
		   const size_t *countp, double *ip)
{
   return NC_get_vara(ncid,varid,startp,countp, (void *)ip,T_double);
}

int
nc_get_vara_ubyte(int ncid, int varid,
		  const size_t *startp, const size_t *countp, unsigned char *ip)
{
   return NC_get_vara(ncid,varid,startp,countp, (void *)ip,T_ubyte);
}

int
nc_get_vara_ushort(int ncid, int varid,
		   const size_t *startp, const size_t *countp, unsigned short *ip)
{
   return NC_get_vara(ncid,varid,startp,countp, (void *)ip,T_ushort);
}

int
nc_get_vara_uint(int ncid, int varid,
		 const size_t *startp, const size_t *countp, unsigned int *ip)
{
   return NC_get_vara(ncid,varid,startp,countp, (void *)ip,T_uint);
}

int
nc_get_vara_longlong(int ncid, int varid,
		     const size_t *startp, const size_t *countp, long long *ip)
{
   return NC_get_vara(ncid,varid,startp,countp, (void *)ip,T_longlong);
}

int
nc_get_vara_ulonglong(int ncid, int varid,
		      const size_t *startp, const size_t *countp, unsigned long long *ip)
{
   return NC_get_vara(ncid,varid,startp,countp, (void *)ip,NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_get_vara_string(int ncid, int varid,
		   const size_t *startp, const size_t *countp, char* *ip)
{
   return NC_get_vara(ncid,varid,startp,countp, (void *)ip,NC_STRING);
}

#endif /*USE_NETCDF4*/
/**@}*/

/** \ingroup variables
Read a single datum from a variable.

Inputs are the netCDF ID, the variable ID, a multidimensional index
that specifies which value to get, and the address of a location into
which the data value will be read. The value is converted from the
external data type of the variable, if necessary.

The nc_get_var1() function will read a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

Other nc_get_var1_ functions will convert data to the desired output
type as needed.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param indexp Index vector with one element for each dimension.

\param ip Pointer where the data will be copied. Memory must be
allocated by the user before this function is called.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
*/
/** \{ */
int
nc_get_var1(int ncid, int varid, const size_t *indexp, void *ip)
{
   return NC_get_var1(ncid, varid, indexp, ip, NC_NAT);
}

int
nc_get_var1_text(int ncid, int varid, const size_t *indexp, char *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_CHAR);
}

int
nc_get_var1_schar(int ncid, int varid, const size_t *indexp, signed char *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_BYTE);
}

int
nc_get_var1_uchar(int ncid, int varid, const size_t *indexp, unsigned char *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_UBYTE);
}

int
nc_get_var1_short(int ncid, int varid, const size_t *indexp, short *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_SHORT);
}

int
nc_get_var1_int(int ncid, int varid, const size_t *indexp, int *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_INT);
}

int
nc_get_var1_long(int ncid, int varid, const size_t *indexp,
		 long *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, longtype);
}

int
nc_get_var1_float(int ncid, int varid, const size_t *indexp,
		  float *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_FLOAT);
}

int
nc_get_var1_double(int ncid, int varid, const size_t *indexp,
		   double *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_DOUBLE);
}

int
nc_get_var1_ubyte(int ncid, int varid, const size_t *indexp,
		  unsigned char *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_UBYTE);
}

int
nc_get_var1_ushort(int ncid, int varid, const size_t *indexp,
		   unsigned short *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_USHORT);
}

int
nc_get_var1_uint(int ncid, int varid, const size_t *indexp,
		 unsigned int *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_UINT);
}

int
nc_get_var1_longlong(int ncid, int varid, const size_t *indexp,
		     long long *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_INT64);
}

int
nc_get_var1_ulonglong(int ncid, int varid, const size_t *indexp,
		      unsigned long long *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_get_var1_string(int ncid, int varid, const size_t *indexp, char* *ip)
{
   return NC_get_var1(ncid, varid, indexp, (void *)ip, NC_STRING);
}
#endif /*USE_NETCDF4*/
/** \} */

/** \ingroup variables
Read an entire variable in one call.

This function will read all the values from a netCDF variable of an
open netCDF dataset.

This is the simplest interface to use for reading the value of a
scalar variable or when all the values of a multidimensional variable
can be read at once. The values are read into consecutive locations
with the last dimension varying fastest. The netCDF dataset must be in
data mode.

Take care when using this function with record variables (variables
that use the ::NC_UNLIMITED dimension). If you try to read all the
values of a record variable into an array but there are more records
in the file than you assume, more data will be read than you expect,
which may cause a segmentation violation. To avoid such problems, it
is better to use the nc_get_vara interfaces for variables that use the
::NC_UNLIMITED dimension.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc_get_var() function will read a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param ip Pointer where the data will be copied. Memory must be
allocated by the user before this function is called.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
*/
/** \{ */
int
nc_get_var(int ncid, int varid, void *ip)
{
   return NC_get_var(ncid, varid, ip, NC_NAT);
}

int
nc_get_var_text(int ncid, int varid, char *ip)
{
   return NC_get_var(ncid, varid, (void *)ip, NC_CHAR);
}

int
nc_get_var_schar(int ncid, int varid, signed char *ip)
{
   return NC_get_var(ncid, varid, (void *)ip, NC_BYTE);
}

int
nc_get_var_uchar(int ncid, int varid, unsigned char *ip)
{
   return NC_get_var(ncid,varid, (void *)ip, NC_UBYTE);
}

int
nc_get_var_short(int ncid, int varid, short *ip)
{
   return NC_get_var(ncid, varid, (void *)ip, NC_SHORT);
}

int
nc_get_var_int(int ncid, int varid, int *ip)
{
   return NC_get_var(ncid,varid, (void *)ip, NC_INT);
}

int
nc_get_var_long(int ncid, int varid, long *ip)
{
   return NC_get_var(ncid,varid, (void *)ip, longtype);
}

int
nc_get_var_float(int ncid, int varid, float *ip)
{
   return NC_get_var(ncid,varid, (void *)ip, NC_FLOAT);
}

int
nc_get_var_double(int ncid, int varid, double *ip)
{
   return NC_get_var(ncid,varid, (void *)ip, NC_DOUBLE);
}

int
nc_get_var_ubyte(int ncid, int varid, unsigned char *ip)
{
   return NC_get_var(ncid,varid, (void *)ip, NC_UBYTE);
}

int
nc_get_var_ushort(int ncid, int varid, unsigned short *ip)
{
   return NC_get_var(ncid,varid, (void *)ip, NC_USHORT);
}

int
nc_get_var_uint(int ncid, int varid, unsigned int *ip)
{
   return NC_get_var(ncid,varid, (void *)ip, NC_UINT);
}

int
nc_get_var_longlong(int ncid, int varid, long long *ip)
{
   return NC_get_var(ncid,varid, (void *)ip, NC_INT64);
}

int
nc_get_var_ulonglong(int ncid, int varid, unsigned long long *ip)
{
   return NC_get_var(ncid,varid, (void *)ip,NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_get_var_string(int ncid, int varid, char* *ip)
{
   return NC_get_var(ncid,varid, (void *)ip,NC_STRING);
}
#endif /*USE_NETCDF4*/
/** \} */

/** \ingroup variables
Read a strided array from a variable.

This function reads a subsampled (strided) array section of values
from a netCDF variable of an open netCDF dataset. The subsampled array
section is specified by giving a corner, a vector of edge lengths, and
a stride vector. The values are read with the last dimension of the
netCDF variable varying fastest. The netCDF dataset must be in data
mode.

The nc_get_vars() function will read a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param startp Start vector with one element for each dimension to \ref
specify_hyperslab.

\param countp Count vector with one element for each dimension to \ref
specify_hyperslab.

\param stridep Stride vector with one element for each dimension to
\ref specify_hyperslab.

\param ip Pointer where the data will be copied. Memory must be
allocated by the user before this function is called.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
*/
/** \{ */
int
nc_get_vars(int ncid, int varid, const size_t * startp,
	     const size_t * countp, const ptrdiff_t * stridep,
	     void *ip)
{
   nc_type xtype = NC_NAT;
   int stat = nc_inq_vartype(ncid, varid, &xtype);
   if(stat != NC_NOERR) return stat;
   return NC_get_vars(ncid, varid, startp, countp, stridep, ip, xtype);
}

int
nc_get_vars_text(int ncid, int varid, const size_t *startp,
		 const size_t *countp, const ptrdiff_t * stridep,
		 char *ip)
{
   return NC_get_vars(ncid,varid,startp, countp, stridep, (void *)ip, NC_CHAR);
}

int
nc_get_vars_schar(int ncid, int varid, const size_t *startp,
		  const size_t *countp, const ptrdiff_t * stridep,
		  signed char *ip)
{
   return NC_get_vars(ncid,varid,startp, countp, stridep, (void *)ip, NC_BYTE);
}

int
nc_get_vars_uchar(int ncid, int varid, const size_t *startp,
		  const size_t *countp, const ptrdiff_t * stridep,
		  unsigned char *ip)
{
   return NC_get_vars(ncid,varid,startp, countp, stridep, (void *)ip, T_uchar);
}

int
nc_get_vars_short(int ncid, int varid, const size_t *startp,
		  const size_t *countp, const ptrdiff_t *stridep,
		  short *ip)
{
   return NC_get_vars(ncid,varid,startp, countp, stridep, (void *)ip, NC_SHORT);
}

int
nc_get_vars_int(int ncid, int varid, const size_t *startp,
		const size_t *countp, const ptrdiff_t * stridep,
		int *ip)
{
   return NC_get_vars(ncid,varid,startp, countp, stridep, (void *)ip, NC_INT);
}

int
nc_get_vars_long(int ncid, int varid, const size_t *startp,
		 const size_t *countp, const ptrdiff_t * stridep,
		 long *ip)
{
   return NC_get_vars(ncid,varid,startp, countp, stridep, (void *)ip, T_long);
}

int
nc_get_vars_float(int ncid, int varid, const size_t *startp,
		  const size_t *countp, const ptrdiff_t * stridep,
		  float *ip)
{
   return NC_get_vars(ncid,varid,startp, countp, stridep, (void *)ip, T_float);
}

int
nc_get_vars_double(int ncid, int varid, const size_t *startp,
		   const size_t *countp, const ptrdiff_t * stridep,
		   double *ip)
{
   return NC_get_vars(ncid,varid,startp, countp, stridep, (void *)ip, T_double);
}

int
nc_get_vars_ubyte(int ncid, int varid, const size_t *startp,
		  const size_t *countp, const ptrdiff_t * stridep,
		  unsigned char *ip)
{
   return NC_get_vars(ncid,varid, startp, countp, stridep, (void *)ip, T_ubyte);
}

int
nc_get_vars_ushort(int ncid, int varid, const size_t *startp,
		   const size_t *countp, const ptrdiff_t * stridep,
		   unsigned short *ip)
{
   return NC_get_vars(ncid,varid,startp,countp, stridep, (void *)ip, T_ushort);
}

int
nc_get_vars_uint(int ncid, int varid, const size_t *startp,
		 const size_t *countp, const ptrdiff_t * stridep,
		 unsigned int *ip)
{
   return NC_get_vars(ncid,varid,startp, countp, stridep, (void *)ip, T_uint);
}

int
nc_get_vars_longlong(int ncid, int varid, const size_t *startp,
		     const size_t *countp, const ptrdiff_t * stridep,
		     long long *ip)
{
   return NC_get_vars(ncid, varid, startp, countp, stridep, (void *)ip, T_longlong);
}

int
nc_get_vars_ulonglong(int ncid, int varid, const size_t *startp,
		      const size_t *countp, const ptrdiff_t * stridep,
		      unsigned long long *ip)
{
   return NC_get_vars(ncid, varid, startp, countp, stridep, (void *)ip, NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_get_vars_string(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t * stridep,
		   char* *ip)
{
   return NC_get_vars(ncid, varid, startp, countp, stridep, (void *)ip, NC_STRING);
}
#endif /*USE_NETCDF4*/
/** \} */

/** \ingroup variables
Read a mapped array from a variable.

The nc_get_varm_ type family of functions reads a mapped array section
of values from a netCDF variable of an open netCDF dataset. The mapped
array section is specified by giving a corner, a vector of edge
lengths, a stride vector, and an index mapping vector. The index
mapping vector is a vector of integers that specifies the mapping
between the dimensions of a netCDF variable and the in-memory
structure of the internal data array. No assumptions are made about
the ordering or length of the dimensions of the data array. The netCDF
dataset must be in data mode.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc_get_varm() function will only read a variable of an
atomic type; it will not read user defined types. For this
function, the type of the data in memory must match the type
of the variable - no data conversion is done.

@deprecated Use of this family of functions is discouraged,
although it will continue to be supported.
The reason is the complexity of the
algorithm makes its use difficult for users to properly use.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param startp Start vector with one element for each dimension to \ref
specify_hyperslab.

\param countp Count vector with one element for each dimension to \ref
specify_hyperslab.

\param stridep Stride vector with one element for each dimension to
\ref specify_hyperslab.

\param imapp Mapping vector with one element for each dimension to
\ref specify_hyperslab.

\param ip Pointer where the data will be copied. Memory must be
allocated by the user before this function is called.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
*/
/** \{ */
int
nc_get_varm(int ncid, int varid, const size_t * startp,
	    const size_t * countp, const ptrdiff_t * stridep,
	    const ptrdiff_t * imapp, void *ip)
{
   return NC_get_varm(ncid, varid, startp, countp, stridep, imapp, ip, NC_NAT);
}

int
nc_get_varm_schar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep,
		  const ptrdiff_t *imapp, signed char *ip)
{
   return NC_get_varm(ncid, varid, startp, countp,
		      stridep, imapp, (void *)ip, NC_BYTE);
}

int
nc_get_varm_uchar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  unsigned char *ip)
{
   return NC_get_varm(ncid,varid,startp,countp,stridep,imapp, (void *)ip,T_uchar);
}

int
nc_get_varm_short(int ncid, int varid, const size_t *startp,
		  const size_t *countp, const ptrdiff_t *stridep,
		  const ptrdiff_t *imapp, short *ip)
{
   return NC_get_varm(ncid,varid,startp,countp,stridep,imapp, (void *)ip,NC_SHORT);
}

int
nc_get_varm_int(int ncid, int varid,
		const size_t *startp, const size_t *countp,
		const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		int *ip)
{
   return NC_get_varm(ncid,varid,startp,countp,stridep,imapp, (void *)ip,NC_INT);
}

int
nc_get_varm_long(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		 long *ip)
{
   return NC_get_varm(ncid,varid,startp,countp,stridep,imapp, (void *)ip,T_long);
}

int
nc_get_varm_float(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  float *ip)
{
   return NC_get_varm(ncid,varid,startp,countp,stridep,imapp, (void *)ip,T_float);
}

int
nc_get_varm_double(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		   double *ip)
{
   return NC_get_varm(ncid,varid,startp,countp,stridep,imapp, (void *)ip,T_double);
}

int
nc_get_varm_ubyte(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  unsigned char *ip)
{
   return NC_get_varm(ncid,varid,startp,countp,stridep,
		      imapp, (void *)ip, T_ubyte);
}

int
nc_get_varm_ushort(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		   unsigned short *ip)
{
   return NC_get_varm(ncid, varid, startp, countp, stridep,
		      imapp, (void *)ip, T_ushort);
}

int
nc_get_varm_uint(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		 unsigned int *ip)
{
   return NC_get_varm(ncid, varid, startp, countp,
		      stridep, imapp, (void *)ip, T_uint);
}

int
nc_get_varm_longlong(int ncid, int varid, const size_t *startp,
		     const size_t *countp, const ptrdiff_t *stridep,
		     const ptrdiff_t *imapp, long long *ip)
{
   return NC_get_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)ip, T_longlong);
}

int
nc_get_varm_ulonglong(int ncid, int varid,
		      const size_t *startp, const size_t *countp,
		      const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		      unsigned long long *ip)
{
   return NC_get_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)ip, NC_UINT64);
}

int
nc_get_varm_text(int ncid, int varid, const size_t *startp,
		 const size_t *countp, const ptrdiff_t *stridep,
		 const ptrdiff_t *imapp, char *ip)
{
   return NC_get_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)ip, NC_CHAR);
}

#ifdef USE_NETCDF4
int
nc_get_varm_string(int ncid, int varid, const size_t *startp,
		   const size_t *countp, const ptrdiff_t *stridep,
		   const ptrdiff_t *imapp, char **ip)
{
   return NC_get_varm(ncid, varid, startp, countp, stridep, imapp,
		      (void *)ip, NC_STRING);
}


/** \ingroup variables
\internal
 */
static int
NC_get_var(int ncid, int varid, void *value, nc_type memtype)
{
   int ndims;
   size_t shape[NC_MAX_VAR_DIMS];
   int stat = nc_inq_varndims(ncid,varid, &ndims);
   if(stat) return stat;
   stat = NC_getshape(ncid,varid, ndims, shape);
   if(stat) return stat;
   return NC_get_vara(ncid, varid, NC_coord_zero, shape, value, memtype);
}

/** \internal
\ingroup variables
 */
static int
NC_get_var1(int ncid, int varid, const size_t *coord, void* value, nc_type memtype)
{
   return NC_get_vara(ncid, varid, coord, NC_coord_one, value, memtype);
}


/** \internal
\ingroup variables
 Most dispatch tables will use the default procedures
*/
int
NCDEFAULT_get_vars(int ncid, int varid, const size_t * start,
	    const size_t * edges, const ptrdiff_t * stride,
	    void *value0, nc_type memtype)
{
  /* Rebuilt get_vars code to simplify and avoid use of get_varm */

   int status = NC_NOERR;
   int i,simplestride,isrecvar;
   int rank;
   struct Odometer odom;
   nc_type vartype = NC_NAT;
   NC* ncp;
   int memtypelen;
   size_t vartypelen;
   char* value = (char*)value0;
   size_t numrecs;
   size_t varshape[NC_MAX_VAR_DIMS];
   size_t mystart[NC_MAX_VAR_DIMS];
   size_t myedges[NC_MAX_VAR_DIMS];
   ptrdiff_t mystride[NC_MAX_VAR_DIMS];
   char *memptr = NULL;

   status = NC_check_id (ncid, &ncp);
   if(status != NC_NOERR) return status;

   status = nc_inq_vartype(ncid, varid, &vartype);
   if(status != NC_NOERR) return status;

   if(memtype == NC_NAT) memtype = vartype;

   /* compute the variable type size */
   status = nc_inq_type(ncid,vartype,NULL,&vartypelen);
   if(status != NC_NOERR) return status;

   if(memtype > NC_MAX_ATOMIC_TYPE)
	memtypelen = (int)vartypelen;
    else
	memtypelen = nctypelen(memtype);

   /* Check gross internal/external type compatibility */
   if(vartype != memtype) {
      /* If !atomic, the two types must be the same */
      if(vartype > NC_MAX_ATOMIC_TYPE
         || memtype > NC_MAX_ATOMIC_TYPE)
	 return NC_EBADTYPE;
      /* ok, the types differ but both are atomic */
      if(memtype == NC_CHAR || vartype == NC_CHAR)
	 return NC_ECHAR;
   }

   /* Get the variable rank */
   status = nc_inq_varndims(ncid, varid, &rank);
   if(status != NC_NOERR) return status;

   /* Get variable dimension sizes */
   isrecvar = NC_is_recvar(ncid,varid,&numrecs);
   NC_getshape(ncid,varid,rank,varshape);

   /* Optimize out using various checks */
   if (rank == 0) {
      /*
       * The variable is a scalar; consequently,
       * there s only one thing to get and only one place to put it.
       * (Why was I called?)
       */
      size_t edge1[1] = {1};
      return NC_get_vara(ncid, varid, start, edge1, value, memtype);
   }

   /* Do various checks and fixups on start/edges/stride */
   simplestride = 1; /* assume so */
   for(i=0;i<rank;i++) {
	size_t dimlen;
	mystart[i] = (start == NULL ? 0 : start[i]);
	if(edges == NULL) {
	   if(i == 0 && isrecvar)
  	      myedges[i] = numrecs - start[i];
	   else
	      myedges[i] = varshape[i] - mystart[i];
	} else
	    myedges[i] = edges[i];
	if(myedges[i] == 0)
	    return NC_NOERR; /* cannot read anything */
	mystride[i] = (stride == NULL ? 1 : stride[i]);
	if(mystride[i] <= 0
	   /* cast needed for braindead systems with signed size_t */
           || ((unsigned long) mystride[i] >= X_INT_MAX))
           return NC_ESTRIDE;
  	if(mystride[i] != 1) simplestride = 0;
        /* illegal value checks */
	dimlen = (i == 0 && isrecvar ? numrecs : varshape[i]);
        /* mystart is unsigned, never < 0 */
	if(mystart[i] >= dimlen)
	  return NC_EINVALCOORDS;
        /* myedges is unsigned, never < 0 */
	if(mystart[i] + myedges[i] > dimlen)
	  return NC_EEDGE;
   }
   if(simplestride) {
      return NC_get_vara(ncid, varid, mystart, myedges, value, memtype);
   }

   /* memptr indicates where to store the next value */
   memptr = value;

   odom_init(&odom,rank,mystart,myedges,mystride);

   /* walk the odometer to extract values */
   while(odom_more(&odom)) {
      int localstatus = NC_NOERR;
      /* Read a single value */
      localstatus = NC_get_vara(ncid,varid,odom.index,nc_sizevector1,memptr,memtype);
      /* So it turns out that when get_varm is used, all errors are
         delayed and ERANGE will be overwritten by more serious errors.
      */
      if(localstatus != NC_NOERR) {
	    if(status == NC_NOERR || localstatus != NC_ERANGE)
	       status = localstatus;
      }
      memptr += memtypelen;
      odom_next(&odom);
   }
   return status;
}

/** \internal
\ingroup variables
 */
int
NCDEFAULT_get_varm(int ncid, int varid, const size_t *start,
	    const size_t *edges, const ptrdiff_t *stride,
	    const ptrdiff_t *imapp, void *value0, nc_type memtype)
{
   int status = NC_NOERR;
   nc_type vartype = NC_NAT;
   int varndims,maxidim;
   NC* ncp;
   int memtypelen;
   char* value = (char*)value0;

   status = NC_check_id (ncid, &ncp);
   if(status != NC_NOERR) return status;

/*
  if(NC_indef(ncp)) return NC_EINDEFINE;
*/

   status = nc_inq_vartype(ncid, varid, &vartype);
   if(status != NC_NOERR) return status;
   /* Check that this is an atomic type */
   if(vartype > NC_MAX_ATOMIC_TYPE)
	return NC_EMAPTYPE;

   status = nc_inq_varndims(ncid, varid, &varndims);
   if(status != NC_NOERR) return status;

   if(memtype == NC_NAT) {
      memtype = vartype;
   }

   if(memtype == NC_CHAR && vartype != NC_CHAR)
      return NC_ECHAR;
   else if(memtype != NC_CHAR && vartype == NC_CHAR)
      return NC_ECHAR;

   memtypelen = nctypelen(memtype);

   maxidim = (int) varndims - 1;

   if (maxidim < 0)
   {
      /*
       * The variable is a scalar; consequently,
       * there s only one thing to get and only one place to put it.
       * (Why was I called?)
       */
      size_t edge1[1] = {1};
      return NC_get_vara(ncid, varid, start, edge1, value, memtype);
   }

   /*
    * else
    * The variable is an array.
    */
   {
      int idim;
      size_t *mystart = NULL;
      size_t *myedges;
      size_t *iocount;    /* count vector */
      size_t *stop;   /* stop indexes */
      size_t *length; /* edge lengths in bytes */
      ptrdiff_t *mystride;
      ptrdiff_t *mymap;
      size_t varshape[NC_MAX_VAR_DIMS];
      int isrecvar;
      size_t numrecs;

      /* Compute some dimension related values */
      isrecvar = NC_is_recvar(ncid,varid,&numrecs);
      NC_getshape(ncid,varid,varndims,varshape);

      /*
       * Verify stride argument; also see if stride is all ones
       */
      if(stride != NULL) {
	 int stride1 = 1;
	 for (idim = 0; idim <= maxidim; ++idim)
	 {
            if (stride[idim] == 0
		/* cast needed for braindead systems with signed size_t */
                || ((unsigned long) stride[idim] >= X_INT_MAX))
            {
	       return NC_ESTRIDE;
            }
	    if(stride[idim] != 1) stride1 = 0;
	 }
         /* If stride1 is true, and there is no imap
            then call get_vara directly.
         */
         if(stride1 && imapp == NULL) {
	     return NC_get_vara(ncid, varid, start, edges, value, memtype);
	 }
      }

      /* assert(sizeof(ptrdiff_t) >= sizeof(size_t)); */
      /* Allocate space for mystart,mystride,mymap etc.all at once */
      mystart = (size_t *)calloc((size_t)(varndims * 7), sizeof(ptrdiff_t));
      if(mystart == NULL) return NC_ENOMEM;
      myedges = mystart + varndims;
      iocount = myedges + varndims;
      stop = iocount + varndims;
      length = stop + varndims;
      mystride = (ptrdiff_t *)(length + varndims);
      mymap = mystride + varndims;

      /*
       * Initialize I/O parameters.
       */
      for (idim = maxidim; idim >= 0; --idim)
      {
	 mystart[idim] = start != NULL
	    ? start[idim]
	    : 0;

	 if (edges != NULL && edges[idim] == 0)
	 {
	    status = NC_NOERR;    /* read/write no data */
	    goto done;
	 }

#ifdef COMPLEX
	 myedges[idim] = edges != NULL
	    ? edges[idim]
	    : idim == 0 && isrecvar
	    ? numrecs - mystart[idim]
	    : varshape[idim] - mystart[idim];
#else
	 if(edges != NULL)
	    myedges[idim] = edges[idim];
	 else if (idim == 0 && isrecvar)
	    myedges[idim] = numrecs - mystart[idim];
	 else
	    myedges[idim] = varshape[idim] - mystart[idim];
#endif

	 mystride[idim] = stride != NULL
	    ? stride[idim]
	    : 1;

	 /* Remember: in netCDF-2 imapp is byte oriented, not index oriented
	  *           Starting from netCDF-3, imapp is index oriented */
#ifdef COMPLEX
	 mymap[idim] = (imapp != NULL
			? imapp[idim]
			: (idim == maxidim ? 1
			   : mymap[idim + 1] * (ptrdiff_t) myedges[idim + 1]));
#else
	 if(imapp != NULL)
	    mymap[idim] = imapp[idim];
	 else if (idim == maxidim)
	    mymap[idim] = 1;
	 else
	    mymap[idim] =
	       mymap[idim + 1] * (ptrdiff_t) myedges[idim + 1];
#endif
	 iocount[idim] = 1;
	 length[idim] = ((size_t)mymap[idim]) * myedges[idim];
	 stop[idim] = (mystart[idim] + myedges[idim] * (size_t)mystride[idim]);
      }

      /*
       * Check start, edges
       */
      for (idim = maxidim; idim >= 0; --idim)
      {
	 size_t dimlen =
	    idim == 0 && isrecvar
	    ? numrecs
	    : varshape[idim];
	 if (mystart[idim] >= dimlen)
	 {
	    status = NC_EINVALCOORDS;
	    goto done;
	 }

	 if (mystart[idim] + myedges[idim] > dimlen)
	 {
	    status = NC_EEDGE;
	    goto done;
	 }

      }


      /* Lower body */
      /*
       * As an optimization, adjust I/O parameters when the fastest
       * dimension has unity stride both externally and internally.
       * In this case, the user could have called a simpler routine
       * (i.e. ncvar$1()
       */
      if (mystride[maxidim] == 1
	  && mymap[maxidim] == 1)
      {
	 iocount[maxidim] = myedges[maxidim];
	 mystride[maxidim] = (ptrdiff_t) myedges[maxidim];
	 mymap[maxidim] = (ptrdiff_t) length[maxidim];
      }

      /*
       * Perform I/O.  Exit when done.
       */
      for (;;)
      {
	 /* TODO: */
	 int lstatus = NC_get_vara(ncid, varid, mystart, iocount,
				   value, memtype);
	 if (lstatus != NC_NOERR) {
	    if(status == NC_NOERR || lstatus != NC_ERANGE)
	       status = lstatus;
	 }
	 /*
	  * The following code permutes through the variable s
	  * external start-index space and it s internal address
	  * space.  At the UPC, this algorithm is commonly
	  * called "odometer code".
	  */
	 idim = maxidim;
        carry:
	 value += (((int)mymap[idim]) * memtypelen);
	 mystart[idim] += (size_t)mystride[idim];
	 if (mystart[idim] == stop[idim])
	 {
	    size_t l = (length[idim] * (size_t)memtypelen);
	    value -= l;
	    mystart[idim] = start[idim];
	    if (--idim < 0)
	       break; /* normal return */
	    goto carry;
	 }
      } /* I/O loop */
     done:
      free(mystart);
   } /* variable is array */
   return status;
}

/** \} */
#endif /*USE_NETCDF4*/

/** \name Writing Data to Variables

Functions to write data from variables. */
/*! \{ */ /* All these functions are part of this named group... */

/** \ingroup variables
Write an array of values to a variable.

The values to be written are associated with the netCDF variable by
assuming that the last dimension of the netCDF variable varies fastest
in the C interface. The netCDF dataset must be in data mode. The array
to be written is specified by giving a corner and a vector of edge
lengths to \ref specify_hyperslab.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc_put_var() function will write a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param startp Start vector with one element for each dimension to \ref
specify_hyperslab.

\param countp Count vector with one element for each dimension to \ref
specify_hyperslab.

\param op Pointer where the data will be copied. Memory must be
allocated by the user before this function is called.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_EEDGE Start+count exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
 */
/**@{*/
int
nc_put_vara(int ncid, int varid, const size_t *startp,
	    const size_t *countp, const void *op)
{
   nc_type xtype;
   int stat = nc_inq_vartype(ncid, varid, &xtype);
   if(stat != NC_NOERR) return stat;
   return NC_put_vara(ncid, varid, startp, countp, op, xtype);
}

int
nc_put_vara_text(int ncid, int varid, const size_t *startp,
		 const size_t *countp, const char *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void*)op, NC_CHAR);
}

int
nc_put_vara_schar(int ncid, int varid, const size_t *startp,
		  const size_t *countp, const signed char *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, NC_BYTE);
}

int
nc_put_vara_uchar(int ncid, int varid, const size_t *startp,
		  const size_t *countp, const unsigned char *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, T_uchar);
}

int
nc_put_vara_short(int ncid, int varid, const size_t *startp,
		  const size_t *countp, const short *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, NC_SHORT);
}

int
nc_put_vara_int(int ncid, int varid, const size_t *startp,
		const size_t *countp, const int *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, NC_INT);
}

int
nc_put_vara_long(int ncid, int varid, const size_t *startp,
		 const size_t *countp, const long *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, T_long);
}

int
nc_put_vara_float(int ncid, int varid, const size_t *startp,
		  const size_t *countp, const float *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, T_float);
}

int
nc_put_vara_double(int ncid, int varid, const size_t *startp,
		   const size_t *countp, const double *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, T_double);
}

int
nc_put_vara_ubyte(int ncid, int varid, const size_t *startp,
		  const size_t *countp, const unsigned char *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, T_ubyte);
}

int
nc_put_vara_ushort(int ncid, int varid, const size_t *startp,
		   const size_t *countp, const unsigned short *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, T_ushort);
}

int
nc_put_vara_uint(int ncid, int varid, const size_t *startp,
		 const size_t *countp, const unsigned int *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, T_uint);
}

int
nc_put_vara_longlong(int ncid, int varid, const size_t *startp,
		     const size_t *countp, const long long *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, T_longlong);
}

int
nc_put_vara_ulonglong(int ncid, int varid, const size_t *startp,
		      const size_t *countp, const unsigned long long *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_vara_string(int ncid, int varid, const size_t *startp,
		   const size_t *countp, const char* *op)
{
   return NC_put_vara(ncid, varid, startp, countp, (void *)op, NC_STRING);
}

#endif /*USE_NETCDF4*/
/**@}*/

/** \ingroup variables
Write one datum.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param indexp Index vector with one element for each dimension.

\param op Pointer from where the data will be copied.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_EEDGE Start+count exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
 */
/**@{*/
int
nc_put_var1(int ncid, int varid, const size_t *indexp, const void *op)
{
   return NC_put_var1(ncid, varid, indexp, op, NC_NAT);
}

int
nc_put_var1_text(int ncid, int varid, const size_t *indexp, const char *op)
{
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_CHAR);
}

int
nc_put_var1_schar(int ncid, int varid, const size_t *indexp, const signed char *op)
{
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_BYTE);
}

int
nc_put_var1_uchar(int ncid, int varid, const size_t *indexp, const unsigned char *op)
{
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_UBYTE);
}

int
nc_put_var1_short(int ncid, int varid, const size_t *indexp, const short *op)
{
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_SHORT);
}

int
nc_put_var1_int(int ncid, int varid, const size_t *indexp, const int *op)
{
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_INT);
}

int
nc_put_var1_long(int ncid, int varid, const size_t *indexp, const long *op)
{
   return NC_put_var1(ncid, varid, indexp, (void*)op, longtype);
}

int
nc_put_var1_float(int ncid, int varid, const size_t *indexp, const float *op)
{
   return NC_put_var1(ncid, varid, indexp, (void*)op, NC_FLOAT);
}

int
nc_put_var1_double(int ncid, int varid, const size_t *indexp, const double *op)
{
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_DOUBLE);
}

int
nc_put_var1_ubyte(int ncid, int varid, const size_t *indexp, const unsigned char *op)
{
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_UBYTE);
}

int
nc_put_var1_ushort(int ncid, int varid, const size_t *indexp, const unsigned short *op)
{
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_USHORT);
}

int
nc_put_var1_uint(int ncid, int varid, const size_t *indexp, const unsigned int *op)
{
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_UINT);
}

int
nc_put_var1_longlong(int ncid, int varid, const size_t *indexp, const long long *op)
{
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_INT64);
}

int
nc_put_var1_ulonglong(int ncid, int varid, const size_t *indexp, const unsigned long long *op)
{
   return NC_put_var1(ncid, varid, indexp, (void *)op, NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_var1_string(int ncid, int varid, const size_t *indexp, const char* *op)
{
   return NC_put_var1(ncid, varid, indexp, (void*)op, NC_STRING);
}
#endif /*USE_NETCDF4*/
/**@}*/

/** \ingroup variables
Write an entire variable with one call.

The nc_put_var_ type family of functions write all the values of a
variable into a netCDF variable of an open netCDF dataset. This is the
simplest interface to use for writing a value in a scalar variable or
whenever all the values of a multidimensional variable can all be
written at once. The values to be written are associated with the
netCDF variable by assuming that the last dimension of the netCDF
variable varies fastest in the C interface. The values are converted
to the external data type of the variable, if necessary.

Take care when using this function with record variables (variables
that use the ::NC_UNLIMITED dimension). If you try to write all the
values of a record variable into a netCDF file that has no record data
yet (hence has 0 records), nothing will be written. Similarly, if you
try to write all the values of a record variable but there are more
records in the file than you assume, more in-memory data will be
accessed than you supply, which may result in a segmentation
violation. To avoid such problems, it is better to use the nc_put_vara
interfaces for variables that use the ::NC_UNLIMITED dimension.

The functions for types ubyte, ushort, uint, longlong, ulonglong, and
string are only available for netCDF-4/HDF5 files.

The nc_put_var() function will write a variable of any type, including
user defined type. For this function, the type of the data in memory
must match the type of the variable - no data conversion is done.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param op Pointer from where the data will be copied.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_EEDGE Start+count exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
 */
/**@{*/
int
nc_put_var(int ncid, int varid, const void *op)
{
   return NC_put_var(ncid, varid, op, NC_NAT);
}

int
nc_put_var_text(int ncid, int varid, const char *op)
{
   return NC_put_var(ncid,varid,(void*)op,NC_CHAR);
}

int
nc_put_var_schar(int ncid, int varid, const signed char *op)
{
   return NC_put_var(ncid,varid,(void*)op,NC_BYTE);
}

int
nc_put_var_uchar(int ncid, int varid, const unsigned char *op)
{
   return NC_put_var(ncid,varid,(void*)op,T_uchar);
}

int
nc_put_var_short(int ncid, int varid, const short *op)
{
   return NC_put_var(ncid,varid,(void*)op,NC_SHORT);
}

int
nc_put_var_int(int ncid, int varid, const int *op)
{
   return NC_put_var(ncid,varid,(void*)op,NC_INT);
}

int
nc_put_var_long(int ncid, int varid, const long *op)
{
   return NC_put_var(ncid,varid,(void*)op,T_long);
}

int
nc_put_var_float(int ncid, int varid, const float *op)
{
   return NC_put_var(ncid,varid,(void*)op,T_float);
}

int
nc_put_var_double(int ncid, int varid, const double *op)
{
   return NC_put_var(ncid,varid,(void*)op,T_double);
}

int
nc_put_var_ubyte(int ncid, int varid, const unsigned char *op)
{
   return NC_put_var(ncid,varid,(void*)op,T_ubyte);
}

int
nc_put_var_ushort(int ncid, int varid, const unsigned short *op)
{
   return NC_put_var(ncid,varid,(void*)op,T_ushort);
}

int
nc_put_var_uint(int ncid, int varid, const unsigned int *op)
{
   return NC_put_var(ncid,varid,(void*)op,T_uint);
}

int
nc_put_var_longlong(int ncid, int varid, const long long *op)
{
   return NC_put_var(ncid,varid,(void*)op,T_longlong);
}

int
nc_put_var_ulonglong(int ncid, int varid, const unsigned long long *op)
{
   return NC_put_var(ncid,varid,(void*)op,NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_var_string(int ncid, int varid, const char* *op)
{
   return NC_put_var(ncid,varid,(void*)op,NC_STRING);
}
#endif /*USE_NETCDF4*/
/**\} */

/** \ingroup variables
Write a strided array of values to a variable.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param startp Start vector with one element for each dimension to \ref
specify_hyperslab.

\param countp Count vector with one element for each dimension to \ref
specify_hyperslab.

\param stridep Stride vector with one element for each dimension to
\ref specify_hyperslab.

\param op Pointer where the data will be copied. Memory must be
allocated by the user before this function is called.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_EEDGE Start+count exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
 */
/**@{*/
int
nc_put_vars(int ncid, int varid, const size_t *startp,
	     const size_t *countp, const ptrdiff_t *stridep,
	     const void *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, op, NC_NAT);
}

int
nc_put_vars_text(int ncid, int varid, const size_t *startp,
		 const size_t *countp, const ptrdiff_t *stridep,
		 const char *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep,(void*)op,NC_CHAR);
}

int
nc_put_vars_schar(int ncid, int varid, const size_t *startp,
		  const size_t *countp, const ptrdiff_t *stridep,
		  const signed char *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep,(void*)op,NC_BYTE);
}

int
nc_put_vars_uchar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep,
		  const unsigned char *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, (void *)op, T_uchar);
}

int
nc_put_vars_short(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep,
		  const short *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, (void *)op, NC_SHORT);
}

int
nc_put_vars_int(int ncid, int varid,
		const size_t *startp, const size_t *countp,
		const ptrdiff_t *stridep,
		const int *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, (void *)op, NC_INT);
}

int
nc_put_vars_long(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t *stridep,
		 const long *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, (void *)op, T_long);
}

int
nc_put_vars_float(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep,
		  const float *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, (void *)op, T_float);
}

int
nc_put_vars_double(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep,
		   const double *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, (void *)op, T_double);
}

int
nc_put_vars_ubyte(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep,
		  const unsigned char *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, (void *)op, T_ubyte);
}

int
nc_put_vars_ushort(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep,
		   const unsigned short *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, (void *)op, T_ushort);
}

int
nc_put_vars_uint(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t *stridep,
		 const unsigned int *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, (void *)op, T_uint);
}

int
nc_put_vars_longlong(int ncid, int varid,
		     const size_t *startp, const size_t *countp,
		     const ptrdiff_t *stridep,
		     const long long *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, (void *)op, T_longlong);
}

int
nc_put_vars_ulonglong(int ncid, int varid,
		      const size_t *startp, const size_t *countp,
		      const ptrdiff_t *stridep,
		      const unsigned long long *op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, (void *)op, NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_vars_string(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep,
		   const char**op)
{
   return NC_put_vars(ncid, varid, startp, countp, stridep, (void *)op, NC_STRING);
}
#endif /*USE_NETCDF4*/
/**\} */

/** \ingroup variables
Write a mapped array of values to a variable.

The nc_put_varm() function will only write a variable of an
atomic type; it will not write user defined types. For this
function, the type of the data in memory must match the type
of the variable - no data conversion is done.

@deprecated Use of this family of functions is discouraged,
although it will continue to be supported.
The reason is the complexity of the
algorithm makes its use difficult for users to properly use.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param startp Start vector with one element for each dimension to \ref
specify_hyperslab.

\param countp Count vector with one element for each dimension to \ref
specify_hyperslab.

\param stridep Stride vector with one element for each dimension to
\ref specify_hyperslab.

\param imapp Mapping vector with one element for each dimension to
\ref specify_hyperslab.

\param op Pointer where the data will be copied. Memory must be
allocated by the user before this function is called.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTVAR Variable not found.
\returns ::NC_EINVALCOORDS Index exceeds dimension bound.
\returns ::NC_EEDGE Start+count exceeds dimension bound.
\returns ::NC_ERANGE One or more of the values are out of range.
\returns ::NC_EINDEFINE Operation not allowed in define mode.
\returns ::NC_EBADID Bad ncid.
 */
/**@{*/
int
nc_put_varm (int ncid, int varid, const size_t *startp,
	     const size_t *countp, const ptrdiff_t *stridep,
	     const ptrdiff_t *imapp, const void *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, op, NC_NAT);
}

int
nc_put_varm_text(int ncid, int varid, const size_t *startp,
		 const size_t *countp, const ptrdiff_t *stridep,
		 const ptrdiff_t *imapp, const char *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, NC_CHAR);
}

int
nc_put_varm_schar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  const signed char *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, NC_BYTE);
}

int
nc_put_varm_uchar(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  const unsigned char *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, T_uchar);
}

int
nc_put_varm_short(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  const short *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, NC_SHORT);
}

int
nc_put_varm_int(int ncid, int varid,
		const size_t *startp, const size_t *countp,
		const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		const int *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, NC_INT);
}

int
nc_put_varm_long(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		 const long *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, T_long);
}

int
nc_put_varm_float(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  const float *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, T_float);
}

int
nc_put_varm_double(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		   const double *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, T_double);
}

int
nc_put_varm_ubyte(int ncid, int varid,
		  const size_t *startp, const size_t *countp,
		  const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		  const unsigned char *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, T_ubyte);
}

int
nc_put_varm_ushort(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		   const unsigned short *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, T_ushort);
}

int
nc_put_varm_uint(int ncid, int varid,
		 const size_t *startp, const size_t *countp,
		 const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		 const unsigned int *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, T_uint);
}

int
nc_put_varm_longlong(int ncid, int varid,
		     const size_t *startp, const size_t *countp,
		     const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		     const long long *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, T_longlong);
}

int
nc_put_varm_ulonglong(int ncid, int varid,
		      const size_t *startp, const size_t *countp,
		      const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		      const unsigned long long *op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, NC_UINT64);
}

#ifdef USE_NETCDF4
int
nc_put_varm_string(int ncid, int varid,
		   const size_t *startp, const size_t *countp,
		   const ptrdiff_t *stridep, const ptrdiff_t *imapp,
		   const char**op)
{
   return NC_put_varm(ncid, varid, startp, countp, stridep, imapp, (void *)op, NC_STRING);
}
#endif /*USE_NETCDF4*/
/**\} */

/** \ingroup variables
Free string space allocated by the library.

When you read string type the library will allocate the storage space
for the data. This storage space must be freed, so pass the pointer
back to this function, when you're done with the data, and it will
free the string memory.

\param len The number of character arrays in the array.
\param data The pointer to the data array.

\returns ::NC_NOERR No error.
*/
int
nc_free_string(size_t len, char **data)
{
   int i;
   for (i = 0; i < len; i++)
      free(data[i]);
   return NC_NOERR;
}

/** \internal
\ingroup variables
*/
static int
NC_put_var(int ncid, int varid, const void *value, nc_type memtype)
{
   int ndims;
   size_t shape[NC_MAX_VAR_DIMS];
   int stat = nc_inq_varndims(ncid,varid, &ndims);
   if(stat) return stat;
   stat = NC_getshape(ncid,varid, ndims, shape);
   if(stat) return stat;
   return NC_put_vara(ncid, varid, NC_coord_zero, shape, value, memtype);
}

/** \internal
\ingroup variables
*/
static int
NC_put_var1(int ncid, int varid, const size_t *coord, const void* value,
	    nc_type memtype)
{
   return NC_put_vara(ncid, varid, coord, NC_coord_one, value, memtype);
}


/** \internal
\ingroup variables
*/
int
NCDEFAULT_put_vars(int ncid, int varid, const size_t * start,
	    const size_t * edges, const ptrdiff_t * stride,
	    const void *value0, nc_type memtype)
{
  /* Rebuilt put_vars code to simplify and avoid use of put_varm */

   int status = NC_NOERR;
   int i,isstride1,isrecvar;
   int rank;
   struct Odometer odom;
   nc_type vartype = NC_NAT;
   NC* ncp;
   size_t vartypelen;
   int memtypelen;
   const char* value = (const char*)value0;
   int nrecdims;                /* number of record dims for a variable */
   int is_recdim[NC_MAX_VAR_DIMS]; /* for variable's dimensions */
   size_t varshape[NC_MAX_VAR_DIMS];
   size_t mystart[NC_MAX_VAR_DIMS];
   size_t myedges[NC_MAX_VAR_DIMS];
   ptrdiff_t mystride[NC_MAX_VAR_DIMS];
   const char* memptr = value;

   status = NC_check_id (ncid, &ncp);
   if(status != NC_NOERR) return status;

   status = nc_inq_vartype(ncid, varid, &vartype);
   if(status != NC_NOERR) return status;

   if(memtype == NC_NAT) memtype = vartype;

   /* compute the variable type size */
   status = nc_inq_type(ncid,vartype,NULL,&vartypelen);
   if(status != NC_NOERR) return status;

   if(memtype > NC_MAX_ATOMIC_TYPE)
	memtypelen = (int)vartypelen;
    else
	memtypelen = nctypelen(memtype);

   /* Check gross internal/external type compatibility */
   if(vartype != memtype) {
      /* If !atomic, the two types must be the same */
      if(vartype > NC_MAX_ATOMIC_TYPE
         || memtype > NC_MAX_ATOMIC_TYPE)
	 return NC_EBADTYPE;
      /* ok, the types differ but both are atomic */
      if(memtype == NC_CHAR || vartype == NC_CHAR)
	 return NC_ECHAR;
   }

   /* Get the variable rank */
   status = nc_inq_varndims(ncid, varid, &rank);
   if(status != NC_NOERR) return status;

   /* Get variable dimension sizes */
   status = NC_inq_recvar(ncid,varid,&nrecdims,is_recdim);
   if(status != NC_NOERR) return status;
   isrecvar = (nrecdims > 0);
   NC_getshape(ncid,varid,rank,varshape);

   /* Optimize out using various checks */
   if (rank == 0) {
      /*
       * The variable is a scalar; consequently,
       * there is only one thing to get and only one place to put it.
       * (Why was I called?)
       */
      size_t edge1[1] = {1};
      return NC_put_vara(ncid, varid, start, edge1, value0, memtype);
   }

   /* Do various checks and fixups on start/edges/stride */
   isstride1 = 1; /* assume so */
   for(i=0;i<rank;i++) {
	size_t dimlen;
	mystart[i] = (start == NULL ? 0 : start[i]);
	if(edges == NULL) {
#if 0
	   if(i == 0 && isrecvar)
  	      myedges[i] = numrecs - start[i];
#else
	   if(is_recdim[i] && isrecvar)
  	      myedges[i] = varshape[i] - start[i];
#endif
	   else
	      myedges[i] = varshape[i] - mystart[i];
	} else
	    myedges[i] = edges[i];
	if(myedges[i] == 0)
	    return NC_NOERR; /* cannot write anything */
	mystride[i] = (stride == NULL ? 1 : stride[i]);
	if(mystride[i] <= 0
	   /* cast needed for braindead systems with signed size_t */
           || ((unsigned long) mystride[i] >= X_INT_MAX))
           return NC_ESTRIDE;
  	if(mystride[i] != 1) isstride1 = 0;
        /* illegal value checks */
#if 0
	dimlen = (i == 0 && isrecvar ? numrecs : varshape[i]);
	if(i == 0 && isrecvar) {/*do nothing*/}
#else
	dimlen = varshape[i];
	if(is_recdim[i]) {/*do nothing*/}
#endif
        else {
	  /* mystart is unsigned, will never be < 0 */
	  if(mystart[i] > dimlen)
	    return NC_EINVALCOORDS;
          /* myediges is unsigned, will never be < 0 */
	  if(mystart[i] + myedges[i] > dimlen)
	    return NC_EEDGE;
       }
   }
   if(isstride1) {
      return NC_put_vara(ncid, varid, mystart, myedges, value, memtype);
   }

   /* Initial version uses and odometer to walk the variable
      and read each value one at a time. This can later be optimized
      to read larger chunks at a time.
    */


   odom_init(&odom,rank,mystart,myedges,mystride);

   /* walk the odometer to extract values */
   while(odom_more(&odom)) {
      int localstatus = NC_NOERR;
      /* Write a single value */
      localstatus = NC_put_vara(ncid,varid,odom.index,nc_sizevector1,memptr,memtype);
      /* So it turns out that when get_varm is used, all errors are
         delayed and ERANGE will be overwritten by more serious errors.
      */
      if(localstatus != NC_NOERR) {
	    if(status == NC_NOERR || localstatus != NC_ERANGE)
	       status = localstatus;
      }
      memptr += memtypelen;
      odom_next(&odom);
   }
   return status;
}

/** \internal
\ingroup variables
*/
int
NCDEFAULT_put_varm(
   int ncid,
   int varid,
   const size_t * start,
   const size_t * edges,
   const ptrdiff_t * stride,
   const ptrdiff_t * imapp,
   const void *value0,
   nc_type memtype)
{
   int status = NC_NOERR;
   nc_type vartype = NC_NAT;
   int varndims = 0;
   int maxidim = 0;
   NC* ncp;
   int memtypelen;
   const char* value = (char*)value0;

   status = NC_check_id (ncid, &ncp);
   if(status != NC_NOERR) return status;

/*
  if(NC_indef(ncp)) return NC_EINDEFINE;
  if(NC_readonly (ncp)) return NC_EPERM;
*/

   /* mid body */
   status = nc_inq_vartype(ncid, varid, &vartype);
   if(status != NC_NOERR) return status;
   /* Check that this is an atomic type */
   if(vartype > NC_MAX_ATOMIC_TYPE)
	return NC_EMAPTYPE;

   status = nc_inq_varndims(ncid, varid, &varndims);
   if(status != NC_NOERR) return status;

   if(memtype == NC_NAT) {
      memtype = vartype;
   }

   if(memtype == NC_CHAR && vartype != NC_CHAR)
      return NC_ECHAR;
   else if(memtype != NC_CHAR && vartype == NC_CHAR)
      return NC_ECHAR;

   memtypelen = nctypelen(memtype);

   maxidim = (int) varndims - 1;

   if (maxidim < 0)
   {
      /*
       * The variable is a scalar; consequently,
       * there s only one thing to get and only one place to put it.
       * (Why was I called?)
       */
      size_t edge1[1] = {1};
      return NC_put_vara(ncid, varid, start, edge1, value, memtype);
   }

   /*
    * else
    * The variable is an array.
    */
   {
      int idim;
      size_t *mystart = NULL;
      size_t *myedges = 0;
      size_t *iocount= 0;    /* count vector */
      size_t *stop = 0;   /* stop indexes */
      size_t *length = 0; /* edge lengths in bytes */
      ptrdiff_t *mystride = 0;
      ptrdiff_t *mymap= 0;
      size_t varshape[NC_MAX_VAR_DIMS];
      int isrecvar;
      size_t numrecs;
      int stride1; /* is stride all ones? */

      /*
       * Verify stride argument.
       */
      stride1 = 1;		/*  assume ok; */
      if(stride != NULL) {
	 for (idim = 0; idim <= maxidim; ++idim) {
            if ((stride[idim] == 0)
		/* cast needed for braindead systems with signed size_t */
                || ((unsigned long) stride[idim] >= X_INT_MAX))
	    {
	       return NC_ESTRIDE;
            }
	    if(stride[idim] != 1) stride1 = 0;
	 }
      }

      /* If stride1 is true, and there is no imap, then call get_vara
         directly
      */
      if(stride1 && imapp == NULL) {
	 return NC_put_vara(ncid, varid, start, edges, value, memtype);
      }

      /* Compute some dimension related values */
      isrecvar = NC_is_recvar(ncid,varid,&numrecs);
      NC_getshape(ncid,varid,varndims,varshape);

      /* assert(sizeof(ptrdiff_t) >= sizeof(size_t)); */
      mystart = (size_t *)calloc((size_t)(varndims * 7), sizeof(ptrdiff_t));
      if(mystart == NULL) return NC_ENOMEM;
      myedges = mystart + varndims;
      iocount = myedges + varndims;
      stop = iocount + varndims;
      length = stop + varndims;
      mystride = (ptrdiff_t *)(length + varndims);
      mymap = mystride + varndims;

      /*
       * Initialize I/O parameters.
       */
      for (idim = maxidim; idim >= 0; --idim)
      {
	 mystart[idim] = start != NULL
	    ? start[idim]
	    : 0;

	 if (edges != NULL && edges[idim] == 0)
	 {
	    status = NC_NOERR;    /* read/write no data */
	    goto done;
	 }

	 myedges[idim] = edges != NULL
	    ? edges[idim]
	    : idim == 0 && isrecvar
    	        ? numrecs - mystart[idim]
	        : varshape[idim] - mystart[idim];
	 mystride[idim] = stride != NULL
	    ? stride[idim]
	    : 1;
	 mymap[idim] = imapp != NULL
	    ? imapp[idim]
	    : idim == maxidim
	        ? 1
	        : mymap[idim + 1] * (ptrdiff_t) myedges[idim + 1];

	 iocount[idim] = 1;
	 length[idim] = ((size_t)mymap[idim]) * myedges[idim];
	 stop[idim] = mystart[idim] + myedges[idim] * (size_t)mystride[idim];
      }

      /*
       * Check start, edges
       */
      for (idim = isrecvar; idim < maxidim; ++idim)
      {
	 if (mystart[idim] > varshape[idim])
	 {
	    status = NC_EINVALCOORDS;
	    goto done;
	 }
	 if (mystart[idim] + myedges[idim] > varshape[idim])
	 {
	    status = NC_EEDGE;
	    goto done;
	 }
      }

      /* Lower body */
      /*
       * As an optimization, adjust I/O parameters when the fastest
       * dimension has unity stride both externally and internally.
       * In this case, the user could have called a simpler routine
       * (i.e. ncvar$1()
       */
      if (mystride[maxidim] == 1
	  && mymap[maxidim] == 1)
      {
	 iocount[maxidim] = myedges[maxidim];
	 mystride[maxidim] = (ptrdiff_t) myedges[maxidim];
	 mymap[maxidim] = (ptrdiff_t) length[maxidim];
      }

      /*
       * Perform I/O.  Exit when done.
       */
      for (;;)
      {
	 /* TODO: */
	 int lstatus = NC_put_vara(ncid, varid, mystart, iocount,
				   value, memtype);
	 if (lstatus != NC_NOERR) {
	    if(status == NC_NOERR || lstatus != NC_ERANGE)
	       status = lstatus;
	 }

	 /*
	  * The following code permutes through the variable s
	  * external start-index space and it s internal address
	  * space.  At the UPC, this algorithm is commonly
	  * called "odometer code".
	  */
	 idim = maxidim;
        carry:
	 value += (mymap[idim] * memtypelen);
	 mystart[idim] += (size_t)mystride[idim];
	 if (mystart[idim] == stop[idim])
	 {
	    size_t l = (length[idim] * (size_t)memtypelen);
	    value -= l;
	    mystart[idim] = start[idim];
	    if (--idim < 0)
	       break; /* normal return */
	    goto carry;
	 }
      } /* I/O loop */
     done:
      free(mystart);
   } /* variable is array */
   return status;
}

/**************************************************/
/* Originally in dgroup.c*/

/*! \ingroup groups Get the length of a group name given an ID.

  @param[in] ncid  The ncid of the group in question.
  @param[out] lenp Pointer to memory to hold the length of the name of the group in question.

  @returns Error code or ::NC_NOERR for no error.

*/
int
nc_inq_grpname_len(int ncid, size_t *lenp)
{
    int stat = nc_inq_grpname_full(ncid,lenp,NULL);
    return stat;
}

/*! \ingroup groups Get a group ncid given the group name.

  @param[in] ncid      The ncid of the file.
  @param[in] grp_name  The name of the group in question.
  @param[out] grp_ncid Pointer to memory to hold the identifier of the group in question.

  @returns Error code or ::NC_NOERR for no error.

\note{This has same semantics as nc_inq_ncid}

*/
int nc_inq_grp_ncid(int ncid, const char *grp_name, int *grp_ncid)
{
    return nc_inq_ncid(ncid,grp_name,grp_ncid);
}

/**************************************************/
/* Originally in denum.c */

/** \ingroup user_types
Learn about a user-define enumeration type. 

\param ncid \ref ncid

\param xtype Typeid to inquire about.

\param name \ref object_name of type will be copied here. \ref
ignored_if_null.

\param base_nc_typep Typeid if the base type of the enum.\ref
ignored_if_null.

\param base_sizep Pointer that will get the size in bytes of the base
type. \ref ignored_if_null.

\param num_membersp Pointer that will get the number of members
defined for this enum type. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_enum(int ncid, nc_type xtype, char *name, nc_type *base_nc_typep, 
	    size_t *base_sizep, size_t *num_membersp)
{
    int class = 0;
    int stat = nc_inq_user_type(ncid, xtype, name, base_sizep, 
				base_nc_typep, num_membersp, &class);
    if(stat != NC_NOERR) return stat;
    if(class != NC_ENUM) stat = NC_EBADTYPE;
    return stat;
}

/**************************************************/
/* Originally in dvlen.c */

/** 
\ingroup user_types 
Free memory in a VLEN object. 

When you read VLEN type the library will actually allocate the storage
space for the data. This storage space must be freed, so pass the
pointer back to this function, when you're done with the data, and it
will free the vlen memory.

The function nc_free_vlens() is more useful than this function,
because it can free an array of VLEN objects.

\param vl pointer to the vlen object.

\returns ::NC_NOERR No error.
*/
int
nc_free_vlen(nc_vlen_t *vl)
{
   free(vl->p);
   return NC_NOERR;
}

/** 
\ingroup user_types 
Free an array of vlens given the number of elements and an array. 

When you read VLEN type the library will actually allocate the storage
space for the data. This storage space must be freed, so pass the
pointer back to this function, when you're done with the data, and it
will free the vlen memory.

\param len number of elements in the array.
\param vlens pointer to the vlen object.

\returns ::NC_NOERR No error.
*/ 
int
nc_free_vlens(size_t len, nc_vlen_t vlens[])
{
   int ret;
   size_t i;

   for(i = 0; i < len; i++) 
      if ((ret = nc_free_vlen(&vlens[i])))
	 return ret;

   return NC_NOERR;
}

/** \ingroup user_types
Learn about a VLEN type.

\param ncid \ref ncid
\param xtype The type of the VLEN to inquire about. 
\param name \ref object_name of the type. \ref ignored_if_null.

\param datum_sizep A pointer to a size_t, this will get the size of
one element of this vlen. \ref ignored_if_null.

\param base_nc_typep Pointer to get the base type of the VLEN. \ref
ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_vlen(int ncid, nc_type xtype, char *name, size_t *datum_sizep, nc_type *base_nc_typep)
{
    int class = 0;
    int stat = nc_inq_user_type(ncid,xtype,name,datum_sizep,base_nc_typep,NULL,&class);
    if(stat != NC_NOERR) return stat;
    if(class != NC_VLEN) stat = NC_EBADTYPE;
    return stat;
}
/*! \} */  /* End of named group ...*/

/**************************************************/
/* Originally in dopaque.c */

/** \ingroup user_types
Learn about an opaque type.

\param ncid \ref ncid

\param xtype Typeid to inquire about.

\param name The \ref object_name of this type will be
copied here. \ref ignored_if_null.

\param sizep The size of the type will be copied here. \ref
ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_opaque(int ncid, nc_type xtype, char *name, size_t *sizep)
{
    int class = 0;
    int stat = nc_inq_user_type(ncid,xtype,name,sizep,NULL,NULL,&class);
    if(stat != NC_NOERR) return stat;
    if(class != NC_OPAQUE) stat = NC_EBADTYPE;
    return stat;
}

/**************************************************/
/* Originally from dcompound.c */

/**  \ingroup user_types
Learn about a compound type. Get the number of fields, len, and
name of a compound type.

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param name Returned \ref object_name of compound type. \ref
ignored_if_null.

\param sizep Returned size of compound type in bytes. \ref ignored_if_null.

\param nfieldsp The number of fields in the compound type will be
placed here. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_compound(int ncid, nc_type xtype, char *name, 
		size_t *sizep, size_t *nfieldsp)
{
   int class = 0;
   int stat = nc_inq_user_type(ncid,xtype,name,sizep,NULL,nfieldsp,&class);
   if(stat != NC_NOERR) return stat;
   if(class != NC_COMPOUND) stat = NC_EBADTYPE;
   return stat;
}

/**  \ingroup user_types
Learn the name of a compound type.

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param name Returned \ref object_name of compound type. \ref
ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_compound_name(int ncid, nc_type xtype, char *name)
{
   return nc_inq_compound(ncid,xtype,name,NULL,NULL);
}

/**  \ingroup user_types
Learn the size of a compound type. 

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param sizep Returned size of compound type in bytes. \ref
ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_compound_size(int ncid, nc_type xtype, size_t *sizep)
{
   return nc_inq_compound(ncid,xtype,NULL,sizep,NULL);
}

/**  \ingroup user_types
Learn the number of fields in a compound type. 

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param nfieldsp The number of fields in the compound type will be
placed here. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_compound_nfields(int ncid, nc_type xtype, size_t *nfieldsp)
{
   return nc_inq_compound(ncid,xtype,NULL,NULL,nfieldsp);
}

/**  \ingroup user_types
Get information about one of the fields of a compound type. 

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param fieldid A zero-based index number specifying a field in the
compound type.

\param offsetp A pointer which will get the offset of the field. \ref
ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_compound_fieldoffset(int ncid, nc_type xtype, int fieldid, 
			    size_t *offsetp)
{
   return nc_inq_compound_field(ncid,xtype,fieldid,NULL,offsetp,NULL,NULL,NULL);
}

/**  \ingroup user_types
Get information about one of the fields of a compound type. 

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param fieldid A zero-based index number specifying a field in the
compound type.

\param field_typeidp A pointer which will get the typeid of the
field. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_compound_fieldtype(int ncid, nc_type xtype, int fieldid, 
			  nc_type *field_typeidp)
{
   return nc_inq_compound_field(ncid,xtype,fieldid,NULL,NULL,field_typeidp,NULL,NULL);
}

/**  \ingroup user_types
Get information about one of the fields of a compound type. 

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param fieldid A zero-based index number specifying a field in the
compound type.

\param ndimsp A pointer which will get the number of dimensions of the
field. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_compound_fieldndims(int ncid, nc_type xtype, int fieldid, 
			   int *ndimsp)
{
   return nc_inq_compound_field(ncid,xtype,fieldid,NULL,NULL,NULL,ndimsp,NULL);
}

/**  \ingroup user_types
Get information about one of the fields of a compound type. 

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param fieldid A zero-based index number specifying a field in the
compound type.

\param dim_sizesp A pointer which will get the dimension sizes of the
field. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_compound_fielddim_sizes(int ncid, nc_type xtype, int fieldid, 
			       int *dim_sizesp)
{
   return nc_inq_compound_field(ncid, xtype, fieldid,
					    NULL, NULL, NULL, NULL,
					    dim_sizesp);
}

/**************************************************/
/* Originally from dparallel.c */

/**
 This function creates a file for use with parallel I/O.
*/
int
nc_create_par(const char *path, int cmode, MPI_Comm comm,
	      MPI_Info info, int *ncidp)
{
#ifndef USE_PARALLEL
   return NC_ENOPAR;
#else
   NC_MPI_INFO data;

   /* One of these two parallel IO modes must be chosen by the user,
    * or else pnetcdf must be in use. */
   if (!(cmode & NC_MPIIO || cmode & NC_MPIPOSIX) &&
       !(cmode & NC_PNETCDF))
      return NC_EINVAL;

   data.comm = comm;
   data.info = info;
   return NC_create(path, cmode, 0, 0, NULL, 1, &data, ncidp);
#endif /* USE_PARALLEL */
}

/*! \ingroup datasets
  This function opens a file for parallel I/O.
*/
int
nc_open_par(const char *path, int mode, MPI_Comm comm,
	    MPI_Info info, int *ncidp)
{
#ifndef USE_PARALLEL
   return NC_ENOPAR;
#else
   NC_MPI_INFO mpi_data;

   /* One of these two parallel IO modes must be chosen by the user,
    * or else pnetcdf must be in use. */
   if ((mode & NC_MPIIO) || (mode & NC_MPIPOSIX)) {
	/* ok */
   } else if(mode & NC_PNETCDF) {
	/* ok */
   } else
      return NC_EINVAL;

   mpi_data.comm = comm;
   mpi_data.info = info;

   return NC_open(path, mode, 0, NULL, 1, &mpi_data, ncidp);
#endif /* USE_PARALLEL */
}

/*! \ingroup datasets
 Fortran needs to pass MPI comm/info as integers.
*/
int
nc_open_par_fortran(const char *path, int mode, int comm,
		    int info, int *ncidp)
{
#ifndef USE_PARALLEL
   return NC_ENOPAR;
#else
   MPI_Comm comm_c;
   MPI_Info info_c;

   /* Convert fortran comm and info to C comm and info, if there is a
    * function to do so. Otherwise just pass them. */
#ifdef HAVE_MPI_COMM_F2C
   comm_c = MPI_Comm_f2c(comm);
   info_c = MPI_Info_f2c(info);
#else
   comm_c = (MPI_Comm)comm;
   info_c = (MPI_Info)info;
#endif
   return nc_open_par(path, mode, comm_c, info_c, ncidp);
#endif
}

/* when calling from fortran: convert MPI_Comm and MPI_Info to C */
int
nc_create_par_fortran(const char *path, int cmode, int comm,
		      int info, int *ncidp)
{
#ifndef USE_PARALLEL
   return NC_ENOPAR;
#else
   MPI_Comm comm_c;
   MPI_Info info_c;

   /* Convert fortran comm and info to C comm and info, if there is a
    * function to do so. Otherwise just pass them. */
#ifdef HAVE_MPI_COMM_F2C
   comm_c = MPI_Comm_f2c(comm);
   info_c = MPI_Info_f2c(info);
#else
   comm_c = (MPI_Comm)comm;
   info_c = (MPI_Info)info;
#endif

   return nc_create_par(path, cmode, comm_c, info_c, ncidp);
#endif
}

/**************************************************/
/* Odometer Code */

static void
odom_init(struct Odometer* odom,
	    int rank,
	    const size_t* start, const size_t* edges, const ptrdiff_t* stride)
{
    int i;
    memset(odom,0,sizeof(struct Odometer));
    odom->rank = rank;
    assert(odom->rank <= NC_MAX_VAR_DIMS);
    for(i=0;i<odom->rank;i++) {
	odom->start[i] = (start != NULL ? start[i] : 0);
	odom->edges[i] = (edges != NULL ? edges[i] : 1);
	odom->stride[i] = (stride != NULL ? stride[i] : 1);
	odom->stop[i] = odom->start[i] + (odom->edges[i]*(size_t)odom->stride[i]);
	odom->index[i] = odom->start[i];
    }
}

static int
odom_more(struct Odometer* odom)
{
    return (odom->index[0] < odom->stop[0]);
}

static int
odom_next(struct Odometer* odom)
{
    int i;
    if(odom->rank == 0) return 0;
    for(i=odom->rank-1;i>=0;i--) {
        odom->index[i] += (size_t)odom->stride[i];
        if(odom->index[i] < odom->stop[i]) break;
	if(i == 0) return 0; /* leave the 0th entry if it overflows*/
	odom->index[i] = odom->start[i]; /* reset this position*/
    }
    return 1;
}

/**
Certain functions are in the dispatch table,
but not in the netcdf.h API. These need to
be exposed for use in delegation such as
in libdap2.
*/

int
NCDISPATCH_inq_var_all(int ncid, int varid, char *name, nc_type *xtypep,
               int *ndimsp, int *dimidsp, int *nattsp,
               int *shufflep, int *deflatep, int *deflate_levelp,
               int *fletcher32p, int *contiguousp, size_t *chunksizesp,
               int *no_fill, void *fill_valuep, int *endiannessp,
	       int *options_maskp, int *pixels_per_blockp)
{
   return NC_inq_var_all(
      ncid, varid, name, xtypep,
      ndimsp, dimidsp, nattsp,
      shufflep, deflatep, deflate_levelp, fletcher32p,
      contiguousp, chunksizesp,
      no_fill, fill_valuep,
      endiannessp,
      options_maskp, 
      pixels_per_blockp);
}

int
NCDISPATCH_get_att(int ncid, int varid, const char* name, void* value, nc_type t)
{
   return NC_get_att(ncid,varid,name,value,t);

}
