NetCDF-4 Filter Support
============================
<!-- double header is needed to workaround doxygen bug -->

NetCDF-4 Filter Support {#filters}
============================

[TOC]

The netCDF library supports a general filter mechanism to apply various
kinds of filters to datasets before reading or writing.

The netCDF enhanced (aka netCDF-4) library inherits this
capability since it depends on the HDF5 library.
The HDF5 library (1.8.11 and later) supports filters,
and netCDF is based closely on that underlying HDF5 mechanism.

Filters assume that a variable has chunking
defined and each chunk is filtered before
writing and "unfiltered" after reading and
before passing the data to the user.

In the event that multiple filters are defined on a variable,
they are applied in first-defined order on writing
and on the reverse order when reading.

The most common kind of filter is a compression-decompression
filter, and that is the focus of this document.

For now, this document is strongly influenced by the HDF5 mechanism.
When other implementations (e.g. Zarr) support filters, this document
will have multiple sections: one for each mechanism.

# Enabling A HDF5 Compression Filter {#filters_enable}

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

In order to compress a variable with an HDF5 compliant filter,
the netcdf-c library
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

Filter ids are assigned by the HDF group. See [4] for a current
list of assigned filter ids.  Note that ids above 32767 can be
used for testing without registration.

The first two pieces of  information can be provided in one of three ways:
using __ncgen__, via an API call, or via command line parameters to __nccopy__.
In any case, remember that filtering also requires setting chunking, so the
variable must also be marked with chunking information. If compression
is set for a non-chunked variable, the variable will forcibly be
converted to chunked using a default chunking algorithm.

## Using The API {#filters_API}
The necessary API methods are included in __netcdf_filter.h__ by default.
These functions implicitly use the HDF5 mechanisms and may produce an
error if applied to a file format that is not compatible with the
HDF5 mechanism.

1. Add a filter to the set of filters to be used when writing a variable.
````
int nc_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams, const unsigned int* parms);
````
This must be invoked after the variable has been created and before
__nc_enddef__ is invoked.

2. Query a variable to obtain information about the
first filter associated with the variable.
````
int nc_inq_var_filter(int ncid, int varid, unsigned int* idp, size_t* nparams, unsigned int* params);
````
The filter id will be returned in the __idp__ argument (if non-NULL),
the number of parameters in __nparamsp__ and the actual parameters in
__params__.  As is usual with the netcdf API, one is expected to call
this function twice. The first time to get __nparams__ and the
second to get the parameters in client-allocated memory.

3. Query a variable to obtain a list of all filters associated with a variable.
````
int nc_inq_var_filterids(int ncid, int varid, int format, size_t* nfilters, unsigned int* filterids);
````
The number of filters is determined by __nfilters__.
The set of filter ids will be returned in __filterids__. 
As is usual with the netcdf API, one is expected to call
this function twice. The first time to get __nbytes__ and the
second to get the filter ids in client-allocated memory.

4. Query a variable to obtain information about a specific
filter associated with the variable.
````
int nc_inq_var_filter_info(int ncid, int varid, unsigned int id,  size_t* nparamsp, unsigned int* params);
````
The __id__ filter id indicates the filter of interest.
The actual parameters are in __params__.
The number of parameters is returned in nparamsp.
As is usual with the netcdf API, one is expected to call
this function twice. The first time to get __nparams__ and the
second to get the parameters in client-allocated memory.

## Using ncgen {#filters_NCGEN}

In a CDL file, compression of a variable can be specified
by annotating it with the following attribute:

* ''_Filter'' -- a string containing a comma separated list of
constants specifying (1) the filter id to apply, and (2)
a vector of constants representing the
parameters for controlling the operation of the specified filter.
See the section on the <a href="#filters_syntax">parameter encoding syntax</a>
for the details on the allowable kinds of constants.

This is a "special" attribute, which means that
it will normally be invisible when using
__ncdump__ unless the -s flag is specified.

This attribute may be repeated to specify multiple filters.
As a rule, use the ___Deflate__ attribute instead of ___Filter__
(but using ___Filter__ to specify deflation will work).

### Example CDL File (Data elided)

````
netcdf bzip2szip {
dimensions:
  dim0 = 4 ; dim1 = 4 ; dim2 = 4 ; dim3 = 4 ;
variables:
  float var(dim0, dim1, dim2, dim3) ;
    var:_Filter = "307,9|szip,32,32" ; // bzip2+szip
    var:_Storage = "chunked" ;
    var:_ChunkSizes = 4, 4, 4, 4 ;
data:
...
}
````

## Using nccopy {#filters_NCCOPY}

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
See the section on the <a href="#filters_syntax">parameter encoding syntax</a>
for the details on the allowable kinds of constants.

The "-F" option can be used repeatedly,
as long as a different variable is
specified for each occurrence.

It can be convenient to specify that the same compression is to be
applied to more than one variable. To support this, two additional
*-F* cases are defined.

1. ````-F *,...``` means apply the filter to all variables in the dataset.
2. ````-F v1&v2&..,...``` means apply the filter to a multiple variables.

Multiple filters can be specified using the pipeline notions '|'.
For example

1. ````-F v1&v2,307,9|4,32,32`` means apply filter 307 (bzip2) then filter 4 (szip) to the multiple variables.

Note that the characters '*', '&', and '|' are bash reserved characters,
so you will probably need to escape or quote the filter spec in
that environment.

As a rule, any input filter on an input variable will be applied
to the equivalent output variable -- assuming the output file type
is netcdf-4. It is, however, sometimes convenient to suppress
output compression either totally or on a per-variable basis.
Total suppression of output filters can be accomplished by specifying
a special case of "-F", namely this.
````
nccopy -F none input.nc output.nc
````
The expression ````-F *,none```` is equivalent to ````-F none````.

Suppression of output filtering for a specific set of variables
can be accomplished using these formats.
````
nccopy -F "var,none" input.nc output.nc
nccopy -F "v1&v2&...,none" input.nc output.nc
````
where "var" and the "vi" are the fully qualified name of a variable.

The rules for all possible cases of the "-F none" flag are defined
by this table.

<table>
<tr><th>-F none<th>-Fvar,...<th>Input Filter<th>Applied Output Filter
<tr><td>true<td>undefined<td>NA<td>unfiltered
<tr><td>true<td>none<td>NA<td>unfiltered
<tr><td>true<td>defined<td>NA<td>use output filter(s)
<tr><td>false<td>undefined<td>defined<td>use input filter(s)
<tr><td>false<td>none<td>NA<td>unfiltered
<tr><td>false<td>defined<td>NA<td>use output filter(s)
<tr><td>false<td>undefined<td>undefined<td>unfiltered
<tr><td>false<td>defined<td>defined<td>use output filter(s)
</table> 

# HDF5 Parameter Encode/Decode {#filters_paramcoding}

The parameters passed to an HDF5 format filter are encoded
internally as a vector of 32-bit unsigned integers. It may be
that the parameters required by a filter can naturally be
encoded as unsigned integers.  The bzip2 compression filter, for
example, expects a single integer value from zero thru
nine. This encodes naturally as a single unsigned integer.

Note that signed integers and single-precision (32-bit) float values
also can easily be represented as 32 bit unsigned integers by
proper casting to an unsigned integer so that the bit pattern
is preserved. Simple integer values of type short or char
(or the unsigned versions) can also be mapped to an unsigned
integer by truncating to 16 or 8 bits respectively and then
zero extending.

Machine byte order (aka endian-ness) is an issue for passing
some kinds of parameters. You might define the parameters when
compressing on a little endian machine, but later do the
decompression on a big endian machine.

When using HDF5 format filters, byte order is not an
issue for 32-bit values because HDF5 takes care of converting
them between the local machine byte order and network byte
order.

Parameters whose size is larger than 32-bits present a byte order problem.
This specifically includes double precision floats and (signed or unsigned)
64-bit integers. For these cases, the machine byte order issue must be
handled, in part, by the compression code. This is because HDF5 will treat,
for example, an unsigned long long as two 32-bit unsigned integers
and will convert each to network order separately. This means that
on a machine whose byte order is different than the machine in which
the parameters were initially created, the two integers will be separately
endian converted. But this will be incorrect for 64-bit values.

So, we have this situation (for HDF5 only):

1. the 8 bytes come in as native machine order for the machine
   doing the call to *nc_def_var_filter*.
2. HDF5 divides the 8 bytes into 2 four byte pieces and ensures that each piece
   is in network (big) endian order.
3. When the filter is called, the two pieces are returned in the same order
   but with the bytes in each piece consistent with the native machine order
   for the machine executing the filter.

Note that the situation for non-HDF5 filters may be different
since the arguments are potentially 64 bit.

## Encoding Algorithms for HDF5

In order to properly extract the correct 8-byte value, we need to ensure
that the values stored in the HDF5 file have a known format independent of
the native format of the creating machine.

The idea is to do sufficient manipulation so that HDF5
will store the 8-byte value as a little endian value
divided into two 4-byte integers.
Note that little-endian is used as the standard
because it is the most common machine format.
When read, the filter code needs to be aware of this convention
and do the appropriate conversions.

This leads to the following set of rules.

### Encoding 

1. Encode on little endian (LE) machine: no special action is required.
   The 8-byte value is passed to HDF5 as two 4-byte integers. HDF5 byte
   swaps each integer and stores it in the file.
2. Encode on a big endian (BE) machine: several steps are required:

   1. Do an 8-byte byte swap to convert the original value to little-endian
      format.
   2. Since the encoding machine is BE, HDF5 will just store the value.
      So it is necessary to simulate little endian encoding by byte-swapping
      each 4-byte integer separately. 
   3. This doubly swapped pair of integers is then passed to HDF5 and is stored
      unchanged.

### Decoding 

1. Decode on LE machine: no special action is required.
   HDF5 will get the two 4-bytes values from the file and byte-swap each
   separately. The concatenation of those two integers will be the expected
   LE value.
2. Decode on a big endian (BE) machine: the inverse of the encode case must
   be implemented.

   1. HDF5 sends the two 4-byte values to the filter.
   2. The filter must then byte-swap each 4-byte value independently.
   3. The filter then must concatenate the two 4-byte values into a single
      8-byte value. Because of the encoding rules, this 8-byte value will
      be in LE format.
   4. The filter must finally do an 8-byte byte-swap on that 8-byte value
      to convert it to desired BE format.

To support these rules, some utility programs exist and are discussed in
<a href="#filters_appendixa">Appendix A</a>.

# Filter Specification Syntax {#filters_syntax}

The utilities
<a href="#NCGEN">ncgen</a>
and
<a href="#NCCOPY">nccopy</a>,
and also the output of __ncdump__,
support the specification of filter ids, formats, and parameters
in text format. The BNF specification is defined in
<a href="#filters_appendixc">Appendix C</a>.
Basically, These specifications consist of a filter id
a comma, and then a sequence of
comma separated constants representing the parameters
The id may be a name (case-insensitive) if it is known to the netcdf library
(see <a href="#filters_appendixd">Appendix D</a>).
otherwise it must be an unsigned integer.
The constants are converted within
the utility to a proper set of unsigned int constants (see the
<a href="#ParamEncode">parameter encoding section</a>).

To simplify things, various kinds of constants can be specified
rather than just simple unsigned integers. The utilities will encode
them properly using the rules specified in 
the section on <a href="#filters_paramcoding">parameter encode/decode</a>.

The currently supported constants are as follows.
<table>
<tr halign="center"><th>Example<th>Type<th>Format Tag<th>Notes
<tr><td>-17b<td>signed 8-bit byte<td>b|B<td>Truncated to 8 bits and zero extended to 32 bits
<tr><td>23ub<td>unsigned 8-bit byte<td>u|U b|B<td>Truncated to 8 bits and zero extended to 32 bits
<tr><td>-25S<td>signed 16-bit short<td>s|S<td>Truncated to 16 bits and zero extended to 32 bits
<tr><td>27US<td>unsigned 16-bit short<td>u|U s|S<td>Truncated to 16 bits and zero extended to 32 bits
<tr><td>-77<td>implicit signed 32-bit integer<td>Leading minus sign and no tag<td>
<tr><td>77<td>implicit unsigned 32-bit integer<td>No tag<td>
<tr><td>93U<td>explicit unsigned 32-bit integer<td>u|U<td>
<tr><td>789f<td>32-bit float<td>f|F<td>
<tr><td>12345678.12345678d<td>64-bit double<td>d|D<td>LE encoding
<tr><td>-9223372036854775807L<td>64-bit signed long long<td>l|L<td>LE encoding
<tr><td>18446744073709551615UL<td>64-bit unsigned long long<td>u|U l|L<td>LE encoding
</table>
Some things to note.

1. In all cases, except for an untagged positive integer,
   the format tag is required and determines how the constant
   is converted to one or two unsigned int values.
1. For an untagged positive integer, the constant is treated
   as of the smallest type into which it fits (i.e. 8,16,32, or 64 bit).
1. For signed byte and short, the value is sign extended to 32 bits
   and then treated as an unsigned int value.
1. For double, and signed|unsigned long long, they are converted
   as specified in the section on
   <a href="#filters_paramcoding">parameter encode/decode</a>.
1. In order to support mutiple filters, the argument to ''_Filter''
   may be a pipeline separated  (using '|') to specify a list of filters specs.

Dynamic Loading Process {#filters_Process}
==========

The documentation[1,2] for the HDF5 dynamic loading was (at the time
this was written) out-of-date with respect to the actual HDF5 code
(see HDF5PL.c). So, the following discussion is largely derived
from looking at the actual code. This means that it is subject to change.

Plugin directory {#filters_Plugindir}
----------------

The HDF5 loader expects plugins to be in a specified plugin directory.
The default directory is:
   * "/usr/local/hdf5/lib/plugin” for linux/unix operating systems (including Cygwin)
   * “%ALLUSERSPROFILE%\\hdf5\\lib\\plugin” for Windows systems, although the code
     does not appear to explicitly use this path.

The default may be overridden using the environment variable
__HDF5_PLUGIN_PATH__.

Plugin Library Naming {#filters_Pluginlib}
---------------------

Given a plugin directory, HDF5 examines every file in that
directory that conforms to a specified name pattern
as determined by the platform on which the library is being executed.
<table>
<tr halign="center"><th>Platform<th>Basename<th>Extension
<tr halign="left"><td>Linux<td>lib*<td>.so*
<tr halign="left"><td>OSX<td>lib*<td>.so*
<tr halign="left"><td>Cygwin<td>cyg*<td>.dll*
<tr halign="left"><td>Windows<td>*<td>.dll
</table>

Plugin Verification {#filters_Pluginverify}
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

Debugging {#filters_Debug}
-------
Debugging plugins can be very difficult. You will probably
need to use the old printf approach for debugging the filter itself.

One case worth mentioning is when you have a dataset that is
using an unknown filter. For this situation, you need to
identify what filter(s) are used in the dataset. This can
be accomplished using this command.
````
ncdump -s -h <dataset filename>
````
Since ncdump is not being asked to access the data (the -h flag), it
can obtain the filter information without failures. Then it can print
out the filter id and the parameters (the -s flag).

Test Cases {#filters_TestCase}
-------
Within the netcdf-c source tree, the directory
__netcdf-c/nc_test4__ contains a number of test cases for
testing dynamic filter writing and reading. These include
* __test_filter.c__ -- tests simple compression/decompression using
the bzip2 compressor in the directory __plugins__.
* __test_filterparser.c__ -- validates parameter passing.
* __test_filter_reg.c__ -- test user defined filter registration
(see <a href="#filters_appendixb">Appendix B</a>).
* __test_multifilter.c__ -- tests applying multiple filters to a
single variable: bzip2, deflate(zip), and szip (if enabled).
* __test_filter.sh__ -- test driver to execute the above tests.

These tests are disabled if __--enable-shared__
is not set or if __--enable-netcdf-4__ is not set.

Example {#filters_Example}
-------
A slightly simplified version of the filter test case is also
available as an example within the netcdf-c source tree
directory __netcdf-c/examples/C__. The test is called __filter_example.c__
and it is executed as part of the __run_examples4.sh__ shell script.
The test case demonstrates dynamic filter writing and reading.

The files __example/C/hdf5plugins/Makefile.am__
and  __example/C/hdf5plugins/CMakeLists.txt__
demonstrate how to build the hdf5 plugin for bzip2.

Notes
==========

Memory Allocation Issues
-----------

Starting with HDF5 version 1.10.x, the plugin code MUST be
careful when using the standard *malloc()*, *realloc()*, and
*free()* function.

In the event that the code is allocating, reallocating, for
free'ing memory that either came from or will be exported to the
calling HDF5 library, then one MUST use the corresponding HDF5
functions *H5allocate_memory()*, *H5resize_memory()*,
*H5free_memory()* [5] to avoid memory failures.

Additionally, if your filter code leaks memory, then the HDF5 library
generates a failure something like this.
````
H5MM.c:232: H5MM_final_sanity_check: Assertion `0 == H5MM_curr_alloc_bytes_s' failed.
````

One can look at the the code in plugins/H5Zbzip2.c and H5Zmisc.c to see this.

SZIP Issues
-----------
The current szip plugin code in the HDF5 library has some
behaviors that can catch the unwary.  These are handled
internally to (mostly) hide them so that they should not affect
users.  Specifically, this filter may do two things.

1. Add extra parameters to the filter parameters: going from
   the two parameters provided by the user to four parameters
   for internal use. It turns out that the two parameters provided
   when calling nc_def_var_filter correspond to the first two
   parameters of the four parameters returned by nc_inq_var_filter.
2. Change the values of some parameters: the value of the
   __options_mask__ argument is known to add additional flag bits,
   and the __pixels_per_block__ parameter may be modified.

The reason for these changes is has to do with the fact that
the szip API provided by the underlying H5Pset_szip function
is actually a subset of the capabilities of the real szip implementation.
Presumably this is for historical reasons.

In any case, if the caller uses the __nc_inq_var_szip__
or the __nc_inq_var_filter__ functions, then
the parameter values returned may differ from those originally specified.

Supported Systems
-----------------
The current matrix of OS X build systems known to work is as follows.
<table>
<tr><th>Build System<th>Supported OS
<tr><td>Automake<td>Linux, Cygwin
<tr><td>Cmake<td>Linux, Cygwin, Visual Studio
</table>

Generic Plugin Build
--------------------
If you do not want to use Automake or Cmake, the following
has been known to work.
````
gcc -g -O0 -shared -o libbzip2.so <plugin source files>  -L${HDF5LIBDIR} -lhdf5_hl -lhdf5 -L${ZLIBDIR} -lz
````

Appendix A. Support Utilities {#filters_appendixa}
==========

Several functions are exported from the netcdf-c library
for use by client programs and by filter implementations.

1. ````int NC_parsefilterlist(const char* listspec, int* formatp, size_t* nfiltersp, NC_Filterspec*** filtersp);
    * listspec contains the text of a sequence of filter specs.
    * formatp will contain the format of the filters (currently only HDF5 is supported)
    * nfiltersp will contain the number of filters found
    * filtersp will contain a vector of pointers to the parsed filters
      -- the caller must free; also note that the number of filters will
      depend on the format.
    This function can parse filter spec strings as defined in 
    the section on <a href="#filters_syntax">Filter Specification Syntax</a>.

2. ````int NC_hdf5_filterfix8(unsigned char* mem8, int decode);````
    * mem8 is a pointer to the 8-byte value either to fix.
    * decode is 1 if the function should apply the 8-byte decoding algorithm
      else apply the encoding algorithm.
    This function implements the 8-byte conversion algorithms for HDF5
    Before calling *nc_def_var_filter* (unless *NC_parsefilterspec* was used),
    the client must call this function with the decode argument set to 0.
    Inside the filter code, this function should be called with the decode
    argument set to 1.

Examples of the use of these functions can be seen in the test program
*nc_test4/tst_filterparser.c*.

Appendix B. Programmatic Filter Definition {#filters_appendixb}
==========

HDF5 provides an API [6] to allow for the programmatic definition
of filters -- as opposed to using the HDF5_PLUGIN_PATH environment variable.
The idea is that instead of using dynamic shared libraries, the filter code
is compiled into the application and the relevant information
(namely an instance of *H5Z_class2_t*) is passed to the HDF5 library API.
Because it is anticipated that in the future, other plugin formats
will be used, this netcdf-c API is deliberately more general than
strictly required by HDF5.

## API Concepts

Three concepts are used in this API.

1. Format - this is an integer defining the format of the plugin.
   Currently, only *NC_FILTER_FORMAT_HDF5* is defined and corresponds
   to the existing HDF5 plugin format.
2. ID - this is an integer that is a unique identifier for the filter.
3. The structure H5Z_class2_t (defined in H5Zpublic.h).
   This carries the information required by HDF5 to use the filter.
   To avoid having to include the definition of H5Z_class2_t
   when netcdf_filter is used, a pointer to H5Z_class2_t
   is replaced with a __void*__ pointer.

### NetCDF API

The following function signatures are provided (see *netcdf_filter.h*).

1. Register a filter

        int nc_filter_client_register(unsigned int id, void*/*H5Z_class2_t* */);
    Register a filter of given id and H5Z_class_t info.

2. Unregister a filter

        int nc_filter_client_unregister(int id);
    Unregister the filter specified by the id. Note that only
    filters registered using 'nc_filter_register' can be unregistered.

3. Inquire about a filter

        int nc_filter_client_inq(unsigned int id,  void*/*H5Z_class2_t* */);
    Inquire about the filter specified by the id. Note that only
    filters registered using 'nc_filter_register' can be inquired.
    The second argument is filled with a copy of the original argument to
    'nc_filter_register'.

### Example

````
    static H5Z_class2_t H5Z_REG[1] = {
        ...
    };
    ...
    H5Z_class2_t* h5info = &H5Z_REG[0];
    H5Z_class2_t h5inq;
    ...
    stat = nc_user_filter_register(FILTER_ID,(void*)&h5info);
    ...
    stat = nc_user_filter_inq(FILTER_ID, &h5inq);
    ...
    stat = nc_user_filter_unregister(FILTER_ID);
````

Appendix C. BNF for Specifying Filters in Utilities {#filters_appendixc}
==========

````
speclist:   spec
          | speclist '|' spec
          ;
spec:   filterid
      | filterid ',' parameterlist
      ;
filterid:   unsigned32
	  | name
          ;
parameterlist:   parameter
               | parameterlist ',' parameter
               ;
parameter: unsigned32

where
int: <32 bit integer>
unsigned32: <32 bit unsigned integer>
name: [a-zA-Z][a-zA-Z0-9-_]*
````

Appendix D. Known Filter Names {#filters_appendixd}
==========
<table>
<tr><th>Name<th>Id<th>Description
<tr><td>zip<td>2<td>Standard zlib compression
<tr><td>zlib<tr>2<td>
<tr><td>deflate<td>2<td>
<tr><td>szip<td>4<td>Standard szip compression
<tr><td>bzip2<td>307<td>BZIP2 lossless compression used by PyTables
<tr><td>lzf<td>32000<td>LZF lossless compression used by H5Py project
<tr><td>blosc<td>32001<td>Blosc lossless compression used by PyTables
<tr><td>mafisc<td>32002<td>Modified LZMA compression filter, MAFISC (Multidimensional Adaptive Filtering Improved Scientific data Compression)
<tr><td>snappy<td>32003<td>Snappy lossless compression.
<tr><td>lz4<td>32004<td>LZ4 fast lossless compression algorithm
<tr><td>apax<td>32005<td>Samplify's APAX Numerical Encoding Technology
<tr><td>cbf<td>32006<td>All imgCIF/CBF compressions and decompressions, including Canonical, Packed, Packed Vesrsion 2, Byte Offset and Nibble Offset.
<tr><td>jpeg-xr<td>32007<td>Enables images to be compressed/decompressed with JPEG-XR compression
<tr><td>bitshuffle<td>32008<td>Extreme version of shuffle filter that shuffles data at bit level instead of byte level.
<tr><td>spdp<td>32009<td>SPDP fast lossless compression algorithm for single- and double-precision floating-point data.
<tr><td>lpc-rice<td>32010<td>LPC-Rice multi-threaded lossless compression
<tr><td>ccsds-123<td>32011<td>ESA CCSDS-123 multi-threaded compression filter
<tr><td>jpeg-ls<td>32012<td>CharLS JPEG-LS multi-threaded compression filter
<tr><td>zfp<td>32013<td>Rate, accuracy or precision bounded compression for floating-point arrays
<tr><td>fpzip<td>32014<td>Fast and Efficient Lossy or Lossless Compressor for Floating-Point Data
<tr><td>zstandard<td>32015<td>Real-time compression algorithm with wide range of compression / speed trade-off and fast decoder
<tr><td>b3d<td>32016<td>GPU based image compression method developed for light-microscopy applications
<tr><td>sz<td>32017<td>An error-bounded lossy compressor for scientific floating-point data
<tr><td>fcidecomp<td>32018<td>EUMETSAT CharLS compression filter for use with netCDF
<tr><td>user-defined<td>32768<td>First user-defined filter
</table>


# References {#filters_References}

1. https://support.hdfgroup.org/HDF5/doc/Advanced/DynamicallyLoadedFilters/HDF5DynamicallyLoadedFilters.pdf
2. https://support.hdfgroup.org/HDF5/doc/TechNotes/TechNote-HDF5-CompressionTroubleshooting.pdf
3. https://portal.hdfgroup.org/display/support/Contributions#Contributions-filters
4. https://support.hdfgroup.org/services/contributions.html#filters
5. https://support.hdfgroup.org/HDF5/doc/RM/RM_H5.html
6. https://confluence.hdfgroup.org/display/HDF5/Filters

# Point of Contact

__Author__: Dennis Heimbigner<br>
__Email__: dmh at ucar dot edu
__Initial Version__: 1/10/2018<br>
__Last Revised__: 1/27/2020

