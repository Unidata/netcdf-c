The NetCDF NCZarr Implementation
============================
<!-- double header is needed to workaround doxygen bug -->

# The NetCDF NCZarr Implementation {#nczarr_head}

\tableofcontents

# NCZarr Introduction {#nczarr_introduction}

Beginning with netCDF version 4.8.0, the Unidata NetCDF group has extended the netcdf-c library to support data stored using the Zarr data model and storage format [4,6]. As part of this support, netCDF adds support for accessing data stored using cloud storage (e.g. Amazon S3 <a href="#ref_aws">[1]</a> ).

The goal of this project, then, is to provide maximum interoperability between the netCDF Enhanced (netcdf-4) data model and the Zarr version 2 <a href="#ref_zarr">[4]</a><!-- or Version 3 <a href="#ref_zarrv3">[13]</a>--> data model. This is embodied in the netcdf-c library so that it is possible to use the netcdf API to read and write Zarr formatted datasets.

In order to better support the netcdf-4 data model, the netcdf-c library implements a limited set of extensions to the *Zarr* data model. 
This extended model is referred to as *NCZarr*.
Additionally, another goal is to ensure interoperability between *NCZarr*
formatted files and standard (aka pure) *Zarr* formatted files.
This means that (1) an *NCZarr* file can be read by any other *Zarr* library (and especially the Zarr-python library), and (2) a standard *Zarr* file can be read by netCDF. Of course, there limitations in that other *Zarr* libraries will not use the extra, *NCZarr* meta-data, and netCDF will have to "fake" meta-data not provided by a pure *Zarr* file.

As a secondary -- but equally important -- goal, it must be possible to use
the NCZarr library to read and write datasets that are pure Zarr,
which means that none of the NCZarr extensions are used. This feature does come
with some costs, namely that information contained in the netcdf-4
data model may be lost in the pure Zarr dataset.

Notes on terminology in this document.
* The term "dataset" is used to refer to all of the Zarr objects constituting
   the meta-data and data. 
* NCZarr currently is not thread-safe. So any attempt to use it with parallelism, including MPIO, is likely to fail.

# The NCZarr Data Model {#nczarr_data_model}

NCZarr uses a data model that, by design, extends the Zarr Version 2 Specification <!--or Version 3 Specification-->.

__Note Carefully__: a legal _NCZarr_ dataset is expected to also be a legal _Zarr_ dataset.
The inverse is true also. A legal _Zarr_ dataset is expected to also be a legal _NCZarr_ dataset, where "legal" means it conforms to the Zarr specification(s).
In addition, certain non-Zarr features are allowed and used.
Specifically the XArray [7] ''\_ARRAY\_DIMENSIONS'' attribute is one such.

There are two other, secondary assumption:

1. The actual storage format in which the dataset is stored -- a zip file, for example -- can be read by the _Zarr_ implementation.
2. The compressors (aka filters) used by the dataset can be encoded/decoded by the implementation. NCZarr uses HDF5-style filters, so ensuring access to such filters is somewhat complicated. See [the companion document on
filters](./md_filters.html "filters") for details.

Briefly, the data model supported by NCZarr is netcdf-4 minus
the user-defined types and full String type support.
However, a restricted form of String type
is supported (see Appendix D).
As with netcdf-4, chunking is supported.  Filters and compression
are also [supported](./md_filters.html "filters").

Specifically, the model supports the following.
- "Atomic" types: char, byte, ubyte, short, ushort, int, uint, int64, uint64, string.
- Shared (named) dimensions
- Unlimited dimensions
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
- Contiguous or compact storage

Note that contiguous and compact are not actually supported
because they are HDF5 specific.
When specified, they are treated as chunked where the file consists of only one chunk.
This means that testing for contiguous or compact is not possible; the _nc_inq_var_chunking_ function will always return NC_CHUNKED and the chunksizes will be the same as the dimension sizes of the variable's dimensions.

Additionally, it should be noted that NCZarr supports scalar variables,
but Zarr Version 2 does not; Zarr V2 only supports dimensioned variables.
In order to support interoperability, NCZarr V2 does the following.
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
2. If one of the supported AWS SDKs is installed, then it is possible to directly read and write datasets stored in the Amazon S3 cloud storage.

# Accessing Data Using the NCZarr Prototocol {#nczarr_accessing_data}

In order to access a NCZarr data source through the netCDF API, the file name normally used is replaced with a URL with a specific format.
Note specifically that there is no NC_NCZARR flag for the mode argument of _nc_create_ or _nc_open_.
In this case, it is indicated by the URL path.

## URL Format
The URL is the usual format.
````
protocol:://host:port/path?query#fragment
````
See the document "quickstart_paths" for details about
using URLs.

There are, however, some details that are important.
- Protocol: this should be _https_ or _s3_,or _file_.
  The _s3_ scheme is equivalent to "https" plus setting "mode=s3".
  Specifying "file" is mostly used for testing, but also for directory tree or zipfile format storage.

## Client Parameters

The fragment part of a URL is used to specify information that is interpreted to specify what data format is to be used, as well as additional controls for that data format.

For reading, _key=value_ pairs are provided for specifying the storage format.
- mode=nczarr|zarr

Additional pairs are provided to specify the Zarr version.
- mode=v2<!--|v3-->

Additional pairs are provided to specify the storage medium: Amazon S3 vs File tree vs Zip file.
- mode=file|zip|s3

Note that when reading, an attempt will be made to infer the
format and Zarr version and storage medium format by probing the
file. If inferencing fails, then it is reported.  In this case,
the client may need to add specific mode flags to avoid
inferencing.

Typically one will specify three mode flags: one to indicate what format
to use and one to specify the way the dataset is to be stored<!--,and one to specify the Zarr format version-->.
For example, a common one is "mode=zarr,file<!--,v2-->"
<!--If not specified, the version will be the default specified when
the netcdf-c library was built.-->

Obviously, when creating a file, inferring the type of file to create
is not possible so the mode flags must be set specifically.
This means that both the storage medium and the exact storage
format must be specified.
Using _mode=nczarr_ causes the URL to be interpreted as a
reference to a dataset that is stored in NCZarr format.
The _zarr_ mode tells the library to use NCZarr, but to restrict its operation to operate on pure Zarr.
<!--The _v2_ mode specifies Version 2 and _v3_mode specifies Version 3.
If the version is not specified, it will default to the value specified when the netcdf-c library was built.-->

The modes _s3_, _file_, and _zip_ tell the library what storage medium
driver to use.
* The _s3_ driver stores data using Amazon S3 or some equivalent.
* The _file_ driver stores data in a directory tree.
* The _zip_ driver stores data in a local zip file.

As an aside, it should be the case that zipping a _file_
format directory tree will produce a file readable by the
_zip_ storage format, and vice-versa.

By default, the XArray convention is supported for Zarr Version 2
and used for both NCZarr files and pure Zarr files.
<!--It is not needed for Version 3 and is ignored.-->
This means that every variable in the root group whose named dimensions
are also in the root group will have an attribute called
*\_ARRAY\_DIMENSIONS* that stores those dimension names.
The _noxarray_ mode tells the library to disable the XArray support.

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
the root of the dataset. The term __File__ will often be used as a synonym.
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
This is a straightforward tradeoff of space over time.

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

## Zmap Implementations

The primary zmap implementation is _s3_ (i.e. _mode=nczarr,s3_) and indicates that the Amazon S3 cloud storage -- or some related appliance -- is to be used.
Another storage format uses a file system tree of directories and files (_mode=nczarr,file_).
A third storage format uses a zip file (_mode=nczarr,zip_).
The latter two are used mostly for debugging and testing.
However, the _file_ and _zip_ formats are important because they are intended to match corresponding storage formats used by the Python Zarr implementation.
Hence it should serve to provide interoperability between NCZarr and the Python Zarr, although this interoperability has had only limited testing.

Examples of the typical URL form for _file_ and _zip_ are as follows.
````
file:///xxx/yyy/testdata.file#mode=nczarr,file
file:///xxx/yyy/testdata.zip#mode=nczarr,zip
````

Note that the extension (e.g. ".file" in "testdata.file")
is arbitrary, so this would be equally acceptable.
````
file:///xxx/yyy/testdata.anyext#mode=nczarr,file
````
As with other URLS (e.g. DAP), these kind of URLS can be passed as the path argument to, for example, __ncdump__.

# NCZarr versus Pure Zarr. {#nczarr_purezarr}

The NCZARR format extends the pure Zarr format by adding extra attributes such as ''\_nczarr\_array'' inside the ''.zattr'' object.
It is possible to suppress the use of these extensions so that the netcdf library can write a pure zarr formatted file. But this probably unnecessary
since these attributes should be readable by any other Zarr implementation.
But these extra attributes might be seen as clutter and so it is possible
to suppress them when writing using *mode=zarr*.

Reading of pure Zarr files created using other implementations is a necessary
compatibility feature of NCZarr.
This requirement imposed some constraints on the reading of Zarr datasets using the NCZarr implementation.
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

## Addressing Style

The notion of "addressing style" may need some expansion. Amazon S3 accepts two forms for specifying the endpoint for accessing the data (see the document "quickstart_path").

1. Virtual -- the virtual addressing style places the bucket in the host part of a URL.
For example:

```
https://<bucketname>.s2.&lt;region&gt.amazonaws.com/
```

2. Path -- the path addressing style places the bucket in at the front of the path part of a URL.
For example:

```
https://s3.&lt;region&gt.amazonaws.com/<bucketname>/
```

The NCZarr code will accept either form, although internally, it is standardized on path style.
The reason for this is that the bucket name forms the initial segment in the keys.

# Zarr vs NCZarr {#nczarr_vs_zarr}

## Data Model

The NCZarr storage format is almost identical to that of the the standard Zarr format.
The data model differs as follows.

1. Zarr only supports anonymous dimensions -- NCZarr supports only shared (named) dimensions.
2. Zarr attributes are untyped -- or perhaps more correctly characterized as of type string.
3. Zarr does not explicitly support unlimited dimensions -- NCZarr does support them.

## Storage Medium

Consider both NCZarr and Zarr, and assume S3 notions of bucket and object.
In both systems, Groups and Variables (Array in Zarr) map to S3 objects.
Containment is modeled using the fact that the dataset's key is a prefix of the variable's key.
So for example, if variable _v1_ is contained in top level group g1 -- _/g1 -- then the key for _v1_ is _/g1/v_.
Additional meta-data information is stored in special objects whose name start with ".z".

In Zarr Version 2, the following special objects exist.
1. Information about a group is kept in a special object named _.zgroup_;
so for example the object _/g1/.zgroup_.
2. Information about an array is kept as a special object named _.zarray_;
so for example the object _/g1/v1/.zarray_.
3. Group-level attributes and variable-level attributes are stored in a special object named _.zattr_;
so for example the objects _/g1/.zattr_ and _/g1/v1/.zattr_.
4. Chunk data is stored in objects named "\<n1\>.\<n2\>...,\<nr\>" where the ni are positive integers representing the chunk index for the ith dimension.

The first three contain meta-data objects in the form of a string representing a JSON-formatted dictionary. 
The NCZarr format uses the same objects as Zarr, but inserts NCZarr
specific attributes in the *.zattr* object to hold NCZarr specific information
The value of each of these attributes is a JSON dictionary containing a variety
of NCZarr specific information.

These NCZarr-specific attributes are as follows:

_\_nczarr_superblock\__ -- this is in the top level group's *.zattr* object.
It is in effect the "superblock" for the dataset and contains
any netcdf specific dataset level information.
It is also used to verify that a given key is the root of a dataset.
Currently it contains keys that are ignored and exist only to ensure that
older netcdf library versions do not crash.
* "version" -- the NCZarr version defining the format of the dataset (deprecated).

_\_nczarr_group\__ -- this key appears in every group's _.zattr_ object.
It contains any netcdf specific group information.
Specifically it contains the following keys:
* "dimensions" -- the name and size of shared dimensions defined in this group, as well an optional flag indictating if the dimension is UNLIMITED.
* "arrays" -- the name of variables defined in this group.
* "groups" -- the name of sub-groups defined in this group.
These lists allow walking the NCZarr dataset without having to use the potentially costly search operation.

_\_nczarr_array\__ -- this key appears in the *.zattr* object associated
with a _.zarray_ object.
It contains netcdf specific array information.
Specifically it contains the following keys:
* dimension_references -- the fully qualified names of the shared dimensions referenced by the variable.
* storage -- indicates if the variable is chunked vs contiguous in the netcdf sense. Also signals if a variable is scalar.

_\_nczarr_attr\__ -- this attribute appears in every _.zattr_ object.
Specifically it contains the following keys:
* types -- the types of all attributes in the _.zattr_ object.

## Translation {#nczarr_translation}

With some loss of netcdf-4 information, it is possible for an nczarr library to read the pure Zarr format and for other zarr libraries to read the nczarr format.

The latter case, zarr reading nczarr, is trivial because all of the nczarr metadata is stored as ordinary, String valued (but JSON syntax), attributes.

The former case, nczarr reading zarr is possible assuming the nczarr code can simulate or infer the contents of the missing _\_nczarr\_xxx_ attributes.
As a rule this can be done as follows.
1. _\_nczarr_group\__ -- The list of contained variables and sub-groups can be computed using the search API to list the keys "contained" in the key for a group.
The search looks for occurrences of _.zgroup_, _.zattr_, _.zarray_ to infer the keys for the contained groups, attribute sets, and arrays (variables).
Constructing the set of "shared dimensions" is carried out
by walking all the variables in the whole dataset and collecting
the set of unique integer shapes for the variables.
For each such dimension length, a top level dimension is created
named  "_Anonymous_Dimension_<len>" where len is the integer length.
2. _\_nczarr_array\__ -- The dimension referencess are inferred by using the shape in _.zarray_ and creating references to the simulated shared dimensions.
netcdf specific information.
3. _\_nczarr_attr\__ -- The type of each attribute is inferred by trying to parse the first attribute value string.

# Compatibility {#nczarr_compatibility}

In order to accommodate existing implementations, certain mode tags are provided to tell the NCZarr code to look for information used by specific implementations.

## XArray

The Xarray [7] Zarr implementation uses its own mechanism for specifying shared dimensions.
It uses a special attribute named ''_ARRAY_DIMENSIONS''.
The value of this attribute is a list of dimension names (strings).
An example might be ````["time", "lon", "lat"]````.
It is almost equivalent to the ````_nczarr_array "dimension_references" list````, except that the latter uses fully qualified names so the referenced dimensions can be anywhere in the dataset. The Xarray dimension list differs from the netcdf-4 shared dimensions in two ways.
1. Specifying Xarray in a non-root group has no meaning in the current Xarray specification.
2. A given name can be associated with different lengths, even within a single array. This is considered an error in NCZarr.

The Xarray ''_ARRAY_DIMENSIONS'' attribute is supported for both NCZarr and pure Zarr.
If possible, this attribute will be read/written by default,
but can be suppressed if the mode value "noxarray" is specified.
If detected, then these dimension names are used to define shared dimensions.
The following conditions will cause ''_ARRAY_DIMENSIONS'' to not be written.
* The variable is not in the root group,
* Any dimension referenced by the variable is not in the root group.

Note that this attribute is not needed for Zarr Version 3, and is ignored.

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
    ncgen -4 -lb -o 's3://s3.uswest-1.amazonaws.com/datasetbucket\#mode=zarr dataset.cdl
    ```
5. Create an nczarr file using the s3 protocol with a specific profile
    ```
    ncgen -4 -lb -o "s3://datasetbucket/rootkey\#mode=nczarr&awsprofile=unidata" dataset.cdl
    ```
    Note that the URL is internally translated to this
    ````
    "https://s2.&lt;region&gt.amazonaws.com/datasetbucket/rootkey\#mode=nczarr&awsprofile=unidata"
    ````
# Appendix A. Building NCZarr Support {#nczarr_build}

Currently the following build cases are known to work.
Note that this does not include S3 support.
A separate tabulation of S3 support is in the document _cloud.md_.

<table>
<tr><td><u>Operating System</u><td><u>Build System</u><td><u>NCZarr</u>
<tr><td>Linux            <td> Automake     <td> yes
<tr><td>Linux            <td> CMake        <td> yes
<tr><td>Cygwin           <td> Automake     <td> yes
<tr><td>Cygwin           <td> CMake        <td> yes
<tr><td>OSX              <td> Automake     <td> yes
<tr><td>OSX              <td> CMake        <td> yes
<tr><td>Visual Studio    <td> CMake        <td> yes
</table>

## Automake

The relevant ./configure options are as follows.

1. *--disable-nczarr* -- disable the NCZarr support.

## CMake

The relevant CMake flags are as follows.

1. *-DNETCDF_ENABLE_NCZARR=off* -- equivalent to the Automake *--disable-nczarr* option.
## Testing NCZarr S3 Support {#nczarr_testing_S3_support}

The relevant tests for S3 support are in the _nczarr_test_ directory.
Currently, by default, testing of S3 with NCZarr is supported only for Unidata members of the NetCDF Development Group.
This is because it uses a Unidata-specific bucket that is inaccessible to the general user.

### NetCDF Build

In order to build netcdf-c with S3 sdk support,
the following options must be specified for ./configure.
````
--enable-s3
````
If you have access to the Unidata bucket on Amazon, then you can
also test S3 support with this option.
````
--with-s3-testing=yes
````

### NetCDF CMake Build

Enabling S3 support is controlled by this cmake option:
````
-DNETCDF_ENABLE_S3=ON
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
-DAWSSDK_DIR=${AWSSDK_ROOT_DIR}/lib/cmake/AWSSDK
````

# Appendix B. Amazon S3 Imposed Limits {#nczarr_s3limits}

The Amazon S3 cloud storage imposes some significant limits that are inherited by NCZarr (and Zarr also, for that matter).

Some of the relevant limits are as follows:
1. The maximum object size is 5 Gigabytes with a total for all objects limited to 5 Terabytes.
2. S3 key names can be any UNICODE name with a maximum length of 1024 bytes.
Note that the limit is defined in terms of bytes and not (Unicode) characters.
This affects the depth to which groups can be nested because the key encodes the full path name of a group.

# Appendix C. JSON Attribute Convention. {#nczarr_json}

The Zarr V2 <!--(and V3)--> specification is somewhat vague on what is a legal
value for an attribute. The examples all show one of two cases:
1. A simple JSON scalar atomic values (e.g. int, float, char, etc), or
2. A JSON array  of such values.

However, the Zarr specification can be read to infer that the value
can in fact be any legal JSON expression.
This "convention" is currently used routinely to help support various
attributes created by other packages where the attribute is a
complex JSON expression.  An example is the GDAL Driver
convention <a href='#ref_gdal'>[12]</a>, where the value is a complex
JSON dictionary.

In order for NCZarr to be as consistent as possible with Zarr,
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
There are multiple cases to consider.

1. The netcdf attribute **is not** of type NC_CHAR and its value is a single atomic value.
    * Convert to an equivalent JSON atomic value and write that JSON expression.
    * Compute the Zarr equivalent dtype and store in the NCZarr metadata.

2. The netcdf attribute **is not** of type NC_CHAR and its value is a vector of atomic values.
    * Convert to an equivalent JSON array of atomic values and write that JSON expression.
    * Compute the Zarr equivalent dtype and store in the NCZarr metadata.

3. The netcdf attribute **is** of type NC_CHAR and its value &ndash; taken as a single sequence of characters &ndash;
**is** parseable as a legal JSON expression.
    * Parse to produce a JSON expression and write that expression.
    * Use "|J0" as the dtype and store in the NCZarr metadata.

4. The netcdf attribute **is** of type NC_CHAR and its value &ndash; taken as a single sequence of characters &ndash;
**is not** parseable as a legal JSON expression.
    * Convert to a JSON string and write that expression
    * Use ">S1" as the dtype and store in the NCZarr metadata.

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

3. The attribute is any other JSON structure.
    * Un-parse the expression to an equivalent sequence of characters, and then store it as of type NC_CHAR.

## Notes

1. If a  character valued attributes's value can be parsed as a legal JSON expression, then it will be stored as such.
2. Reading and writing are *almost* idempotent in that the sequence of
actions "read-write-read" is equivalent to a single "read" and "write-read-write" is equivalent to a single "write".
The "almost" caveat is necessary because (1) whitespace may be added or lost during the sequence of operations,
and (2) numeric precision may change.

# Appendix D. Support for string types

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

So when reading data with a pure zarr implementation
the above types should always appear as strings,
and the type that signals NC_CHAR (in NCZarr)
would be handled by Zarr as a string of length 1.

<!--
# Appendix E. Zarr Version 3: NCZarr Version 3 Meta-Data Representation. {#nczarr_version3}

For Zarr version 3, the added NCZarr specific metadata is stored 
as attributes pretty much the same as for Version 2.
Specifically, the following Netcdf-4 meta-data information needs to be captured by NCZarr:
    1. Shared dimensions: name and size.
    2. Unlimited dimensions: which dimensions are unlimited.
    3. Attribute types.
    4. Netcdf types not included in Zarr: currently "char" and "string".
    5. Zarr types not included in Netcdf: currently only "complex(32|64)"
This extra netcdfd-4 meta-data to attributes so as to not interfere with existing implementations.

## Supported Types
Zarr version 3 supports the following "atomic" types:
bool, int8, uint8, int16, uint16, int32, uint32, int64, uint64, float32, float64.
It also defines two structured type: complex64 and complex128.

NCZarr supports all of the atomic types.
Specialized support is provided for the following
Netcdf types: char, string.
The Zarr types bool and  complex64 are not yet supported, but will be added shortly.
The type complex128 is not supported at all.

The Zarr type "bool" will appear in the netcdf types as
the enum type "_bool" whose netcdf declaration is as follows:
````
ubyte enum _bool_t {FALSE=0, TRUE=1};
````
The type complex64 will be supported by by defining this compound type:
````
compound _Complex64_t { float64 i; float64 j;}
````

Strings present a problem because there is a proposal
to add variable length strings to the Zarr version 3 specification;
fixed-length strings would not be supported at all.
But strings are important in Netcdf, so a forward compatible
representation is provided where the type is string
and its maximum size is specified.

For arrays, the Netcdf types "char" and "string" are stored
in the Zarr file as of type "uint8" and "r<8*n>", respectively
where _n_ is the maximum length of the string in bytes (not characters).
The fact that they represent "char" and "string" is encoded in the "_nczarr_array" attribute (see below).

## NCZarr Superblock
The *_nczarr_superblock* attribute is used as a useful marker to signal that a file is in fact NCZarr as opposed to Zarr.
This attribute is stored in the *zarr.info* attributes in the root group of the Zarr file.
The relevant attribute has the following format:
````
"_nczarr_superblock": {
    "version": "3.0.0",
    format": 3
}
````
## Group Annotations
The optional *_nczarr_group* attribute is stored in the attributes of a Zarr group within
the *zarr.json* object in that group.
The relevant attribute has the following format:
````
"_nczarr_group": {
\"dimensions\": [{name: <dimname>, size: <integer>, unlimited: 1|0},...],
\"arrays\": ["<name>",...],
\"subgroups\": ["<name>",...]
}
````
Its purpose is two-fold:
1. record the objects immediately within that group
2. define netcdf-4 dimenension objects within that group.

## Array Annotations
In order to support Netcdf concepts in Zarr, it may be necessary
to annotate a Zarr array with extra information.
The optional *_nczarr_array* attribute is stored in the attributes of a Zarr array within
the *zarr.json* object in that array.
The relevant attribute has the following format:
````
"_nczarr_array": {
\"dimension_references\": [\"/g1/g2/d1\", \"/d2\",...],
\"type_alias\": "<string indicating special type aliasing>" // optional
}
````
The *dimension_references* key is an expansion of the "dimensions" key
found in the *zarr.json* object for an array.
The problem with "dimensions" is that it specifies a simple name for each
dimension, whereas netcdf-4 requires that the array references dimension objects
that may appear in groups anywhere in the file. These references are encoded
as FQNs "pointing" to a specific dimension declaration (see *_nczarr_group* attribute
defined previously).

FQN is an acronym for "Fully Qualified Name".
It is a series of names separated by the "/" character, much
like a file system path.
It identifies the group in which the dimension is ostensibly "defined" in the Netcdf sense.
For example ````/d1```` defines a dimension "d1" defined in the root group.
Similarly ````/g1/g2/d2```` defines a dimension "d2" defined in the
group g2, which in turn is a subgroup of group g1, which is a subgroup
of the root group.

The *type_alias* key is used to annotate the type of an array
to allow discovery of netcdf-4 specific types.
Specifically, there are three current cases:
| dtype | type_alias |
| ----- | ---------- |
| uint8 | char       |
| rn    | string     |
| uint8 | json       |

If, for example, an array's dtype is specified as *uint8*, then it may be that
it is actually of unsigned 8-bit integer type. But it may actually be of some
netcdf-4 type that is encoded as *uint8* in order to be recognized by other -- pure zarr--
implementations. So, for example, if the netcdf-4 type is *char*, then the array's
dtype is *uint8*, but its type alias is *char*.

## Attribute Type Annotation
In Zarr version 3, group and array attributes are stored inside
the corresponding _zarr.info_. object under the dictionary key "attributes".
Note that this decision is still under discussion and it may be changed
to store attributes in an object separate from _zarr.info_.

Regardless of where the attributes are stored, and in order to
support netcdf-4 typed attributes, the per-attribute information
is stored as a special attribute called _\_nczarr_attrs\__ defined to hold
NCZarr specific attribute information. Currently, it only holds
the attribute typing information.
It can appear in any *zarr.json* object: group or array.

Its form is this:
````
"_nczarr_attrs": {
    "attribute_types": [
        {"name": "attr1", "configuration": {"type": "<dtype>"}},
	...
	]
}
````
There is one entry for every attribute (including itself) giving the type
of that attribute.
It should be noted that Zarr allows the value of an attribute to be an arbitrary
JSON-encoded structure. In order to support this in netcdf-4, is such a structure
is encountered as an attribute value, then it typed as *json* (see previously
described table).

## Codec Specification
The Zarr version 3 representation of codecs is slightly different
than that used by Zarr version 2.
In version 2, the codec is represented by this JSON template.
````
{"id": "<codec name>" "<param>": "<value>", "<param>": "<value>", ...}
````
In version 3, the codec is represented by this JSON template.
````
{"name": "<codec name>" "configuration": {"<param>": "<value>", "<param>": "<value>", ...}}
````
-->

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
<a name="ref_awssdk_conda">[11]</a> [Conda-forge packages / aws-sdk-cpp](https://anaconda.org/conda-forge/aws-sdk-cpp)<br>
<a name="ref_gdal">[12]</a> [GDAL Zarr](https://gdal.org/drivers/raster/zarr.html)<br>
<!--
<a name="ref_nczarrv3">[13]</a> [NetCDF ZARR Data Model Specification Version 3](https://zarr-specs.readthedocs.io/en/latest/specs.html)
-->

# Change Log {#nczarr_changelog}
[Note: minor text changes are not included.]

Note, this log was only started as of 8/11/2022 and is not
intended to be a detailed chronology. Rather, it provides highlights
that will be of interest to NCZarr users. In order to see exact changes,
It is necessary to use the 'git diff' command.

## 03/31/2024
1. Document the change to V2 to using attributes to hold NCZarr metadata.

## 01/31/2024
1. Add description of support for Zarr version 3 as an appendix.

## 3/10/2023
1. Move most of the S3 text to the cloud.md document.

## 8/29/2022
1. Zarr fixed-size string types are now supported.

## 8/11/2022
1. The NCZarr specific keys have been converted to lower-case
(e.g. "_nczarr_attr" instead of "_NCZARR_ATTR"). Upper case is
accepted for back compatibility.

2. The legal values of an attribute has been extended to
include arbitrary JSON expressions; see Appendix D for more details.

# Point of Contact {#nczarr_poc}

__Author__: Dennis Heimbigner<br>
__Email__: dmh at ucar dot edu<br>
__Initial Version__: 4/10/2020<br>
__Last Revised__: 4/02/2024
