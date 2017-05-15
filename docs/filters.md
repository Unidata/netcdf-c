Filter Support in netCDF-4 (Enhanced) {#compress}
=================================

[TOC]

Introduction {#compress_intro}
==================

The HDF5 library (1.8.11 and later) 
supports a general filter mechanism to apply various
kinds of filters to datasets before reading or writing. 
The netCDF enhanced (aka netCDF-4) library inherits this
capability since it depends on the HDF5 library.

Filters assume that a variable has chunking
defined and each chunk is filtered before
writing and "unfiltered" after reading and
before passing the data to the user.

The most common kind of filter is a compression-decompression
filter, and that is the focus of this document.

HDF5 supports dynamic loading of compression filters using the following
process for reading of compressed data.

1. Assume that we have a dataset with one or more variables that
were compressed using some algorithm. How the dataset was compressed
will be discussed subsequently.

2. Shared libraries or DLLs exist that implement the compress/decompress
algorithm. These libraries have a specific API so that the HDF5 library
can locate, load, and utilize the compressor.
These libraries are expected to installed in a specific
directory.

Enabling A Compression Filter {#Enable}
=============================

In order to compress a variable, the netcdf-c library
must be given three pieces of information:
(1) some unique identifier for the filter to be used,
(2) a vector of parameters for
controlling the action of the compression filter, and
(3) a shared library implementation of the filter.

The meaning of the parameters is, of course,
completely filter dependent and the filter
description [3] needs to be consulted. For
bzip2, for example, a single parameter is provided
representing the compression level.
It is legal to provide a zero-length set of parameters.
Defaults are not provided, so this assumes that
the filter can operate with zero parameters.

The first two pieces of  information can be provided in one of three ways:
using __ncgen__, via an API call, or via command line parameters to __nccopy__.
In any case, remember that filtering also requires setting chunking, so the
variable must also be marked with chunking information.

Using ncgen {#NCGEN}
-------------

In a CDL file, compression of a variable can be specified
by annotating it with the following attribute:

1. ''_Filter'' -- a string containing a comma separated list of
unsigned integers specifying (1) the filter id to apply, and (2)
a vector of unsigned integers representing the
parameters for controlling the operation of the specified filter.

This is a "special" attribute, which means that
it will normally be invisible when using
__ncdump__ unless the -s flag is specified.

Example CDL File (Data elided)
------------------------------
````
netcdf bzip2 {
dimensions:
  dim0 = 4 ; dim1 = 4 ; dim2 = 4 ; dim3 = 4 ;
variables:
  float var(dim0, dim1, dim2, dim3) ;
    var:_Filter = "307,9" ;
    var:_Storage = "chunked" ;
    var:_ChunkSizes = 4, 4, 4, 4 ;
data:
...
}
````

Using The API {#API}
-------------
The include file, __netcdf_filter.h__ defines
an API method for setting the filter to be used
when writing a variable. The relevant signature is
as follows.
````
int nc_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams, const unsigned int* parms);
````
This must be invoked after the variable has been created and before
__nc_enddef__ is invoked.

It is also possible to query a variable to obtain information about
any associated filter using this signature.
````
int nc_inq_var_filter(int ncid, int varid, unsigned int* idp, size_t* nparams, unsigned int* params);

````
The filter id wil be returned in the __idp__ argument (if non-NULL),
the number of parameters in __nparamsp__ and the actual parameters in
__params__.  As is usual with the netcdf API, one is expected to call
this function twice. The first time to get __nparams__ and the
second to get the parameters in client-allocated memory.

Using nccopy {#NCCOPY}
-------------
When copying a netcdf file using __nccopy__ it is possible
to specify filter information for any output variable by
using the "-F" option on the command line; for example:
````
nccopy -F "var,307,9" unfiltered.nc filtered.nc
````
Assume that __unfiltered.nc__ has a chunked but not bzip2 compressed
variable named "var". This command will create that variable in
the __filtered.nc__ output file but using filter with id 307
(i.e. bzip2) and with parameter(s) 9 indicating the compression level.

The "-F" option can be used repeatedly as long as the variable name
part is different. A different filter id and parameters can be
specified for each occurrence.

Dynamic Loading Process {#Process}
==========

The documentation[1,2] for the HDF5 dynamic loading was (at the time
this was written) out-of-date with respect to the actual HDF5 code
(see HDF5PL.c). So, the following discussion is largely derived
from looking at the actual code. This means that it is subject to change.

Plugin directory {#Plugindir}
----------------

The HDF5 loader expects plugins to be in a specified plugin directory.
The default directory is:
   * "/usr/local/hdf5/lib/plugin” for linux/unix operating systems (including Cygwin)
   * “%ALLUSERSPROFILE%\\hdf5\\lib\\plugin” for Windows systems, although the code
     does not appear to explicitly use this path.

The default may be overridden using the environment variable
__HDF5_PLUGIN_PATH__.

Plugin Library Naming {#Pluginlib}
---------------------

Given a plugin directory, HDF5 examines every file in that
directory that conforms to a specified name pattern
as determined by the platform on which the library is being executed.
<table>
<tr halign="center"><th>Platform<th>Basename<th>Extension
<tr halign="left"><td>Linux<td>lib*<td>.so*
<tr halign="left"><td>OSX<td>lib*<td>.dylib*
<tr halign="left"><td>Cygwin<td>cyg*<td>.dll*
<tr halign="left"><td>Windows<td>*<td>.dll
</table>

Plugin Verification {#Pluginverify}
-------------------
For each dynamic library located using the previous patterns,
HDF5 attempts to load the library and attempts to obtain information
from it. Specifically, It looks for two functions with the following
signatures.

1. __H5PL_type_t H5PLget_plugin_type(void)__ --
This function is expected to return the constant value
__H5PL_TYPE_FILTER__ to indicate that this is a filter library.
2. __const void* H5PLget_plugin_info(void)__ --
This function returns a pointer to a table of type __H5Z_class2_t__.
This table contains the necessary information needed to utilize the
filter both for reading and for writing. In particular, it specifies
the filter id implemented by the library and if must match that id
specified for the variable in __nc_def_var_filter__ in order to be used.

If plugin verification fails, then that plugin is ignored and
the search continues for another, matching plugin.

Debugging {#Debug}
-------
Debugging plugins can be very difficult. You will probably
need to use the old printf approach for debugging the filter itself.

One case worth mentioning is when you have a dataset that is
using an unkown filter. For this situation, you need to
identify what filter(s) are used in the dataset. This can
be accomplished using this command.
````
ncdump -s -h <dataset filename>
````
Since ncdump is not being asked to access the data (the -h flag), it
can obtain the filter information without failures. Then it can print
out the filter id and the parameters (the -s flag).

Test Case {#TestCase}
-------
Within the netcdf-c source tree, the directory __netcdf-c/nc_test4/filter_test__ contains
a test case for testing dynamic filter writing and reading
using bzip2. The test case is fragile and is only known to work for Linux
and Cygwin. To avoid spurious failures, it must be explicitly
invoked by entering the directory and issuing the command
````
make clean all check
````

Although it is fragile, this test can  serve as a complete example for building
plugins for other filters.

References {#References}
==========

1. https://support.hdfgroup.org/HDF5/doc/Advanced/DynamicallyLoadedFilters/HDF5DynamicallyLoadedFilters.pdf
2. https://support.hdfgroup.org/HDF5/doc/TechNotes/TechNote-HDF5-CompressionTroubleshooting.pdf
3. https://support.hdfgroup.org/services/filters.html

