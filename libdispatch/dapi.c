/** \file dapi.c

This file contains the "first-level" api code.
First level code is code that directly calls the
dispatch layer.  This file also contains a number
of static functions. It may also reference exported
functions (see dexport.c).

As a rule, if a function calls a dispatch table
method or calls NC_check_id, then it should be in
this file.

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
/* Forward*/
static int NC_check_file_type(const char *path, int flags, void *parameters,int* model, int* version);
static int NC_interpret_magic_number(char* magic, int* model, int* version, int use_parallel);

/**************************************************/

extern int NC_initialized;
extern int NC_finalized;

/**************************************************/
/* Originally in dfile.c */

/** \defgroup datasets NetCDF File and Data I/O

NetCDF opens datasets as files or remote access URLs.

A netCDF dataset that has not yet been opened can only be referred to
by its dataset name. Once a netCDF dataset is opened, it is referred
to by a netCDF ID, which is a small non-negative integer returned when
you create or open the dataset. A netCDF ID is much like a file
descriptor in C or a logical unit number in FORTRAN. In any single
program, the netCDF IDs of distinct open netCDF datasets are
distinct. A single netCDF dataset may be opened multiple times and
will then have multiple distinct netCDF IDs; however at most one of
the open instances of a single netCDF dataset should permit
writing. When an open netCDF dataset is closed, the ID is no longer
associated with a netCDF dataset.

Functions that deal with the netCDF library include:
- Get version of library.
- Get error message corresponding to a returned error code.

The operations supported on a netCDF dataset as a single object are:
- Create, given dataset name and whether to overwrite or not.
- Open for access, given dataset name and read or write intent.
- Put into define mode, to add dimensions, variables, or attributes.
- Take out of define mode, checking consistency of additions.
- Close, writing to disk if required.
- Inquire about the number of dimensions, number of variables,
number of global attributes, and ID of the unlimited dimension, if
any.
- Synchronize to disk to make sure it is current.
- Set and unset nofill mode for optimized sequential writes.
- After a summary of conventions used in describing the netCDF
interfaces, the rest of this chapter presents a detailed description
of the interfaces for these operations.
*/

/** \ingroup datasets
Get the file pathname (or the opendap URL) which was used to
open/create the ncid's file.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param pathlen Pointer where length of path will be returned. Ignored
if NULL.

\param path Pointer where path name will be copied. Space must already
be allocated. Ignored if NULL.

\returns ::NC_NOERR No error.

\returns ::NC_EBADID Invalid ncid passed.
*/
int
nc_inq_path(int ncid, size_t *pathlen, char *path)
{
   NC* ncp;
   int stat = NC_NOERR;
   if ((stat = NC_check_id(ncid, &ncp)))
      return stat;
   if(ncp->path == NULL) {
	if(pathlen) *pathlen = 0;
	if(path) path[0] = '\0';
   } else {
       if (pathlen) *pathlen = strlen(ncp->path);
       if (path) strcpy(path, ncp->path);
   }
   return stat;
}
/*! \} */  /* End of \defgroup ...*/

/** \ingroup datasets
Put open netcdf dataset into define mode

The function nc_redef puts an open netCDF dataset into define mode, so
dimensions, variables, and attributes can be added or renamed and
attributes can be deleted.

For netCDF-4 files (i.e. files created with NC_NETCDF4 in the cmode in
their call to nc_create()), it is not necessary to call nc_redef()
unless the file was also created with NC_STRICT_NC3. For straight-up
netCDF-4 files, nc_redef() is called automatically, as needed.

For all netCDF-4 files, the root ncid must be used. This is the ncid
returned by nc_open() and nc_create(), and points to the root of the
hierarchy tree for netCDF-4 files.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\returns ::NC_NOERR No error.

\returns ::NC_EBADID Bad ncid.

\returns ::NC_EBADGRPID The ncid must refer to the root group of the
file, that is, the group returned by nc_open() or nc_create().

\returns ::NC_EINDEFINE Already in define mode.

\returns ::NC_EPERM File is read-only.

<h1>Example</h1>

Here is an example using nc_redef to open an existing netCDF dataset
named foo.nc and put it into define mode:

\code
#include <netcdf.h>
   ...
int status = NC_NOERR;
int ncid;
   ...
status = nc_open("foo.nc", NC_WRITE, &ncid);
if (status != NC_NOERR) handle_error(status);
   ...
status = nc_redef(ncid);
if (status != NC_NOERR) handle_error(status);
\endcode
 */
int
nc_redef(int ncid)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->redef(ncid);
}

/** \ingroup datasets
Leave define mode

The function nc_enddef() takes an open netCDF dataset out of define
mode. The changes made to the netCDF dataset while it was in define
mode are checked and committed to disk if no problems
occurred. Non-record variables may be initialized to a "fill value" as
well with nc_set_fill(). The netCDF dataset is then placed in data
mode, so variable data can be read or written.

It's not necessary to call nc_enddef() for netCDF-4 files. With netCDF-4
files, nc_enddef() is called when needed by the netcdf-4 library. User
calls to nc_enddef() for netCDF-4 files still flush the metadata to
disk.

This call may involve copying data under some circumstances. For a
more extensive discussion see File Structure and Performance.

For netCDF-4/HDF5 format files there are some variable settings (the
compression, endianness, fletcher32 error correction, and fill value)
which must be set (if they are going to be set at all) between the
nc_def_var() and the next nc_enddef(). Once the nc_enddef() is called,
these settings can no longer be changed for a variable.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

If you use a group id (in a netCDF-4/HDF5 file), the enddef
will apply to the entire file. That means the enddef will not just end
define mode in one group, but in the entire file.

\returns ::NC_NOERR no error

\returns ::NC_EBADID Invalid ncid passed.

<h1>Example</h1>

Here is an example using nc_enddef() to finish the definitions of a new
netCDF dataset named foo.nc and put it into data mode:

\code
     #include <netcdf.h>
        ...
     int status = NC_NOERR;
     int ncid;
        ...
     status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
     if (status != NC_NOERR) handle_error(status);

        ...  create dimensions, variables, attributes

     status = nc_enddef(ncid);
     if (status != NC_NOERR) handle_error(status);
\endcode
 */
int
nc_enddef(int ncid)
{
   int status = NC_NOERR;
   NC *ncp;
   status = NC_check_id(ncid, &ncp);
   if(status != NC_NOERR) return status;
   return ncp->dispatch->_enddef(ncid,0,1,0,1);
}

/** \ingroup datasets
Leave define mode with performance tuning

The function nc__enddef takes an open netCDF dataset out of define
mode. The changes made to the netCDF dataset while it was in define
mode are checked and committed to disk if no problems
occurred. Non-record variables may be initialized to a "fill value" as
well with nc_set_fill(). The netCDF dataset is then placed in data mode,
so variable data can be read or written.

This call may involve copying data under some circumstances. For a
more extensive discussion see File Structure and Performance.

\warning This function exposes internals of the netcdf version 1 file
format. Users should use nc_enddef() in most circumstances. This
function may not be available on future netcdf implementations.

The classic netcdf file format has three sections, the "header"
section, the data section for fixed size variables, and the data
section for variables which have an unlimited dimension (record
variables).

The header begins at the beginning of the file. The index (offset) of
the beginning of the other two sections is contained in the
header. Typically, there is no space between the sections. This causes
copying overhead to accrue if one wishes to change the size of the
sections, as may happen when changing names of things, text attribute
values, adding attributes or adding variables. Also, for buffered i/o,
there may be advantages to aligning sections in certain ways.

The minfree parameters allow one to control costs of future calls to
nc_redef, nc_enddef() by requesting that minfree bytes be available at
the end of the section.

The align parameters allow one to set the alignment of the beginning
of the corresponding sections. The beginning of the section is rounded
up to an index which is a multiple of the align parameter. The flag
value ALIGN_CHUNK tells the library to use the bufrsize (see above) as
the align parameter. It has nothing to do with the chunking
(multidimensional tiling) features of netCDF-4.

The file format requires mod 4 alignment, so the align parameters are
silently rounded up to multiples of 4. The usual call,

\code
     nc_enddef(ncid);
\endcode

is equivalent to

\code
     nc__enddef(ncid, 0, 4, 0, 4);
\endcode

The file format does not contain a "record size" value, this is
calculated from the sizes of the record variables. This unfortunate
fact prevents us from providing minfree and alignment control of the
"records" in a netcdf file. If you add a variable which has an
unlimited dimension, the third section will always be copied with the
new variable added.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param h_minfree Sets the pad at the end of the "header" section.

\param v_align Controls the alignment of the beginning of the data
section for fixed size variables.

\param v_minfree Sets the pad at the end of the data section for fixed
size variables.

\param r_align Controls the alignment of the beginning of the data
section for variables which have an unlimited dimension (record
variables).

\returns ::NC_NOERR No error.

\returns ::NC_EBADID Invalid ncid passed.

 */
int
nc__enddef(int ncid, size_t h_minfree, size_t v_align, size_t v_minfree,
	   size_t r_align)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->_enddef(ncid,h_minfree,v_align,v_minfree,r_align);
}

/** \ingroup datasets
Synchronize an open netcdf dataset to disk

The function nc_sync() offers a way to synchronize the disk copy of a
netCDF dataset with in-memory buffers. There are two reasons you might
want to synchronize after writes:
- To minimize data loss in case of abnormal termination, or
- To make data available to other processes for reading immediately
  after it is written. But note that a process that already had the
  dataset open for reading would not see the number of records
  increase when the writing process calls nc_sync(); to accomplish this,
  the reading process must call nc_sync.

This function is backward-compatible with previous versions of the
netCDF library. The intent was to allow sharing of a netCDF dataset
among multiple readers and one writer, by having the writer call
nc_sync() after writing and the readers call nc_sync() before each
read. For a writer, this flushes buffers to disk. For a reader, it
makes sure that the next read will be from disk rather than from
previously cached buffers, so that the reader will see changes made by
the writing process (e.g., the number of records written) without
having to close and reopen the dataset. If you are only accessing a
small amount of data, it can be expensive in computer resources to
always synchronize to disk after every write, since you are giving up
the benefits of buffering.

An easier way to accomplish sharing (and what is now recommended) is
to have the writer and readers open the dataset with the NC_SHARE
flag, and then it will not be necessary to call nc_sync() at
all. However, the nc_sync() function still provides finer granularity
than the NC_SHARE flag, if only a few netCDF accesses need to be
synchronized among processes.

It is important to note that changes to the ancillary data, such as
attribute values, are not propagated automatically by use of the
NC_SHARE flag. Use of the nc_sync() function is still required for this
purpose.

Sharing datasets when the writer enters define mode to change the data
schema requires extra care. In previous releases, after the writer
left define mode, the readers were left looking at an old copy of the
dataset, since the changes were made to a new copy. The only way
readers could see the changes was by closing and reopening the
dataset. Now the changes are made in place, but readers have no
knowledge that their internal tables are now inconsistent with the new
dataset schema. If netCDF datasets are shared across redefinition,
some mechanism external to the netCDF library must be provided that
prevents access by readers during redefinition and causes the readers
to call nc_sync before any subsequent access.

When calling nc_sync(), the netCDF dataset must be in data mode. A
netCDF dataset in define mode is synchronized to disk only when
nc_enddef() is called. A process that is reading a netCDF dataset that
another process is writing may call nc_sync to get updated with the
changes made to the data by the writing process (e.g., the number of
records written), without having to close and reopen the dataset.

Data is automatically synchronized to disk when a netCDF dataset is
closed, or whenever you leave define mode.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\returns ::NC_NOERR No error.

\returns ::NC_EBADID Invalid ncid passed.
 */
int
nc_sync(int ncid)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->sync(ncid);
}

/** \ingroup datasets
No longer necessary for user to invoke manually.


\warning Users no longer need to call this function since it is called
automatically by nc_close() in case the dataset is in define mode and
something goes wrong with committing the changes. The function
nc_abort() just closes the netCDF dataset, if not in define mode. If
the dataset is being created and is still in define mode, the dataset
is deleted. If define mode was entered by a call to nc_redef(), the
netCDF dataset is restored to its state before definition mode was
entered and the dataset is closed.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\returns ::NC_NOERR No error.

<h1>Example</h1>

Here is an example using nc_abort to back out of redefinitions of a
dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int ncid, status, latid;
        ...
     status = nc_open("foo.nc", NC_WRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_redef(ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_def_dim(ncid, "lat", 18L, &latid);
     if (status != NC_NOERR) {
        handle_error(status);
        status = nc_abort(ncid);
        if (status != NC_NOERR) handle_error(status);
     }
\endcode

 */
int
nc_abort(int ncid)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;

#ifdef USE_REFCOUNT
   /* What to do if refcount > 0? */
   /* currently, forcibly abort */
   ncp->refcount = 0;
#endif

   stat = ncp->dispatch->abort(ncid);
   del_from_NCList(ncp);
   free_NC(ncp);
   return stat;
}

/** \ingroup datasets
Close an open netCDF dataset

If the dataset in define mode, nc_enddef() will be called before
closing. (In this case, if nc_enddef() returns an error, nc_abort() will
automatically be called to restore the dataset to the consistent state
before define mode was last entered.) After an open netCDF dataset is
closed, its netCDF ID may be reassigned to the next netCDF dataset
that is opened or created.

\param ncid NetCDF ID, from a previous call to nc_open() or nc_create().

\returns ::NC_NOERR No error.

\returns ::NC_EBADID Invalid id passed.

\returns ::NC_EBADGRPID ncid did not contain the root group id of this
file. (NetCDF-4 only).

<h1>Example</h1>

Here is an example using nc_close to finish the definitions of a new
netCDF dataset named foo.nc and release its netCDF ID:

\code
     #include <netcdf.h>
        ...
     int status = NC_NOERR;
     int ncid;
        ...
     status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
     if (status != NC_NOERR) handle_error(status);

        ...   create dimensions, variables, attributes

     status = nc_close(ncid);
     if (status != NC_NOERR) handle_error(status);
\endcode

 */
int
nc_close(int ncid)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;

#ifdef USE_REFCOUNT
   ncp->refcount--;
   if(ncp->refcount <= 0)
#endif
   {

	   stat = ncp->dispatch->close(ncid);
       /* Remove from the nc list */
       del_from_NCList(ncp);
       free_NC(ncp);
   }
   return stat;
}

/** \ingroup datasets
Change the fill-value mode to improve write performance.

This function is intended for advanced usage, to optimize writes under
some circumstances described below. The function nc_set_fill() sets the
fill mode for a netCDF dataset open for writing and returns the
current fill mode in a return parameter. The fill mode can be
specified as either ::NC_FILL or ::NC_NOFILL. The default behavior
corresponding to ::NC_FILL is that data is pre-filled with fill values,
that is fill values are written when you create non-record variables
or when you write a value beyond data that has not yet been
written. This makes it possible to detect attempts to read data before
it was written. For more information on the use of fill values see
Fill Values. For information about how to define your own fill values
see Attribute Conventions.

The behavior corresponding to ::NC_NOFILL overrides the default behavior
of prefilling data with fill values. This can be used to enhance
performance, because it avoids the duplicate writes that occur when
the netCDF library writes fill values that are later overwritten with
data.

A value indicating which mode the netCDF dataset was already in is
returned. You can use this value to temporarily change the fill mode
of an open netCDF dataset and then restore it to the previous mode.

After you turn on ::NC_NOFILL mode for an open netCDF dataset, you must
be certain to write valid data in all the positions that will later be
read. Note that nofill mode is only a transient property of a netCDF
dataset open for writing: if you close and reopen the dataset, it will
revert to the default behavior. You can also revert to the default
behavior by calling nc_set_fill() again to explicitly set the fill mode
to ::NC_FILL.

There are three situations where it is advantageous to set nofill
mode:
- Creating and initializing a netCDF dataset. In this case, you should
  set nofill mode before calling nc_enddef() and then write completely
  all non-record variables and the initial records of all the record
  variables you want to initialize.
- Extending an existing record-oriented netCDF dataset. Set nofill
  mode after opening the dataset for writing, then append the
  additional records to the dataset completely, leaving no intervening
  unwritten records.
- Adding new variables that you are going to initialize to an existing
  netCDF dataset. Set nofill mode before calling nc_enddef() then write
  all the new variables completely.

If the netCDF dataset has an unlimited dimension and the last record
was written while in nofill mode, then the dataset may be shorter than
if nofill mode was not set, but this will be completely transparent if
you access the data only through the netCDF interfaces.

The use of this feature may not be available (or even needed) in
future releases. Programmers are cautioned against heavy reliance upon
this feature.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param fillmode Desired fill mode for the dataset, either ::NC_NOFILL or
::NC_FILL.

\param old_modep Pointer to location for returned current fill mode of
the dataset before this call, either ::NC_NOFILL or ::NC_FILL.

\returns ::NC_NOERR No error.

\returns ::NC_EBADID The specified netCDF ID does not refer to an open
netCDF dataset.

\returns ::NC_EPERM The specified netCDF ID refers to a dataset open for
read-only access.

\returns ::NC_EINVAL The fill mode argument is neither ::NC_NOFILL nor
::NC_FILL.

<h1>Example</h1>

Here is an example using nc_set_fill() to set nofill mode for subsequent
writes of a netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int ncid, status, old_fill_mode;
        ...
     status = nc_open("foo.nc", NC_WRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);

        ...     write data with default prefilling behavior

     status = nc_set_fill(ncid, ::NC_NOFILL, &old_fill_mode);
     if (status != NC_NOERR) handle_error(status);

        ...    write data with no prefilling
\endcode
 */
int
nc_set_fill(int ncid, int fillmode, int *old_modep)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->set_fill(ncid,fillmode,old_modep);
}

/**
\internal

\deprecated This function was used in the old days with the Cray at
NCAR. The Cray is long gone, and this call is supported only for
backward compatibility.

\returns ::NC_NOERR No error.

\returns ::NC_EBADID Invalid ncid passed.
 */
int
nc_inq_base_pe(int ncid, int *pe)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_base_pe(ncid,pe);
}

/**
\internal

\deprecated This function was used in the old days with the Cray at
NCAR. The Cray is long gone, and this call is supported only for
backward compatibility.

\returns ::NC_NOERR No error.

\returns ::NC_EBADID Invalid ncid passed.
 */
int
nc_set_base_pe(int ncid, int pe)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->set_base_pe(ncid,pe);
}

/** \ingroup datasets
Inquire about the binary format of a netCDF file
as presented by the API.

This function returns the (rarely needed) format version.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param formatp Pointer to location for returned format version, one of
NC_FORMAT_CLASSIC, NC_FORMAT_64BIT_OFFSET, NC_FORMAT_CDF5, NC_FORMAT_NETCDF4,
NC_FORMAT_NETCDF4_CLASSIC.

\returns ::NC_NOERR No error.

\returns ::NC_EBADID Invalid ncid passed.

 */
int
nc_inq_format(int ncid, int *formatp)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_format(ncid,formatp);
}

/** \ingroup datasets
Obtain more detailed (vis-a-vis nc_inq_format)
format information about an open dataset.

Note that the netcdf API will present the file
as if it had the format specified by nc_inq_format.
The true file format, however, may not even be
a netcdf file; it might be DAP, HDF4, or PNETCDF,
for example. This function returns that true file type.
It also returns the effective mode for the file.

\param ncid NetCDF ID, from a previous call to nc_open() or
nc_create().

\param formatp Pointer to location for returned true format.

\param modep Pointer to location for returned mode flags.

Refer to the actual list in the file netcdf.h to see the
currently defined set.

\returns ::NC_NOERR No error.

\returns ::NC_EBADID Invalid ncid passed.

 */
int
nc_inq_format_extended(int ncid, int *formatp, int *modep)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_format_extended(ncid,formatp,modep);
}

/**\ingroup datasets
Inquire about a file or group.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param ndimsp Pointer to location for returned number of dimensions
defined for this netCDF dataset. Ignored if NULL.

\param nvarsp Pointer to location for returned number of variables
defined for this netCDF dataset. Ignored if NULL.

\param nattsp Pointer to location for returned number of global
attributes defined for this netCDF dataset. Ignored if NULL.

\param unlimdimidp Pointer to location for returned ID of the
unlimited dimension, if there is one for this netCDF dataset. If no
unlimited length dimension has been defined, -1 is returned. Ignored
if NULL.  If there are multiple unlimited dimensions (possible only
for netCDF-4 files), only a pointer to the first is returned, for
backward compatibility.  If you want them all, use nc_inq_unlimids().

\returns ::NC_NOERR No error.

\returns ::NC_EBADID Invalid ncid passed.

<h1>Example</h1>

Here is an example using nc_inq to find out about a netCDF dataset
named foo.nc:

\code
     #include <netcdf.h>
        ...
     int status, ncid, ndims, nvars, ngatts, unlimdimid;
        ...
     status = nc_open("foo.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);
     if (status != NC_NOERR) handle_error(status);
\endcode
 */
int
nc_inq(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *unlimdimidp)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq(ncid,ndimsp,nvarsp,nattsp,unlimdimidp);
}

/**\ingroup datasets
Inquire about a type.

Given an ncid and a typeid, get the information about a type. This
function will work on any type, including atomic and any user defined
type, whether compound, opaque, enumeration, or variable length array.

For even more information about a user defined type nc_inq_user_type().

\param ncid The ncid for the group containing the type (ignored for
atomic types).

\param xtype The typeid for this type, as returned by nc_def_compound,
nc_def_opaque, nc_def_enum, nc_def_vlen, or nc_inq_var, or as found in
netcdf.h in the list of atomic types (NC_CHAR, NC_INT, etc.).

\param name If non-NULL, the name of the user defined type will be
copied here. It will be NC_MAX_NAME bytes or less. For atomic types,
the type name from CDL will be given.

\param size If non-NULL, the (in-memory) size of the type in bytes
will be copied here. VLEN type size is the size of nc_vlen_t. String
size is returned as the size of a character pointer. The size may be
used to malloc space for the data, no matter what the type.

\returns ::NC_NOERR No error.

\returns ::NC_EBADTYPE Bad typeid.

\returns ::NC_ENOTNC4 Seeking a user-defined type in a netCDF-3 file.

\returns ::NC_ESTRICTNC3 Seeking a user-defined type in a netCDF-4 file
for which classic model has been turned on.

\returns ::NC_EBADGRPID Bad group ID in ncid.

\returns ::NC_EBADID Type ID not found.

\returns ::NC_EHDFERR An error was reported by the HDF5 layer.

<h1>Example</h1>

This example is from the test program tst_enums.c, and it uses all the
possible inquiry functions on an enum type.

\code
           if (nc_inq_user_type(ncid, typeids[0], name_in, &base_size_in, &base_nc_type_in,
                                &nfields_in, &class_in)) ERR;
           if (strcmp(name_in, TYPE_NAME) || base_size_in != sizeof(int) ||
               base_nc_type_in != NC_INT || nfields_in != NUM_MEMBERS || class_in != NC_ENUM) ERR;
           if (nc_inq_type(ncid, typeids[0], name_in, &base_size_in)) ERR;
           if (strcmp(name_in, TYPE_NAME) || base_size_in != sizeof(int)) ERR;
           if (nc_inq_enum(ncid, typeids[0], name_in, &base_nc_type, &base_size_in, &num_members)) ERR;
           if (strcmp(name_in, TYPE_NAME) || base_nc_type != NC_INT || num_members != NUM_MEMBERS) ERR;
           for (i = 0; i < NUM_MEMBERS; i++)
           {
              if (nc_inq_enum_member(ncid, typeid, i, name_in, &value_in)) ERR;
              if (strcmp(name_in, member_name[i]) || value_in != member_value[i]) ERR;
              if (nc_inq_enum_ident(ncid, typeid, member_value[i], name_in)) ERR;
              if (strcmp(name_in, member_name[i])) ERR;
           }

           if (nc_close(ncid)) ERR;
\endcode
 */
int
nc_inq_type(int ncid, nc_type xtype, char *name, size_t *size)
{
   NC* ncp;
   int stat;

   /* Do a quick triage on xtype */
   if(xtype <= NC_NAT) return NC_EBADTYPE;
   /* For compatibility, we need to allow inq about
      atomic types, even if ncid is ill-defined */
   if(xtype <= ATOMICTYPEMAX4) {
      if(name) strncpy(name,NC_atomictypename(xtype),NC_MAX_NAME);
      if(size) *size = NC_atomictypelen(xtype);
      return NC_NOERR;
   }
   /* Apparently asking about a user defined type, so we need
      a valid ncid */
   stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) /* bad ncid */
      return NC_EBADTYPE;
   /* have good ncid */
   return ncp->dispatch->inq_type(ncid,xtype,name,size);
}

/**
\internal
\ingroup dispatch

Create a file, calling the appropriate dispatch create call.

For create, we have the following pieces of information to use to
determine the dispatch table:
- table specified by override
- path
- cmode

\param path The file name of the new netCDF dataset.

\param cmode The creation mode flag, the same as in nc_create().

\param initialsz This parameter sets the initial size of the file at creation
time. This only applies to classic and 64-bit offset files.

\param basepe Deprecated parameter from the Cray days.

\param chunksizehintp A pointer to the chunk size hint. This only
applies to classic and 64-bit offset files.

\param useparallel Non-zero if parallel I/O is to be used on this
file.

\param parameters Pointer to MPI comm and info.

\param ncidp Pointer to location where returned netCDF ID is to be
stored.

\returns ::NC_NOERR No error.
*/
int
NC_create(const char *path0, int cmode, size_t initialsz,
	  int basepe, size_t *chunksizehintp, int useparallel,
	  void* parameters, int *ncidp)
{
   int stat = NC_NOERR;
   NC* ncp = NULL;
   NC_Dispatch* dispatcher = NULL;
   /* Need three pieces of information for now */
   int model = NC_FORMATX_UNDEFINED; /* one of the NC_FORMATX values */
   int isurl = 0;   /* dap or cdmremote or neither */
   int xcmode = 0; /* for implied cmode flags */
   char* path = NULL;

   TRACE(nc_create);
   if(path0 == NULL)
	return NC_EINVAL;
   /* Initialize the dispatch table. The function pointers in the
    * dispatch table will depend on how netCDF was built
    * (with/without netCDF-4, DAP, CDMREMOTE). */
   if(!NC_initialized)
   {
      if ((stat = nc_initialize()))
	 return stat;
   }

#ifdef WINPATH
   /* Need to do path conversion */
   path = NCpathcvt(path0);
fprintf(stderr,"XXX: path0=%s path=%s\n",path0,path); fflush(stderr);
#else
   path = nulldup(path0);
#endif

#ifdef USE_REFCOUNT
   /* If this path is already open, then fail */
   ncp = find_in_NCList_by_name(path);
   if(ncp != NULL) {
	nullfree(path);
	return NC_ENFILE;
   }
#endif

    {
	char* newpath = NULL;
        model = NC_urlmodel(path,cmode,&newpath);
        isurl = (model != 0);
        if(isurl) {
	    nullfree(path);
	    path = newpath;
	}
    }

   /* Look to the incoming cmode for hints */
   if(model == NC_FORMATX_UNDEFINED) {
#ifdef USE_NETCDF4
      if((cmode & NC_NETCDF4) == NC_NETCDF4)
	model = NC_FORMATX_NC4;
      else
#endif
#ifdef USE_PNETCDF
      /* pnetcdf is used for parallel io on CDF-1, CDF-2, and CDF-5 */
      if((cmode & NC_MPIIO) == NC_MPIIO)
	model = NC_FORMATX_PNETCDF;
      else
#endif
	{}
    }
    if(model == NC_FORMATX_UNDEFINED) {
      /* Check default format (not formatx) */
      int format = nc_get_default_format();
      switch (format) {
#ifdef USE_NETCDF4
	 case NC_FORMAT_NETCDF4:
	    xcmode |= NC_NETCDF4;
	    model = NC_FORMATX_NC4;
	    break;
	 case NC_FORMAT_NETCDF4_CLASSIC:
	    xcmode |= NC_CLASSIC_MODEL;
	    model = NC_FORMATX_NC4;
	    break;
#endif
	 case NC_FORMAT_CDF5:
	    xcmode |= NC_64BIT_DATA;
	    model = NC_FORMATX_NC3;
	    break;
	 case NC_FORMAT_64BIT_OFFSET:
	    xcmode |= NC_64BIT_OFFSET;
	    model = NC_FORMATX_NC3;
	    break;
	 case NC_FORMAT_CLASSIC:
	    model = NC_FORMATX_NC3;
	    break;
	 default:
	    model = NC_FORMATX_NC3;
	    break;
      }
   }

   /* Add inferred flags */
   cmode |= xcmode;

   /* Clean up illegal combinations */
   if((cmode & (NC_64BIT_OFFSET|NC_64BIT_DATA)) == (NC_64BIT_OFFSET|NC_64BIT_DATA))
	cmode &= ~(NC_64BIT_OFFSET); /*NC_64BIT_DATA=>NC_64BIT_OFFSET*/

   if((cmode & NC_MPIIO) && (cmode & NC_MPIPOSIX))
      return  NC_EINVAL;

   if (dispatcher == NULL)
   {

      /* Figure out what dispatcher to use */
#ifdef USE_NETCDF4
      if(model == (NC_FORMATX_NC4))
 	dispatcher = NC4_dispatch_table;
      else
#endif /*USE_NETCDF4*/
#ifdef USE_PNETCDF
      if(model == (NC_FORMATX_PNETCDF))
	dispatcher = NCP_dispatch_table;
      else
#endif
      if(model == (NC_FORMATX_NC3))
 	dispatcher = NC3_dispatch_table;
      else
	 return NC_ENOTNC;
   }

   /* Create the NC* instance and insert its dispatcher */
   stat = new_NC(dispatcher,path,cmode,&ncp);
   nullfree(path); path = NULL; /* no longer needed */
   
   if(stat) return stat;

   /* Add to list of known open files and define ext_ncid */
   add_to_NCList(ncp);

#ifdef USE_REFCOUNT
   /* bump the refcount */
   ncp->refcount++;
#endif

   /* Assume create will fill in remaining ncp fields */
   if ((stat = dispatcher->create(ncp->path, cmode, initialsz, basepe, chunksizehintp,
				   useparallel, parameters, dispatcher, ncp))) {
	del_from_NCList(ncp); /* oh well */
	free_NC(ncp);
     } else {
       if(ncidp)*ncidp = ncp->ext_ncid;
     }
   return stat;
}

/**
\internal
\ingroup dispatch

Open a netCDF file (or remote dataset) calling the appropriate
dispatch function.

For open, we have the following pieces of information to use to determine the dispatch table.
- table specified by override
- path
- cmode
- the contents of the file (if it exists), basically checking its magic number.

\returns ::NC_NOERR No error.
*/
int
NC_open(const char *path0, int cmode,
	int basepe, size_t *chunksizehintp,
        int useparallel, void* parameters,
        int *ncidp)
{
   int stat = NC_NOERR;
   NC* ncp = NULL;
   NC_Dispatch* dispatcher = NULL;
   int inmemory = ((cmode & NC_INMEMORY) == NC_INMEMORY);
   /* Need pieces of information for now to decide model*/
   int model = 0;
   int isurl = 0;
   int version = 0;
   int flags = 0;
   char* path = NULL;

   TRACE(nc_open);
   if(!NC_initialized) {
      stat = nc_initialize();
      if(stat) return stat;
   }

#ifdef WINPATH
   /* Need to do path conversion */
   path = NCpathcvt(path0);
fprintf(stderr,"XXX: path0=%s path=%s\n",path0,path); fflush(stderr);
#else
   path = nulldup(path0);
#endif

#ifdef USE_REFCOUNT
   /* If this path is already open, then bump the refcount and return it */
   ncp = find_in_NCList_by_name(path);
   if(ncp != NULL) {
	nullfree(path);
	ncp->refcount++;
	if(ncidp) *ncidp = ncp->ext_ncid;
	return NC_NOERR;
   }
#endif

   if(!inmemory) {
	char* newpath = NULL;
        model = NC_urlmodel(path,cmode,&newpath);
        isurl = (model != 0);
	if(isurl) {
	    nullfree(path);
	    path = newpath;
	} else
	    nullfree(newpath);
    }
    if(model == 0) {
	version = 0;
	/* Try to find dataset type */
	if(useparallel) flags |= NC_MPIIO;
	if(inmemory) flags |= NC_INMEMORY;
	stat = NC_check_file_type(path,flags,parameters,&model,&version);
        if(stat == NC_NOERR) {
   	if(model == 0)
	    return NC_ENOTNC;
	} else /* presumably not a netcdf file */
	    return stat;
    }

   if(model == 0) {
	fprintf(stderr,"Model == 0\n");
	return NC_ENOTNC;
   }

   /* Force flag consistentcy */
   if(model == NC_FORMATX_NC4 || model == NC_FORMATX_DAP4)
      cmode |= NC_NETCDF4;
   else if(model == NC_FORMATX_DAP2) {
      cmode &= ~NC_NETCDF4;
      cmode &= ~NC_PNETCDF;
      cmode &= ~NC_64BIT_OFFSET;
   } else if(model == NC_FORMATX_NC3) {
      cmode &= ~NC_NETCDF4; /* must be netcdf-3 (CDF-1, CDF-2, CDF-5) */
      /* User may want to open file using the pnetcdf library */
      if(cmode & NC_PNETCDF) {
         /* dispatch is determined by cmode, rather than file format */
         model = NC_FORMATX_PNETCDF;
      }
      /* For opening an existing file, flags NC_64BIT_OFFSET and NC_64BIT_DATA
       * will be ignored, as the file is already in either CDF-1, 2, or 5
       * format. However, below we add the file format info to cmode so the
       * internal netcdf file open subroutine knows what file format to open.
       * The mode will be saved in ncp->mode, to be used by
       * nc_inq_format_extended() to report the file format.
       * See NC3_inq_format_extended() in libsrc/nc3internal.c for example.
       */
      if(version == 2) cmode |= NC_64BIT_OFFSET;
      else if(version == 5) {
        cmode |= NC_64BIT_DATA;
        cmode &= ~(NC_64BIT_OFFSET); /*NC_64BIT_DATA=>NC_64BIT_OFFSET*/
      }
   } else if(model == NC_FORMATX_PNETCDF) {
     cmode &= ~(NC_NETCDF4|NC_64BIT_OFFSET);
     cmode |= NC_64BIT_DATA;
   }

   if((cmode & NC_MPIIO && cmode & NC_MPIPOSIX))
     return  NC_EINVAL;

   /* override any other table choice */
   if(dispatcher != NULL) goto havetable;

   /* Figure out what dispatcher to use */
#if defined(ENABLE_DAP)
   if(model == (NC_FORMATX_DAP2))
	dispatcher = NCD2_dispatch_table;
   else
#endif
#if defined(ENABLE_DAP4)
   if(model == (NC_FORMATX_DAP4))
	dispatcher = NCD4_dispatch_table;
   else
#endif
#if  defined(USE_PNETCDF)
   if(model == (NC_FORMATX_PNETCDF))
	dispatcher = NCP_dispatch_table;
   else
#endif
#if defined(USE_NETCDF4)
   if(model == (NC_FORMATX_NC4))
	dispatcher = NC4_dispatch_table;
   else
#endif
   if(model == (NC_FORMATX_NC3))
	dispatcher = NC3_dispatch_table;
   else
      return  NC_ENOTNC;

havetable:

   if(dispatcher == NULL)
	return NC_ENOTNC;

   /* Create the NC* instance and insert its dispatcher */
   stat = new_NC(dispatcher,path,cmode,&ncp);
   nullfree(path); path = NULL; /* no longer need path */
   if(stat) return stat;

   /* Add to list of known open files */
   add_to_NCList(ncp);

#ifdef USE_REFCOUNT
   /* bump the refcount */
   ncp->refcount++;
#endif

   /* Assume open will fill in remaining ncp fields */
   stat = dispatcher->open(ncp->path, cmode, basepe, chunksizehintp,
			   useparallel, parameters, dispatcher, ncp);
   if(stat == NC_NOERR) {
     if(ncidp) *ncidp = ncp->ext_ncid;
   } else {
	del_from_NCList(ncp);
	free_NC(ncp);
   }
   return stat;
}

/**************************************************/
/* Originally in datt.c */
/*
Attribute functions
*/

/** \defgroup attributes Attributes

Attributes hold metadata about data and files.

\image html ncatts.png "Attributes store metadata."

Attributes may be associated with a netCDF variable to specify such
properties as units, special values, maximum and minimum valid values,
scaling factors, and offsets. 

Attributes for a netCDF dataset are defined when the dataset is first
created, while the netCDF dataset is in define mode. Additional
attributes may be added later by reentering define mode. 

A netCDF attribute has a netCDF variable to which it is assigned, a
name, a type, a length, and a sequence of one or more values. 

An attribute is designated by its variable ID and name. When an
attribute name is not known, it may be designated by its variable ID
and number in order to determine its name, using the function
nc_inq_attname().

The attributes associated with a variable are typically defined
immediately after the variable is created, while still in define
mode. The data type, length, and value of an attribute may be changed
even when in data mode, as long as the changed attribute requires no
more space than the attribute as originally defined.

It is also possible to have attributes that are not associated with
any variable. These are called global attributes and are identified by
using ::NC_GLOBAL as a variable pseudo-ID. Global attributes are usually
related to the netCDF dataset as a whole and may be used for purposes
such as providing a title or processing history for a netCDF dataset.

Operations supported on attributes are:
- Create an attribute, given its variable ID, name, data type, length,
  and value.
- Get attribute's data type and length from its variable ID and name.
- Get attribute's value from its variable ID and name.
- Copy attribute from one netCDF variable to another.
- Get name of attribute from its number.
- Rename an attribute.
- Delete an attribute. 
*/

/*! \} */  /* End of \defgroup ...*/

/** \name Deleting and Renaming Attributes

Functions to delete or rename an attribute. */

/** \ingroup attributes
Rename an attribute.

The function nc_rename_att() changes the name of an attribute. If the
new name is longer than the original name, the netCDF dataset must be
in define mode. You cannot rename an attribute to have the same name
as another attribute of the same variable.

\param ncid NetCDF or group ID, from a previous call to nc_open(), 
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID of the attribute's variable, or ::NC_GLOBAL for
a global attribute.

\param name Attribute \ref object_name. 

\param newname The new attribute \ref object_name. 

<h1>Example</h1>

Here is an example using nc_rename_att() to rename the variable
attribute units to Units for a variable rh in an existing netCDF
dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status;
     int  ncid;  
     int  rh_id; 
        ...
     status = nc_open("foo.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_rename_att(ncid, rh_id, "units", "Units");
     if (status != NC_NOERR) handle_error(status);
\endcode
 */
int
nc_rename_att(int ncid, int varid, const char *name, const char *newname)
{

   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   TRACE(nc_rename_att);
   return ncp->dispatch->rename_att(ncid, varid, name, newname);
}

/** \ingroup attributes

Delete an attribute.

The function nc_del_att() deletes a netCDF attribute from an open
netCDF dataset. The netCDF dataset must be in define mode.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID of the attribute's variable, or ::NC_GLOBAL
for a global attribute.

\param name Attribute name. 

<h1>Example</h1>

Here is an example using nc_del_att() to delete the variable attribute
Units for a variable rh in an existing netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status;     
     int  ncid;      
     int  rh_id;     
        ...
     status = nc_open("foo.nc", NC_WRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_redef(ncid); 
     if (status != NC_NOERR) handle_error(status);
     status = nc_del_att(ncid, rh_id, "Units");
     if (status != NC_NOERR) handle_error(status);
     status = nc_enddef(ncid);
     if (status != NC_NOERR) handle_error(status);
\endcode
 */
int
nc_del_att(int ncid, int varid, const char *name)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   TRACE(nc_del_att);
   stat = ncp->dispatch->del_att(ncid, varid, name);
   return stat;
}

/*! \} */  /* End of named group ...*/

/** \name Learning about Attributes

Functions to learn about the attributes in a file. */
/*! \{ */ /* All these functions are part of this named group... */

/**
\ingroup attributes
Return information about a netCDF attribute.

The function nc_inq_att returns the attribute's type and length.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID of the attribute's variable, or ::NC_GLOBAL
for a global attribute.

\param name Pointer to the location for the returned attribute \ref
object_name. \ref ignored_if_null.

\param xtypep Pointer to location for returned attribute \ref
data_type. \ref ignored_if_null.

\param lenp Pointer to location for returned number of values
currently stored in the attribute. For attributes of type ::NC_CHAR,
you should not assume that this includes a trailing zero byte; it
doesn't if the attribute was stored without a trailing zero byte, for
example from a FORTRAN program. Before using the value as a C string,
make sure it is null-terminated. \ref ignored_if_null.

\section nc_inq_att_example Example

Here is an example using nc_inq_att() to find out the type and length of
a variable attribute named valid_range for a netCDF variable named rh
and a global attribute named title in an existing netCDF dataset named
foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status;             
     int  ncid;               
     int  rh_id;              
     nc_type vr_type, t_type; 
     size_t  vr_len, t_len;   
     
        ...
     status = nc_open("foo.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_att (ncid, rh_id, "valid_range", &vr_type, &vr_len);
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_att (ncid, NC_GLOBAL, "title", &t_type, &t_len);
     if (status != NC_NOERR) handle_error(status);
\endcode
*/
int
nc_inq_att(int ncid, int varid, const char *name, nc_type *xtypep, size_t *lenp)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_att(ncid, varid, name, xtypep, lenp);
}

/**
\ingroup attributes
Find an attribute ID.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID of the attribute's variable, or ::NC_GLOBAL for
a global attribute.

\param name Attribute \ref object_name. 

\param idp Pointer to location for returned attribute number that
specifies which attribute this is for this variable (or which global
attribute). If you already know the attribute name, knowing its number
is not very useful, because accessing information about an attribute
requires its name.
*/
int
nc_inq_attid(int ncid, int varid, const char *name, int *idp)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_attid(ncid, varid, name, idp);
}

/**
\ingroup attributes
Find the name of an attribute.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as 
nc_inq_ncid().

\param varid Variable ID of the attribute's variable, or ::NC_GLOBAL
for a global attribute.

\param attnum Attribute number. The attributes for each variable are
numbered from 0 (the first attribute) to natts-1, where natts is the
number of attributes for the variable, as returned from a call to
nc_inq_varnatts().

\param name Pointer to the location for the returned attribute \ref
object_name.  
*/
int
nc_inq_attname(int ncid, int varid, int attnum, char *name)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_attname(ncid, varid, attnum, name);
}

/*! \} */  /* End of named group ...*/


/** \ingroup attributes
\internal
Common api code for getting an attribute of some type
*/

int
NC_get_att(int ncid, int varid, const char *name, void *value, nc_type xtype)
{
   NC* ncp;
   int stat = NC_NOERR;

   if ((stat = NC_check_id(ncid, &ncp)))
      return stat;

   TRACE(NC_get_att);
   return ncp->dispatch->get_att(ncid, varid, name, value, xtype);
}

/** \ingroup attributes
\internal
Common api code for putting an attribute of some type
*/

int
NC_put_att(int ncid, int varid, const char *name, nc_type type, size_t len, const void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_NOERR;

   /* set global _FillValue is not allowed */
   if (varid == NC_GLOBAL && name != NULL && !strcmp(name, _FillValue))
       return NC_EGLOBAL;

   if ((stat = NC_check_id(ncid, &ncp)))
      return stat;

   TRACE(NC_put_att);
   return ncp->dispatch->put_att(ncid, varid, name, type, len, value, memtype);
}

/**************************************************/
/* Originally in ddim.c */

/*! \defgroup dimensions Dimensions

Dimensions are used to define the shape of data in netCDF.

Dimensions for a netCDF dataset are defined when it is created, while
the netCDF dataset is in define mode. Additional dimensions may be
added later by reentering define mode. A netCDF dimension has a name
and a length. In a netCDF classic or 64-bit offset file, at most one
dimension can have the unlimited length, which means variables using
this dimension can grow along this dimension. In a netCDF-4 file
multiple unlimited dimensions are supported.

There is a suggested limit (1024) to the number of dimensions that can
be defined in a single netCDF dataset. The limit is the value of the
predefined macro NC_MAX_DIMS. The purpose of the limit is to make
writing generic applications simpler. They need only provide an array
of NC_MAX_DIMS dimensions to handle any netCDF dataset. The
implementation of the netCDF library does not enforce this advisory
maximum, so it is possible to use more dimensions, if necessary, but
netCDF utilities that assume the advisory maximums may not be able to
handle the resulting netCDF datasets.

NC_MAX_VAR_DIMS, which must not exceed NC_MAX_DIMS, is the maximum
number of dimensions that can be used to specify the shape of a single
variable.  It is also intended to simplify writing generic
applications.

Ordinarily, the name and length of a dimension are fixed when the
dimension is first defined. The name may be changed later, but the
length of a dimension (other than the unlimited dimension) cannot be
changed without copying all the data to a new netCDF dataset with a
redefined dimension length.

Dimension lengths in the C interface are type size_t rather than type
int to make it possible to access all the data in a netCDF dataset on
a platform that only supports a 16-bit int data type, for example
MSDOS. If dimension lengths were type int instead, it would not be
possible to access data from variables with a dimension length greater
than a 16-bit int can accommodate.

A netCDF dimension in an open netCDF dataset is referred to by a small
integer called a dimension ID. In the C interface, dimension IDs are
0, 1, 2, ..., in the order in which the dimensions were defined.

Operations supported on dimensions are:
- Create a dimension, given its name and length.
- Get a dimension ID from its name.
- Get a dimension's name and length from its ID.
- Rename a dimension.

*/

/*! \{*/ /* All these functions are part of the above defgroup... */

/** \name Deleting and Renaming Dimensions

Functions to delete or rename an dimension. */
/*! \{ */ /* All these functions are part of this named group... */

/*!

Define a new dimension. The function nc_def_dim adds a new
dimension to an open netCDF dataset in define mode. It returns (as an
argument) a dimension ID, given the netCDF ID, the dimension name, and
the dimension length. At most one unlimited length dimension, called
the record dimension, may be defined for each classic or 64-bit offset
netCDF dataset. NetCDF-4 datasets may have multiple unlimited
dimensions.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param name Name of the dimension to be created.

\param len Length of the dimension to be created. Use NC_UNLIMITED for
unlimited dimensions.

\param idp Pointer where dimension ID will be stored.

\retval ::NC_NOERR No error.
\returns ::NC_EBADID Not a valid ID.
\returns ::NC_ENOTINDEFINE Not in define mode.
\returns ::NC_EDIMSIZE Invalid dimension size.
\returns ::NC_EUNLIMIT NC_UNLIMITED size already in use
\returns ::NC_EMAXDIMS NC_MAX_DIMS exceeded
\returns ::NC_ENAMEINUSE String match to name in use
\returns ::NC_ENOMEM Memory allocation (malloc) failure
\returns ::NC_EPERM Write to read only

\section nc_def_dim_example Example

Here is an example using nc_def_dim() to create a dimension named lat of
length 18 and a unlimited dimension named rec in a new netCDF dataset
named foo.nc:

\code
     #include <netcdf.h>
        ...
     int status, ncid, latid, recid;
        ...
     status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_def_dim(ncid, "lat", 18L, &latid);
     if (status != NC_NOERR) handle_error(status);
     status = nc_def_dim(ncid, "rec", NC_UNLIMITED, &recid);
     if (status != NC_NOERR) handle_error(status);
\endcode

 */
int
nc_def_dim(int ncid, const char *name, size_t len, int *idp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    TRACE(nc_def_dim);
    return ncp->dispatch->def_dim(ncid, name, len, idp);
}

/*!
Find the ID of a dimension from the name.

The function nc_inq_dimid returns (as an argument) the ID of a netCDF
dimension, given the name of the dimension. If ndims is the number of
dimensions defined for a netCDF dataset, each dimension has an ID
between 0 and ndims-1.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param name Name of the dimension.

\param idp Pointer where dimension ID will be stored.

\returns ::NC_NOERR   No error.
\returns ::NC_EBADID  Not a valid ID.
\returns ::NC_EBADDIM Invalid dimension ID or name.
 */
int
nc_inq_dimid(int ncid, const char *name, int *idp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    TRACE(nc_inq_dimid);
    return ncp->dispatch->inq_dimid(ncid,name,idp);
}

/*!
Find the name and length of a dimension.

The length for the unlimited dimension, if any, is the number of
records written so far.

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
will be null-terminated.

\param lenp Pointer to location for returned length of dimension. For
the unlimited dimension, this is the number of records written so far.

\returns ::NC_NOERR   No error.
\returns ::NC_EBADID  Not a valid ID.
\returns ::NC_EBADDIM Invalid dimension ID or name.

\section nc_inq_dim_example Example

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
nc_inq_dim(int ncid, int dimid, char *name, size_t *lenp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    TRACE(nc_inq_dim);
    return ncp->dispatch->inq_dim(ncid,dimid,name,lenp);
}

/*!
Rename a dimension.

This function renames an existing dimension in a netCDF dataset open
for writing. You cannot rename a dimension to have the same name as
another dimension.

For netCDF classic and 64-bit offset files, if the new name is longer
than the old name, the netCDF dataset must be in define mode.

For netCDF-4 files the dataset is switched to define more for the
rename, regardless of the name length.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param dimid Dimension ID, from a previous call to nc_inq_dimid() or
nc_def_dim().

\param name New name for dimension. Must be a null-terminated string
with length less than NC_MAX_NAME.

\returns ::NC_NOERR      No error.
\returns ::NC_EBADID     Not a valid ID.
\returns ::NC_EBADDIM    Invalid dimension ID or name.
\returns ::NC_ENAMEINUSE String match to name in use
\returns ::NC_ENOMEM     Memory allocation (malloc) failure
\returns ::NC_EPERM      Write to read only
\section nc_rename_dim_example Example

Here is an example using nc_rename_dim to rename the dimension lat to
latitude in an existing netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int status, ncid, latid;
        ...
     status = nc_open("foo.nc", NC_WRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_redef(ncid);
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_dimid(ncid, "lat", &latid);
     if (status != NC_NOERR) handle_error(status);
     status = nc_rename_dim(ncid, latid, "latitude");
     if (status != NC_NOERR) handle_error(status);
     status = nc_enddef(ncid);
     if (status != NC_NOERR) handle_error(status);
\endcode
 */
int
nc_rename_dim(int ncid, int dimid, const char *name)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    TRACE(nc_rename_dim);
    return ncp->dispatch->rename_dim(ncid,dimid,name);
}

/*!
Find the number of dimensions.

In a classic model netCDF file, this function returns the number of
defined dimensions. In a netCDF-4/HDF5 file, this function returns the
number of dimensions available in the group specified by ncid, which
may be less than the total number of dimensions in a file. In a
netCDF-4/HDF5 file, dimensions are in all sub-groups, sub-sub-groups,
etc.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param ndimsp Pointer where number of dimensions will be
written. Ignored if NULL.

\returns ::NC_NOERR  No error.
\returns ::NC_EBADID Not a valid ID.

 */
int
nc_inq_ndims(int ncid, int *ndimsp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    if(ndimsp == NULL) return NC_NOERR;
    TRACE(nc_inq_ndims);
    return ncp->dispatch->inq(ncid,ndimsp,NULL,NULL,NULL);
}

/*!
Find the ID of the unlimited dimension.

This function finds the ID of the unlimited dimension. For
netCDF-4/HDF5 files (which may have more than one unlimited
dimension), the ID of the first unlimited dimesnion is returned. For
these files, nc_inq_unlimdims() will return all the unlimited dimension IDs.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param unlimdimidp Pointer where unlimited dimension ID will be
stored. If there is no unlimited dimension, -1 will be stored
here. Ignored if NULL.

\returns ::NC_NOERR  No error.
\returns ::NC_EBADID Not a valid ID.

 */
int
nc_inq_unlimdim(int ncid, int *unlimdimidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    TRACE(nc_inq_unlimdim);
    return ncp->dispatch->inq_unlimdim(ncid,unlimdimidp);
}

/*! \} */  /* End of named group ...*/

/*! \} */ /* End of defgroup. */

/**************************************************/
/* Originally in dvar.c */

/** \defgroup variables Variables

Variables hold multi-dimensional arrays of data.

Variables for a netCDF dataset are defined when the dataset is
created, while the netCDF dataset is in define mode. Other variables
may be added later by reentering define mode. A netCDF variable has a
name, a type, and a shape, which are specified when it is defined. A
variable may also have values, which are established later in data
mode.

Ordinarily, the name, type, and shape are fixed when the variable is
first defined. The name may be changed, but the type and shape of a
variable cannot be changed. However, a variable defined in terms of
the unlimited dimension can grow without bound in that dimension.

A netCDF variable in an open netCDF dataset is referred to by a small
integer called a variable ID.

Variable IDs reflect the order in which variables were defined within
a netCDF dataset. Variable IDs are 0, 1, 2,..., in the order in which
the variables were defined. A function is available for getting the
variable ID from the variable name and vice-versa.

Attributes (see Attributes) may be associated with a variable to
specify such properties as units.

Operations supported on variables are:
- Create a variable, given its name, data type, and shape.
- Get a variable ID from its name.
- Get a variable's name, data type, shape, and number of attributes
  from its ID.
- Put a data value into a variable, given variable ID, indices, and value.
- Put an array of values into a variable, given variable ID, corner
  indices, edge lengths, and a block of values.
- Put a subsampled or mapped array-section of values into a variable,
  given variable ID, corner indices, edge lengths, stride vector,
  index mapping vector, and a block of values.
- Get a data value from a variable, given variable ID and indices.
- Get an array of values from a variable, given variable ID, corner
  indices, and edge lengths.
- Get a subsampled or mapped array-section of values from a variable,
  given variable ID, corner indices, edge lengths, stride vector, and
  index mapping vector.
- Rename a variable.

\section language_types Language Types Corresponding to netCDF
External Data Types

NetCDF supported six atomic data types through version 3.6.0 (char,
byte, short, int, float, and double). Starting with version 4.0, many
new atomic and user defined data types are supported (unsigned int
types, strings, compound types, variable length arrays, enums,
opaque).

The additional data types are only supported in netCDF-4/HDF5
files. To create netCDF-4/HDF5 files, use the HDF5 flag in
nc_create. (see nc_create).

\section classic_types NetCDF-3 Classic and 64-Bit Offset Data Types

NetCDF-3 classic and 64-bit offset files support 6 atomic data types,
and none of the user defined datatype introduced in NetCDF-4.

The following table gives the netCDF-3 external data types and the
corresponding type constants for defining variables in the C
interface:

<table>
<tr><td>Type</td><td>C define</td><td>Bits</td></tr>
<tr><td>byte</td><td>NC_BYTE</td><td>8</td></tr>
<tr><td>char</td><td>NC_CHAR</td><td>8</td></tr>
<tr><td>short</td><td>NC_SHORT</td><td>16</td></tr>
<tr><td>int</td><td>NC_INT</td><td>32</td></tr>
<tr><td>float</td><td>NC_FLOAT</td><td>32</td></tr>
<tr><td>double</td><td>NC_DOUBLE</td><td>64</td></tr>
</table>

The first column gives the netCDF external data type, which is the
same as the CDL data type. The next column gives the corresponding C
pre-processor macro for use in netCDF functions (the pre-processor
macros are defined in the netCDF C header-file netcdf.h). The last
column gives the number of bits used in the external representation of
values of the corresponding type.

\section netcdf_4_atomic NetCDF-4 Atomic Types

NetCDF-4 files support all of the atomic data types from netCDF-3,
plus additional unsigned integer types, 64-bit integer types, and a
string type.

<table>
<tr><td>Type</td><td>C define</td><td>Bits

<tr><td>byte</td><td>NC_BYTE</td><td>8</td></tr>
<tr><td>unsigned byte </td><td>NC_UBYTE^</td><td> 8</td></tr>
<tr><td>char </td><td>NC_CHAR </td><td>8</td></tr>
<tr><td>short </td><td>NC_SHORT </td><td>16</td></tr>
<tr><td>unsigned short </td><td>NC_USHORT^ </td><td>16</td></tr>
<tr><td>int </td><td>NC_INT </td><td>32</td></tr>
<tr><td>unsigned int </td><td>NC_UINT^ </td><td>32</td></tr>
<tr><td>unsigned long long </td><td>NC_UINT64^ </td><td>64</td></tr>
<tr><td>long long </td><td>NC_INT64^ </td><td>64</td></tr>
<tr><td>float </td><td>NC_FLOAT </td><td>32</td></tr>
<tr><td>double </td><td>NC_DOUBLE </td><td>64</td></tr>
<tr><td>char ** </td><td>NC_STRING^ </td><td>string length + 1</td></tr>
</table>

^This type was introduced in netCDF-4, and is not supported in netCDF
classic or 64-bit offset format files, or in netCDF-4 files if they
are created with the NC_CLASSIC_MODEL flags.
 */

/** \name Defining Variables

Use these functions to define variables.
 */
/*! \{ */

/**
\ingroup variables
Define a new variable.

This function adds a new variable to an open netCDF dataset or group.
It returns (as an argument) a variable ID, given the netCDF ID,
the variable name, the variable type, the number of dimensions, and a
list of the dimension IDs.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param name Variable \ref object_name.

\param xtype \ref data_type of the variable.

\param ndims Number of dimensions for the variable. For example, 2
specifies a matrix, 1 specifies a vector, and 0 means the variable is
a scalar with no dimensions. Must not be negative or greater than the
predefined constant ::NC_MAX_VAR_DIMS.

\param dimidsp Vector of ndims dimension IDs corresponding to the
variable dimensions. For classic model netCDF files, if the ID of the
unlimited dimension is included, it must be first. This argument is
ignored if ndims is 0. For expanded model netCDF4/HDF5 files, there
may be any number of unlimited dimensions, and they may be used in any
element of the dimids array.

\param varidp Pointer to location for the returned variable ID.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTINDEFINE Not in define mode.
\returns ::NC_ESTRICTNC3 Attempting netcdf-4 operation on strict nc3 netcdf-4 file.
\returns ::NC_EMAXVARS NC_MAX_VARS exceeded
\returns ::NC_EBADTYPE Bad type.
\returns ::NC_EINVAL Invalid input.
\returns ::NC_ENAMEINUSE Name already in use.
\returns ::NC_EPERM Attempt to create object in read-only file.

\section nc_def_var_example Example

Here is an example using nc_def_var to create a variable named rh of
type double with three dimensions, time, lat, and lon in a new netCDF
dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status;
     int  ncid;
     int  lat_dim, lon_dim, time_dim;
     int  rh_id;
     int  rh_dimids[3];
        ...
     status = nc_create("foo.nc", NC_NOCLOBBER, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...

     status = nc_def_dim(ncid, "lat", 5L, &lat_dim);
     if (status != NC_NOERR) handle_error(status);
     status = nc_def_dim(ncid, "lon", 10L, &lon_dim);
     if (status != NC_NOERR) handle_error(status);
     status = nc_def_dim(ncid, "time", NC_UNLIMITED, &time_dim);
     if (status != NC_NOERR) handle_error(status);
        ...

     rh_dimids[0] = time_dim;
     rh_dimids[1] = lat_dim;
     rh_dimids[2] = lon_dim;
     status = nc_def_var (ncid, "rh", NC_DOUBLE, 3, rh_dimids, &rh_id);
     if (status != NC_NOERR) handle_error(status);
\endcode

 */
int
nc_def_var(int ncid, const char *name, nc_type xtype,
	   int ndims,  const int *dimidsp, int *varidp)
{
   NC* ncp;
   int stat = NC_NOERR;

   if ((stat = NC_check_id(ncid, &ncp)))
      return stat;
   TRACE(nc_def_var);
   return ncp->dispatch->def_var(ncid, name, xtype, ndims, dimidsp, varidp);
}
/*! \} */

/** \name Rename a Variable

Rename a variable.
 */
/*! \{ */

/** Rename a variable.
\ingroup variables

This function changes the name of a netCDF variable in an open netCDF
file or group. You cannot rename a variable to have the name of any existing
variable.

For classic format, 64-bit offset format, and netCDF-4/HDF5 with
classic mode, if the new name is longer than the old name, the netCDF
dataset must be in define mode.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param name New name of the variable.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
\returns ::NC_EBADNAME Bad name.
\returns ::NC_EMAXNAME Name is too long.
\returns ::NC_ENAMEINUSE Name in use.
\returns ::NC_ENOMEM Out of memory.

\section nc_rename_var_example Example

Here is an example using nc_rename_var to rename the variable rh to
rel_hum in an existing netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status;
     int  ncid;
     int  rh_id;
        ...
     status = nc_open("foo.nc", NC_WRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_redef(ncid);
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
     status = nc_rename_var (ncid, rh_id, "rel_hum");
     if (status != NC_NOERR) handle_error(status);
     status = nc_enddef(ncid);
     if (status != NC_NOERR) handle_error(status);
\endcode

*/
int
nc_rename_var(int ncid, int varid, const char *name)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   TRACE(nc_rename_var);
   return ncp->dispatch->rename_var(ncid, varid, name);
}
/*! \} */


#ifdef USE_NETCDF4
/** \ingroup variables

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param size The total size of the raw data chunk cache, in bytes.

\param nelems The number of chunk slots in the raw data chunk cache.

\param preemption The preemption, a value between 0 and 1 inclusive
that indicates how much chunks that have been fully read are favored
for preemption. A value of zero means fully read chunks are treated no
differently than other chunks (the preemption is strictly LRU) while a
value of one means fully read chunks are always preempted before other
chunks.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
\returns ::NC_ESTRICTNC3 Attempting netcdf-4 operation on strict nc3 netcdf-4 file.
\returns ::NC_EINVAL Invalid input
 */
int
nc_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems,
		       float preemption)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->set_var_chunk_cache(ncid, varid, size,
					      nelems, preemption);
}

/** \ingroup variables

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param sizep The total size of the raw data chunk cache, in bytes,
will be put here. \ref ignored_if_null.

\param nelemsp The number of chunk slots in the raw data chunk cache
hash table will be put here. \ref ignored_if_null.

\param preemptionp The preemption will be put here. The preemtion
value is between 0 and 1 inclusive and indicates how much chunks that
have been fully read are favored for preemption. A value of zero means
fully read chunks are treated no differently than other chunks (the
preemption is strictly LRU) while a value of one means fully read
chunks are always preempted before other chunks. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
\returns ::NC_ESTRICTNC3 Attempting netcdf-4 operation on strict nc3 netcdf-4 file.
\returns ::NC_EINVAL Invalid input
*/
int
nc_get_var_chunk_cache(int ncid, int varid, size_t *sizep, size_t *nelemsp,
		       float *preemptionp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->get_var_chunk_cache(ncid, varid, sizep,
					      nelemsp, preemptionp);
}

int
nc_def_var_deflate(int ncid, int varid, int shuffle, int deflate, int deflate_level)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_deflate(ncid,varid,shuffle,deflate,deflate_level);
}

int
nc_def_var_fletcher32(int ncid, int varid, int fletcher32)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_fletcher32(ncid,varid,fletcher32);
}

/*! Define chunking parameters for a variable

\ingroup variables

The function nc_def_var_chunking sets the chunking parameters for a variable in a netCDF-4 file. It can set the chunk sizes to get chunked storage, or it can set the contiguous flag to get contiguous storage.

The total size of a chunk must be less than 4 GiB. That is, the product of all chunksizes and the size of the data (or the size of nc_vlen_t for VLEN types) must be less than 4 GiB.

This function may only be called after the variable is defined, but before nc_enddef is called. Once the chunking parameters are set for a variable, they cannot be changed.

Note that this does not work for scalar variables. Only non-scalar variables can have chunking.

@param[in] ncid NetCDF ID, from a previous call to nc_open or nc_create.
@param[in] varid Variable ID.
@param[in] storage If ::NC_CONTIGUOUS, then contiguous storage is used for this variable. Variables with one or more unlimited dimensions cannot use contiguous storage. If contiguous storage is turned on, the chunksizes parameter is ignored. If ::NC_CHUNKED, then chunked storage is used for this variable. Chunk sizes may be specified with the chunksizes parameter or default sizes will be used if that parameter is NULL.
@param[in] chunksizesp A pointer to an array list of chunk sizes. The array must have one chunksize for each dimension of the variable. If ::NC_CONTIGUOUS storage is set, then the chunksizes parameter is ignored.

@returns ::NC_NOERR No error.
@returns ::NC_EBADID Bad ID.
@returns ::NC_ENOTNC4 Not a netCDF-4 file.
@returns ::NC_ELATEDEF This variable has already been the subject of a nc_enddef call.  In netCDF-4 files nc_enddef will be called automatically for any data read or write. Once nc_enddef has been called after the nc_def_var call for a variable, it is impossible to set the chunking for that variable.
@returns ::NC_ENOTINDEFINE Not in define mode.  This is returned for netCDF classic or 64-bit offset files, or for netCDF-4 files, when they wwere created with NC_STRICT_NC3 flag. See \ref nc_create.
@returns ::NC_EPERM Attempt to create object in read-only file.
@returns ::NC_EBADCHUNK Retunrs if the chunk size specified for a variable is larger than the length of the dimensions associated with variable.

\section nc_def_var_chunking_example Example

In this example from libsrc4/tst_vars2.c, chunksizes are set with nc_var_def_chunking, and checked with nc_var_inq_chunking.

\code
        printf("**** testing chunking...");
        {
     #define NDIMS5 1
     #define DIM5_NAME "D5"
     #define VAR_NAME5 "V5"
     #define DIM5_LEN 1000

           int dimids[NDIMS5], dimids_in[NDIMS5];
           int varid;
           int ndims, nvars, natts, unlimdimid;
           nc_type xtype_in;
           char name_in[NC_MAX_NAME + 1];
           int data[DIM5_LEN], data_in[DIM5_LEN];
           size_t chunksize[NDIMS5] = {5};
           size_t chunksize_in[NDIMS5];
           int storage_in;
           int i, d;

           for (i = 0; i < DIM5_LEN; i++)
              data[i] = i;

           if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
           if (nc_def_dim(ncid, DIM5_NAME, DIM5_LEN, &dimids[0])) ERR;
           if (nc_def_var(ncid, VAR_NAME5, NC_INT, NDIMS5, dimids, &varid)) ERR;
           if (nc_def_var_chunking(ncid, varid, NC_CHUNKED, chunksize)) ERR;
           if (nc_put_var_int(ncid, varid, data)) ERR;

           if (nc_inq_var_chunking(ncid, varid, &storage_in, chunksize_in)) ERR;
           for (d = 0; d < NDIMS5; d++)
              if (chunksize[d] != chunksize_in[d]) ERR;
           if (storage_in != NC_CHUNKED) ERR;
\endcode

*/
int
nc_def_var_chunking(int ncid, int varid, int storage,
		    const size_t *chunksizesp)
{
    NC* ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_chunking(ncid, varid, storage,
					   chunksizesp);
}

int
nc_def_var_fill(int ncid, int varid, int no_fill, const void *fill_value)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_fill(ncid,varid,no_fill,fill_value);
}

int
nc_def_var_endian(int ncid, int varid, int endian)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_var_endian(ncid,varid,endian);
}

#endif /* USE_NETCDF4 */

/** \name Learning about Variables

Functions to learn about the variables in a file. */
/*! \{ */ /* All these functions are part of this named group... */

/**
\ingroup variables
Find the ID of a variable, from the name.

The function nc_inq_varid returns the ID of a netCDF variable, given
its name.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param name Name of the variable.

\param varidp Pointer to location for returned variable ID.  \ref
ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.

\section nc_inq_varid_example4 Example

Here is an example using nc_inq_varid to find out the ID of a variable
named rh in an existing netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status, ncid, rh_id;
        ...
     status = nc_open("foo.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
\endcode
 */
int
nc_inq_varid(int ncid, const char *name, int *varidp)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_varid(ncid, name, varidp);
}

/**
\ingroup variables
Learn about a variable.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param name Returned \ref object_name of variable. \ref
ignored_if_null.

\param xtypep Pointer where typeid will be stored. \ref ignored_if_null.

\param ndimsp Pointer where number of dimensions will be
stored. \ref ignored_if_null.

\param dimidsp Pointer where array of dimension IDs will be
stored. \ref ignored_if_null.

\param nattsp Pointer where number of attributes will be
stored. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.

\section nc_inq_var_example5 Example

Here is an example using nc_inq_var() to find out about a variable named
rh in an existing netCDF dataset named foo.nc:

\code
     #include <netcdf.h>
        ...
     int  status
     int  ncid;
     int  rh_id;
     nc_type rh_type;
     int rh_ndims;
     int  rh_dimids[NC_MAX_VAR_DIMS];
     int rh_natts
        ...
     status = nc_open ("foo.nc", NC_NOWRITE, &ncid);
     if (status != NC_NOERR) handle_error(status);
        ...
     status = nc_inq_varid (ncid, "rh", &rh_id);
     if (status != NC_NOERR) handle_error(status);
     status = nc_inq_var (ncid, rh_id, 0, &rh_type, &rh_ndims, rh_dimids,
                          &rh_natts);
     if (status != NC_NOERR) handle_error(status);
\endcode

 */
int
nc_inq_var(int ncid, int varid, char *name, nc_type *xtypep,
	   int *ndimsp, int *dimidsp, int *nattsp)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   TRACE(nc_inq_var);
   return ncp->dispatch->inq_var_all(ncid, varid, name, xtypep, ndimsp,
				     dimidsp, nattsp, NULL, NULL, NULL,
				     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
}

/**
\ingroup variables
Learn the name of a variable.

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param name Returned variable name. The caller must allocate space for
the returned name. The maximum length is ::NC_MAX_NAME. Ignored if
NULL.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
 */
int
nc_inq_varname(int ncid, int varid, char *name)
{
   return nc_inq_var(ncid, varid, name, NULL, NULL,
		     NULL, NULL);
}

/** Learn the type of a variable.
\ingroup variables

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param typep Pointer where typeid will be stored. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
 */
int
nc_inq_vartype(int ncid, int varid, nc_type *typep)
{
   return nc_inq_var(ncid, varid, NULL, typep, NULL,
		     NULL, NULL);
}

/** Learn how many dimensions are associated with a variable.
\ingroup variables

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param ndimsp Pointer where number of dimensions will be
stored. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
 */
int
nc_inq_varndims(int ncid, int varid, int *ndimsp)
{
   return nc_inq_var(ncid, varid, NULL, NULL, ndimsp, NULL, NULL);
}

/** Learn the dimension IDs associated with a variable.
\ingroup variables

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param dimidsp Pointer where array of dimension IDs will be
stored. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
 */
int
nc_inq_vardimid(int ncid, int varid, int *dimidsp)
{
   return nc_inq_var(ncid, varid, NULL, NULL, NULL,
		     dimidsp, NULL);
}

/** Learn how many attributes are associated with a variable.
\ingroup variables

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param nattsp Pointer where number of attributes will be
stored. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
 */
int
nc_inq_varnatts(int ncid, int varid, int *nattsp)
{
   if (varid == NC_GLOBAL)
      return nc_inq_natts(ncid,nattsp);
   /*else*/
   return nc_inq_var(ncid, varid, NULL, NULL, NULL, NULL,
		     nattsp);
}

#ifdef USE_NETCDF4
/** \ingroup variables
Learn the storage and deflate settings for a variable.

This is a wrapper for NC_inq_var_all().

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param shufflep A 1 will be written here if the shuffle filter is
turned on for this variable, and a 0 otherwise. \ref ignored_if_null.

\param deflatep If this pointer is non-NULL, the nc_inq_var_deflate
function will write a 1 if the deflate filter is turned on for this
variable, and a 0 otherwise. \ref ignored_if_null.

\param deflate_levelp If the deflate filter is in use for this
variable, the deflate_level will be writen here. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTNC4 Not a netCDF-4 file.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
*/
int
nc_inq_var_deflate(int ncid, int varid, int *shufflep, int *deflatep,
		   int *deflate_levelp)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   TRACE(nc_inq_var_deflate);
   return ncp->dispatch->inq_var_all(
      ncid, varid,
      NULL, /*name*/
      NULL, /*xtypep*/
      NULL, /*ndimsp*/
      NULL, /*dimidsp*/
      NULL, /*nattsp*/
      shufflep, /*shufflep*/
      deflatep, /*deflatep*/
      deflate_levelp, /*deflatelevelp*/
      NULL, /*fletcher32p*/
      NULL, /*contiguousp*/
      NULL, /*chunksizep*/
      NULL, /*nofillp*/
      NULL, /*fillvaluep*/
      NULL, /*endianp*/
      NULL, /*optionsmaskp*/
      NULL /*pixelsp*/
      );
}

/** \ingroup variables
Learn the szip settings of a variable.

This function returns the szip settings for a variable. NetCDF does
not allow variables to be created with szip (due to license problems
with the szip library), but we do enable read-only access of HDF5
files with szip compression.

This is a wrapper for NC_inq_var_all().

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param options_maskp The szip options mask will be copied to this
pointer. \ref ignored_if_null.

\param pixels_per_blockp The szip pixels per block will be copied
here. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTNC4 Not a netCDF-4 file.
\returns ::NC_ENOTVAR Invalid variable ID.
*/
int
nc_inq_var_szip(int ncid, int varid, int *options_maskp, int *pixels_per_blockp)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   TRACE(nc_inq_var_szip);
   return ncp->dispatch->inq_var_all(
      ncid, varid,
      NULL, /*name*/
      NULL, /*xtypep*/
      NULL, /*ndimsp*/
      NULL, /*dimidsp*/
      NULL, /*nattsp*/
      NULL, /*shufflep*/
      NULL, /*deflatep*/
      NULL, /*deflatelevelp*/
      NULL, /*fletcher32p*/
      NULL, /*contiguousp*/
      NULL, /*chunksizep*/
      NULL, /*nofillp*/
      NULL, /*fillvaluep*/
      NULL, /*endianp*/
      options_maskp, /*optionsmaskp*/
      pixels_per_blockp /*pixelsp*/
      );
}

/** \ingroup variables
Learn the checksum settings for a variable.

This is a wrapper for NC_inq_var_all().

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param fletcher32p Will be set to ::NC_FLETCHER32 if the fletcher32
checksum filter is turned on for this variable, and ::NC_NOCHECKSUM if
it is not. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTNC4 Not a netCDF-4 file.
\returns ::NC_ENOTVAR Invalid variable ID.
*/
int
nc_inq_var_fletcher32(int ncid, int varid, int *fletcher32p)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   TRACE(nc_inq_var_fletcher32);
   return ncp->dispatch->inq_var_all(
      ncid, varid,
      NULL, /*name*/
      NULL, /*xtypep*/
      NULL, /*ndimsp*/
      NULL, /*dimidsp*/
      NULL, /*nattsp*/
      NULL, /*shufflep*/
      NULL, /*deflatep*/
      NULL, /*deflatelevelp*/
      fletcher32p, /*fletcher32p*/
      NULL, /*contiguousp*/
      NULL, /*chunksizep*/
      NULL, /*nofillp*/
      NULL, /*fillvaluep*/
      NULL, /*endianp*/
      NULL, /*optionsmaskp*/
      NULL /*pixelsp*/
      );
}

/** \ingroup variables

This is a wrapper for NC_inq_var_all().

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param storagep Address of returned storage property, returned as
::NC_CONTIGUOUS if this variable uses contiguous storage, or
::NC_CHUNKED if it uses chunked storage. \ref ignored_if_null.

\param chunksizesp The chunksizes will be copied here. \ref
ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTNC4 Not a netCDF-4 file.
\returns ::NC_ENOTVAR Invalid variable ID.


\section nc_inq_var_chunking_example Example

\code
        printf("**** testing contiguous storage...");
        {
     #define NDIMS6 1
     #define DIM6_NAME "D5"
     #define VAR_NAME6 "V5"
     #define DIM6_LEN 100

           int dimids[NDIMS6], dimids_in[NDIMS6];
           int varid;
           int ndims, nvars, natts, unlimdimid;
           nc_type xtype_in;
           char name_in[NC_MAX_NAME + 1];
           int data[DIM6_LEN], data_in[DIM6_LEN];
           size_t chunksize_in[NDIMS6];
           int storage_in;
           int i, d;

           for (i = 0; i < DIM6_LEN; i++)
              data[i] = i;


           if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
           if (nc_def_dim(ncid, DIM6_NAME, DIM6_LEN, &dimids[0])) ERR;
           if (dimids[0] != 0) ERR;
           if (nc_def_var(ncid, VAR_NAME6, NC_INT, NDIMS6, dimids, &varid)) ERR;
           if (nc_def_var_chunking(ncid, varid, NC_CONTIGUOUS, NULL)) ERR;
           if (nc_put_var_int(ncid, varid, data)) ERR;


           if (nc_inq_var_chunking(ncid, 0, &storage_in, chunksize_in)) ERR;
           if (storage_in != NC_CONTIGUOUS) ERR;
\endcode

*/
int
nc_inq_var_chunking(int ncid, int varid, int *storagep, size_t *chunksizesp)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   TRACE(nc_inq_var_chunking);
   return ncp->dispatch->inq_var_all(ncid, varid, NULL, NULL, NULL, NULL,
				     NULL, NULL, NULL, NULL, NULL, storagep,
				     chunksizesp, NULL, NULL, NULL, NULL, NULL);
}

/** \ingroup variables
Learn the fill mode of a variable.

The fill mode of a variable is set by nc_def_var_fill().

This is a wrapper for NC_inq_var_all().

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param no_fill Pointer to an integer which will get a 1 if no_fill
mode is set for this variable. \ref ignored_if_null.

\param fill_valuep A pointer which will get the fill value for this
variable. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
*/
int
nc_inq_var_fill(int ncid, int varid, int *no_fill, void *fill_valuep)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   TRACE(nc_inq_var_fill);
   return ncp->dispatch->inq_var_all(
      ncid, varid,
      NULL, /*name*/
      NULL, /*xtypep*/
      NULL, /*ndimsp*/
      NULL, /*dimidsp*/
      NULL, /*nattsp*/
      NULL, /*shufflep*/
      NULL, /*deflatep*/
      NULL, /*deflatelevelp*/
      NULL, /*fletcher32p*/
      NULL, /*contiguousp*/
      NULL, /*chunksizep*/
      no_fill, /*nofillp*/
      fill_valuep, /*fillvaluep*/
      NULL, /*endianp*/
      NULL, /*optionsmaskp*/
      NULL /*pixelsp*/
      );
}

/** \ingroup variables
Find the endianness of a variable.

This is a wrapper for NC_inq_var_all().

\param ncid NetCDF or group ID, from a previous call to nc_open(),
nc_create(), nc_def_grp(), or associated inquiry functions such as
nc_inq_ncid().

\param varid Variable ID

\param endianp Storage which will get ::NC_ENDIAN_LITTLE if this
variable is stored in little-endian format, ::NC_ENDIAN_BIG if it is
stored in big-endian format, and ::NC_ENDIAN_NATIVE if the endianness
is not set, and the variable is not created yet.

\returns ::NC_NOERR No error.
\returns ::NC_ENOTNC4 Not a netCDF-4 file.
\returns ::NC_EBADID Bad ncid.
\returns ::NC_ENOTVAR Invalid variable ID.
*/
int
nc_inq_var_endian(int ncid, int varid, int *endianp)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   TRACE(nc_inq_var_endian);
   return ncp->dispatch->inq_var_all(
      ncid, varid,
      NULL, /*name*/
      NULL, /*xtypep*/
      NULL, /*ndimsp*/
      NULL, /*dimidsp*/
      NULL, /*nattsp*/
      NULL, /*shufflep*/
      NULL, /*deflatep*/
      NULL, /*deflatelevelp*/
      NULL, /*fletcher32p*/
      NULL, /*contiguousp*/
      NULL, /*chunksizep*/
      NULL, /*nofillp*/
      NULL, /*fillvaluep*/
      endianp, /*endianp*/
      NULL, /*optionsmaskp*/
      NULL /*pixelsp*/
      );
}

/*! Return number and list of unlimited dimensions.

In netCDF-4 files, it's possible to have multiple unlimited
dimensions. This function returns a list of the unlimited dimension
ids visible in a group.

Dimensions are visible in a group if they have been defined in that
group, or any ancestor group.

\param ncid NetCDF group ID, from a previous call to nc_open, nc_create, nc_def_grp, etc.
\param nunlimdimsp A pointer to an int which will get the number of visible unlimited dimensions. Ignored if NULL.
\param unlimdimidsp A pointer to an already allocated array of int which will get the ids of all visible unlimited dimensions. Ignored if NULL. To allocate the correct length for this array, call nc_inq_unlimdims with a NULL for this parameter and use the nunlimdimsp parameter to get the number of visible unlimited dimensions.

This function will return one of the following values.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad group id.
\returns ::NC_ENOTNC4 Attempting a netCDF-4 operation on a netCDF-3 file. NetCDF-4 operations can only be performed on files defined with a create mode which includes flag HDF5. (see nc_open).
\returns ::NC_ESTRICTNC3 This file was created with the strict netcdf-3 flag, therefore netcdf-4 operations are not allowed. (see nc_open).
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.

 */
int
nc_inq_unlimdims(int ncid, int *nunlimdimsp, int *unlimdimidsp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
   TRACE(nc_inq_unlimdims);
    return ncp->dispatch->inq_unlimdims(ncid, nunlimdimsp,
					unlimdimidsp);
}

#endif /* USE_NETCDF4 */

/*!

Used in libdap2 and libdap4.

@param[in] ncid               ncid for file.
@param[in] varid              varid for variable in question.
@param[out] name              Pointer to memory to contain the name of the variable.
@param[out] xtypep            Pointer to memory to contain the type of the variable.
@param[out] ndimsp            Pointer to memory to store the number of associated dimensions for the variable.
@param[out] dimidsp           Pointer to memory to store the dimids associated with the variable.
@param[out] nattsp            Pointer to memory to store the number of attributes associated with the variable.
@param[out] shufflep          Pointer to memory to store shuffle information associated with the variable.
@param[out] deflatep          Pointer to memory to store compression type associated with the variable.
@param[out] deflate_levelp    Pointer to memory to store compression level associated with the variable.
@param[out] fletcher32p       Pointer to memory to store compression information associated with the variable.
@param[out] contiguousp       Pointer to memory to store contiguous-data information associated with the variable.
@param[out] chunksizesp       Pointer to memory to store chunksize information associated with the variable.
@param[out] no_fill           Pointer to memory to store whether or not there is a fill value associated with the variable.
@param[out] fill_valuep       Pointer to memory to store the fill value (if one exists) for the variable.
@param[out] endiannessp       Pointer to memory to store endianness value. One of ::NC_ENDIAN_BIG ::NC_ENDIAN_LITTLE ::NC_ENDIAN_NATIVE
@param[out] options_maskp     Pointer to memory to store mask options information.
@param[out] pixels_per_blockp Pointer to memory to store pixels-per-block information for chunked data.

\note Expose access to NC_inq_var_all().

\internal
\ingroup variables


*/
int
NC_inq_var_all(int ncid, int varid, char *name, nc_type *xtypep,
               int *ndimsp, int *dimidsp, int *nattsp,
               int *shufflep, int *deflatep, int *deflate_levelp,
               int *fletcher32p, int *contiguousp, size_t *chunksizesp,
               int *no_fill, void *fill_valuep, int *endiannessp,
	       int *options_maskp, int *pixels_per_blockp)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_var_all(
      ncid, varid, name, xtypep,
      ndimsp, dimidsp, nattsp,
      shufflep, deflatep, deflate_levelp, fletcher32p,
      contiguousp, chunksizesp,
      no_fill, fill_valuep,
      endiannessp,
      options_maskp,
      pixels_per_blockp);
}

/*! \} */  /* End of named group ...*/


/**************************************************/
/* Originally in dvarget.c */

/** \internal
\ingroup variables

 */
int
NC_get_vara(int ncid, int varid,
	    const size_t *start, const size_t *edges,
            void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
   if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif

   if(edges == NULL) {
      size_t shape[NC_MAX_VAR_DIMS];
      int ndims;
      stat = nc_inq_varndims(ncid, varid, &ndims);
      if(stat != NC_NOERR) return stat;
      stat = NC_getshape(ncid,varid,ndims,shape);
      if(stat != NC_NOERR) return stat;
      stat = ncp->dispatch->get_vara(ncid,varid,start,shape,value,memtype);
   } else
      stat =  ncp->dispatch->get_vara(ncid,varid,start,edges,value,memtype);
   return stat;
}

/** \ingroup variables
\internal
Called by externally visible nc_get_vars_xxx routines
*/
int
NC_get_vars(int ncid, int varid, const size_t *start,
	    const size_t *edges, const ptrdiff_t *stride, void *value,
	    nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
   if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif
   return ncp->dispatch->get_vars(ncid,varid,start,edges,stride,value,memtype);
}

/** \ingroup variables
\internal
Called by externally visible nc_get_varm_xxx routines
 */
int
NC_get_varm(int ncid, int varid, const size_t *start,
	    const size_t *edges, const ptrdiff_t *stride, const ptrdiff_t* map,
	    void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
   if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif
   return ncp->dispatch->get_varm(ncid,varid,start,edges,stride,map,value,memtype);
}

/**************************************************/
/* Originally in dvarput.c */

/** \internal
\ingroup variables
*/
int
NC_put_vara(int ncid, int varid, const size_t *start,
	    const size_t *edges, const void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   if(edges == NULL) {
      size_t shape[NC_MAX_VAR_DIMS];
      int ndims;
      stat = nc_inq_varndims(ncid, varid, &ndims);
      if(stat != NC_NOERR) return stat;
      stat = NC_getshape(ncid, varid, ndims, shape);
      if(stat != NC_NOERR) return stat;
      return ncp->dispatch->put_vara(ncid, varid, start, shape, value, memtype);
   } else
      return ncp->dispatch->put_vara(ncid, varid, start, edges, value, memtype);
}
/** \internal
\ingroup variables
*/
int
NC_put_vars(int ncid, int varid, const size_t *start,
	    const size_t *edges, const ptrdiff_t *stride,
	    const void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
   if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif
   return ncp->dispatch->put_vars(ncid,varid,start,edges,stride,value,memtype);
}

/** \internal
\ingroup variables
*/
int
NC_put_varm(int ncid, int varid, const size_t *start,
	    const size_t *edges, const ptrdiff_t *stride, const ptrdiff_t* map,
	    const void *value, nc_type memtype)
{
   NC* ncp;
   int stat = NC_check_id(ncid, &ncp);

   if(stat != NC_NOERR) return stat;
#ifdef USE_NETCDF4
   if(memtype >= NC_FIRSTUSERTYPEID) memtype = NC_NAT;
#endif
   return ncp->dispatch->put_varm(ncid,varid,start,edges,stride,map,value,memtype);
}

/**************************************************/
/* Originally in dgroup.c */

/** \defgroup groups Groups

NetCDF-4 added support for hierarchical groups within netCDF datasets.

Groups are identified with a ncid, which identifies both the open
file, and the group within that file. When a file is opened with
nc_open or nc_create, the ncid for the root group of that file is
provided. Using that as a starting point, users can add new groups, or
list and navigate existing groups or rename a group.

All netCDF calls take a ncid which determines where the call will take
its action. For example, the nc_def_var function takes a ncid as its
first parameter. It will create a variable in whichever group its ncid
refers to. Use the root ncid provided by nc_create or nc_open to
create a variable in the root group. Or use nc_def_grp to create a
group and use its ncid to define a variable in the new group.

Variable are only visible in the group in which they are defined. The
same applies to attributes. “Global” attributes are associated with
the group whose ncid is used.

Dimensions are visible in their groups, and all child groups.

Group operations are only permitted on netCDF-4 files - that is, files
created with the HDF5 flag in nc_create(). Groups are not compatible
with the netCDF classic data model, so files created with the
::NC_CLASSIC_MODEL file cannot contain groups (except the root group).

Encoding both the open file id and group id in a single integer
currently limits the number of groups per netCDF-4 file to no more
than 32767.  Similarly, the number of simultaneously open netCDF-4
files in one program context is limited to 32767.

 */

/** \{*/ /* All these functions are part of the above defgroup... */

/*! Return the group ID for a group given the name.


  @param[in] ncid      A valid file or group ncid.
  @param[in] name      The name of the group you are querying.
  @param[out] grp_ncid Pointer to memory to hold the group ncid.

  @returns Error code or ::NC_NOERR or no error.

 */
int
nc_inq_ncid(int ncid, const char *name, int *grp_ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_ncid(ncid,name,grp_ncid);
}

/*! Get a list of groups or subgroups from a file or groupID.

  @param[in]  ncid    The ncid of the file or parent group.
  @param[out] numgrps Pointer to memory to hold the number of groups.
  @param[out] ncids   Pointer to memory to hold the ncid for each group.

  @returns Error code or ::NC_NOERR for no error.

 */
int
nc_inq_grps(int ncid, int *numgrps, int *ncids)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grps(ncid,numgrps,ncids);
}

/*! Get the name of a group given an ID.

  @param[in]  ncid The ncid of the file or parent group.
  @param[out] name The name of the group associated with the id.

  @returns Error code or ::NC_NOERR for no error.
*/
int
nc_inq_grpname(int ncid, char *name)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grpname(ncid,name);
}

/*! Get the full path/groupname of a group/subgroup given an ID.

  @param[in]  ncid      The ncid of the file or parent group.
  @param[out] lenp      Pointer to memory to hold the length of the full name.
  @param[out] full_name Pointer to memory to hold the full name of the group including root/parent.

  @returns Error code or ::NC_NOERR for no error.

*/

int
nc_inq_grpname_full(int ncid, size_t *lenp, char *full_name)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grpname_full(ncid,lenp,full_name);
}

/*! Get the ID of the parent based on a group ID.

  @param[in] ncid         The ncid of the group in question.
  @param[out] parent_ncid Pointer to memory to hold the identifier of the parent of the group in question.

  @returns Error code or ::NC_NOERR for no error.

 */
int nc_inq_grp_parent(int ncid, int *parent_ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grp_parent(ncid,parent_ncid);
}

/*! Get the full ncid given a group name.

  @param[in] ncid      The ncid of the file.
  @param[in] full_name The full name of the group in question.
  @param[out] grp_ncid Pointer to memory to hold the identifier of the full group in question.

  @returns Error code or ::NC_NOERR for no error.

 */
int nc_inq_grp_full_ncid(int ncid, const char *full_name, int *grp_ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_grp_full_ncid(ncid,full_name,grp_ncid);
}


/*! Get a list of varids associated with a group given a group ID.

  @param[in] ncid    The ncid of the group in question.
  @param[out] nvars  Pointer to memory to hold the number of variables in the group in question.
  @param[out] varids Pointer to memory to hold the variable ids contained by the group in question.

  @returns Error code or ::NC_NOERR for no error.

*/
int nc_inq_varids(int ncid, int *nvars, int *varids)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_varids(ncid,nvars,varids);
}

/*! Retrieve a list of dimension ids associated with a group.

  @param[in] ncid    The ncid of the group in question.
  @param[out] ndims  Pointer to memory to contain the number of dimids associated with the group.
  @param[out] dimids Pointer to memory to contain the number of dimensions associated with the group.
  @param[in] include_parents If non-zero, parent groups are also traversed.

  @returns Error code or ::NC_NOERR for no error.

 */
int nc_inq_dimids(int ncid, int *ndims, int *dimids, int include_parents)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_dimids(ncid,ndims,dimids,include_parents);
}

/*! Retrieve a list of types associated with a group

  @param[in] ncid     The ncid for the group in question.
  @param[out] ntypes  Pointer to memory to hold the number of typeids contained by the group in question.
  @param[out] typeids Pointer to memory to hold the typeids contained by the group in question.

  @returns Error code or ::NC_NOERR for no error.

*/

int nc_inq_typeids(int ncid, int *ntypes, int *typeids)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_typeids(ncid,ntypes,typeids);
}

/*! Define a new group.

  The function nc_def_grp() adds a new
  group to an open netCDF dataset in define mode.  It returns (as an
  argument) a group id, given the parent ncid and the name of the group.

  A group may be a top-level group if it is passed the ncid of the file,
  or a sub-group if passed the ncid of an existing group.

  @param[in]  parent_ncid The ncid of the parent for the group.
  @param[in]  name        Name of the new group.
  @param[out] new_ncid    Pointer to memory to hold the new ncid.

  @returns Error code or ::NC_NOERR for no error.

  @retval ::NC_NOERR No error.
  @retval ::NC_ENOTNC4 Not an nc4 file.
  @retval ::NC_ENOTINDEFINE Not in define mode.
  @retval ::NC_ESTRICTNC3 Not permissible in nc4 classic mode.
  @retval ::NC_EPERM Write to read only.
  @retval ::NC_ENOMEM Memory allocation (malloc) failure.
  @retval ::NC_ENAMEINUSE String match to name in use.

  \section nc_def_grp_example Example

  Here is an example using nc_def_grp() to create a new group.

  \code{.c}

  #include <netcdf.h>
  ...
  int status, ncid, grpid, latid, recid;
  ...

  \endcode

*/
int nc_def_grp(int parent_ncid, const char *name, int *new_ncid)
{
    NC* ncp;
    int stat = NC_check_id(parent_ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_grp(parent_ncid,name,new_ncid);
}

/*! Rename a group.

  @param[in] grpid The ID for the group in question.
  @param[in] name  The new name for the group.

  @returns Error code or ::NC_NOERR for no error.

*/
int nc_rename_grp(int grpid, const char *name)
{
    NC* ncp;
    int stat = NC_check_id(grpid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->rename_grp(grpid,name);
}

/*! Print the metadata for a file.

  @param[in] ncid The ncid of an open file.

  @returns Error code or ::NC_NOERR for no error.

 */
int nc_show_metadata(int ncid)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->show_metadata(ncid);
}

/** \} */

/**************************************************/
/* Originally in dtype.c */

/** \defgroup user_types User-Defined Types

User defined types allow for more complex data structures.

NetCDF-4 has added support for four different user defined data
types. User defined type may only be used in files created with the
::NC_NETCDF4 and without ::NC_CLASSIC_MODEL.
- compound type: like a C struct, a compound type is a collection of
types, including other user defined types, in one package.
- variable length array type: used to store ragged arrays.
- opaque type: This type has only a size per element, and no other
  type information.
- enum type: Like an enumeration in C, this type lets you assign text
  values to integer values, and store the integer values.

Users may construct user defined type with the various nc_def_*
functions described in this section. They may learn about user defined
types by using the nc_inq_ functions defined in this section.

Once types are constructed, define variables of the new type with
nc_def_var (see nc_def_var). Write to them with nc_put_var1,
nc_put_var, nc_put_vara, or nc_put_vars. Read data of user-defined
type with nc_get_var1, nc_get_var, nc_get_vara, or nc_get_vars (see
\ref variables).

Create attributes of the new type with nc_put_att (see nc_put_att_
type). Read attributes of the new type with nc_get_att (see
\ref attributes).
*/

/** \{ */


/** \internal
\ingroup user_types
Learn if two types are equal

\param ncid1 \ref ncid of first typeid.
\param typeid1 First typeid.
\param ncid2 \ref ncid of second typeid.
\param typeid2 Second typeid.
\param equal Pointer to int. A non-zero value will be copied here if
the two types are equal, a zero if they are not equal.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_type_equal(int ncid1, nc_type typeid1, int ncid2,
		  nc_type typeid2, int *equal)
{
    NC* ncp1;
    int stat = NC_check_id(ncid1,&ncp1);
    if(stat != NC_NOERR) return stat;
    return ncp1->dispatch->inq_type_equal(ncid1,typeid1,ncid2,typeid2,equal);
}

/** \name Learning about User-Defined Types

    Functions to learn about any kind of user-defined type. */
/*! \{ */ /* All these functions are part of this named group... */

/** \ingroup user_types

Find a type by name. Given a group ID and a type name, find the ID of
the type. If the type is not found in the group, then the parents are
searched. If still not found, the entire file is searched.

\param ncid \ref ncid
\param name \ref object_name of type to search for.
\param typeidp Typeid of named type will be copied here, if it is
found.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_typeid(int ncid, const char *name, nc_type *typeidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_typeid(ncid,name,typeidp);
}

/** \ingroup user_types
Learn about a user defined type.

Given an ncid and a typeid, get the information about a user defined
type. This function will work on any user defined type, whether
compound, opaque, enumeration, or variable length array.

\param ncid \ref ncid

\param xtype The typeid

\param name The \ref object_name will be copied here. \ref
ignored_if_null.

\param size the (in-memory) size of the type in bytes will be copied
here. VLEN type size is the size of nc_vlen_t. String size is returned
as the size of a character pointer. The size may be used to malloc
space for the data, no matter what the type. \ref ignored_if_null.

\param base_nc_typep The base type will be copied here for enum and
VLEN types. \ref ignored_if_null.

\param nfieldsp The number of fields will be copied here for enum and
compound types. \ref ignored_if_null.

\param classp Return the class of the user defined type, ::NC_VLEN,
::NC_OPAQUE, ::NC_ENUM, or ::NC_COMPOUND. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_user_type(int ncid, nc_type xtype, char *name, size_t *size,
		 nc_type *base_nc_typep, size_t *nfieldsp, int *classp)
{
    NC *ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_user_type(ncid, xtype, name, size,
					base_nc_typep, nfieldsp, classp);
}
/*! \} */  /* End of named group ...*/

/** \} */

/**************************************************/
/* Originally in denum.c */

/** \name Enum Types
    Functions to create and learn about enum types. */
/*! \{ */ /* All these functions are part of this named group... */

/** \ingroup user_types
Create an enum type. Provide an ncid, a name, and a base integer type.

After calling this function, fill out the type with repeated calls to
nc_insert_enum(). Call nc_insert_enum() once for each value you wish
to make part of the enumeration.

\param ncid \ref ncid

\param base_typeid The base integer type for this enum. Must be one
of: ::NC_BYTE, ::NC_UBYTE, ::NC_SHORT, ::NC_USHORT, ::NC_INT,
::NC_UINT, ::NC_INT64, ::NC_UINT64.

\param name \ref object_name of new type.

\param typeidp A pointer to an nc_type. The typeid of the new type
will be placed there.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
\returns ::NC_ENAMEINUSE That name is in use.
\returns ::NC_EMAXNAME Name exceeds max length NC_MAX_NAME.
\returns ::NC_EBADNAME Name contains illegal characters.
\returns ::NC_EPERM Attempt to write to a read-only file.
\returns ::NC_ENOTINDEFINE Not in define mode. 
 */
int
nc_def_enum(int ncid, nc_type base_typeid, const char *name, nc_type *typeidp)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_enum(ncid,base_typeid,name,typeidp);
}

/** \ingroup user_types
Insert a named member into a enum type. 

\param ncid \ref ncid
\param xtype
\param name The identifier (\ref object_name) of the new member. 
\param value The value that is to be associated with this member. 

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
\returns ::NC_ENAMEINUSE That name is in use.
\returns ::NC_EMAXNAME Name exceeds max length NC_MAX_NAME.
\returns ::NC_EBADNAME Name contains illegal characters.
\returns ::NC_EPERM Attempt to write to a read-only file.
\returns ::NC_ENOTINDEFINE Not in define mode. 
 */
int
nc_insert_enum(int ncid, nc_type xtype, const char *name, 
	       const void *value)
{
    NC *ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->insert_enum(ncid, xtype, name,
				      value);
}


/** \ingroup user_types
Learn about a about a member of an enum type. 

\param ncid \ref ncid

\param xtype Typeid of the enum type.

\param idx Index to the member to inquire about.

\param name The identifier (\ref object_name) of this member will be
copied here. \ref ignored_if_null.

\param value The value of this member will be copied here. \ref
ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_enum_member(int ncid, nc_type xtype, int idx, char *name, 
		   void *value)
{
    NC *ncp;
    int stat = NC_check_id(ncid, &ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_enum_member(ncid, xtype, idx, name, value);
}

/** \ingroup user_types
Get the name which is associated with an enum member value. 

\param ncid \ref ncid

\param xtype Typeid of the enum type.

\param value Value of interest.

\param identifier The identifier (\ref object_name) of this value will
be copied here. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_enum_ident(int ncid, nc_type xtype, long long value, 
		  char *identifier)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->inq_enum_ident(ncid,xtype,value,identifier);
}
/*! \} */  /* End of named group ...*/

/**************************************************/
/* Originally in dvlen.c */

/** \name Variable Length Array Types

    Functions to create and learn about VLEN types. */
/*! \{ */ /* All these functions are part of this named group... */

/** 
\ingroup user_types
Use this function to define a variable length array type.

\param ncid \ref ncid
\param name \ref object_name of new type.

\param base_typeid The typeid of the base type of the VLEN. For
example, for a VLEN of shorts, the base type is ::NC_SHORT. This can be
a user defined type.

\param xtypep A pointer to an nc_type variable. The typeid of the new
VLEN type will be set here.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
\returns ::NC_ENAMEINUSE That name is in use.
\returns ::NC_EMAXNAME Name exceeds max length NC_MAX_NAME.
\returns ::NC_EBADNAME Name contains illegal characters.
\returns ::NC_EPERM Attempt to write to a read-only file.
\returns ::NC_ENOTINDEFINE Not in define mode. 
 */
int
nc_def_vlen(int ncid, const char *name, nc_type base_typeid, nc_type *xtypep)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_vlen(ncid,name,base_typeid,xtypep);
}


/** \internal
\ingroup user_types

Put a VLEN element. This function writes an element of a VLEN for the
Fortran APIs.

\param ncid \ref ncid
\param typeid1 Typeid of the VLEN.
\param vlen_element Pointer to the element of the VLEN.
\param len Lenth of the VLEN element.
\param data VLEN data.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
\returns ::NC_EPERM Attempt to write to a read-only file.
 */
int
nc_put_vlen_element(int ncid, int typeid1, void *vlen_element, size_t len, const void *data)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->put_vlen_element(ncid,typeid1,vlen_element,len,data);
}

/** 
\internal
\ingroup user_types

Get a VLEN element. This function reads an element of a VLEN for the
Fortran APIs.

\param ncid \ref ncid
\param typeid1 Typeid of the VLEN.
\param vlen_element Pointer to the element of the VLEN.
\param len Lenth of the VLEN element.
\param data VLEN data.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_get_vlen_element(int ncid, int typeid1, const void *vlen_element, 
		    size_t *len, void *data)
{
    NC *ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->get_vlen_element(ncid, typeid1, vlen_element, 
					   len, data);
}

/**************************************************/
/* Originally in dopaque.c */

/** \ingroup user_types
Create an opaque type. Provide a size and a name.

\param ncid \ref ncid
\param size The size of each opaque object in bytes.
\param name \ref object_name of the new type.
\param xtypep Pointer where the new typeid for this type is returned.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
\returns ::NC_ENAMEINUSE That name is in use.
\returns ::NC_EMAXNAME Name exceeds max length NC_MAX_NAME.
\returns ::NC_EBADNAME Name contains illegal characters.
\returns ::NC_EPERM Attempt to write to a read-only file.
\returns ::NC_ENOTINDEFINE Not in define mode. 
 */
int
nc_def_opaque(int ncid, size_t size, const char *name, nc_type *xtypep)
{
    NC* ncp;
    int stat = NC_check_id(ncid,&ncp);
    if(stat != NC_NOERR) return stat;
    return ncp->dispatch->def_opaque(ncid,size,name,xtypep);
}


/*! \} */  /* End of named group ...*/

/**************************************************/
/* Originally from dcompound.c */

/** \name Compound Types
    Functions to create and learn about compound types. */
/*! \{ */ /* All these functions are part of this named group... */

/** \ingroup user_types

Create a compound type. Provide an ncid, a name, and a total size (in
bytes) of one element of the completed compound type.

After calling this function, fill out the type with repeated calls to
nc_insert_compound(). Call nc_insert_compound() once for each field
you wish to insert into the compound type.

\param ncid \ref ncid
\param size The size, in bytes, of the compound type. 
\param name \ref object_name of the created type.
\param typeidp The type ID of the new type is copied here. 

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENAMEINUSE That name is in use.
\returns ::NC_EMAXNAME Name exceeds max length NC_MAX_NAME.
\returns ::NC_EBADNAME Name contains illegal characters.
\returns ::NC_ESTRICTNC3 Attempting a netCDF-4 operation on a netCDF-3 file.
\returns ::NC_ENOTNC4 This file was created with the strict netcdf-3 flag, therefore netcdf-4 operations are not allowed. (see nc_open).
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
\returns ::NC_EPERM Attempt to write to a read-only file.
\returns ::NC_ENOTINDEFINE Not in define mode. 

\section nc_def_compound_example Example

\code
struct s1
{
int i1;
int i2;
};
struct s1 data[DIM_LEN], data_in[DIM_LEN];
 
if (nc_create(FILE_NAME, NC_NETCDF4, &ncid)) ERR;
if (nc_def_compound(ncid, sizeof(struct s1), SVC_REC, &typeid)) ERR;
if (nc_insert_compound(ncid, typeid, BATTLES_WITH_KLINGONS,
HOFFSET(struct s1, i1), NC_INT)) ERR;
if (nc_insert_compound(ncid, typeid, DATES_WITH_ALIENS,
HOFFSET(struct s1, i2), NC_INT)) ERR;
if (nc_def_dim(ncid, STARDATE, DIM_LEN, &dimid)) ERR;
if (nc_def_var(ncid, SERVICE_RECORD, typeid, 1, dimids, &varid)) ERR;
if (nc_put_var(ncid, varid, data)) ERR;
if (nc_close(ncid)) ERR;
\endcode
*/
int
nc_def_compound(int ncid, size_t size, const char *name, 
		nc_type *typeidp)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->def_compound(ncid,size,name,typeidp);
}

/** \ingroup user_types
Insert a named field into a compound type.

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param name The \ref object_name of the new field.

\param offset Offset in byte from the beginning of the compound type
for this field.

\param field_typeid The type of the field to be inserted. 

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENAMEINUSE That name is in use.
\returns ::NC_EMAXNAME Name exceeds max length NC_MAX_NAME.
\returns ::NC_EBADNAME Name contains illegal characters.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
\returns ::NC_EPERM Attempt to write to a read-only file.
\returns ::NC_ENOTINDEFINE Not in define mode. 
*/
int
nc_insert_compound(int ncid, nc_type xtype, const char *name, 
		   size_t offset, nc_type field_typeid)
{
   NC *ncp;
   int stat = NC_check_id(ncid, &ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->insert_compound(ncid, xtype, name,
					 offset, field_typeid);
}

/** \ingroup user_types
Insert a named array field into a compound type.

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param name The \ref object_name of the new field.

\param offset Offset in byte from the beginning of the compound type
for this field.

\param field_typeid The type of the field to be inserted. 

 \param ndims Number of dimensions in array.

 \param dim_sizes Array of dimension sizes.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENAMEINUSE That name is in use.
\returns ::NC_EMAXNAME Name exceeds max length NC_MAX_NAME.
\returns ::NC_EBADNAME Name contains illegal characters.
\returns ::NC_ESTRICTNC3 Attempting a netCDF-4 operation on a netCDF-3 file.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
\returns ::NC_EPERM Attempt to write to a read-only file.
\returns ::NC_ENOTINDEFINE Not in define mode. 
*/
int
nc_insert_array_compound(int ncid, nc_type xtype, const char *name, 
			 size_t offset, nc_type field_typeid, 
			 int ndims, const int *dim_sizes)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->insert_array_compound(ncid,xtype,name,offset,field_typeid,ndims,dim_sizes);
}

/**  \ingroup user_types
Get information about one of the fields of a compound type. 

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param fieldid A zero-based index number specifying a field in the
compound type.

\param name Returned \ref object_name of the field. \ref
ignored_if_null.

\param offsetp A pointer which will get the offset of the field. \ref
ignored_if_null.

\param field_typeidp A pointer which will get the typeid of the
field. \ref ignored_if_null.

\param ndimsp A pointer which will get the number of dimensions of the
field. \ref ignored_if_null.

\param dim_sizesp A pointer which will get the dimension sizes of the
field. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_compound_field(int ncid, nc_type xtype, int fieldid, 
		      char *name, size_t *offsetp, 
		      nc_type *field_typeidp, int *ndimsp, 
		      int *dim_sizesp)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_compound_field(ncid, xtype, fieldid,
					    name, offsetp, field_typeidp,
					    ndimsp, dim_sizesp);
}

/**  \ingroup user_types
Get information about one of the fields of a compound type. 

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param fieldid A zero-based index number specifying a field in the
compound type.

\param name Returned \ref object_name of the field. \ref
ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_compound_fieldname(int ncid, nc_type xtype, int fieldid, 
			  char *name)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_compound_field(ncid, xtype, fieldid,
					    name, NULL, NULL, NULL,
					    NULL);
}

/**  \ingroup user_types
Learn the Index of a Named Field in a Compound Type. Get the index
 * of a field in a compound type from the name.

\param ncid \ref ncid

\param xtype The typeid for this compound type, as returned by
nc_def_compound(), or nc_inq_var().

\param name \ref object_name of the field. \ref ignored_if_null.

\param fieldidp A pointer which will get the index of the named
field. \ref ignored_if_null.

\returns ::NC_NOERR No error.
\returns ::NC_EBADID Bad \ref ncid.
\returns ::NC_EBADTYPE Bad type id.
\returns ::NC_ENOTNC4 Not an netCDF-4 file, or classic model enabled.
\returns ::NC_EHDFERR An error was reported by the HDF5 layer.
 */
int
nc_inq_compound_fieldindex(int ncid, nc_type xtype, const char *name, 
			   int *fieldidp)
{
   NC* ncp;
   int stat = NC_check_id(ncid,&ncp);
   if(stat != NC_NOERR) return stat;
   return ncp->dispatch->inq_compound_fieldindex(ncid,xtype,name,fieldidp);
}
/*! \} */  /* End of named group ...*/

/**************************************************/
/* Originally from dparallel.c */

/* This function will change the parallel access of a variable from
 * independent to collective. */
int
nc_var_par_access(int ncid, int varid, int par_access)
{
    NC* ncp;
    int stat = NC_NOERR;
    if ((stat = NC_check_id(ncid, &ncp)))
       return stat;
#ifndef USE_PARALLEL
    return NC_ENOPAR;
#else
    return ncp->dispatch->var_par_access(ncid,varid,par_access);
#endif
}

/*!
Given an existing file, figure out its format
and return that format value (NC_FORMATX_XXX)
in model arg.
*/
static int
NC_check_file_type(const char *path, int flags, void *parameters,
		   int* model, int* version)
{
   char magic[MAGIC_NUMBER_LEN];
   int status = NC_NOERR;
   int diskless = ((flags & NC_DISKLESS) == NC_DISKLESS);
   int use_parallel = ((flags & NC_MPIIO) == NC_MPIIO);
   int inmemory = (diskless && ((flags & NC_INMEMORY) == NC_INMEMORY));

   *model = 0;

    if(inmemory)  {
	NC_MEM_INFO* meminfo = (NC_MEM_INFO*)parameters;
	if(meminfo == NULL || meminfo->size < MAGIC_NUMBER_LEN)
	    {status = NC_EDISKLESS; goto done;}
	memcpy(magic,meminfo->memory,MAGIC_NUMBER_LEN);
    } else {/* presumably a real file */
       /* Get the 4-byte magic from the beginning of the file. Don't use posix
        * for parallel, use the MPI functions instead. */
#ifdef USE_PARALLEL
	if (use_parallel) {
	    MPI_File fh;
	    MPI_Status mstatus;
	    int retval;
	    MPI_Comm comm = MPI_COMM_WORLD;
	    MPI_Info info = MPI_INFO_NULL;

	    if(parameters != NULL) {
	        comm = ((NC_MPI_INFO*)parameters)->comm;
		info = ((NC_MPI_INFO*)parameters)->info;
	    }
	    if((retval = MPI_File_open(comm,(char*)path,MPI_MODE_RDONLY,info,
				       &fh)) != MPI_SUCCESS)
		{status = NC_EPARINIT; goto done;}
	    if((retval = MPI_File_read(fh, magic, MAGIC_NUMBER_LEN, MPI_CHAR,
				 &mstatus)) != MPI_SUCCESS)
		{status = NC_EPARINIT; goto done;}
	    if((retval = MPI_File_close(&fh)) != MPI_SUCCESS)
		{status = NC_EPARINIT; goto done;}
	} else
#endif /* USE_PARALLEL */
	{
	    FILE *fp;
	    size_t i;
#ifdef HAVE_FILE_LENGTH_I64
          __int64 file_len = 0;
#endif

	    if(path == NULL || strlen(path)==0)
		{status = NC_EINVAL; goto done;}

	    if (!(fp = fopen(path, "r")))
		{status = errno; goto done;}

#ifdef HAVE_SYS_STAT_H
	    /* The file must be at least MAGIC_NUMBER_LEN in size,
	       or otherwise the following fread will exhibit unexpected
  	       behavior. */

        /* Windows and fstat have some issues, this will work around that. */
#ifdef HAVE_FILE_LENGTH_I64
          if((file_len = _filelengthi64(fileno(fp))) < 0) {
            fclose(fp);
            status = errno;
            goto done;
          }


          if(file_len < MAGIC_NUMBER_LEN) {
            fclose(fp);
            status = NC_ENOTNC;
            goto done;
          }
else
	  { int fno = fileno(fp);
	    if(!(fstat(fno,&st) == 0)) {
	        fclose(fp);
	        status = errno;
	        goto done;
	    }
	    if(st.st_size < MAGIC_NUMBER_LEN) {
              fclose(fp);
              status = NC_ENOTNC;
              goto done;
	    }
	  }
#endif //HAVE_FILE_LENGTH_I64

#endif //HAVE_SYS_STAT_H

	    i = fread(magic, MAGIC_NUMBER_LEN, 1, fp);
	    fclose(fp);
	    if(i == 0)
		{status = NC_ENOTNC; goto done;}
	    if(i != 1)
		{status = errno; goto done;}
	}
    } /* !inmemory */

    /* Look at the magic number */
    status = NC_interpret_magic_number(magic,model,version,use_parallel);

done:
   return status;
}

/*!
  Interpret the magic number found in the header of a netCDF file.

  This function interprets the magic number/string contained in the header of a netCDF file and sets the appropriate NC_FORMATX flags.

  @param[in] magic Pointer to a character array with the magic number block.
  @param[out] model Pointer to an integer to hold the corresponding netCDF type.
  @param[out] version Pointer to an integer to hold the corresponding netCDF version.
  @param[in] use_parallel 1 if using parallel, 0 if not.
  @return Returns an error code or 0 on success.

\internal
\ingroup datasets

*/
static int
NC_interpret_magic_number(char* magic, int* model, int* version, int use_parallel)
{
    int status = NC_NOERR;
    /* Look at the magic number */
    /* Ignore the first byte for HDF */
#ifdef USE_NETCDF4
    if(magic[1] == 'H' && magic[2] == 'D' && magic[3] == 'F') {
	*model = NC_FORMATX_NC4;
	*version = 5;
#ifdef USE_HDF4
    } else if(magic[0] == '\016' && magic[1] == '\003'
              && magic[2] == '\023' && magic[3] == '\001') {
	*model = NC_FORMATX_NC4;
	*version = 4;
#endif
    } else
#endif
    if(magic[0] == 'C' && magic[1] == 'D' && magic[2] == 'F') {
        if(magic[3] == '\001') {
            *version = 1; /* netcdf classic version 1 */
	    *model = NC_FORMATX_NC3;
         } else if(magic[3] == '\002') {
            *version = 2; /* netcdf classic version 2 */
	    *model = NC_FORMATX_NC3;
         } else if(magic[3] == '\005') {
            *version = 5; /* cdf5 (including pnetcdf) file */
	    *model = NC_FORMATX_NC3;
	 } else
	    {status = NC_ENOTNC; goto done;}
     } else
        {status = NC_ENOTNC; goto done;}
done:
     return status;
}

