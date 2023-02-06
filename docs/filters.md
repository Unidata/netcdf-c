Appendix D. NetCDF-4 Filter Support {#filters} 
==================================

[TOC]

> See @ref nc_filters_quickstart for tips to get started quickly with NetCDF-4 Filter Support. 

## Filters Overview {#filters_overview}

NetCDF-C filters have some features of which the user
should be aware.

* ***Auto Install of filters***<br>
An option is now provided to automatically install
HDF5 filters into a default location, or optionally
into a user-specified location. This is described in
[Appendix H](#filters_appendixh)
(with supporting information in [Appendix G](#filters_appendixg)). 

* ***NCZarr Filter Support***<br>
[NCZarr filters](#filters_nczarr) are now supported.
This essentially means that it is possible to specify
Zarr Codecs (Zarr equivalent of filters) in Zarr files
and have them processed using HDF5-style wrapper shared libraries.
Zarr filters can be used even if HDF5 support is disabled
in the netCDF-C library.

## Introduction to Filters {#filters_introduction}

The netCDF library supports a general filter mechanism to apply
various kinds of filters to datasets before reading or writing.
The most common kind of filter is a compression-decompression
filter, and that is the focus of this document.
But non-compression filters &ndash; fletcher32, for example &ndash; also exist.

The netCDF enhanced (aka netCDF-4) library inherits this
capability since it depends on the HDF5 library.  The HDF5
library (1.8.11 and later) supports filters, and netCDF is based
closely on that underlying HDF5 mechanism.

Filters assume that a variable has chunking defined and each
chunk is filtered before writing and "unfiltered" after reading
and before passing the data to the user.  In the event that
multiple filters are defined on a variable, they are applied in
first-defined order on writing and on the reverse order when
reading.

This document describes the support for HDF5 filters and also
the newly added support for NCZarr filters.

## A Warning on Backward Compatibility {#filters_compatibility}

The API defined in this document should accurately reflect the
current state of filters in the netCDF-c library.  Be aware that
there was a short period in which the filter code was undergoing
some revision and extension.  Those extensions have largely been
reverted.  Unfortunately, some users may experience some
compilation problems for previously working code because of
these reversions.  In that case, please revise your code to
adhere to this document. Apologies are extended for any
inconvenience.

A user may encounter an incompatibility if any of the following appears in user code.

* The function *\_nc\_inq\_var\_filter* was returning the error value NC\_ENOFILTER  if a variable had no associated filters.
  It has been reverted to the previous case where it returns NC\_NOERR and the returned filter id was set to zero if the variable had no filters.
* The function *nc\_inq\_var\_filterids* was renamed to *nc\_inq\_var\_filter\_ids*.
* Some auxilliary functions for parsing textual filter specifications have been moved to the file *netcdf\_aux.h*. See [Appendix A](#filters_appendixa).
* All of the "filterx" functions have been removed. This is unlikely to cause problems because they had limited visibility.

For additional information, see [Appendix B](#filters_appendixb).

## Enabling A HDF5 Compression Filter {#filters_enable}

HDF5 supports dynamic loading of compression filters using the
following process for reading of compressed data.

1. Assume that we have a dataset with one or more variables that were compressed using some algorithm.
  How the dataset was compressed will be discussed subsequently.
2. Shared libraries or DLLs exist that implement the compress/decompress algorithm.
  These libraries have a specific API so that the HDF5 library can locate, load, and utilize the compressor.
3. These libraries are expected to installed in a specific directory.

In order to compress a variable with an HDF5 compliant filter,
the netcdf-c library must be given three pieces of information:

1. some unique identifier for the filter to be used,
2. a vector of parameters for controlling the action of the compression filter, and
3. access to a shared library implementation of the filter.

The meaning of the parameters is, of course, completely filter
dependent and the filter description [3] needs to be consulted.
For bzip2, for example, a single parameter is provided
representing the compression level.  It is legal to provide a
zero-length set of parameters.  Defaults are not provided, so
this assumes that the filter can operate with zero parameters.

Filter ids are assigned by the HDF group.  See [4] for a current
list of assigned filter ids.  Note that ids above 32767 can be
used for testing without registration.

The first two pieces of information can be provided in one of
three ways: (1) using *ncgen*, (2) via an API call, or (3) via
command line parameters to *nccopy*.  In any case, remember that
filtering also requires setting chunking, so the variable must
also be marked with chunking information.  If compression is set
for a non-chunked variable, the variable will forcibly be
converted to chunked using a default chunking algorithm.

## Using The API {#filters_API}
The necessary API methods are included in *netcdf\_filter.h* by default.
These functions implicitly use the HDF5 mechanisms and may produce an error if applied to a file format that is not compatible with the HDF5 mechanism.

### nc\_def\_var\_filter
Add a filter to the set of filters to be used when writing a variable. This must be invoked after the variable has been created and before *nc\_enddef* is invoked.
````
    int nc_def_var_filter(int ncid, int varid, unsigned int id,
                          size_t nparams, const unsigned int* params);
````
Arguments:

* ncid &mdash; File and group ID.
* varid &mdash; Variable ID.
* id &mdash; Filter ID.
* nparams &mdash;  Number of filter parameters.
* params &mdash; Filter parameters (a vector of unsigned integers)

Return codes:

* NC\_NOERR &mdash; No error.
* NC\_ENOTNC4 &mdash; Not a netCDF-4 file.
* NC\_EBADID &mdash; Bad ncid or bad filter id
* NC\_ENOTVAR &mdash; Invalid variable ID.
* NC\_EINDEFINE &mdash; called when not in define mode
* NC\_ELATEDEF &mdash; called after variable was created
* NC\_EINVAL &mdash; Scalar variable, or parallel enabled and parallel filters not supported or nparams or params invalid.

### nc\_inq\_var\_filter\_ids
Query a variable to obtain a list of the ids of all filters associated with that variable.
````
int nc_inq_var_filter_ids(int ncid, int varid, size_t* nfiltersp, unsigned int* filterids);
````
Arguments:

* ncid &mdash; File and group ID.
* varid &mdash; Variable ID.
* nfiltersp &mdash; Stores number of filters found; may be zero.
* filterids &mdash; Stores set of filter ids.

Return codes:

* NC\_NOERR &mdash; No error.
* NC\_ENOTNC4 &mdash; Not a netCDF-4 file.
* NC\_EBADID &mdash; Bad ncid
* NC\_ENOTVAR &mdash; Invalid variable ID.

The number of filters associated with the variable is stored in *nfiltersp* (it may be zero).
The set of filter ids will be returned in *filterids*.
As is usual with the netcdf API, one is expected to call this function twice.
The first time to set *nfiltersp* and the second to get the filter ids in client-allocated memory.
Any of these arguments can be NULL, in which case no value is returned.

### nc\_inq\_var\_filter\_info
Query a variable to obtain information about a specific filter associated with the variable.
````
int nc_inq_var_filter_info(int ncid, int varid, unsigned int id,  size_t* nparamsp, unsigned int* params);
````
Arguments:

* ncid &mdash; File and group ID.
* varid &mdash; Variable ID.
* id &mdash; The filter id of interest.
* nparamsp &mdash; Stores number of parameters.
* params &mdash; Stores set of filter parameters.

Return codes:

* NC\_NOERR &mdash; No error.
* NC\_ENOTNC4 &mdash; Not a netCDF-4 file.
* NC\_EBADID &mdash; Bad ncid
* NC\_ENOTVAR &mdash; Invalid variable ID.
* NC\_ENOFILTER &mdash; Filter not defined for the variable.

The *id* indicates the filter of interest.
The actual parameters are stored in *params*.
The number of parameters is returned in *nparamsp*.
As is usual with the netcdf API, one is expected to call this function twice.
The first time to set *nparamsp* and the second to get the parameters in client-allocated memory.
Any of these arguments can be NULL, in which case no value is returned.
If the specified id is not attached to the variable, then NC\_ENOFILTER is returned.

### nc\_inq\_var\_filter
Query a variable to obtain information about the first filter associated with the variable.
When netcdf-c was modified to support multiple filters per variable, the utility of this function became redundant since it returns info only about the first defined filter for the variable.
Internally, it is implemented using the functions *nc\_inq\_var\_filter\_ids* and *nc\_inq\_filter\_info*.

````
int nc_inq_var_filter(int ncid, int varid, unsigned int* idp, size_t* nparamsp, unsigned int* params);
````

Arguments:

* ncid &mdash; File and group ID.
* varid &mdash; Variable ID.
* idp &mdash; Stores the id of the first found filter, set to zero if variable has no filters.
* nparamsp &mdash; Stores number of parameters.
* params &mdash; Stores set of filter parameters.

Return codes:

* NC\_NOERR &mdash; No error.
* NC\_ENOTNC4 &mdash; Not a netCDF-4 file.
* NC\_EBADID &mdash; Bad ncid
* NC\_ENOTVAR &mdash; Invalid variable ID.

The filter id will be returned in the *idp* argument.
If there are no filters, then zero is stored in this argument.
Otherwise, the number of parameters is stored in *nparamsp* and the actual parameters in *params*.
As is usual with the netcdf API, one is expected to call this function twice.
The first time to get *nparamsp* and the second to get the parameters in client-allocated memory.
Any of these arguments can be NULL, in which case no value is returned.

## Using ncgen {#filters_NCGEN}

In a CDL file, compression of a variable can be specified by annotating it with the following attribute:

* *\_Filter* &mdash; a string containing a comma separated list of constants specifying (1) the filter id to apply, and (2) a vector of constants representing the parameters for controlling the operation of the specified filter.
See the section on the <a href="#filters_syntax">parameter encoding syntax</a> for the details on the allowable kinds of constants.

This is a "special" attribute, which means that it will normally be invisible when using *ncdump* unless the -s flag is specified.

For backward compatibility it is probably better to use the *\_Deflate* attribute instead of *\_Filter*. But using *\_Filter* to specify deflation will work.

Multiple filters can be specified for a given variable by using the "|" separator.
Alternatively, this attribute may be repeated to specify multiple filters.

Note that the lexical order of declaration is important when more than one filter is specified for a variable because it determines the order in which the filters are applied.

### Example CDL File (Data elided)

````
netcdf bzip2szip {
dimensions:
  dim0 = 4 ; dim1 = 4 ; dim2 = 4 ; dim3 = 4 ;
variables:
  float var(dim0, dim1, dim2, dim3) ;
    var:_Filter = "307,9|4,32,32" ; // bzip2 then szip
    var:_Storage = "chunked" ;
    var:_ChunkSizes = 4, 4, 4, 4 ;
data:
...
}
````

Note that the assigned filter id for bzip2 is 307 and for szip it is 4.

## Using nccopy {#filters_NCCOPY}

When copying a netcdf file using *nccopy* it is possible to specify filter information for any output variable by using the "-F" option on the command line; for example:

    nccopy -F "var,307,9" unfiltered.nc filtered.nc

Assume that *unfiltered.nc* has a chunked but not bzip2 compressed variable named "var".
This command will copy that variable to the *filtered.nc* output file but using filter with id 307 (i.e. bzip2) and with parameter(s) 9 indicating the compression level.
See the section on the <a href="#filters_syntax">parameter encoding syntax</a> for the details on the allowable kinds of constants.

The "-F" option can be used repeatedly, as long as a different variable is specified for each occurrence.

It can be convenient to specify that the same compression is to be applied to more than one variable. To support this, two additional *-F* cases are defined.

1. *-F \*,...* means apply the filter to all variables in the dataset.
2. *-F v1&v2&..,...* means apply the filter to multiple variables.

Multiple filters can be specified using the pipeline notions '|'.
For example

1. *-F v1&v2,307,9|4,32,32* means apply filter 307 (bzip2) then filter 4 (szip) to the multiple variables.

Note that the characters '\*', '\&', and '\|' are shell reserved characters, so you will probably need to escape or quote the filter spec in that environment.

As a rule, any input filter on an input variable will be applied to the equivalent output variable &mdash; assuming the output file type is netcdf-4.
It is, however, sometimes convenient to suppress output compression either totally or on a per-variable basis.
Total suppression of output filters can be accomplished by specifying a special case of "-F", namely this.

    nccopy -F none input.nc output.nc

The expression *-F \*,none* is equivalent to *-F none*.

Suppression of output filtering for a specific set of variables can be accomplished using these formats.

    nccopy -F "var,none" input.nc output.nc
    nccopy -F "v1&v2&...,none" input.nc output.nc

where "var" and the "vi" are the fully qualified name of a variable.

The rules for all possible cases of the "-F none" flag are defined by this table.
<table>
<tr><th>-F none<th>-Fvar,...<th>Input Filter<th>Applied Output Filter
<tr><td>true<td>undefined<td>NA<td>unfiltered
<tr><td>true<td>none<td>NA<td>unfiltered
<tr><td>true<td>defined<td>NA<td>use output filter(s)
<tr><td>false<td>undefined<td>defined<td>use input filter(s)
<tr><td>false<td>none<td>NA<td>unfiltered
<tr><td>false<td>defined<td>undefined<td>use output filter(s)
<tr><td>false<td>undefined<td>undefined<td>unfiltered
<tr><td>false<td>defined<td>defined<td>use output filter(s)
</table> 

## Filter Specification Syntax  {#filters_syntax}

The utilities <a href="#NCGEN">ncgen</a> and <a href="#NCCOPY">nccopy</a>, and also the output of *ncdump*, support the specification of filter ids, formats, and parameters in text format.
The BNF specification is defined in [Appendix C](#filters_appendixc).
Basically, These specifications consist of a filter id, a comma, and then a sequence of
comma separated constants representing the parameters.
The constants are converted within the utility to a proper set of unsigned int constants (see the <a href="#ParamEncode">parameter encoding section</a>).

To simplify things, various kinds of constants can be specified rather than just simple unsigned integers.
The *ncgen* and *nccopy* programs will encode them properly using the rules specified in the section on <a href="#filters_paramcoding">parameter encode/decode</a>.
Since the original types are lost after encoding, *ncdump* will always show a simple list of unsigned integer constants.

The currently supported constants are as follows.
<table>
<tr halign="center"><th>Example<th>Type<th>Format Tag<th>Notes
<tr><td>-17b<td>signed 8-bit byte<td>b|B<td>Truncated to 8 bits and sign extended to 32 bits
<tr><td>23ub<td>unsigned 8-bit byte<td>u|U b|B<td>Truncated to 8 bits and zero extended to 32 bits
<tr><td>-25S<td>signed 16-bit short<td>s|S<td>Truncated to 16 bits and sign extended to 32 bits
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

1. In all cases, except for an untagged positive integer, the format tag is required and determines how the constant is converted to one or two unsigned int values.
2. For an untagged positive integer, the constant is treated as of the smallest type into which it fits (i.e. 8,16,32, or 64 bit).
3. For signed byte and short, the value is sign extended to 32 bits and then treated as an unsigned int value, but maintaining the bit-pattern.
4. For double, and signed|unsigned long long, they are converted as specified in the section on <a href="#filters_paramcoding">parameter encode/decode</a>.
5. In order to support mutiple filters, the argument to *\_Filter* may be a pipeline separated  (using '|') to specify a list of filters specs.

## Dynamic Loading Process {#filters_Process}

Each filter is assumed to be compiled into a separate dynamically loaded library.
For HDF5 conformant filters, these filter libraries are assumed to be in some specific location.
The details for writing such a filter are defined in the HDF5 documentation[1,2].

### Plugin directory {#filters_plugindir}

The HDF5 loader searches for plugins in a number of directories.
This search is contingent on the presence or absence of the environment
variable named ***HDF5_PLUGIN_PATH***.

As with all other "...PATH" variables, it is a sequence of absolute
directories separated by a separator character. For *nix* operating systems,
this separator is the colon (':') character. For Windows and Mingw, the
separator is the semi-colon (';') character. So for example:

* Linux:   export HDF5_PLUGIN_PATH=/usr/lib:/usr/local/lib
* Windows: export HDF5_PLUGIN_PATH=c:\\ProgramData\\hdf5\\plugin;c:\\tools\\lib

If HDF5_PLUGIN_PATH is defined, then the loader will search each directory
in the path from left to right looking for shared libraries with specific
exported symbols representing the entry points into the library.

If HDF5_PLUGIN_PATH is not defined, the loader defaults to using
these default directories:

* Linux:  /usr/local/hdf5/lib/plugin
* Windows: %ALLUSERSPROFILE%\\hdf5\\lib\\plugin

It should be noted that there is a difference between the search order
for HDF5 versus NCZarr. The HDF5 loader will search only the directories
specificed in HDF5_PLUGIN_PATH. In NCZarr, the loader
searches HDF5_PLUGIN_PATH and as a last resort,
it also searches the default directory.

### Plugin Library Naming {#filters_Pluginlib}

Given a plugin directory, HDF5 examines every file in that directory
that conforms to a specified name pattern as determined by the
platform on which the library is being executed.

<table>
<tr halign="center"><th>Platform<th>Basename<th>Extension
<tr halign="left"><td>Linux<td>lib*<td>.so*
<tr halign="left"><td>OSX<td>lib*<td>.dylib*
<tr halign="left"><td>Cygwin<td>cyg*<td>.dll*
<tr halign="left"><td>Windows<td>*<td>.dll
</table>

### Plugin Verification {#filters_Pluginverify}

For each dynamic library located using the previous patterns,
HDF5 attempts to load the library and attempts to obtain
information from it.  Specifically, It looks for two functions
with the following signatures.

1. *H5PL\_type\_t H5PLget\_plugin\_type(void)* &mdash; This function is expected to return the constant value *H5PL\_TYPE\_FILTER* to indicate that this is a filter library.
2. *const void* H5PLget\_plugin\_info(void)* &mdash; This function returns a pointer to a table of type *H5Z\_class2\_t*.
    This table contains the necessary information needed to utilize the filter both for reading and for writing.
    In particular, it specifies the filter id implemented by the library and it must match that id specified for the variable in *nc\_def\_var\_filter* in order to be used.

If plugin verification fails, then that plugin is ignored and the search continues for another, matching plugin.

## NCZarr Filter Support {#filters_nczarr}

The inclusion of Zarr support in the netcdf-c library creates the need to provide a new representation consistent with the way that Zarr files store filter information.
For Zarr, filters are represented using the JSON notation.
Each filter is defined by a JSON dictionary, and each such filter dictionary
is guaranteed to have a key named "id" whose value is a unique string defining the filter algorithm: "lz4" or "bzip2", for example.

The parameters of the filter are defined by additional &mdash; algorithm specific &mdash; keys in the filter dictionary.
One commonly used filter is "blosc", which has a JSON dictionary of this form.
````
    {
    "id": "blosc",
    "cname": "lz4",
    "clevel": 5,
    "shuffle": 1
    }
````
So it has three parameters:

1. "cname" &mdash; the sub-algorithm used by the blosc compressor, LZ4 in this case.
2. "clevel" &mdash; the compression level, 5 in this case.
3. "shuffle" &mdash; is the input shuffled before compression, yes (1) in this case.

NCZarr has four constraints that must be met.

1. It must store its filter information in its metadata in the above JSON dictionary format.
2. It is required to re-use the HDF5 filter implementations.
This is to avoid having to rewrite the filter implementations
This means that some mechanism is needed to translate between the HDF5 id+parameter model and the Zarr JSON dictionary model.
3. It must be possible to modify the set of visible parameters in response to environment information such as the type of the associated variable; this is required to mimic the corresponding HDF5 capability.
4. It must be possible to use filters even if HDF5 support is disabled.

Note that the term "visible parameters" is used here to refer to the parameters provided by `nc_def_var_filter` or those stored in the dataset's metadata as provided by the JSON codec. The term "working parameters" refers to the parameters given to the compressor itself and derived from the visible parameters.

The standard authority for defining Zarr filters is the list supported by the NumCodecs project [7].
Comparing the set of standard filters (aka codecs) defined by NumCodecs to the set of standard filters defined by HDF5 [3], it can be seen that the two sets overlap, but each has filters not defined by the other.

Note also that it is undesirable that a specific set of filters/codecs be built into the NCZarr implementation.
Rather, it is preferable for there be some extensible way to associate the JSON with the code implementing the codec. This mirrors the plugin model used by HDF5.

The mechanism provided to address these issues is similar to that taken by HDF5.
A shared library must exist that has certain well-defined entry points that allow the NCZarr code to determine information about a Codec.
The shared library exports a well-known function name to access Codec information and relate it to a corresponding HDF5 implementation,
Note that the shared library may optionally be the same library containing the HDF5
filter processor.

### Processing Overview

There are several paths by which the NCZarr filter API is invoked.

1. The nc\_def\_var\_filter function is invoked on a variable or
(1a) the metadata for a variable is read when opening an existing variable that has associated Codecs.
2. The visible parameters are converted to a set of working parameters.
3. The filter is invoked with the working parameters.
4. The dataset is closed using the final set of visible parameters.

#### Step 1: Invoking nc\_def\_var\_filter

In this case, the filter plugin is located and the set of visible parameters (from nc\_def\_var\_filter) are provided.

#### Step 1a: Reading metadata

In this case, the codec is read from the metadata and must be converted to a visible set of HDF5 style parameters.
It is possible that this set of visible parameters differs from the set that was provided by nc\_def\_var\_filter.
If this is important, then the filter implementation is responsible for marking this difference using, for example, different number of parameters or some differing value.

#### Step 2: Convert visible parameters to working parameters

Given environmental information such as the associated variable's base type, the visible parameters
are converted to a potentially larger set of working parameters; additionally provide the opportunity
to modify the visible parameters.

#### Step 3: Invoking the filter

As chunks are read or written, the filter is repeatedly invoked using the working parameters.

#### Step 4: Closing the dataset

The visible parameters from step 2 are stored in the dataset's metadata.
It is desirable to determine if the set of visible parameters changes.
If no change is detected, then re-writing the compressor metadata may be avoided.

### Client API

Currently, there is no way to specify use of a filter via Codec through
the netcdf-c API. Rather, one must know the HDF5 id and parameters of
the filter of interest and use the functions *nc\_def\_var\_filter* and *nc\_inq\_var\_filter*.
Internally, the NCZarr code will use information about known Codecs to convert the HDF5 filter reference to the corresponding Codec.
This restriction also holds for the specification of filters in *ncgen* and *nccopy*.
This limitation may be lifted in the future.

### Special Codecs Attribute

A new special attribute is defined called *\_Codecs* in parallel to the current *\_Filters* special attribute. Its value is a string containing the JSON representation of the Codecs associated with a given variable. 
This can be especially useful when a file is unreadable because it uses a filter not available to the netcdf-c library.
That is, no implementation was found in the e.g. *HDF5\_PLUGIN\_PATH* directory.
In this case *ncdump -hs* will display the raw Codec information so that it may be possible to see what filter is missing.

### Pre-Processing Filter Libraries

The process for using filters for NCZarr is defined to operate in several steps.
First, as with HDF5, all shared libraries in a specified directory
(e.g. *HDF5\_PLUGIN\_PATH*) are scanned.
They are interrogated to see what kind of library they implement, if any.
This interrogation operates by seeing if certain well-known (function) names are defined in this library.

There will be two library types:

1. HDF5 &mdash; exports a specific API: `H5Z_plugin_type` and `H5Z_get_plugin_info`.
2. Codec &mdash; exports a specific API: `NCZ_get_codec_info`

Note that a given library can export either or both of these APIs.
This means that we can have three types of libraries:

1. HDF5 only
2. Codec only
3. HDF5 + Codec

Suppose that our *HDF5\_PLUGIN\_PATH* location has an HDF5-only library.
Then by adding a corresponding, separate, Codec-only library to that same location, it is possible to make an HDF5 library usable by NCZarr.
It is possible to do this without having to modify the HDF5-only library.
Over time, it is possible to merge an HDF5-only library with a Codec-only library to produce a single, combined library.

### Using Plugin Libraries

The netcdf-c library processes all of the shared libraries by interrogating each one for the well-known APIs and recording the result.
Any libraries that do not export one or both of the well-known APIs is ignored.

Internally, the netcdf-c library pairs up each HDF5 library API with a corresponding Codec API by invoking the relevant well-known functions
(See [Appendix E](#filters_appendixe).
This results in this table for associated codec and hdf5 libraries.
<table>
<tr><th>HDF5 API<th>Codec API<th>Action
<tr><td>Not defined<td>Not defined<td>Ignore
<tr><td>Defined<td>Not defined<td>Ignore
<tr><td>Defined<td>Defined<td>NCZarr usable
</table>

### Filter Defaults Library

As a special case, a shared library may be created to hold
defaults for a common set of filters.
Basically, there is a specially defined function that returns
a vector of codec APIs. These defaults are used only if
no other library provides codec information for a filter.
Currently, the defaults library provides codec defaults
for Shuffle, Fletcher32, Deflate (zlib), and SZIP.

### Using the Codec API

Given a set of filters for which the HDF5 API and the Codec API
are defined, it is then possible to use the APIs to invoke the
filters and to process the meta-data in Codec JSON format.

#### Writing an NCZarr Container

When writing, the user program will invoke the NetCDF API function *nc\_def\_var\_filter*.
This function is currently defined to operate using HDF5-style id and parameters (unsigned ints).
The netcdf-c library examines its list of known filters to find one matching the HDF5 id provided by *nc\_def\_var\_filter*.
The set of parameters provided is stored internally.
Then during writing of data, the corresponding HDF5 filter is invoked to encode the data.

When it comes time to write out the meta-data, the stored HDF5-style parameters are passed to a specific Codec function to obtain the corresponding JSON representation. Again see [Appendix E](#filters_appendixe).
This resulting JSON is then written in the NCZarr metadata. 

#### Reading an NCZarr Container

When reading, the netcdf-c library will read the metadata for a given variable and will see that some set of filters are applied to this variable.
The metadata is encoded as Codec-style JSON.

Given a JSON Codec, it is parsed to provide a JSON dictionary containing the string "id" and the set of parameters as various keys.
The netcdf-c library examines its list of known filters to find one matching the Codec "id" string.
The JSON is passed to a Codec function to obtain the corresponding HDF5-style *unsigned int* parameter vector.
These parameters are stored for later use.

### Supporting Filter Chains

HDF5 supports *filter chains*, which is a sequence of filters where the output of one filter is provided as input to the next filter in the sequence.
When encoding, the filters are executed in the "forward" direction,
while when decoding the filters are executed in the "reverse" direction.

In the Zarr meta-data, a filter chain is divided into two parts:
the "compressor" and the "filters". The former is a single JSON codec
as described above. The latter is an ordered JSON array of codecs.
So if compressor is something like
    "compressor": {"id": "c"...}
and the filters array is like this:
    "filters": [ {"id": "f1"...}, {"id": "f2"...}...{"id": "fn"...}]
then the filter chain is (f1,f2,...fn,c) with f1 being applied first and c being applied last when encoding. On decode, the filter chain is executed in the order (c,fn...f2,f1).

So, an HDF5 filter chain is divided into two parts, where the last filter in the chain is assigned as the "compressor" and the remaining
filters are assigned as the "filters".
But independent of this, each codec, whether a compressor or a filter,
is stored in the JSON dictionary form described earlier.

### Extensions

The Codec style, using JSON, has the ability to provide very complex parameters that may be hard to encode as a vector of unsigned integers.
It might be desirable to consider exporting a JSON-base API out of the netcdf-c API to support user access to this complexity.
This would mean providing some alternate version of `nc_def_var_filter` that takes a string-valued argument instead of a vector of unsigned ints.
This extension is unlikely to be implemented until a compelling use-case is encountered. 

One bad side-effect of this is that we then may have two classes of plugins.
One class can be used by both HDF5 and NCZarr, and a second class that is usable only with NCZarr.

### Using The NetCDF-C Plugins

As part of its testing, the NetCDF build process creates a number of shared libraries in the *netcdf-c/plugins* (or sometimes *netcdf-c/plugins/.libs*) directory.
If you need a filter from that set, you may be able to set *HDF5\_PLUGIN\_PATH*
to point to that directory or you may be able to copy the shared libraries out of that directory to your own location.

## Debugging {#filters_debug}

Depending on the debugger one uses, debugging plugins can be very difficult.
It may be necessary to use the old printf approach for debugging the filter itself.

One case worth mentioning is when there is a dataset that is using an unknown filter.
For this situation, you need to identify what filter(s) are used in the dataset.
This can be accomplished using this command.

    ncdump -s -h <dataset filename>

Since ncdump is not being asked to access the data (the -h flag), it can obtain the filter information without failures.
Then it can print out the filter id and the parameters as well as the Codecs (via the -s flag).

### Test Cases {#filters_TestCase}

Within the netcdf-c source tree, the directory two directories contain test cases for testing dynamic filter operation.

* *netcdf-c/nc\_test4* provides tests for testing HDF5 filters.
* *netcdf-c/nczarr\_test* provides tests for testing NCZarr filters.

These tests are disabled if *--disable-shared* or if *--disable-filter-tests* is specified
or if *--disable-plugins* is specified.

### HDF5 Example {#filters_Example}

A slightly simplified version of one of the HDF5 filter test cases is also available as an example within the netcdf-c source tree directory *netcdf-c/examples/C*.
The test is called *filter\_example.c* and it is executed as part of the *run\_examples4.sh* shell script.
The test case demonstrates dynamic filter writing and reading.

The files *example/C/hdf5plugins/Makefile.am* and  *example/C/hdf5plugins/CMakeLists.txt* demonstrate how to build the hdf5 plugin for bzip2.

## Notes

### Order of Invocation for Multiple Filters 

When multiple filters are defined on a variable, the order of application, when writing data to the file, is same as the order in which *nc\_def\_var\_filter*is called.
When reading a file the order of application is of necessity the reverse.

There are some special cases.

1. The fletcher32 filter is always applied first, if enabled.
2. If *nc\_def\_var\_filter*or *nc\_def\_var\_deflate*or *nc\_def\_var\_szip*is called multiple times with the same filter id, but possibly with different sets of parameters, then the position of that filter in the sequence of applictions does not change.
    However the last set of parameters specified is used when actually writing the dataset.
3. Deflate and shuffle &mdash; these two are inextricably linked in the current API, but have quite different semantics.
    If you call *nc\_def\_var\_deflate*multiple times, then the previous rule applies with respect to deflate.
    However, the shuffle filter, if enabled, is *always* applied before applying any other filters, except fletcher32.
4. Once a filter is defined for a variable, it cannot be removed nor can its position in the filter order be changed.

### Memory Allocation Issues

Starting with HDF5 version 1.10.*, the plugin code MUST be careful when using the standard *malloc()*, *realloc()*, and *free()* function.

In the event that the code is allocating, reallocating, for
free'ing memory that either came from or will be exported to the
calling HDF5 library, then one MUST use the corresponding HDF5
functions *H5allocate\_memory()*, *H5resize\_memory()*,
*H5free\_memory()* [5] to avoid memory failures.

Additionally, if your filter code leaks memory, then the HDF5 library generates a failure something like this.

    H5MM.c:232: H5MM_final_sanity_check: Assertion `0 == H5MM_curr_alloc_bytes_s' failed.

One can look at the the code in plugins/H5Zbzip2.c and H5Zmisc.c as illustrations.

### SZIP Issues

The current szip plugin code in the HDF5 library has some behaviors that can catch the unwary.
These are handled internally to (mostly) hide them so that they should not affect users.
Specifically, this filter may do two things.

1. Add extra parameters to the filter parameters: going from the two parameters provided by the user to four parameters for internal use.
    It turns out that the two parameters provided when calling nc\_def\_var\_filter correspond to the first two parameters of the four parameters returned by nc\_inq\_var\_filter.
2. Change the values of some parameters: the value of the *options\_mask* argument is known to add additional flag bits, and the *pixels\_per\_block* parameter may be modified.

The reason for these changes is has to do with the fact that the szip API provided by the underlying H5Pset\_szip function is actually a subset of the capabilities of the real szip implementation.
Presumably this is for historical reasons.

In any case, if the caller uses the *nc\_inq\_var\_szip* or the *nc\_inq\_var\_filter* functions, then the parameter values returned may differ from those originally specified.

It should also be noted that the HDF5 szip filter wrapper that
is invoked depends on the configuration of the netcdf-c library.
If the HDF5 installation supports szip, then the NCZarr szip
will use the HDF5 wrapper. If HDF5 does not support szip, or HDF5
is not enabled, then the plugins directory will contain a local
HDF5 szip wrapper to be used by NCZarr. This can be confusing,
but is generally transparent to the use since the plugins
HDF5 szip wrapper was taken from the HDF5 code base.

### Supported Systems

The current matrix of OS X build systems known to work is as follows.
<table>
<tr><th>Build System<th>Supported OS
<tr><td>Automake<td>Linux, Cygwin, OSX
<tr><td>Cmake<td>Linux, Cygwin, OSX, Visual Studio
</table>

### Generic Plugin Build
If you do not want to use Automake or Cmake, the following has been known to work.

    gcc -g -O0 -shared -o libbzip2.so <plugin source files>  -L${HDF5LIBDIR} -lhdf5\_hl -lhdf5 -L${ZLIBDIR} -lz

## References {#filters_References}

1. https://support.hdfgroup.org/HDF5/doc/Advanced/DynamicallyLoadedFilters/HDF5DynamicallyLoadedFilters.pdf
2. https://support.hdfgroup.org/HDF5/doc/TechNotes/TechNote-HDF5-CompressionTroubleshooting.pdf
3. https://portal.hdfgroup.org/display/support/Registered+Filter+Plugins
4. https://support.hdfgroup.org/services/contributions.html#filters
5. https://support.hdfgroup.org/HDF5/doc/RM/RM\_H5.html
6. https://confluence.hdfgroup.org/display/HDF5/Filters
7. https://numcodecs.readthedocs.io/en/stable/
8. https://github.com/ccr/ccr
9. https://escholarship.org/uc/item/7xd1739k

## Appendix A. HDF5 Parameter Encode/Decode {#filters_appendixa}

The filter id for an HDF5 format filter is an unsigned integer.
Further, the parameters passed to an HDF5 format filter are encoded internally as a vector of 32-bit unsigned integers.
It may be that the parameters required by a filter can naturally be encoded as unsigned integers.
The bzip2 compression filter, for example, expects a single integer value from zero thru nine.
This encodes naturally as a single unsigned integer.

Note that signed integers and single-precision (32-bit) float values also can easily be represented as 32 bit unsigned integers by proper casting to an unsigned integer so that the bit pattern is preserved.
Simple signed integer values of type short or char can also be mapped to an unsigned integer by truncating to 16 or 8 bits respectively and then sign extending. Similarly, unsigned 8 and 16 bit
values can be used with zero extensions.

Machine byte order (aka endian-ness) is an issue for passing some kinds of parameters.
You might define the parameters when compressing on a little endian machine, but later do the decompression on a big endian machine.

When using HDF5 format filters, byte order is not an issue for 32-bit values because HDF5 takes care of converting them between the local machine byte order and network byte order.

Parameters whose size is larger than 32-bits present a byte order problem.
This specifically includes double precision floats and (signed or unsigned) 64-bit integers.
For these cases, the machine byte order issue must be handled, in part, by the compression code.
This is because HDF5 will treat, for example, an unsigned long long as two 32-bit unsigned integers and will convert each to network order separately.
This means that on a machine whose byte order is different than the machine in which the parameters were initially created, the two integers will be separately
endian converted.
But this will be incorrect for 64-bit values.

So, we have this situation (for HDF5 only):

1. the 8 bytes start as native machine order for the machine doing the call to *nc\_def\_var\_filter*.
2. The caller divides the 8 bytes into 2 four byte pieces and passes them to *nc\_def\_var\_filter*.
3. HDF5 takes each four byte piece and ensures that each piece is in network (big) endian order.
4. When the filter is called, the two pieces are returned in the same order but with the bytes in each piece consistent with the native machine order for the machine executing the filter.

### Encoding Algorithms for HDF5

In order to properly extract the correct 8-byte value, we need to ensure that the values stored in the HDF5 file have a known format independent of the native format of the creating machine.

The idea is to do sufficient manipulation so that HDF5 will store the 8-byte value as a little endian value divided into two 4-byte integers.
Note that little-endian is used as the standard because it is the most common machine format.
When read, the filter code needs to be aware of this convention and do the appropriate conversions.

This leads to the following set of rules.

#### Encoding 

1. Encode on little endian (LE) machine: no special action is required.
   The 8-byte value is passed to HDF5 as two 4-byte integers.
   HDF5 byte swaps each integer and stores it in the file.
2. Encode on a big endian (BE) machine: several steps are required:

   1. Do an 8-byte byte swap to convert the original value to little-endian format.
   2. Since the encoding machine is BE, HDF5 will just store the value.
      So it is necessary to simulate little endian encoding by byte-swapping each 4-byte integer separately. 
   3. This doubly swapped pair of integers is then passed to HDF5 and is stored unchanged.

#### Decoding 

1. Decode on LE machine: no special action is required.
    HDF5 will get the two 4-bytes values from the file and byte-swap each separately.
    The concatenation of those two integers will be the expected LE value.
2. Decode on a big endian (BE) machine: the inverse of the encode case must be implemented.

   1. HDF5 sends the two 4-byte values to the filter.
   2. The filter must then byte-swap each 4-byte value independently.
   3. The filter then must concatenate the two 4-byte values into a single 8-byte value.
    Because of the encoding rules, this 8-byte value will be in LE format.
   4. The filter must finally do an 8-byte byte-swap on that 8-byte value to convert it to desired BE format.

To support these rules, some utility programs exist and are discussed in [Appendix B](#filters_appendixb).

## Appendix B. Support Utilities {#filters_appendixb}

Several functions are exported from the netcdf-c library for use by client programs and by filter implementations.
They are defined in the header file *netcdf\_aux.h*.
The h5 tag indicates that they assume that the result of the parse is a set of unsigned integers &mdash; the format used by HDF5.

1. *int ncaux\_h5filterspec\_parse(const char* txt, unsigned int* idp. size\_t* nparamsp, unsigned int** paramsp);*
  * txt contains the text of a sequence of comma separated constants
  * idp will contain the first constant &mdash; the filter id
  * nparamsp will contain the number of params 
  * paramsp will contain a vector of params &mdash; the caller must free
This function can parse single filter spec strings as defined in the section on [Filter Specification Syntax](#filters_syntax).
2. *int ncaux\_h5filterspec\_parselist(const char* txt, int* formatp, size\_t* nspecsp, struct NC\_H5\_Filterspec*** vectorp);*
  * txt contains the text of a sequence '|' separated filter specs.
  * formatp currently always returns 0.
  * nspecsp will return the number of filter specifications.
  * vectorp will return a pointer to a vector of pointers to filter specification instances &mdash; the caller must free.
This function parses a sequence of filter specifications each separated by a '|' character.
The text between '|' separators must be parsable by *ncaux\_h5filterspec\_parse*.
3. *void ncaux\_h5filterspec\_free(struct NC\_H5\_Filterspec* f);*
  * f is a pointer to an instance of *struct NC\_H5\_Filterspec*
  Typically this was returned as an element of the vector returned
  by *\_ncaux\_h5filterspec\_parselist*.   
This reclaims the parameters of the filter spec object as well as the object itself.
4. *int ncaux\_h5filterspec\_fix8(unsigned char* mem8, int decode);*
  * mem8 is a pointer to the 8-byte value either to fix.
  * decode is 1 if the function should apply the 8-byte decoding algorithm
  else apply the encoding algorithm.
This function implements the 8-byte conversion algorithms for HDF5.
Before calling *nc\_def\_var\_filter* (unless *NC\_parsefilterspec* was used), the client must call this function with the decode argument set to 0.
Inside the filter code, this function should be called with the decode argument set to 1.

Examples of the use of these functions can be seen in the test program *nc\_test4/tst\_filterparser.c*.

Some of the above functions use a C struct defined in *netcdf\_filter.h\_.
The definition of that struct is as follows.
````
typedef struct NC_H5_Filterspec {
    unsigned int filterid; /* ID for arbitrary filter. */
    size_t nparams;        /* nparams for arbitrary filter. */
    unsigned int* params;  /* Params for arbitrary filter. */
} NC_H5_Filterspec;
````
This struct in effect encapsulates all of the information about and HDF5 formatted filter &mdash; the id, the number of parameters, and the parameters themselves. 

## Appendix C. Build Flags for Detecting the Filter Mechanism {#filters_appendixc}

The include file *netcdf\_meta.h* contains the following definition.
````
    #define NC_HAS_MULTIFILTERS   1
````
This, in conjunction with the error code *NC\_ENOFILTER* in *netcdf.h* can be used to see what filter mechanism is in place as described in the section on [incompatibities](#filters_compatibility).

1. !defined(NC\_ENOFILTER) && !defined(NC\_HAS\_MULTIFILTERS) &mdash; indicates that the old pre-4.7.4 mechanism is in place.
    It does not support multiple filters.
2. defined(NC\_ENOFILTER) && !defined(NC\_HAS\_MULTIFILTERS) &mdash; indicates that the 4.7.4 mechanism is in place.
    It does support multiple filters, but the error return codes for *nc\_inq\_var\_filter* are different and the filter spec parser functions are in a different location with different names.
3. defined(NC\_ENOFILTER) && defined(NC\_HAS\_MULTIFILTERS) &mdash; indicates that the multiple filters are supported, and that *nc\_inq\_var\_filter* returns a filterid of zero to indicate that a variable has no filters.
    Also, the filter spec parsers have the names and signatures described in this document and are define in *netcdf\_aux.h*.

## Appendix D. BNF for Specifying Filters in Utilities {#filters_appendixd}

````
speclist:   spec
          | speclist '|' spec
          ;
spec:   filterid
      | filterid ',' parameterlist
      ;
filterid:   unsigned32
          ;
parameterlist:   parameter
               | parameterlist ',' parameter
               ;
parameter: unsigned32

where
unsigned32: <32 bit unsigned integer>
````

## Appendix E. Codec API {#filters_appendixe}

The Codec API mirrors the HDF5 API closely. It has one well-known function that can be invoked to obtain information about the Codec as well as pointers to special functions to perform conversions.

### The Codec Plugin API

#### NCZ\_get\_codec\_info

This function returns a pointer to a C struct that provides detailed information about the codec plugin.

##### Signature
````
    void* NCZ_get_codec_info(void);
````
The value returned is actually of type *struct NCZ\_codec\_t*,
but is of type *void\** to allow for extensions.

#### NCZ\_codec\_t
````
typedef struct NCZ_codec_t {
    int version; /* Version number of the struct */
    int sort; /* Format of remainder of the struct;
                 Currently always NCZ_CODEC_HDF5 */
    const char* codecid;            /* The name/id of the codec */
    unsigned int hdf5id; /* corresponding hdf5 id */
    void (*NCZ_codec_initialize)(void);
    void (*NCZ_codec_finalize)(void);
    int (*NCZ_codec_to_hdf5)(const char* codec, int* nparamsp, unsigned** paramsp);
    int (*NCZ_hdf5_to_codec)(size_t nparams, const unsigned* params, char** codecp);
    int (*NCZ_modify_parameters)(int ncid, int varid, size_t* vnparamsp, unsigned** vparamsp, size_t* nparamsp, unsigned** paramsp);
} NCZ_codec_t;
````

The semantics of the non-function fields is as follows:

1. *version* &mdash; Version number of the struct.
2. *sort* &mdash; Format of remainder of the struct; currently always NCZ\_CODEC\_HDF5.
3. *codecid* &mdash; The name/id of the codec.
4. *hdf5id* &mdash;  The corresponding hdf5 id.

#### NCZ\_codec\_to\_hdf5

Given a JSON Codec representation, it will return a corresponding vector of unsigned integers representing the
visible parameters.

##### Signature
````
    int NCZ_codec_to_hdf(const char* codec, int* nparamsp, unsigned** paramsp);
````
##### Arguments
1. codec &mdash; (in) ptr to JSON string representing the codec.
2. nparamsp &mdash; (out) store the length of the converted HDF5 unsigned vector
3. paramsp &mdash; (out) store a pointer to the converted HDF5 unsigned vector; caller must free the returned vector. Note the double indirection.

Return Value: a netcdf-c error code.

#### NCZ\_hdf5\_to\_codec

Given an HDF5 visible parameters vector of unsigned integers and its length,
return a corresponding JSON codec representation of those visible parameters.

##### Signature
````
    int NCZ_hdf5_to_codec)(int ncid, int varid, size_t nparams, const unsigned* params, char** codecp);
````
##### Arguments

1. ncid    &mdash; the variables' containing group
2. varid   &mdash; the containing variable
3. nparams &mdash; (in) the length of the HDF5 visible parameters vector
4. params &mdash; (in) pointer to the HDF5 visible parameters vector.
5. codecp &mdash; (out) store the string representation of the codec; caller must free.

Return Value: a netcdf-c error code.

#### NCZ\_modify\_parameters

Extract environment information from the (ncid,varid) and use it to convert a set of visible parameters
to a set of working parameters; also provide option to modify visible parameters.

##### Signature
````
    int NCZ_modify_parameters(int ncid, int varid, size_t* vnparamsp, unsigned** vparamsp, size_t* wnparamsp, unsigned** wparamsp);
````
##### Arguments

1. ncid &mdash; (in) group id containing the variable.
2. varid &mdash; (in) the id of the variable to which this filter is being attached.
3. vnparamsp &mdash; (in/out) the count of visible parameters
4. vparamsp &mdash; (in/out) the set of visible parameters
5. wnparamsp &mdash; (out) the count of working parameters
4. wparamsp &mdash; (out) the set of working parameters

Return Value: a netcdf-c error code.

#### NCZ\_codec\_initialize

Some compressors may require library initialization.
This function is called as soon as a shared library is loaded and matched with an HDF5 filter.

##### Signature
````
    int NCZ_codec_initialize)(void);
````
Return Value: a netcdf-c error code.

#### NCZ\_codec\_finalize

Some compressors (like blosc) require invoking a finalize function in order to avoid memory loss.
This function is called during a call to *nc\_finalize* to do any finalization.
If the client code does not invoke *nc\_finalize* then memory checkers may complain about lost memory.

##### Signature
````
    int NCZ_codec_finalize)(void);
````
Return Value: a netcdf-c error code.

### Multi-Codec API

As an aid to clients, it is convenient if a single shared library can provide multiple *NCZ\_code\_t* instances at one time.
This API is not intended to be used by plugin developers.
A shared library must only export this function.

#### NCZ\_codec\_info\_defaults

Return a NULL terminated vector of pointers to instances of *NCZ\_codec\_t*.

##### Signature
````
    void* NCZ_codec_info_defaults(void);
````
The value returned is actually of type *NCZ\_codec\_t***,
but is of type *void** to allow for extensions.
The list of returned items are used to try to provide defaults
for any HDF5 filters that have no corresponding Codec.
This is for internal use only.

## Appendix F. Standard Filters  {#filters_appendixf}

Support for a select set of standard filters is built into the NetCDF API.
Generally, they are accessed using the following generic API, where XXXX is
the filter name. As a rule, the names are those used in the HDF5 filter ID naming authority [4] or the NumCodecs naming authority [7].
````
int nc_def_var_XXXX(int ncid, int varid, unsigned filterid, size_t nparams, unsigned* params);
int nc_inq_var_XXXX(int ncid, int varid, int* hasfilter, size_t* nparamsp, unsigned* params);
````
The first function inserts the specified filter into the filter chain for a given variable.
The second function queries the given variable to see if the specified function
is in the filter chain for that variable. The *hasfilter* argument is set
to one if the filter is in the chain and zero otherwise.
As is usual with the netcdf API, one is expected to call this function twice.
The first time to set *nparamsp* and the second to get the parameters in the client-allocated memory argument *params*.
Any of these arguments can be NULL, in which case no value is returned.

Note that NetCDF inherits four filters from HDF5, namely shuffle, fletcher32, deflate (zlib), and szip. The API's for these do not conform to the above API.
So aside from those four, the current set of standard filters is as follows.
<table>
<tr><th>Filter Name<th>Filter ID<th>Reference
<tr><td>zstandard<td>32015<td>https://facebook.github.io/zstd/
<tr><td>bzip2<td>307<td>https://sourceware.org/bzip2/
</table>

It is important to note that in order to use each standard filter, several additonal libraries must be installed.
Consider the zstandard compressor, which is one of the supported standard filters.
When installing the netcdf library, the following other libraries must be installed.

1. *libzstd.so* | *zstd.dll* | *libzstd.dylib* -- The actual zstandard compressor library; typically installed by using your platform specific package manager.
2. The HDF5 wrapper for *libzstd.so* -- There are several options for obtaining this (see [Appendix G](#filters_appendixg).)
3. (Optional) The Zarr wrapper for *libzstd.so* -- you need this if you intend to read/write Zarr datasets that were compressed using zstandard; again see [Appendix G](#filters_appendixg).

## Appendix G. Finding Filters  {#filters_appendixg}

A major problem for filter users is finding an implementation of an HDF5 filter wrapper and (optionally)
its corresponding NCZarr wrapper. There are several ways to do this.

* **--with-plugin-dir** &mdash; An option to *./configure* that will install the necessary wrappers.
  See [Appendix H](#filters_appendixh).

* **HDF5 Assigned Filter Identifiers Repository [3]** &mdash;
HDF5 maintains a page of standard filter identifiers along with
additional contact information. This often includes a pointer
to source code. This will provide only HDF5 wrappers and not NCZarr wrappers.

* **Community Codec Repository** &mdash;
The Community Codec Repository (CCR) project [8] provides
filters, including HDF5 wrappers, for a number of filters.
It does not as yet provide Zarr wrappers.
You can install this library to get access to these supported filters.
It does not currently include the required NCZarr Codec API,
so they are only usable with netcdf-4. This will change in the future.

## Appendix H. Auto-Install of Filter Wrappers {#filters_appendixh}

As part of the overall build process, a number of filter wrappers are built as shared libraries in the "plugins" directory.
These wrappers can be installed as part of the overall netcdf-c installation process.
WARNING: the installer still needs to make sure that the actual filter/compression libraries are installed: e.g. libzstd and/or libblosc.

The target location into which libraries in the "plugins" directory are installed is specified
using a special *./configure* option
````
--with-plugin-dir=<directorypath>
or
--with-plugin-dir
````
or its corresponding *cmake* option.
````
-DPLUGIN_INSTALL_DIR=<directorypath>
or
-DPLUGIN_INSTALL_DIR=YES
````
This option defaults to the value "yes", which means that filters are
installed by default. This can be disabled by one of the following options.
````
--without-plugin-dir (automake)
or
--with-plugin-dir=no (automake)
or
-DPLUGIN_INSTALL_DIR=NO (CMake)
````

If the option is specified with no argument (automake) or with the value "YES" (CMake),
then it defaults (in order) to the following directories:
1. If the HDF5_PLUGIN_PATH environment variable is defined, then last directory in the list of directories in the path is used.
2.  (a) "/usr/local/hdf5/lib/plugin” for linux/unix operating systems (including Cygwin)<br>
    (b) “%ALLUSERSPROFILE%\\hdf5\\lib\\plugin” for Windows and MinGW

If NCZarr is enabled, then in addition to wrappers for the standard filters,
additional libraries will be installed to support NCZarr access to filters.
Currently, this list includes the following:
* shuffle &mdash; shuffle filter
* fletcher32 &mdash; fletcher32 checksum
* deflate &mdash; deflate compression
* (optional) szip &mdash; szip compression, if libsz is available
* bzip2 &mdash; an HDF5 filter for bzip2 compression
* lib__nczh5filters.so &mdash; provide NCZarr support for shuffle, fletcher32, deflate, and (optionally) szip.
* lib__nczstdfilters.so &mdash; provide NCZarr support for bzip2, (optionally)zstandard, and (optionally) blosc.

The shuffle, fletcher32, and deflate filters in this case will
be ignored by HDF5 and only used by the NCZarr code.  But in
order to use them, it needs additional Codec capabilities
provided by the *lib__nczh5filters.so* shared library.  Note also that
if you disable HDF5 support, but leave NCZarr support enabled,
then all of the above filters should continue to work.

### HDF5_PLUGIN_PATH

At the moment, NetCDF uses the existing HDF5 environment variable
*HDF5\_PLUGIN\_PATH* to locate the directories in which filter wrapper
shared libraries are located. This is used both for the HDF5 filter
wrappers but also the NCZarr codec wrappers.

*HDF5\_PLUGIN\_PATH* is a typical Windows or Unix style
path-list.  That is it is a sequence of absolute directory paths
separated by a specific separator character. For Windows, the
separator character is a semicolon (';') and for Unix, it is a a
colon (':').

So, if HDF5_PLUGIN_PATH is defined at build time, and
*--with-plugin-dir* is specified with no argument then the last
directory in the path will be the one into which filter wrappers are
installed. Otherwise the default directories are used.

The important thing to note is that at run-time, there are several cases to consider:

1. HDF5_PLUGIN_PATH is defined and is the same value as it was at build time -- no action needed
2. HDF5_PLUGIN_PATH is defined and is has a different value from build time -- the user is responsible for ensuring that the run-time path includes the same directory used at build time, otherwise this case will fail.
3. HDF5_PLUGIN_DIR is not defined at either run-time or build-time -- no action needed
4. HDF5_PLUGIN_DIR is not defined at run-time but was defined at build-time -- this will probably fail

## Point of Contact {#filters_poc}

*Author*: Dennis Heimbigner<br>
*Email*: dmh at ucar dot edu<br>
*Initial Version*: 1/10/2018<br>
*Last Revised*: 5/18/2022
