Internal Dispatch Table Architecture
============================
<!-- double header is needed to workaround doxygen bug -->

# Internal Dispatch Table Architecture

\tableofcontents

# Introduction {#dispatch_intro}

The netcdf-c library uses an internal dispatch mechanism
as the means for wrapping the netcdf-c API around a wide variety
of underlying storage and stream data formats.
As of last check, the following formats are supported and each
has its own dispatch table.

Warning: some of the listed function signatures may be out of date
and the specific code should be consulted to see the actual parameters.

<table>
<tr><th>Format<td>Directory<th>NC_FORMATX Name
<tr><td>NetCDF-classic<td>libsrc<td>NC_FORMATX_NC3
<tr><td>NetCDF-enhanced<td>libhdf5<td>NC_FORMATX_NC_HDF5
<tr><td>HDF4<td>libhdf4<td>NC_FORMATX_NC_HDF4
<tr><td>PNetCDF<td>libsrcp<td>NC_FORMATX_PNETCDF
<tr><td>DAP2<td>libdap2<td>NC_FORMATX_DAP2
<tr><td>DAP4<td>libdap4<td>NC_FORMATX_DAP4
<tr><td>UDF0<td>N.A.<td>NC_FORMATX_UDF0
<tr><td>UDF1<td>N.A.<td>NC_FORMATX_UDF1
<tr><td>NCZarr<td>libnczarr<td>NC_FORMATX_NCZARR
</table>

Note that UDF0 and UDF1 allow for user-defined dispatch tables to
be implemented.

The idea is that when a user opens or creates a netcdf file, a
specific dispatch table is chosen.  A dispatch table is a struct
containing an entry for (almost) every function in the netcdf-c API.
During execution, netcdf API calls are channeled through that
dispatch table to the appropriate function for implementing that
API call. The functions in the dispatch table are not quite the
same as those defined in *netcdf.h*. For simplicity and
compactness, some netcdf.h API calls are mapped to the same
dispatch table function. In addition to the functions, the first
entry in the table defines the model that this dispatch table
implements. It will be one of the NC_FORMATX_XXX values.
The second entry in the table is the version of the dispatch table.
The rule is that previous entries may not be removed, but new entries
may be added, and adding new entries increases the version number.

The dispatch table represents a distillation of the netcdf API down to
a minimal set of internal operations. The format of the dispatch table
is defined in the file *libdispatch/ncdispatch.h*. Every new dispatch
table must define this minimal set of operations.

# Adding a New Dispatch Table
In order to make this process concrete, let us assume we plan to add
an in-memory implementation of netcdf-3.

## Defining configure.ac flags

Define a *–-enable* flag option for *configure.ac*.  For our
example, we assume the option "--enable-ncm" and the
internal corresponding flag "enable_ncm". If you examine the existing
*configure.ac* and see how, for example, *--enable_dap2* is
defined, then it should be clear how to do it for your code.

## Defining a "name space"

Choose some prefix of characters to identify the new dispatch
system. In effect we are defining a name-space. For our in-memory
system, we will choose "NCM" and "ncm". NCM is used for non-static
procedures to be entered into the dispatch table and ncm for all other
non-static procedures. Note that the chosen prefix should probably start
with "nc" or "NC" in order to avoid name conflicts outside the netcdf-c library.

## Extend include/netcdf.h

Modify the file *include/netcdf.h* to add an NC_FORMATX_XXX flag
by adding a flag for this dispatch format at the appropriate places.
````
  #define NC_FORMATX_NCM  7
````

Add any format specific new error codes.
````
#define NC_ENCM  (?)
````

## Extend include/ncdispatch.h

Modify the file *include/ncdispatch.h* to
add format specific data and initialization functions;
note the use of our NCM namespace.
````
    #ifdef ENABLE_NCM
    extern NC_Dispatch* NCM_dispatch_table;
    extern int NCM_initialize(void);
    #endif
````

## Define the dispatch table functions

Define the functions necessary to fill in the dispatch table. As a
rule, we assume that a new directory is defined, *libsrcm*, say. Within
this directory, we need to define *Makefile.am* and *CMakeLists.txt*.
We also need to define the source files
containing the dispatch table and the functions to be placed in the
dispatch table -– call them *ncmdispatch.c* and *ncmdispatch.h*. Look at
*libsrc/nc3dispatch.[ch]* or *libnczarr/zdispatch.[ch]* for examples.

Similarly, it is best to take existing *Makefile.am* and *CMakeLists.txt*
files (from *libsrcp* for example) and modify them.

## Adding the dispatch code to libnetcdf

Provide for the inclusion of this library in the final libnetcdf
library. This is accomplished by modifying *liblib/Makefile.am* by
adding something like the following.
````
     if ENABLE_NCM
        libnetcdf_la_LIBADD += $(top_builddir)/libsrcm/libnetcdfm.la
     endif
````

## Extend library initialization

Modify the *NC_initialize* function in *liblib/nc_initialize.c* by adding
appropriate references to the NCM dispatch function.
````
     #ifdef ENABLE_NCM
     extern int NCM_initialize(void);
     #endif
     ...
     int NC_initialize(void)
     {
     ...
     #ifdef ENABLE_NCM
         if((stat = NCM_initialize())) return stat;
     #endif
     ...
     }
````

Finalization is handled in an analogous fashion.

## Testing the new dispatch table

Add a directory of tests: *ncm_test*, say. The file *ncm_test/Makefile.am*
will look something like this.
````
     # These files are created by the tests.
     CLEANFILES = ...
     # These are the tests which are always run.
     TESTPROGRAMS = test1 test2 ...
     test1_SOURCES = test1.c ...
     ...
     # Set up the tests.
     check_PROGRAMS = $(TESTPROGRAMS)
     TESTS = $(TESTPROGRAMS)
     # Any extra files required by the tests
     EXTRA_DIST = ...
````

# Top-Level build of the dispatch code

Provide for *libnetcdfm* to be constructed by adding the following to
the top-level *Makefile.am*.

````
     if ENABLE_NCM
     NCM=libsrcm
     NCMTESTDIR=ncm_test
     endif
     ...
     SUBDIRS = ... $(DISPATCHDIR)  $(NCM) ... $(NCMTESTDIR)
````

# Choosing a Dispatch Table

The dispatch table is ultimately chosen by the function
NC_infermodel() in libdispatch/dinfermodel.c. This function is
invoked by the NC_create and the NC_open procedures.  This can
be, unfortunately, a complex process. The detailed operation of 
NC_infermodel() is defined in the companion document in docs/dinternal.md.

In any case, the choice of dispatch table is currently based on the following
pieces of information.

1. The mode argument – this can be used to detect, for example, what kind
of file to create: netcdf-3, netcdf-4, 64-bit netcdf-3, etc.
Using a mode flag is the most common mechanism, in which case
*netcdf.h* needs to be modified to define the relevant mode flag.

2. The file path – this can be used to detect, for example, a DAP url
versus a normal file system file. If the path looks like a URL, then
the fragment part of the URL is examined to determine the specific
dispatch function.

3. The file contents - when the contents of a real file are available,
the contents of the file can be used to determine the dispatch table.
As a rule, this is likely to be useful only for *nc_open*.

4. If the file is being opened vs being created.

5. Is parallel IO available?

The *NC_infermodel* function returns two values.

1. model - this is used by nc_open and nc_create to choose the dispatch table.
2. newpath - in some case, usually URLS, the path may be rewritten to include extra information for use by the dispatch functions.

# Special Dispatch Table Signatures.

The entries in the dispatch table do not necessarily correspond
to the external API. In many cases, multiple related API functions
are merged into a single dispatch table entry.

## Create/Open

The create table entry and the open table entry in the dispatch table
have the following signatures respectively.
````
     int (*create)(const char *path, int cmode,
                size_t initialsz, int basepe, size_t *chunksizehintp,
                int useparallel, void* parameters,
                struct NC_Dispatch* table, NC* ncp);

     int (*open)(const char *path, int mode,
              int basepe, size_t *chunksizehintp,
              int use_parallel, void* parameters,
              struct NC_Dispatch* table, NC* ncp);
````

The key difference is that these are the union of all the possible
create/open signatures from the include/netcdfXXX.h files. Note especially the last
three parameters. The parameters argument is a pointer to arbitrary data
to provide extra info to the dispatcher.
The table argument is included in case the create
function (e.g. *NCM_create_) needs to invoke other dispatch
functions. The very last argument, ncp, is a pointer to an NC
instance. The raw NC instance will have been created by *libdispatch/dfile.c*
and is passed to e.g. open with the expectation that it will be filled in
by the dispatch open function.

## Accessing Data with put_vara() and get_vara()

````
     int (*put_vara)(int ncid, int varid, const size_t *start, const size_t *count,
                          const void *value, nc_type memtype);
````

````
     int (*get_vara)(int ncid, int varid, const size_t *start, const size_t *count,
                     void *value, nc_type memtype);
````

Most of the parameters are similar to the netcdf API parameters. The
last parameter, however, is the type of the data in
memory. Additionally, instead of using an "int islong" parameter, the
memtype will be either ::NC_INT or ::NC_INT64, depending on the value
of sizeof(long). This means that even netcdf-3 code must be prepared
to encounter the ::NC_INT64 type.

## Accessing Attributes with put_attr() and get_attr()

````
     int (*get_att)(int ncid, int varid, const char *name,
                         void *value, nc_type memtype);
````

````
     int (*put_att)(int ncid, int varid, const char *name, nc_type datatype, size_t len,
                    const void *value, nc_type memtype);
````

Again, the key difference is the memtype parameter. As with
put/get_vara, it used ::NC_INT64 to encode the long case.

## Pre-defined Dispatch Functions

It is sometimes not necessary to implement all the functions in the
dispatch table. Some pre-defined functions are available which may be
used in many cases.

## Inquiry Functions

Many of The netCDF inquiry functions operate from an in-memory model of
metadata. Once a file is opened, or a file is created, this
in-memory metadata model is kept up to date. Consequenty the inquiry
functions do not depend on the dispatch layer code. These functions
can be used by all dispatch layers which use the internal netCDF
enhanced data model.

- NC4_inq
- NC4_inq_type
- NC4_inq_dimid
- NC4_inq_dim
- NC4_inq_unlimdim
- NC4_inq_att
- NC4_inq_attid
- NC4_inq_attname
- NC4_get_att
- NC4_inq_varid
- NC4_inq_var_all
- NC4_show_metadata
- NC4_inq_unlimdims
- NC4_inq_ncid
- NC4_inq_grps
- NC4_inq_grpname
- NC4_inq_grpname_full
- NC4_inq_grp_parent
- NC4_inq_grp_full_ncid
- NC4_inq_varids
- NC4_inq_dimids
- NC4_inq_typeids
- NC4_inq_type_equal
- NC4_inq_user_type
- NC4_inq_typeid

## NCDEFAULT get/put Functions

The mapped (varm) get/put functions have been
implemented in terms of the array (vara) functions. So dispatch layers
need only implement the vara functions, and can use the following
functions to get the and varm functions:

- NCDEFAULT_get_varm
- NCDEFAULT_put_varm

For the netcdf-3 format, the strided functions (nc_get/put_vars)
are similarly implemented in terms of the vara functions. So the following
convenience functions are available.

- NCDEFAULT_get_vars
- NCDEFAULT_put_vars

For the netcdf-4 format, the vars functions actually exist, so
the default vars functions are not used.

## Read-Only Functions

Some dispatch layers are read-only (ex. HDF4). Any function which
writes to a file, including nc_create(), needs to return error code
::NC_EPERM. The following read-only functions are available so that
these don't have to be re-implemented in each read-only dispatch layer:

- NC_RO_create
- NC_RO_redef
- NC_RO__enddef
- NC_RO_sync
- NC_RO_set_fill
- NC_RO_def_dim
- NC_RO_rename_dim
- NC_RO_rename_att
- NC_RO_del_att
- NC_RO_put_att
- NC_RO_def_var
- NC_RO_rename_var
- NC_RO_put_vara
- NC_RO_def_var_fill

## Classic NetCDF Only Functions

There are two functions that are only used in the classic code. All
other dispatch layers (except PnetCDF) return error ::NC_ENOTNC3 for
these functions. The following functions are provided for this
purpose:

- NOTNC3_inq_base_pe
- NOTNC3_set_base_pe

# HDF4 Dispatch Layer as a Simple Example

The HDF4 dispatch layer is about the simplest possible dispatch
layer. It is read-only, classic model. It will serve as a nice, simple
example of a dispatch layer.

Note that the HDF4 layer is optional in the netCDF build. Not all
users will have HDF4 installed, and those users will not build with
the HDF4 dispatch layer enabled. For this reason HDF4 code is guarded
as follows.
````
#ifdef USE_HDF4
...
#endif /*USE_HDF4*/
````

Code in libhdf4 is only compiled if HDF4 is
turned on in the build.

### The netcdf.h File

In the main netcdf.h file, we have the following:

````
#define NC_FORMATX_NC_HDF4   (3)
````

### The ncdispatch.h File

In ncdispatch.h we have the following:

````
#ifdef USE_HDF4
extern NC_Dispatch* HDF4_dispatch_table;
extern int HDF4_initialize(void);
extern int HDF4_finalize(void);
#endif
````

### The netcdf_meta.h File

The netcdf_meta.h file allows for easy determination of what features
are in use. For HDF4, It contains the following, set by configure:
````
...
#define NC_HAS_HDF4      0 /*!< HDF4 support. */
...
````

### The hdf4dispatch.h File

The file *hdf4dispatch.h* contains prototypes and
macro definitions used within the HDF4 code in libhdf4. This include
file should not be used anywhere except in libhdf4.

### Initialization Code Changes in liblib Directory

The file *nc_initialize.c* is modified to include the following:
````
#ifdef USE_HDF4
extern int HDF4_initialize(void);
extern int HDF4_finalize(void);
#endif
````

### Changes to libdispatch/dfile.c

In order for a dispatch layer to be used, it must be correctly
determined in functions *NC_open()* or *NC_create()* in *libdispatch/dfile.c*.
HDF4 has a magic number that is detected in
*NC_interpret_magic_number()*, which allows *NC_open* to automatically
detect an HDF4 file.

Once HDF4 is detected, the *model* variable is set to *NC_FORMATX_NC_HDF4*,
and later this is used in a case statement:
````
      case NC_FORMATX_NC_HDF4:
         dispatcher = HDF4_dispatch_table;
         break;
````

This sets the dispatcher to the HDF4 dispatcher, which is defined in
the libhdf4 directory.

### Dispatch Table in libhdf4/hdf4dispatch.c

The file *hdf4dispatch.c* contains the definition of the HDF4 dispatch
table. It looks like this:
````
/* This is the dispatch object that holds pointers to all the
 * functions that make up the HDF4 dispatch interface. */
static NC_Dispatch HDF4_dispatcher = {
NC_FORMATX_NC_HDF4,
NC_DISPATCH_VERSION,
NC_RO_create,
NC_HDF4_open,
NC_RO_redef,
NC_RO__enddef,
NC_RO_sync,
...
NC_NOTNC4_set_var_chunk_cache,
NC_NOTNC4_get_var_chunk_cache,
...
};
````
Note that most functions use some of the predefined dispatch
functions. Functions that start with NC_RO* are read-only, they return
::NC_EPERM. Functions that start with NOTNC4* return ::NC_ENOTNC4.

Only the functions that start with NC_HDF4* need to be implemented for
the HDF4 dispatch layer. There are 6 such functions:

- NC_HDF4_open
- NC_HDF4_abort
- NC_HDF4_close
- NC_HDF4_inq_format
- NC_HDF4_inq_format_extended
- NC_HDF4_get_vara

### HDF4 Reading Code

The code in *hdf4file.c* opens the HDF4 SD dataset, and reads the
metadata. This metadata is stored in the netCDF internal metadata
model, allowing the inq functions to work.

The code in *hdf4var.c* does an *nc_get_vara()* on the HDF4 SD
dataset. This is all that is needed for all the nc_get_* functions to
work.

# Appendix A. Changing NC_DISPATCH_VERSION

When new entries are added to the *struct NC_Dispatch* type `located in include/netcdf_dispatch.h.in` it is necessary to do two things.

1. Bump the NC_DISPATCH_VERSION number
2. Modify the existing dispatch tables to include the new entries.
It if often the case that the new entries do not mean anything for
a given dispatch table. In that case, the new entries may be set to
some variant of *NC_RO_XXX* or *NC_NOTNC4_XXX* *NC_NOTNC3_XXX*.

Modifying the dispatch version requires two steps:
1. Modify the version number in *netcdf-c/configure.ac*, and
2. Modify the version number in *netcdf-c/CMakeLists.txt*.

The two should agree in value.

### NC_DISPATCH_VERSION Incompatibility

When dynamically adding a dispatch table
-- in nc_def_user_format (see libdispatch/dfile.c) --
the version of the new table is compared with that of the built-in
NC_DISPATCH_VERSION; if they differ, then an error is returned from
that function.


# Point of Contact {#dispatch_poc}

*Author*: Dennis Heimbigner<br>
*Email*: dmh at ucar dot edu<br>
*Initial Version*: 12/22/2021<br>
*Last Revised*: 12/22/2021
