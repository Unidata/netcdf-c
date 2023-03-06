The NetCDF NCZarr Implementation
============================
<!-- double header is needed to workaround doxygen bug -->

# The NetCDF NCZarr Implementation {#nczarr_head}

\tableofcontents

# NCZarr Introduction {#nczarr_introduction}

Beginning with netCDF version 4.8.0, the Unidata NetCDF group has extended the netcdf-c library to provide access to cloud storage (e.g. Amazon S3 <a href="#ref_aws">[1]</a> ).
This extension provides a mapping from a subset of the full netCDF Enhanced (aka netCDF-4) data model to a variant of the Zarr <a href="#ref_zarrv2">[4]</a> data model.
The NetCDF version of this storage format is called NCZarr <a href="#ref_nczarr">[4]</a>.

A note on terminology in this document.
1. The term "dataset" is used to refer to all of the Zarr objects constituting
   the meta-data and data. 

There are some important "caveats" of which to be aware when using this software.
1. NCZarr currently is not thread-safe. So any attempt to use it with parallelism, including MPIO, is likely to fail.

# The NCZarr Data Model {#nczarr_data_model}

NCZarr uses a data model <a href="#ref_nczarr">[4]</a> that, by design, extends the Zarr Version 2 Specification <a href="#ref_zarrv2">[6]</a> to add support for the NetCDF-4 data model.

__Note Carefully__: a legal _NCZarr_ dataset is also a legal _Zarr_ dataset under a specific assumption. This assumption is that within Zarr meta-data objects, like ''.zarray'', unrecognized dictionary keys are ignored.
If this assumption is true of an implementation, then the _NCZarr_ dataset is a legal _Zarr_ dataset and should be readable by that _Zarr_ implementation.
The inverse is true also. A legal _Zarr_ dataset is also a legal _NCZarr_
dataset, where "legal" means it conforms to the Zarr version 2 specification.
In addition, certain non-Zarr features are allowed and used.
Specifically the XArray ''\_ARRAY\_DIMENSIONS'' attribute is one such.

There are two other, secondary assumption:

1. The actual storage format in which the dataset is stored -- a zip file, for example -- can be read by the _Zarr_ implementation.
2. The compressors (aka filters) used by the dataset can be encoded/decoded by the implementation. NCZarr uses HDF5-style filters, so ensuring access to such filters is somewhat complicated. See [the companion document on
filters](./md_filters.html "filters") for details.

Briefly, the data model supported by NCZarr is netcdf-4 minus
the user-defined types. However, a restricted form of String type
is supported (see Appendix H).
As with netcdf-4 chunking is supported.  Filters and compression
are also [supported](./md_filters.html "filters").

Specifically, the model supports the following.
- "Atomic" types: char, byte, ubyte, short, ushort, int, uint, int64, uint64, string.
- Shared (named) dimensions
- Attributes with specified types -- both global and per-variable
- Chunking
- Fill values
- Groups
- N-Dimensional variables
- Scalar variables
- Per-variable endianness (big or little)
- Filters (including compression)

With respect to full netCDF-4, the following concepts are
currently unsupported.
- User-defined types (enum, opaque, VLEN, and Compound)
- Unlimited dimensions
- Contiguous or compact storage

Note that contiguous and compact are not actually supported
because they are HDF5 specific.
When specified, they are treated as chunked where the file consists of only one chunk.
This means that testing for contiguous or compact is not possible; the _nc_inq_var_chunking_ function will always return NC_CHUNKED and the chunksizes will be the same as the dimension sizes of the variable's dimensions.

Additionally, it should be noted that NCZarr supports scalar variables,
but Zarr does not; Zarr only supports dimensioned variables.
In order to support interoperability, NCZarr does the following.
1. A scalar variable is recorded in the Zarr metadata as if it has a shape of **[1]**.
2. A note is stored in the NCZarr metadata that this is actually a netCDF scalar variable.

These actions allow NCZarr to properly show scalars in its API while still
maintaining compatibility with Zarr.

# Enabling NCZarr Support {#nczarr_enable}

NCZarr support is enabled by default.
If the _--disable-nczarr_ option is used with './configure', then NCZarr (and Zarr) support is disabled.
If NCZarr support is enabled, then support for datasets stored as files in a directory tree is provided as the only guaranteed mechanism for storing datasets.
However, several addition storage mechanisms are available if additional libraries are installed.

1. Zip format -- if _libzip_ is installed, then it is possible to directly read and write datasets stored in zip files.
2. If the AWS C++ SDK is installed, and _libcurl_ is installed, then it is possible to directly read and write datasets stored in the Amazon S3 cloud storage.

# Accessing Data Using the NCZarr Prototocol {#nczarr_accessing_data}

In order to access a NCZarr data source through the netCDF API, the file name normally used is replaced with a URL with a specific format.
Note specifically that there is no NC_NCZARR flag for the mode argument of _nc_create_ or _nc_open_.
In this case, it is indicated by the URL path.

## URL Format
The URL is the usual format.
````
scheme:://host:port/path?query#fragment format
````
There are some details that are important.
- Scheme: this should be _https_ or _s3_,or _file_.
  The _s3_ scheme is equivalent
  to "https" plus setting "mode=nczarr,s3" (see below).
  Specifying "file" is mostly used for testing, but is used to support
  directory tree or zipfile format storage.
- Host: Amazon S3 defines three forms: _Virtual_, _Path_, and _S3_
  + _Virtual_: the host includes the bucket name as in
    __bucket.s3.&lt;region&gt;.amazonaws.com__
  + _Path_: the host does not include the bucket name, but
    rather the bucket name is the first segment of the path.
    For example __s3.&lt;region&gt;.amazonaws.com/bucket__
  + _S3_: the protocol is "s3:" and if the host is a single name,
    then it is interpreted as the bucket. The region is determined
    using the algorithm in Appendix E.
  + _Other_: It is possible to use other non-Amazon cloud storage, but
    that is cloud library dependent.
- Query: currently not used.
- Fragment: the fragment is of the form _key=value&key=value&..._.
  Depending on the key, the _value_ part may be left out and some
  default value will be used.

## Client Parameters

The fragment part of a URL is used to specify information that is interpreted to specify what data format is to be used, as well as additional controls for that data format.
For NCZarr support, the following _key=value_ pairs are allowed.

- mode=nczarr|zarr|noxarray|file|zip|s3

Typically one will specify two mode flags: one to indicate what format
to use and one to specify the way the dataset is to be stored.
For example, a common one is "mode=zarr,file"

Using _mode=nczarr_ causes the URL to be interpreted as a
reference to a dataset that is stored in NCZarr format.
The _zarr_ mode tells the library to
use NCZarr, but to restrict its operation to operate on pure
Zarr Version 2 datasets.

The modes _s3_, _file_, and _zip_ tell the library what storage
driver to use.
* The _s3_ driver is the default and indicates using Amazon S3 or some equivalent.
* The _file_ format stores data in a directory tree.
* The _zip_ format stores data in a local zip file.

Note that It should be the case that zipping a _file_
format directory tree will produce a file readable by the
_zip_ storage format, and vice-versa.

By default, the XArray convention is supported and used for
both NCZarr files and pure Zarr files. This
means that every variable in the root group whose named dimensions
are also in the root group will have an attribute called
*\_ARRAY\_DIMENSIONS* that stores those dimension names.
The _noxarray_ mode tells the library to disable the XArray support.

The netcdf-c library is capable of inferring additional mode flags based on the flags it finds. Currently we have the following inferences.
- _zarr_ => _nczarr_

So for example: ````...#mode=zarr,zip```` is equivalent to this.
````...#mode=nczarr,zarr,zip
````
<!--
- log=&lt;output-stream&gt;: this control turns on logging output,
  which is useful for debugging and testing.
If just _log_ is used
  then it is equivalent to _log=stderr_.
-->

# NCZarr Map Implementation {#nczarr_mapimpl}

Internally, the nczarr implementation has a map abstraction that allows different storage formats to be used.
This is closely patterned on the same approach used in the Python Zarr implementation, which relies on the Python _MutableMap_ <a href="#ref_python">[5]</a> class.

In NCZarr, the corresponding type is called _zmap_.
The __zmap__ API essentially implements a simplified variant
of the Amazon S3 API.

As with Amazon S3, __keys__ are utf8 strings with a specific structure:
that of a path similar to those of a Unix path with '/' as the
separator for the segments of the path.

As with Unix, all keys have this BNF syntax:
````
key: '/' | keypath ;
keypath: '/' segment | keypath '/' segment ;
segment: <sequence of UTF-8 characters except control characters and '/'>
````
Obviously, one can infer a tree structure from this key structure.
A containment relationship is defined by key prefixes.
Thus one key is "contained" (possibly transitively)
by another if one key is a prefix (in the string sense) of the other.
So in this sense the key "/x/y/z" is contained by the key  "/x/y".

In this model all keys "exist" but only some keys refer to
objects containing content -- aka _content bearing_.
An important restriction is placed on the structure of the tree,
namely that keys are only defined for content-bearing objects.
Further, all the leaves of the tree are these content-bearing objects.
This means that the key for one content-bearing object should not
be a prefix of any other key.

There several other concepts of note.
1. __Dataset__ - a dataset is the complete tree contained by the key defining
the root of the dataset.
Technically, the root of the tree is the key \<dataset\>/.zgroup, where .zgroup can be considered the _superblock_ of the dataset.
2. __Object__ - equivalent of the S3 object; Each object has a unique key
and "contains" data in the form of an arbitrary sequence of 8-bit bytes.

The zmap API defined here isolates the key-value pair mapping
code from the Zarr-based implementation of NetCDF-4.
 It wraps an internal C dispatch table manager for implementing an
abstract data structure implementing the zmap key/object model.
Of special note is the "search" function of the API.

__Search__: The search function has two purposes:
1. Support reading of pure zarr datasets (because they do not explicitly track their contents).
2. Debugging to allow raw examination of the storage. See zdump for example.

The search function takes a prefix path which has a key syntax (see above).
The set of legal keys is the set of keys such that the key references a content-bearing object -- e.g. /x/y/.zarray or /.zgroup.
Essentially this is the set of keys pointing to the leaf objects of the tree of keys constituting a dataset.
This set potentially limits the set of keys that need to be examined during search.

The search function returns a limited set of names, where the set of names are immediate suffixes of a given prefix path.
That is, if _\<prefix\>_ is the prefix path, then search returnsnall _\<name\>_ such that _\<prefix>/\<name\>_ is itself a prefix of a "legal" key.
This can be used to implement glob style searches such as "/x/y/*" or "/x/y/**"

This semantics was chosen because it appears to be the minimum required to implement all other kinds of search using recursion.
It was also chosen to limit the number of names returned from the search.
Specifically
1. Avoid returning keys that are not a prefix of some legal key.
2. Avoid returning all the legal keys in the dataset because that set may be very large; although the implementation may still have to examine all legal keys to get the desired subset.
3. Allow for use of partial read mechanisms such as iterators, if available.
This can support processing a limited set of keys for each iteration.
This is a straighforward tradeoff of space over time.

As a side note, S3 supports this kind of search using common prefixes with a delimiter of '/', although its use is a bit tricky.
For the file system zmap implementation, the legal search keys can be obtained one level at a time, which directly implements the search semantics.
For the zip file implementation, this semantics is not possible, so the whole
tree must be obtained and searched.

__Issues:__

1. S3 limits key lengths to 1024 bytes.
Some deeply nested netcdf files will almost certainly exceed this limit.
2. Besides content, S3 objects can have an associated small set
of what may be called tags, which are themselves of the form of
key-value pairs, but where the key and value are always text.
As far as it is possible to determine, Zarr never uses these tags,
so they are not included in the zmap data structure.

__A Note on Error Codes:__

The zmap API returns some distinguished error code:
1. NC_NOERR if a operation succeeded
2. NC_EEMPTY is returned when accessing a key that has no content.
3. NC_EOBJECT is returned when an object is found which should not exist
4. NC_ENOOBJECT is returned when an object is not found which should exist

This does not preclude other errors being returned such NC_EACCESS or NC_EPERM or NC_EINVAL if there are permission errors or illegal function arguments, for example.
It also does not preclude the use of other error codes internal to the zmap implementation.
So zmap_file, for example, uses NC_ENOTFOUND internally because it is possible to detect the existence of directories and files.
But this does not propagate outside the zmap_file implementation.

## Zmap Implementatons

The primary zmap implementation is _s3_ (i.e. _mode=nczarr,s3_) and indicates that the Amazon S3 cloud storage -- or some related applicance -- is to be used.
Another storage format uses a file system tree of directories and files (_mode=nczarr,file_).
A third storage format uses a zip file (_mode=nczarr,zip_).
The latter two are used mostly for debugging and testing.
However, the _file_ and _zip_ formats are important because they are intended to match corresponding storage formats used by the Python Zarr implementation.
Hence it should serve to provide interoperability between NCZarr and the Python Zarr, although this interoperability has not been tested.

Examples of the typical URL form for _file_ and _zip_ are as follows.
````
file:///xxx/yyy/testdata.file#mode=nczarr,file
file:///xxx/yyy/testdata.zip#mode=nczarr,zip
````

Note that the extension (e.g. ".file" in "testdata.file")
is arbitraty, so this would be equally acceptable.
````
file:///xxx/yyy/testdata.anyext#mode=nczarr,file
````
As with other URLS (e.g. DAP), these kind of URLS can be passed as the path argument to, for example, __ncdump__.

# NCZarr versus Pure Zarr. {#nczarr_purezarr}

The NCZARR format extends the pure Zarr format by adding extra keys such as ''\_NCZARR\_ARRAY'' inside the ''.zarray'' object.
It is possible to suppress the use of these extensions so that the netcdf library can read and write a pure zarr formatted file.
This is controlled by using ''mode=zarr'', which is an alias for the
''mode=nczarr,zarr'' combination.
The primary effects of using pure zarr are described in the [Translation Section](@ref nczarr_translation).

There are some constraints on the reading of Zarr datasets using the NCZarr implementation.

1. Zarr allows some primitive types not recognized by NCZarr.
Over time, the set of unrecognized types is expected to diminish.
Examples of currently unsupported types are as follows:
  * "c" -- complex floating point
  * "m" -- timedelta
  * "M" -- datetime
2. The Zarr dataset may reference filters and compressors unrecognized by NCZarr.
3. The Zarr dataset may store data in column-major order instead of row-major order. The effect of encountering such a dataset is to output the data in the wrong order.

Again, this list should diminish over time.

# Notes on Debugging NCZarr Access {#nczarr_debug}

The NCZarr support has a trace facility.
Enabling this can sometimes give important, but voluminous information.
Tracing can be enabled by setting the environment variable NCTRACING=n,
where _n_ indicates the level of tracing.
A good value of _n_ is 9.

# Zip File Support {#nczarr_zip}

In order to use the _zip_ storage format, the libzip [3] library must be installed.
Note that this is different from zlib.

# Amazon S3 Storage {#nczarr_s3}

The Amazon AWS S3 storage driver currently uses the Amazon AWS S3 Software Development Kit for C++ (aws-s3-sdk-cpp).
In order to use it, the client must provide some configuration information.
Specifically, the ''~/.aws/config'' file should contain something like this.

```
[default]
output = json
aws_access_key_id=XXXX...
aws_secret_access_key=YYYY...
```
See Appendix E for additional information.

## Addressing Style

The notion of "addressing style" may need some expansion.
Amazon S3 accepts two forms for specifying the endpoint for accessing the data.

1. Virtual -- the virtual addressing style places the bucket in the host part of a URL.
For example:
```
https://<bucketname>.s2.&lt;region&gt.amazonaws.com/
```
2. Path -- the path addressing style places the bucket in at the front of the path part of a URL.
For example:
```
https://s2.&lt;region&gt.amazonaws.com/<bucketname>/
```

The NCZarr code will accept either form, although internally, it is standardized on path style.
The reason for this is that the bucket name forms the initial segment in the keys.

# Zarr vs NCZarr {#nczarr_vs_zarr}

## Data Model

The NCZarr storage format is almost identical to that of the the standard Zarr version 2 format.
The data model differs as follows.

1. Zarr only supports anonymous dimensions -- NCZarr supports only shared (named) dimensions.
2. Zarr attributes are untyped -- or perhaps more correctly characterized as of type string.

## Storage Format

Consider both NCZarr and Zarr, and assume S3 notions of bucket and object.
In both systems, Groups and Variables (Array in Zarr) map to S3 objects.
Containment is modeled using the fact that the dataset's key is a prefix of the variable's key.
So for example, if variable _v1_ is contained in top level group g1 -- _/g1 -- then the key for _v1_ is _/g1/v_.
Additional meta-data information is stored in special objects whose name start with ".z".

In Zarr, the following special objects exist.

1. Information about a group is kept in a special object named _.zgroup_;
so for example the object _/g1/.zgroup_.
2. Information about an array is kept as a special object named _.zarray_;
so for example the object _/g1/v1/.zarray_.
3. Group-level attributes and variable-level attributes are stored in a special object named _.zattr_;
so for example the objects _/g1/.zattr_ and _/g1/v1/.zattr_.
4. Chunk data is stored in objects named "\<n1\>.\<n2\>...,\<nr\>" where the ni are positive integers representing the chunk index for the ith dimension.

The first three contain meta-data objects in the form of a string representing a JSON-formatted dictionary. 
The NCZarr format uses the same objects as Zarr, but inserts NCZarr
specific key-value pairs in them to hold NCZarr specific information
The value of each of these keys is a JSON dictionary containing a variety
of NCZarr specific information.

These keys are as follows:

_\_nczarr_superblock\__ -- this is in the top level group -- key _/.zarr_.
It is in effect the "superblock" for the dataset and contains
any netcdf specific dataset level information.
It is also used to verify that a given key is the root of a dataset.
Currently it contains the following key(s):
* "version" -- the NCZarr version defining the format of the dataset.

_\_nczarr_group\__ -- this key appears in every _.zgroup_ object.
It contains any netcdf specific group information.
Specifically it contains the following keys:
* "dims" -- the name and size of shared dimensions defined in this group.
* "vars" -- the name of variables defined in this group.
* "groups" -- the name of sub-groups defined in this group.
These lists allow walking the NCZarr dataset without having to use the potentially costly search operation.

_\_nczarr_array\__ -- this key appears in every _.zarray_ object.
It contains netcdf specific array information.
Specifically it contains the following keys:
* dimrefs -- the names of the shared dimensions referenced by the variable.
* storage -- indicates if the variable is chunked vs contiguous in the netcdf sense.

_\_nczarr_attr\__ -- this key appears in every _.zattr_ object.
This means that technically, it is attribute, but one for which access
is normally surpressed .
Specifically it contains the following keys:
* types -- the types of all of the other attributes in the _.zattr_ object.

## Translation {#nczarr_translation}

With some constraints, it is possible for an nczarr library to read Zarr and for a zarr library to read the nczarr format.
The latter case, zarr reading nczarr is possible if the zarr library is willing to ignore keys whose name it does not recognize; specifically anything beginning with _\_NCZARR\__.

The former case, nczarr reading zarr is also possible if the nczarr can simulate or infer the contents of the missing _\_NCZARR\_XXX_ objects.
As a rule this can be done as follows.
1. _\_nczarr_group\__ -- The list of contained variables and sub-groups can be computed using the search API to list the keys "contained" in the key for a group.
The search looks for occurrences of _.zgroup_, _.zattr_, _.zarray_ to infer the keys for the contained groups, attribute sets, and arrays (variables).
Constructing the set of "shared dimensions" is carried out
by walking all the variables in the whole dataset and collecting
the set of unique integer shapes for the variables.
For each such dimension length, a top level dimension is created
named  ".zdim_<len>" where len is the integer length.
2. _\_nczarr_array\__ -- The dimrefs are inferred by using the shape
in _.zarray_ and creating references to the simulated shared dimension.
netcdf specific information.
3. _\_nczarr_attr\__ -- The type of each attribute is inferred by trying to parse the first attribute value string.

# Compatibility {#nczarr_compatibility}

In order to accomodate existing implementations, certain mode tags are provided to tell the NCZarr code to look for information used by specific implementations.

## XArray

The Xarray <a href="#ref_xarray">[7]</a> Zarr implementation uses its own mechanism for specifying shared dimensions.
It uses a special attribute named ''_ARRAY_DIMENSIONS''.
The value of this attribute is a list of dimension names (strings).
An example might be ````["time", "lon", "lat"]````.
It is essentially equivalent to the ````_nczarr_array "dimrefs" list````, except that the latter uses fully qualified names so the referenced dimensions can be anywhere in the dataset.

As of _netcdf-c_ version 4.8.2, The Xarray ''_ARRAY_DIMENSIONS'' attribute is supported for both NCZarr and pure Zarr.
If possible, this attribute will be read/written by default,
but can be suppressed if the mode value "noxarray" is specified.
If detected, then these dimension names are used to define shared dimensions.
The following conditions will cause ''_ARRAY_DIMENSIONS'' to not be written.
* The variable is not in the root group,
* Any dimension referenced by the variable is not in the root group.

# Examples {#nczarr_examples}

Here are a couple of examples using the _ncgen_ and _ncdump_ utilities.

1. Create an nczarr file using a local directory tree as storage.
    ```
    ncgen -4 -lb -o "file:///home/user/dataset.file#mode=nczarr,file" dataset.cdl
    ```
2. Display the content of an nczarr file using a zip file as storage.
    ```
    ncdump "file:///home/user/dataset.zip#mode=nczarr,zip"
    ```
3. Create an nczarr file using S3 as storage.
    ```
    ncgen -4 -lb -o "s3://s3.us-west-1.amazonaws.com/datasetbucket" dataset.cdl
    ```
4. Create an nczarr file using S3 as storage and keeping to the pure zarr format.
    ```
    ncgen -4 -lb -o "s3://s3.uswest-1.amazonaws.com/datasetbucket#mode=zarr" dataset.cdl
    ```
5. Create an nczarr file using the s3 protocol with a specific profile
    ```
    ncgen -4 -lb -o "s3://datasetbucket/rootkey#mode=nczarr,awsprofile=unidata" dataset.cdl
    ```
    Note that the URLis internally translated to this
    ````
    https://s2.&lt;region&gt.amazonaws.com/datasetbucket/rootkey#mode=nczarr,awsprofile=unidata" dataset.cdl
    ````
    The region is from the algorithm described in Appendix E1.

# References {#nczarr_bib}

<a name="ref_aws">[1]</a> [Amazon Simple Storage Service Documentation](https://docs.aws.amazon.com/s3/index.html)<br>
<a name="ref_awssdk">[2]</a> [Amazon Simple Storage Service Library](https://github.com/aws/aws-sdk-cpp)<br>
<a name="ref_libzip">[3]</a> [The LibZip Library](https://libzip.org/)<br>
<a name="ref_nczarr">[4]</a> [NetCDF ZARR Data Model Specification](https://www.unidata.ucar.edu/blogs/developer/en/entry/netcdf-zarr-data-model-specification)<br>
<a name="ref_python">[5]</a> [Python Documentation: 8.3.
collections — High-performance dataset datatypes](https://docs.python.org/2/library/collections.html)<br>
<a name="ref_zarrv2">[6]</a> [Zarr Version 2 Specification](https://zarr.readthedocs.io/en/stable/spec/v2.html)<br>
<a name="ref_xarray">[7]</a> [XArray Zarr Encoding Specification](http://xarray.pydata.org/en/latest/internals.html#zarr-encoding-specification)<br>
<a name="dynamic_filter_loading">[8]</a> [Dynamic Filter Loading](https://support.hdfgroup.org/HDF5/doc/Advanced/DynamicallyLoadedFilters/HDF5DynamicallyLoadedFilters.pdf)<br>
<a name="official_hdf5_filters">[9]</a> [Officially Registered Custom HDF5 Filters](https://portal.hdfgroup.org/display/support/Registered+Filter+Plugins)<br>
<a name="blosc-c-impl">[10]</a> [C-Blosc Compressor Implementation](https://github.com/Blosc/c-blosc)<br>
<a name="ref_awssdk_conda">[11]</a> [Conda-forge / packages / aws-sdk-cpp](https://anaconda.org/conda-forge/aws-sdk-cpp)<br>
<a name="ref_gdal">[12]</a> [GDAL Zarr](https://gdal.org/drivers/raster/zarr.html)<br>

# Appendix A. Building NCZarr Support {#nczarr_build}

Currently the following build cases are known to work.

<table>
<tr><td><u>Operating System</u><td><u>Build System</u><td><u>NCZarr</u><td><u>S3 Support</u>
<tr><td>Linux            <td> Automake     <td> yes          <td> yes
<tr><td>Linux            <td> CMake        <td> yes          <td> yes
<tr><td>Cygwin           <td> Automake     <td> yes          <td> no
<tr><td>OSX              <td> Automake     <td> unknown      <td> unknown
<tr><td>OSX              <td> CMake        <td> unknown      <td> unknown
<tr><td>Visual Studio    <td> CMake        <td> yes          <td> tests fail
</table>

Note: S3 support includes both compiling the S3 support code as well as running the S3 tests.

## Automake

There are several options relevant to NCZarr support and to Amazon S3 support.
These are as follows.

1. _--disable-nczarr_ -- disable the NCZarr support.
If disabled, then all of the following options are disabled or irrelevant.
2. _--enable-nczarr-s3_ -- Enable NCZarr S3 support.
3. _--enable-nczarr-s3-tests_ -- the NCZarr S3 tests are currently only usable by Unidata personnel, so they are disabled by default.

__A note about using S3 with Automake.__
If S3 support is desired, and using Automake, then LDFLAGS must be properly set, namely to this.
````
LDFLAGS="$LDFLAGS -L/usr/local/lib -laws-cpp-sdk-s3"
````
The above assumes that these libraries were installed in '/usr/local/lib', so the above requires modification if they were installed elsewhere.

Note also that if S3 support is enabled, then you need to have a C++ compiler installed because part of the S3 support code is written in C++.

## CMake {#nczarr_cmake}

The necessary CMake flags are as follows (with defaults)

1. -DENABLE_NCZARR=off -- equivalent to the Automake _--disable-nczarr_ option.
2. -DENABLE_NCZARR_S3=off -- equivalent to the Automake _--enable-nczarr-s3_ option.
3. -DENABLE_NCZARR_S3_TESTS=off -- equivalent to the Automake _--enable-nczarr-s3-tests_ option.

Note that unlike Automake, CMake can properly locate C++ libraries, so it should not be necessary to specify _-laws-cpp-sdk-s3_ assuming that the aws s3 libraries are installed in the default location.
For CMake with Visual Studio, the default location is here:

````
C:/Program Files (x86)/aws-cpp-sdk-all
````

It is possible to install the sdk library in another location.
In this case, one must add the following flag to the cmake command.
````
cmake ... -DAWSSDK_DIR=\<awssdkdir\>
````
where "awssdkdir" is the path to the sdk installation.
For example, this might be as follows.
````
cmake ... -DAWSSDK_DIR="c:\tools\aws-cpp-sdk-all"
````
This can be useful if blanks in path names cause problems in your build environment.

## Testing S3 Support {#nczarr_testing_S3_support}

The relevant tests for S3 support are in the _nczarr_test_ directory.
Currently, by default, testing of S3 with NCZarr is supported only for Unidata members of the NetCDF Development Group.
This is because it uses a Unidata-specific bucket is inaccessible to the general user.

# Appendix B. Building aws-sdk-cpp {#nczarr_s3sdk}

In order to use the S3 storage driver, it is necessary to install the Amazon [aws-sdk-cpp library](https://github.com/aws/aws-sdk-cpp.git).

Building this package from scratch has proven to be a formidable task.
This appears to be due to dependencies on very specific versions of,
for example, openssl.

## *\*nix\** Build

For linux, the following context works. Of course your mileage may vary.
* OS: ubuntu 21
* aws-sdk-cpp version 1.9.96 (or later?)
* Required installed libraries: openssl, libcurl, cmake, ninja (ninja-build in apt)

### AWS-SDK-CPP Build Recipe

````
git clone --recurse-submodules https://www.github.com/aws/aws-sdk-cpp
pushd aws-sdk-cpp
mkdir build
cd build
PREFIX=/usr/local
FLAGS="-DCMAKE_INSTALL_PREFIX=${PREFIX} \
       -DCMAKE_INSTALL_LIBDIR=lib \
       -DCMAKE_MODULE_PATH=${PREFIX}/lib/cmake \
       -DCMAKE_POLICY_DEFAULT_CMP0075=NEW \
       -DBUILD_ONLY=s3 \
       -DENABLE_UNITY_BUILD=ON \
       -DENABLE_TESTING=OFF \
       -DCMAKE_BUILD_TYPE=$CFG \
       -DSIMPLE_INSTALL=ON"
cmake -GNinja $FLAGS ..
ninja all
ninja install
cd ..
popd
````

### NetCDF Build

In order to build netcdf-c with S3 sdk support,
the following options must be specified for ./configure.
````
--enable-nczarr-s3
````
If you have access to the Unidata bucket on Amazon, then you can
also test S3 support with this option.
````
--enable-nczarr-s3-tests
````

## Windows build
It is possible to build and install aws-sdk-cpp. It is also possible
to build netcdf-c using cmake. Unfortunately, testing currently fails.

For Windows, the following context work. Of course your mileage may vary.
* OS: Windows 10 64-bit with Visual Studio community edition 2019.
* aws-sdk-cpp version 1.9.96 (or later?)
* Required installed libraries: openssl, libcurl, cmake

### AWS-SDK-CPP Build Recipe

This command-line build assumes one is using Cygwin or Mingw to provide
tools such as bash.

````
git clone --recurse-submodules https://www.github.com/aws/aws-sdk-cpp
pushd aws-sdk-cpp
mkdir build
cd build
CFG="Release"
PREFIX="c:/tools/aws-sdk-cpp"

FLAGS="-DCMAKE_INSTALL_PREFIX=${PREFIX} \
       -DCMAKE_INSTALL_LIBDIR=lib" \
       -DCMAKE_MODULE_PATH=${PREFIX}/cmake \
       -DCMAKE_POLICY_DEFAULT_CMP0075=NEW \
       -DBUILD_ONLY=s3 \
       -DENABLE_UNITY_BUILD=ON \
       -DCMAKE_BUILD_TYPE=$CFG \
       -DSIMPLE_INSTALL=ON"

rm -fr build
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=${CFG} $FLAGS ..
cmake --build . --config ${CFG}
cmake --install . --config ${CFG}
cd ..
popd
````
Notice that the sdk is being installed in the directory "c:\tools\aws-sdk-cpp"
rather than the default location "c:\Program Files (x86)/aws-sdk-cpp-all"
This is because when using a command line, an install path that contains
blanks may not work.

### NetCDF CMake Build

Enabling S3 support is controlled by these two cmake options:
````
-DENABLE_NCZARR_S3=ON
-DENABLE_NCZARR_S3_TESTS=OFF
````

However, to find the aws sdk libraries,
the following environment variables must be set:
````
AWSSDK_ROOT_DIR="c:/tools/aws-sdk-cpp"
AWSSDKBIN="/cygdrive/c/tools/aws-sdk-cpp/bin"
PATH="$PATH:${AWSSDKBIN}"
````
Then the following options must be specified for cmake.
````
-DAWSSDK_ROOT_DIR=${AWSSDK_ROOT_DIR}
-DAWSSDK_DIR=${AWSSDK_ROOT_DIR}/lib/cmake/AWSSDK"
````

# Appendix C. Amazon S3 Imposed Limits {#nczarr_s3limits}

The Amazon S3 cloud storage imposes some significant limits that are inherited by NCZarr (and Zarr also, for that matter).

Some of the relevant limits are as follows:
1. The maximum object size is 5 Gigabytes with a total for all objects limited to 5 Terabytes.
2. S3 key names can be any UNICODE name with a maximum length of 1024 bytes.
Note that the limit is defined in terms of bytes and not (Unicode) characters.
This affects the depth to which groups can be nested because the key encodes the full path name of a group.

# Appendix D. Alternative Mechanisms for Accessing Remote Datasets {#nczarr_altremote}

The NetCDF-C library contains an alternate mechanism for accessing traditional netcdf-4 files stored in Amazon S3: The byte-range mechanism.
The idea is to treat the remote data as if it was a big file.
This remote "file" can be randomly accessed using the HTTP Byte-Range header.

In the Amazon S3 context, a copy of a dataset, a netcdf-3 or netdf-4 file, is uploaded into a single object in some bucket.
Then using the key to this object, it is possible to tell the netcdf-c library to treat the object as a remote file and to use the HTTP Byte-Range protocol to access the contents of the object.
The dataset object is referenced using a URL with the trailing fragment containing the string ````#mode=bytes````.

An examination of the test program _nc_test/test_byterange.sh_ shows simple examples using the _ncdump_ program.
One such test is specified as follows:
````
https://s3.us-east-1.amazonaws.com/noaa-goes16/ABI-L1b-RadC/2017/059/03/OR_ABI-L1b-RadC-M3C13_G16_s20170590337505_e20170590340289_c20170590340316.nc#mode=bytes
````
Note that for S3 access, it is expected that the URL is in what is called "path" format where the bucket, _noaa-goes16_ in this case, is part of the URL path instead of the host.

The _#mode=bytes_ mechanism generalizes to work with most servers that support byte-range access.
 
Specifically, Thredds servers support such access using the HttpServer access method as can be seen from this URL taken from the above test program.
````
https://thredds-test.unidata.ucar.edu/thredds/fileServer/irma/metar/files/METAR_20170910_0000.nc#bytes
````

# Appendix E. AWS Selection Algorithms. {#nczarr_awsselect}

If byterange support is enabled, the netcdf-c library will parse the files
````
${HOME}/.aws/config
and
${HOME}/.aws/credentials
````
to extract profile names plus a list
of key=value pairs. This example is typical.
````
[default]
    aws_access_key_id=XXXXXXXXXXXXXXXXXXXX
    aws_secret_access_key=YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
    aws_region=ZZZZZZZZZ
````
The keys in the profile will be used to set various parameters in the library

## Profile Selection

The algorithm for choosing the active profile to use is as follows:

1. If the "aws.profile" fragment flag is defined in the URL, then it is used. For example, see this URL.
````
https://...#mode=nczarr,s3&aws.profile=xxx
````
2. If the "AWS.PROFILE" entry in the .rc file (i.e. .netrc or .dodsrc) is set, then it is used.
3. Otherwise the profile "default" is used.

The profile named "none" is a special profile that the netcdf-c library automatically defines.
It should not be defined anywhere else. It signals to the library that no credentialas are to used.
It is equivalent to the "--no-sign-request" option in the AWS CLI.
Also, it must be explicitly specified by name. Otherwise "default" will be used.

## Region Selection

If the specified URL is of the form
````
s3://<bucket>/key
````
Then this is rebuilt to this form:
````
s3://s2.&lt;region&gt.amazonaws.com>/key
````
However this requires figuring out the region to use.
The algorithm for picking an region is as follows.

1. If the "aws.region" fragment flag is defined in the URL, then it is used.
2. The active profile is searched for the "aws_region" key.
3. If the "AWS.REGION" entry in the .rc file (i.e. .netrc or .dodsrc) is set, then it is used.
4. Otherwise use "us-east-1" region.

## Authorization Selection

Picking an access-key/secret-key pair is always determined
by the current active profile. To choose to not use keys
requires that the active profile must be "none".

# Appendix F. NCZarr Version 1 Meta-Data Representation. {#nczarr_version1}

In NCZarr Version 1, the NCZarr specific metadata was represented using new objects rather than as keys in existing Zarr objects.
Due to conflicts with the Zarr specification, that format is deprecated in favor of the one described above.
However the netcdf-c NCZarr support can still read the version 1 format.

The version 1 format defines three specific objects: _.nczgroup_, _.nczarray_,_.nczattr_.
These are stored in parallel with the corresponding Zarr objects. So if there is a key of the form "/x/y/.zarray", then there is also a key "/x/y/.nczarray".
The content of these objects is the same as the contents of the corresponding keys. So the value of the ''_NCZARR_ARRAY'' key is the same as the content of the ''.nczarray'' object. The list of connections is as follows:

* ''.nczarr'' <=> ''_nczarr_superblock_''
* ''.nczgroup <=> ''_nczarr_group_''
* ''.nczarray <=> ''_nczarr_array_''
* ''.nczattr <=> ''_nczarr_attr_''

# Appendix G. JSON Attribute Convention. {#nczarr_json}

The Zarr V2 specification is somewhat vague on what is a legal
value for an attribute. The examples all show one of two cases:
1. A simple JSON scalar atomic values (e.g. int, float, char, etc), or
2. A JSON array  of such values.

However, the Zarr specification can be read to infer that the value
can in fact be any legal JSON expression.
This "convention" is currently used routinely to help support various
attributes created by other packages where the attribute is a
complex JSON expression.  An example is the GDAL Driver
convention <a href="#ref_gdal">[12]</a>, where the value is a complex
JSON dictionary.

In order for NCZarr to be as consistent as possible with Zarr Version 2,
it is desirable to support this convention  for attribute values.
This means that there must be some way to handle an attribute
whose value is not either of the two cases above. That is, its value
is some more complex JSON expression. Ideally both reading and writing
of such attributes should be supported.

One more point. NCZarr attempts to record the associated netcdf
attribute type (encoded in the form of a NumPy "dtype") for each
attribute. This information is stored as NCZarr-specific
metadata. Note that pure Zarr makes no attempt to record such
type information.

The current algorithm to support JSON valued attributes
operates as follows.

## Writing an attribute:
There are mutiple cases to consider.

1. The netcdf attribute **is not** of type NC_CHAR and its value is a single atomic value.
    * Convert to an equivalent JSON atomic value and write that JSON expression.
    * Compute the Zarr equivalent dtype and store in the NCZarr metadata.

2. The netcdf attribute **is not** of type NC_CHAR and its value is a vector of atomic values.
    * Convert to an equivalent JSON array of atomic values and write that JSON expression.
    * Compute the Zarr equivalent dtype and store in the NCZarr metadata.

3. The netcdf attribute **is** of type NC_CHAR and its value &ndash; taken as a single sequence of characters &ndash;
**is** parseable as a legal JSON expression.
    * Parse to produce a JSON expression and write that expression.
    * Use "|U1" as the dtype and store in the NCZarr metadata.

4. The netcdf attribute **is** of type NC_CHAR and its value &ndash; taken as a single sequence of characters &ndash;
**is not** parseable as a legal JSON expression.
    * Convert to a JSON string and write that expression
    * Use "|U1" as the dtype and store in the NCZarr metadata.

## Reading an attribute:

The process of reading and interpreting an attribute value requires two
pieces of information.
* The value of the attribute as a JSON expression, and
* The optional associated dtype of the attribute; note that this may not exist
if, for example, the file is pure zarr.

Given these two pieces of information, the read process is as follows.

1. The JSON expression is a simple JSON atomic value.
    * If the dtype is defined, then convert the JSON to that type of data,
and then store it as the equivalent netcdf vector of size one.
    * If the dtype is not defined, then infer the dtype based on the the JSON value,
and then store it as the equivalent netcdf vector of size one.

2. The JSON expression is an array of simple JSON atomic values.
    * If the dtype is defined, then convert each JSON value in the array to that type of data,
and then store it as the equivalent netcdf vector.
    * If the dtype is not defined, then infer the dtype based on the first JSON value in the array,
and then store it as the equivalent netcdf vector.

3. The JSON expression is an array some of whose values are dictionaries or (sub-)arrays.
    * Un-parse the expression to an equivalent sequence of characters, and then store it as of type NC_CHAR.

3. The JSON expression is a dictionary.
    * Un-parse the expression to an equivalent sequence of characters, and then store it as of type NC_CHAR.

## Notes

1. If a  character valued attributes's value can be parsed as a legal JSON expression, then it will be stored as such.
2. Reading and writing are *almost* idempotent in that the sequence of
actions "read-write-read" is equivalent to a single "read" and "write-read-write" is equivalent to a single "write".
The "almost" caveat is necessary because (1) whitespace may be added or lost during the sequence of operations,
and (2) numeric precision may change.

# Appendix H. Support for string types

Zarr supports a string type, but it is restricted to
fixed size strings. NCZarr also supports such strings,
but there are some differences in order to interoperate
with the netcdf-4/HDF5 variable length strings.

The primary issue to be addressed is to provide a way for user
to specify the maximum size of the fixed length strings. This is
handled by providing the following new attributes:
1. **_nczarr_default_maxstrlen** &mdash;
This is an attribute of the root group. It specifies the default
maximum string length for string types. If not specified, then
it has the value of 128 characters.
2. **_nczarr_maxstrlen** &mdash;
This is a per-variable attribute. It specifies the maximum
string length for the string type associated with the variable.
If not specified, then it is assigned the value of
**_nczarr_default_maxstrlen**.

Note that when accessing a string through the netCDF API, the
fixed length strings appear as variable length strings.  This
means that they are stored as pointers to the string
(i.e. **char\***) and with a trailing nul character.
One consequence is that if the user writes a variable length
string through the netCDF API, and the length of that string
is greater than the maximum string length for a variable,
then the string is silently truncated.
Another consequence is that the user must reclaim the string storage.

Adding strings also requires some hacking to handle the existing
netcdf-c NC_CHAR type, which does not exist in Zarr. The goal
was to choose NumPY types for both the netcdf-c NC_STRING type
and the netcdf-c NC_CHAR type such that if a pure zarr
implementation reads them, it will still work.

For writing variables and NCZarr attributes, the type mapping is as follows:
* ">S1" for NC_CHAR.
* "|S1" for NC_STRING && MAXSTRLEN==1
* "|Sn" for NC_STRING && MAXSTRLEN==n

Admittedly, this encoding is a bit of a hack.

So when reading data with a pure zarr implementaion
the above types should always appear as strings,
and the type that signals NC_CHAR (in NCZarr)
would be handled by Zarr as a string of length 1.

# Change Log {#nczarr_changelog}

Note, this log was only started as of 8/11/2022 and is not
intended to be a detailed chronology. Rather, it provides highlights
that will be of interest to NCZarr users. In order to see exact changes,
It is necessary to use the 'git diff' command.

## 8/29/2022
1. Zarr fixed-size string types are now supported.

## 8/11/2022
1. The NCZarr specific keys have been converted to lower-case
(e.g. "_nczarr_attr" instead of "_NCZARR_ATTR"). Upper case is
accepted for back compatibility.

2. The legal values of an attribute has been extended to
include arbitrary JSON expressions; see Appendix G for more details.

# Point of Contact {#nczarr_poc}

__Author__: Dennis Heimbigner<br>
__Email__: dmh at ucar dot edu<br>
__Initial Version__: 4/10/2020<br>
__Last Revised__: 8/27/2022
