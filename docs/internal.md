Notes On the Internals of the NetCDF-C Library
============================
<!-- double header is needed to workaround doxygen bug -->

# Notes On the Internals of the NetCDF-C Library {#intern_head}

\tableofcontents

This document attempts to record important information about
the internal architecture and operation of the netcdf-c library.

# 1. Including C++ Code in the netcdf-c Library {#intern_c++}

The state of C compiler technology has reached the point where
it is possible to include C++ code into the netcdf-c library
code base. Two examples are:

1. The AWS S3 SDK wrapper *libdispatch/ncs3sdk.cpp* file.
2. The TinyXML wrapper *ncxml\_tinyxml2.cpp* file.

However there are some consequences that must be handled for this to work.
Specifically, the compiler must be told that the C++ runtime is needed
in the following ways.

## Modifications to *lib\_flags.am*
Suppose we have a flag *ENABLE\_XXX* where that XXX
feature entails using C++ code. Then the following must be added
to *lib\_flags.am*
````
if ENABLE_XXX
AM_LDFLAGS += -lstdc++
endif
````

## Modifications to *libxxx/Makefile.am*

The Makefile in which the C++ code is included and compiled
(assumed here to be the *libxxx* directory) must have this set.
````
AM_CXXFLAGS = -std=c++11
````
It is possible that other values (e.g. *-std=c++14*) may also work.

# 2. Managing instances of complex data types

For a long time, there have been known problems with the
management of complex types containing VLENs.  This also
involves the string type because it is stored as a VLEN of
chars.

The term complex type refers to any type that directly or
recursively references a VLEN type. So an array of VLENS, a
compound with a VLEN field, and so on.

In order to properly handle instances of these complex types, it
is necessary to have function that can recursively walk
instances of such types to perform various actions on them.  The
term "deep" is also used to mean recursive.

Two deep walking operations are provided by the netcdf-c library
to aid in managing instances of complex structures.
- free'ing an instance of the complex type
- copying an instance of the complex type.

Previously The netcdf-c library only did shallow free and shallow copy of
complex types. This meant that only the top level was properly
free'd or copied, but deep internal blocks in the instance were
not touched. This led to a host of memory leaks and failures
when the deep data was effectively shared between the netcdf-c library
internally and the user's data. 

Note that the term "vector" is used to mean a contiguous (in
memory) sequence of instances of some type. Given an array with,
say, dimensions 2 X 3 X 4, this will be stored in memory as a
vector of length 2\*3\*4=24 instances.

The use cases are primarily these.

## nc\_get\_vars
Suppose one is reading a vector of instances using nc\_get\_vars
(or nc\_get\_vara or nc\_get\_var, etc.).  These functions will
return the vector in the top-level memory provided.  All
interior blocks (form nested VLEN or strings) will have been
dynamically allocated. Note that computing the size of the vector 
may be tricky because the strides must be taken into account.

After using this vector of instances, it is necessary to free
(aka reclaim) the dynamically allocated memory, otherwise a
memory leak occurs.  So, the recursive reclaim function is used
to walk the returned instance vector and do a deep reclaim of
the data.

Currently functions are defined in netcdf.h that are supposed to
handle this: nc\_free\_vlen(), nc\_free\_vlens(), and
nc\_free\_string().  Unfortunately, these functions only do a
shallow free, so deeply nested instances are not properly
handled by them. They are marked in the description as
deprecated in favor of the newer recursive function.

## nc\_put\_vars

Suppose one is writing a vector of instances using nc\_put\_vars
(or nc\_put\_vara or nc\_put\_var, etc.).  These functions will
write the contents of the vector to the specified variable.
Note that internally, the data passed to the nc\_put\_xxx function is
immediately written so there is no need to copy it internally. But the
caller may need to reclaim the vector of data that was created and passed
in to the nc\_put\_xxx function.

After writing this vector of instances, and assuming it was dynamically
created, at some point it will be necessary to reclaim that data.
So again, the recursive reclaim function can be used
to walk the returned instance vector and do a deep reclaim of
the data.

## nc\_put\_att
Suppose one is writing a vector of instances as the data of an attribute
using, say, nc\_put\_att.

Internally, the incoming attribute data must be copied and stored
so that changes/reclamation of the input data will not affect
the attribute. Note that this copying behavior is different from
writing to a variable, where the data is written immediately.

Again, the code inside the netcdf library used to use only shallow copying
rather than deep copy. As a result, one saw effects such as described
in Github Issue https://github.com/Unidata/netcdf-c/issues/2143.

Also, after defining the attribute, it may be necessary for the user
to free the data that was provided as input to nc\_put\_att() as in the
nc\_put\_xxx functions (previously described).

## nc\_get\_att
Suppose one is reading a vector of instances as the data of an attribute
using, say, nc\_get\_att.

Internally, the existing attribute data must be copied and returned
to the caller, and the caller is responsible for reclaiming
the returned data.

Again, the code inside the netcdf library used to only do shallow copying
rather than deep copy. So this could lead to memory leaks and errors
because the deep data was shared between the library and the user.

## New Instance Walking API

Proper recursive functions were added to the netcdf-c library to
provide reclaim and copy functions and use those as needed.
These functions are defined in libdispatch/dinstance.c and their
signatures are defined in include/netcdf.h. For back
compatibility, corresponding "ncaux\_XXX" functions are defined
in include/netcdf\_aux.h.
````
int nc_reclaim_data(int ncid, nc_type xtypeid, void* memory, size_t count);
int nc_reclaim_data_all(int ncid, nc_type xtypeid, void* memory, size_t count);
int nc_copy_data(int ncid, nc_type xtypeid, const void* memory, size_t count, void* copy);
int nc_copy_data_all(int ncid, nc_type xtypeid, const void* memory, size_t count, void** copyp);
````
There are two variants. The first two, nc\_reclaim\_data() and
nc\_copy\_data(), assume the top-level vector is managed by the
caller. For reclaim, this is so the user can use, for example, a
statically allocated vector. For copy, it assumes the user
provides the space into which the copy is stored.

The second two, nc\_reclaim\_data\_all() and
nc\_copy\_data\_all(), allows the functions to manage the
top-level.  So for nc\_reclaim\_data\_all, the top level is
assumed to be dynamically allocated and will be free'd by
nc\_reclaim\_data\_all().  The nc\_copy\_data\_all() function
will allocate the top level and return a pointer to it to the
user. The user can later pass that pointer to
nc\_reclaim\_data\_all() to reclaim the instance(s).

# Internal Changes
The netcdf-c library internals are changed to use the proper reclaim
and copy functions. This also allows some simplification of the code
since the stdata and vldata fields of NC\_ATT\_INFO are no longer needed.
Currently this is commented out using the SEPDATA \#define macro.
When the bugs are found and fixed, all this code will be removed.

## Optimizations

In order to make these functions as efficient as possible, it is
desirable to classify all types as to whether or not they contain
variable-size data. If a type is fixed sized (i.e. does not contain
variable-size data) then it can be freed or copied as a single chunk.
This significantly increases the performance for such types.
For variable-size types, it is necessary to walk each instance of the type
and recursively reclaim or copy it. As another optimization,
if the type is a vector of strings, then the per-instance walk can be
sped up by doing the reclaim or copy inline.

The rules for classifying types as fixed or variable size are as follows.

1. All atomic types, except string, are fixed size.
2. All enum type and opaque types are fixed size.
3. All string types and VLEN types are variable size.
4. A compound type is fixed size if all of the types of its
   fields are fixed size. Otherwise it has variable size.

The classification of types can be made at the time the type is defined
or is read in from some existing file. The reclaim and copy functions
use this information to speed up the handling of fixed size types.

# Warnings

1. The new API functions require that the type information be
   accessible. This means that you cannot use these functions
   after the file has been closed. After the file is closed, you
   are on your own.

2. There is still one known failure that has not been solved; it is
   possibly an HDF5 memory leak. All the failures revolve around
   some variant of this .cdl file. The proximate cause of failure is
   the use of a VLEN FillValue.
````
        netcdf x {
        types:
          float(*) row_of_floats ;
        dimensions:
          m = 5 ;
        variables:
          row_of_floats ragged_array(m) ;
              row_of_floats ragged_array:_FillValue = {-999} ;
        data:
          ragged_array = {10, 11, 12, 13, 14}, {20, 21, 22, 23}, {30, 31, 32}, 
                         {40, 41}, _ ;
        }
````

# 3. Inferring File Types

As described in the companion document -- docs/dispatch.md --
when nc\_create() or nc\_open() is called, it must figure out what
kind of file is being created or opened.  Once it has figured out
the file kind, the appropriate "dispatch table" can be used
to process that file.

## The Role of URLs

Figuring out the kind of file is referred to as model inference
and is, unfortunately, a complicated process. The complication
is mostly a result of allowing a path argument to be a URL.
Inferring the file kind from a URL requires deep processing of
the URL structure: the protocol, the host, the path, and the fragment
parts in particular. The query part is currently not used because
it usually contains information to be processed by the server
receiving the URL.

The "fragment" part of the URL may be unfamiliar.
The last part of a URL may optionally contain a fragment, which
is syntactically of this form in this pseudo URL specification.
````
<protocol>://<host>/<path>?<query>#<fragment>
````
The form of the fragment is similar to a query and takes this general form.
````
'#'<key>=<value>&<key>=<value>&...
````
The key is a simple name, the value is any sequence of characters,
although URL special characters such as '&' must be URL encoded in
the '%XX' form where each X is a hexadecimal digit.
An example might look like this non-sensical example:
````
https://host.com/path#mode=nczarr,s3&bytes
````
It is important to note that the fragment part is not intended to be
passed to the server, but rather is processed by the client program.
It is this property that allows the netcdf-c library to use it to
pass information deep into the dispatch table code that is processing the
URL.

## Model Inference Inputs

The inference algorithm is given the following information
from which it must determine the kind of file being accessed.

### Mode

The mode is a set of flags that are passed as the second
argument to nc\_create and nc\_open. The set of flags is define in
the netcdf.h header file. Generally it specifies the general
format of the file: netcdf-3 (classic) or netcdf-4 (enhanced).
Variants of these can also be specified, e.g. 64-bit netcdf-3 or
classic netcdf-4.
In the case where the path argument is a simple file path, 
using a mode flag is the most common mechanism for specifying
the model.

### Path
The file path, the first argument to nc\_create and nc\_open,
Can be either a simple file path or a URL.
If it is a URL, then it will be deeply inspected to determine
the model.

### File Contents
When the contents of a real file are available,
the contents of the file can be used to determine the dispatch table.
As a rule, this is likely to be useful only for *nc\_open*.
It also requires access to functions that can open and read at least
the initial part of the file.
As a rule, the initial small prefix of the file is read
and examined to see if it matches any of the so-called
"magic numbers" that indicate the kind of file being read.

### Open vs Create
Is the file being opened or is it being created?

### Parallelism
Is parallel IO available?

## Model Inference Outputs
The inferencing algorithm outputs two pieces of information.

1. model - this is used by nc\_open and nc\_create to choose the dispatch table.
2. newpath - in some case, usually URLS, the path may be rewritten to include extra information for use by the dispatch functions.

The model output is actually a struct containing two fields:

1. implementation - this is a value from the NC\_FORMATX\_xxx
   values in netcdf.h. It generally determines the dispatch
   table to use.
2. format -- this is an NC\_FORMAT\_xxx value defining, in effect,
   the netcdf-format to which the underlying format is to be
   translated. Thus it can tell the netcdf-3 dispatcher that it
   should actually implement CDF5 rather than standard netcdf classic.

## The Inference Algorithm

The construction of the model is primarily carried out by the function
*NC\_infermodel()* (in *libdispatch/dinfermodel.c*).
It is given the following parameters:
1. path -- (IN) absolute file path or URL
2. modep -- (IN/OUT) the set of mode flags given to *NC\_open* or *NC\_create*.
3. iscreate -- (IN) distinguish open from create.
4. useparallel -- (IN) indicate if parallel IO can be used.
5. params -- (IN/OUT) arbitrary data dependent on the mode and path.
6. model -- (IN/OUT) place to store inferred model.
7. newpathp -- (OUT) the canonical rewrite of the path argument.

As a rule, these values are used in the this order of preference
to infer the model.

1. file contents -- highest precedence
2. url (if it is one) -- using the "mode=" key in the fragment (see below).
3. mode flags
4. default format -- lowest precedence
 
The sequence of steps is as follows.

### URL processing -- processuri()

If the path appears to be a URL, then it is parsed
and processed by the processuri function as follows.

1. Protocol --
The protocol is extracted and tested against the list of
legal protocols. If not found, then it is an error.
If found, then it is replaced by a substitute -- if specified.
So, for example, the protocol "dods" is replaced the protocol "http"
(note that at some point "http" will be replaced with "https").
Additionally, one or more "key=value" strings is appended
to the existing fragment of the url. So, again for "dods",
the fragment is extended by the string "mode=dap2".
Thus replacing "dods" does not lose information, but rather transfers
it to the fragment for later use. 

2. Fragment --
After the protocol is processed, the initial fragment processing occurs
by converting it to a list data structure of the form
````
        {<key>,<value>,<key>,<value>,<key>,<value>....}
````

### Macro Processing -- processmacros()

If the fragment list produced by processuri() is non-empty, then
it is processed for "macros". Notice that if the original path
was not a URL, then the fragment list is empty and this
processing will be bypassed.  In any case, It is convenient to
allow some singleton fragment keys to be expanded into larger
fragment components. In effect, those singletons act as
macros. They can help to simplify the user's URL. The term
singleton means a fragment key with no associated value:
"#bytes", for example.

The list of fragments is searched looking for keys whose
value part is NULL or the empty string. Then the table
of macros is searched for that key and if found, then
a key and values is appended to the fragment list and the singleton
is removed.

### Mode Inference -- processinferences()

This function just processes the list of values associated
with the "mode" key. It is similar to a macro in that
certain mode values are added or removed based on tables
of "inferences" and "negations".
Again, the purpose is to allow users to provide simplified URL fragments.

The list of mode values is repeatedly searched and whenever a value
is found that is in the "modeinferences" table, then the associated inference value
is appended to the list of mode values. This process stops when no changes
occur. This form of inference allows the user to specify "mode=zarr"
and have it converted to "mode=nczarr,zarr". This avoids the need for the
dispatch table code to do the same inference.

After the inferences are made, The list of mode values is again
repeatedly searched and whenever a value
is found that is in the "modenegations" table, then the associated negation value
is removed from the list of mode values, assuming it is there. This process stops when no changes
occur. This form of inference allows the user to make sure that "mode=bytes,nczarr"
has the bytes mode take precedence by removing the "nczarr" value. Such illegal
combinations can occur because of previous processing steps.

### Fragment List Normalization
As the fragment list is processed, duplicates appear with the same key.
A function -- cleanfragments() -- is applied to clean up the fragment list
by coalesing the values of duplicate keys and removing duplicate key values.

### S3 Rebuild
If the URL is determined to be a reference to a resource on the Amazon S3 cloud,
then the URL needs to be converted to what is called "path format".
There are four S3 URL formats:

1. Virtual -- ````https://<bucket>.s3.<region>.amazonaws.com/<path>````
2. Path -- ````https://s3.<region>.amazonaws.com/<bucket>/<path>````
3. S3 -- ````s3://<bucket>/<path>````
4. Other -- ````https://<host>/<bucket>/<path>````

The S3 processing converts all of these to the Path format. In the "S3" format case
it is necessary to find or default the region from examining the ".aws" directory files.

### File Rebuild
If the URL protocol is "file" and its path is a relative file path,
then it is made absolute by prepending the path of the current working directory.

In any case, after S3 or File rebuilds, the URL is completely
rebuilt using any modified protocol, host, path, and
fragments. The query is left unchanged in the current algorithm.
The resulting rebuilt URL is passed back to the caller.

### Mode Key Processing
The set of values of the fragment's "mode" key are processed one by one
to see if it is possible to determine the model.
There is a table for format interpretations that maps a mode value
to the model's implementation and format. So for example,
if the mode value "dap2" is encountered, then the model
implementation is set to NC\_FORMATX\_DAP2 and the format
is set to NC\_FORMAT\_CLASSIC.

### Non-Mode Key Processing
If processing the mode does not tell us the implementation, then
all other fragment keys are processed to see if the implementaton
(and format) can be deduced. Currently this does nothing.

### URL Defaults
If the model is still not determined and the path is a URL, then
the implementation is defaulted to DAP2. This is for back
compatibility when all URLS implied DAP2.

### Mode Flags
In the event that the path is not a URL, then it is necessary
to use the mode flags and the isparallel arguments to choose a model.
This is just a straight forward flag checking exercise.

### Content Inference -- check\_file\_type()
If the path is being opened (as opposed to created), then
it may be possible to actually read the first few bytes of the
resource specified by the path and use that to determine the
model. If this succeeds, then it takes precedence over
all other model inferences.

### Flag Consistency
Once the model is known, then the set of mode flags
is modified to be consistent with that information.
So for example, if DAP2 is the model, then all netcdf-4 mode flags 
and some netcdf-3 flags are removed from the set of mode flags
because DAP2 provides only a standard netcdf-classic format.

# 4. Adding a Standard Filter

The standard filter system extends the netcdf-c library API to
support a fixed set of "standard" filters. This is similar to the
way that deflate and szip are currently supported.
For background, the file filter.md should be consulted.

In general, the API for a standard filter has the following prototypes.
The case of zstandard (libzstd) is used as an example.
````
int nc_def_var_zstandard(int ncid, int varid, int level);
int nc_inq_var_zstandard(int ncid, int varid, int* has_filterp, int* levelp);
````
So generally the API has the ncid and the varid as fixed, and then
a list of parameters specific to the filter -- level in this case.
For the inquire function, there is an additional argument -- has\_filterp --
that is set to 1 if the filter is defined for the given variable
and is 0 if not.
The remainder of the inquiry parameters are pointers to memory
into which the parameters are stored -- levelp in this case.

It is important to note that including a standard filter still
requires three supporting objects:

1. The implementing library for the filter. For example,
   libzstd must be installed in order to use the zstandard
   API.
2. A HDF5 wrapper for the filter must be installed in the
   directory pointed to by the HDF5\_PLUGIN\_PATH environment
   variable.
3. (Optional) An NCZarr Codec implementation must be installed
   in the the HDF5\_PLUGIN\_PATH directory.

## Adding a New Standard Filter

The implementation of a standard filter must be loaded from one
of several locations.

1. It can be part of libnetcdf.so (preferred),
2. it can be loaded as part of the client code,
3. or it can be loaded as part of an external library such as libccr.

However, the three objects listed above need to be 
stored in the HDF5\_PLUGIN\_PATH directory, so adding a standard
filter still requires modification to the netcdf build system.
This limitation may be lifted in the future.

### Build Changes
In order to detect a standard library, the following changes
must be made for Automake (configure.ac/Makefile.am)
and CMake (CMakeLists.txt)

#### Configure.ac
Configure.ac must have a block that similar to this that locates
the implementing library.
````
# See if we have libzstd
AC_CHECK_LIB([zstd],[ZSTD_compress],[have_zstd=yes],[have_zstd=no])
if test "x$have_zstd" = "xyes" ; then
   AC_SEARCH_LIBS([ZSTD_compress],[zstd zstd.dll cygzstd.dll], [], [])
   AC_DEFINE([HAVE_ZSTD], [1], [if true, zstd library is available])
fi
AC_MSG_CHECKING([whether libzstd library is available])
AC_MSG_RESULT([${have_zstd}])
````
Note the the entry point (*ZSTD\_compress*) is library dependent
and is used to see if the library is available.

#### Makefile.am

It is assumed you have an HDF5 wrapper for zstd. If you want it
to be built as part of the netcdf-c library then you need to
add the following to *netcdf-c/plugins/Makefile.am*.
````
if HAVE_ZSTD
noinst_LTLIBRARIES += libh5zstd.la
libh5szip_la_SOURCES = H5Zzstd.c H5Zzstd.h
endif
````

````
# Need our version of szip if libsz available and we are not using HDF5
if HAVE_SZ
noinst_LTLIBRARIES += libh5szip.la
libh5szip_la_SOURCES = H5Zszip.c H5Zszip.h
endif
````
#### CMakeLists.txt
In an analog to *configure.ac*, a block like
this needs to be in *netcdf-c/CMakeLists.txt*.
````
FIND_PACKAGE(Zstd)
set_std_filter(Zstd)
````
The FIND\_PACKAGE requires a CMake module for the filter
in the cmake/modules directory.
The *set\_std\_filter* function is a macro.

An entry in the file config.h.cmake.in will also be needed.
````
/* Define to 1 if zstd library available. */
#cmakedefine HAVE_ZSTD 1
````

### Implementation Template
As a template, here is the implementation for zstandard.
It can be used as the template for adding other standard filters.
It is currently located in *netcdf-d/libdispatch/dfilter.c*, but
could be anywhere as indicated above.
````
#ifdef HAVE_ZSTD
int
nc_def_var_zstandard(int ncid, int varid, int level)
{
    int stat = NC_NOERR;
    unsigned ulevel;
    
    if((stat = nc_inq_filter_avail(ncid,H5Z_FILTER_ZSTD))) goto done;
    /* Filter is available */
    /* Level must be between -131072 and 22 on Zstandard v. 1.4.5 (~202009)
       Earlier versions have fewer levels (especially fewer negative levels) */
    if (level < -131072 || level > 22)
        return NC_EINVAL;
    ulevel = (unsigned) level; /* Keep bit pattern */
    if((stat = nc_def_var_filter(ncid,varid,H5Z_FILTER_ZSTD,1,&ulevel))) goto done;
done:
    return stat;
}

int
nc_inq_var_zstandard(int ncid, int varid, int* hasfilterp, int *levelp)
{
    int stat = NC_NOERR;
    size_t nparams;
    unsigned params = 0;
    int hasfilter = 0;
    
    if((stat = nc_inq_filter_avail(ncid,H5Z_FILTER_ZSTD))) goto done;
    /* Filter is available */
    /* Get filter info */
    stat = nc_inq_var_filter_info(ncid,varid,H5Z_FILTER_ZSTD,&nparams,NULL);
    if(stat == NC_ENOFILTER) {stat = NC_NOERR; hasfilter = 0; goto done;}
    if(stat != NC_NOERR) goto done;
    hasfilter = 1;
    if(nparams != 1) {stat = NC_EFILTER; goto done;}
    if((stat = nc_inq_var_filter_info(ncid,varid,H5Z_FILTER_ZSTD,&nparams,&params))) goto done;
done:
    if(levelp) *levelp = (int)params;
    if(hasfilterp) *hasfilterp = hasfilter;
    return stat;
}
#endif /*HAVE_ZSTD*/
````

# Point of Contact {#intern_poc}

*Author*: Dennis Heimbigner<br>
*Email*: dmh at ucar dot edu<br>
*Initial Version*: 12/22/2021<br>
*Last Revised*: 01/25/2022
