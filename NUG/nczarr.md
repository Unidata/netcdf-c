# The NetCDF NCZarr Implementation {#nczarr_head}

[TOC]

# NCZarr Introduction {#nczarr_introduction}

Beginning with netCDF version 4.8.0, the Unidata NetCDF group
has extended the netcdf-c library to provide access to cloud
storage (e.g. Amazon S3
<a href="#ref_aws">[1]</a>
) by providing a mapping from a subset of the full netCDF Enhanced
(aka netCDF-4) data model to a variant of the Zarr
<a href="#ref_zarrv2">[4]</a>
data model that already has mappings to
key-value pair cloud storage systems.
The NetCDF version of this storage format is called NCZarr
<a href="#ref_nczarr">[2]</a>.

# The NCZarr Data Model {#nczarr_data_model}

NCZarr uses a data model <a href="#ref_nczarr">[2]</a> that is,
by design, similar to, but not identical with the Zarr Version 2
Specification <a href="#ref_zarrv2">[4]</a>.  Briefly, the data
model supported by NCZarr is netcdf-4 minus the user-defined
types and the String type. As with netcdf-4 it supports
chunking. Eventually it will also support filters in a manner
similar to the way filters are supported in netcdf-4.

Specifically, the model supports the following.
- "Atomic" types: char, byte, ubyte, short, ushort, int, uint, int64, uint64.
- Shared (named) dimensions
- Attributes with specified types -- both global and per-variable
- Chunking 
- Fill values
- Groups
- N-Dimensional variables
- Per-variable endianness (big or little)

With respect to full netCDF-4, the following concepts are
currently unsupported.
- String type
- User-defined types (enum, opaque, VLEN, and Compound)
- Unlimited dimensions
- Contiguous or compact storage

Note that contiguous and compact are not actually supported  because they are HDF5 specific.
When specified, they are treated as chunked where the file consists of only one chunk.
This means that testing for contiguous or compact is not possible, the _nc_inq_var_chunking_
function will always return NC_CHUNKED and the chunksizes will be the same as the dimension sizes
of the variable's dimensions.

# Enabling NCZarr Support {#nczarr_enable}

NCZarr support is enabled if the _--enable-nczarr_ option
is used with './configure'. If NCZarr support is enabled, then
a usable version of _libcurl_ must be specified
using the _LDFLAGS_ environment variable (similar to the way
that the _HDF5_ libraries are referenced).
Refer to the installation manual for details.
NCZarr support can be disabled using the _--disable-dap_.

# Accessing Data Using the NCZarr Prototocol {#nczarr_accessing_data}

In order to access a NCZarr data source through the netCDF API, the
file name normally used is replaced with a URL with a specific
format. Note specifically that there is no NC_NCZARR flag for
the mode argument of _nc_create_ or _nc_open_. In this case, it is
indicated by the URL path.

## URL Format
The URL is the usual scheme:://host:port/path?query#fragment format.
There are some details that are important.
- Scheme: this should be _https_ or _s3_,or _file_.
  The _s3_ scheme is equivalent
  to "https" plus setting "mode=nczarr" (see below).
  Specifying "file" is mostly used for testing.
- Host: Amazon S3 defines two forms: _Virtual_ and _Path_.
  + _Virtual_: the host includes the bucket name as in
    __bucket.s3.&lt;region&gt;.amazonaws.com__ 
  + _Path_: the host does not include the bucket name, but
    rather the bucket name is the first segment of the path.
    For example __s3.&lt;region&gt;.amazonaws.com/bucket__ 
  + _Other_: It is possible to use other non-Amazon cloud storage, but
    that is cloud library dependent.
- Query: currently not used.
- Fragment: the fragment is of the form _key=value&key=value&..._.
  Depending on the key, the _=value_ part may be left out and some
  default value will be used.

## Client Parameters

The fragment part of a URL is used to specify information
that is interpreted to specify what data format is to be used,
as well as additional controls for that data format.
For NCZarr support, the following _key=value_ pairs are allowd.
- mode=nczarr|zarr|s3|nz4|nzf... -- The mode key specifies
  the particular format to be used by the netcdf-c library for
  interpreting the dataset specified by the URL. Using _mode=nczarr_
  causes the URL to be interpreted as a reference to a dataset
  that is stored in NCZarr format. The modes _s3_, _nz4_, and _nzf_
  tell the library what storage driver to use. The _s3_ is default]
  and indicates using Amazon S3 or some equivalent. The other two,
  _nz4_ and _nzf_ are again for testing. The _zarr_ mode tells the
  library to use NCZarr, but to restrict its operation to operate on
  pure Zarr Version 2 datasets.
- log=&lt;output-stream&gt;: this control turns on logging output,
  which is useful for debugging and testing. If just _log_ is used
  then it is equivalent to _log=stderr_.

# NCZarr Map Implementation {#nczarr_mapimpl}

Internally, the nczarr implementation has a map abstraction
that allows different storage formats to be used.
This is closely patterned on the same approach used in
the Python Zarr implementation, which relies on the Python
_MutableMap_
<a href="#ref_python">[3]</a> class.
In NCZarr, the corresponding type is called _zmap_.

The zmap model is a set of keys where each key maps to
an _object_ that can hold arbitrary data. The keys are assumed to
have following BNF grammar.
````
key:   '/' segment
     | key '/' segment
     ;
````
This key structure induces a tree structure where each segment
matches a node in the tree.  This key/tree duality deliberately
matches that of a typical file system path in e.g. linux. The
key '/' is the root of the tree.

## Datasets

Within the key induced tree, each dataset (in the netCDF sense)
has a root which is specified by a specific key. All objects
making up the dataset (see the section on <a
href="#nczarr_purezarr">NCZarr vs Zarr </a>) reside in objects
(keys) below that dataset root key.

One restriction is that datasets cannot be nested in that
no dataset root key can be a prefix of another dataset root key.

## Zmap Implementatons 

The primary zmap implementation is _s3_ (i.e. _mode=nczarr,s3_) and indicates
that the Amazon S3 cloud storage is to be used. Other storage formats
use a structured NetCDF-4 file format (_mode=nczarr,nz4_), or a
directory tree (_mode=nczarr,nzf_) 
The latter two are used mostly for debugging and testing.
However, the _nzf_ format is important because it is intended
to match a corresponding storage format used by the Python
Zarr implementation. Hence it should serve to provide
interoperability between NCZarr and the Python Zarr.

# NCZarr versus Pure Zarr. {#nczarr_purezarr}

The NCZARR format extends the pure Zarr format by adding
extra objects such as _.nczarr_ and _.ncvar_. It is possible
to suppress the use of these extensions so that the netcdf
library can read and write a pure zarr formatted file.
This is controlled by using _mode=nczarr,zarr_ combination.

# Notes on Debugging NCZarr Access {#nczarr_debug}

The NCZarr support has a logging facility.
Turning on this logging can
sometimes give important information. Logging can be enabled by
using the client parameter "log" or "log=filename",or by
setting the environment variable NCLOGGING.
The first case will send log output to standard error and the
second will send log output to the specified file. The environment
variable is equivalent to _log_.

# Amazon S3 Storage {#nczarr_debug}

The Amazon AWS S3 storage driver currently uses the Amazon
AWS S3 Software Development Kit for C++ (aws-s3-sdk-cpp).
In order to use it, the client must provide some configuration
information. Specifically, the `~/.aws/config` file should
contain something like this.
```
[default]
output = json
aws_access_key_id=XXXX...
aws_secret_access_key=YYYY...
```

## Addressing Style
The notion of "addressing style" may need some expansion.
Amazon S3 accepts two forms for specifying the endpoint
for accessing the data.

1. Virtual -- the virtual addressing style places the bucket in
   the host part of a URL. For example:
```
https://<bucketname>.s2.<region>.amazonaws.com/
```
1. Path -- the path addressing style places the bucket in
   at the front of the path part of a URL. For example:
```
https://s2.<region>.amazonaws.com/<bucketname>/
```

The NCZarr code will accept either form, although internally,
it is standardized on path style. The reason for this
is that the bucket name forms the initial segment
in the keys.

# Zarr vs NCZarr {#nczarr_vs_zarr}

## Data Model

The NCZarr storage format is almost identical to that of
the the standard Zarr version 2 format.
The data model differs as follows.
1. Zarr supports filters -- NCZarr as yet does not
2. Zarr only supports anonymous dimensions -- NCZarr supports
only shared (named) dimensions.
3. Zarr attributes are untyped -- or perhaps more correctly
characterized as of type string.

## Storage Format

Consider both NCZarr and Zarr, and assume S3 notions of bucket
and object. In both systems, Groups and Variables (Array in Zarr)
map to S3 objects. Containment is modelled using the fact that
the container's key is a prefix of the variable's key.
So for example, if variable _v1_ is contained int top level group g1 -- _/g1 --
then the key for _v1_ is _/g1/v_.
Additional information is stored in special objects whose name
start with ".z".
In Zarr, the following special objects exist.
1. Information about a group is kept in a special object named
_.zgroup_; so for example the object _/g1/.zgroup_.
1. Information about an array is kept as a special object named _.zarray_;
so for example the object _/g1/v1/.zarray_.
1. Group-level attributes and variable-level attributes are stored
in a special object named _.zattr_;
so for example the objects _/g1/.zattr_ and _/g1/v1/.zattr.

The NCZarr format uses the same group and variable (array) objects
as Zarr. It also uses the Zarr special _.zXXX_ objects.

However, NCZarr adds some additional special objects.
1. _.nczarr_ -- this is in the top level group -- key _/.nczarr_.
It is in effect the "superblock" for the dataset and contains
any netcdf specific dataset level information. It is also used
to verify that a given key is the root of a dataset.
1. _.nczgroup_ -- this is a parallel object to _.zgroup_ and contains
any netcdf specific group information. Specifically it contains the following.
    * dims -- the name and size of shared dimensions defined in this group.
    * vars -- the name of variables defined in this group.
    * groups -- the name of sub-groups defined in this group.

    These lists allow walking the NCZarr dataset without having to use
    the potentially costly S3 list operation.
1. _.nczvar_ -- this is a parallel object to _.zarray_ and contains
netcdf specific information. Specifically it contains the following.
    * dimrefs -- the names of the shared dimensions referenced by the variable.
    * storage -- indicates if the variable is chunked vs contiguous
               in the netcdf sense.
1 _.nczattr_ -- this is parallel to the .zattr objects and stores
                the attribute type information.

## Translation
With some constraints, it is possible for an nczarr library to read
Zarr and for a zarr library to read the nczarr format.

The latter case, zarr reading nczarr is possible if the zarr library
is willing to ignore objects whose name it does not recognized;
specifically anthing beginning with _.ncz_.

The former case, nczarr reading zarr is also
possible if the nczarr can simulate or infer the contents of
the missing _.nczXXX_ objects. As a rule this can be done as follows.

1. _.nczgroup_ -- The list of contained variables and sub-groups
can be computed using the S3 list operation to list the keys
"contained" in the key for a group. By looking for occurrences
of _.zgroup_, _.zattr_, _.zarray to infer the keys for the
contained groups, attribute sets, and arrays (variables).
Constructing the set of "shared dimensions" is carried out
by walking all the variables in the whole dataset and collecting
the set of unique integer shapes for the variables.
For each such dimension length, a top level dimension is created
named  ".zdim<len>" where len is the integer length. The name
is subject to change.
1. _.nczvar_ -- The dimrefs are inferred by using the shape
in _.zarray_ and creating references to the simulated shared dimension.
netcdf specific information.
1. _.nczattr_ -- The type of each attribute is inferred by trying to parse the first attribute value string.

# Compatibility {#nczarr_compatibility}

In order to accomodate existing implementations, certain mode tags are
provided to tell the NCZarr code to look for information used
by specific implementations.

<!--
## XArray

The Xarray
<a href="#ref_xarray">[5]</a>
Zarr implementation uses its own mechanism for
specifying shared dimensions. It uses a special
attribute named ''_ARRAY_DIMENSIONS''.
The value of this attribute is a list of dimension names (strings), for example ````["time", "lon", "lat"]````.
If enabled and detected, then these dimension names are used
to define shared dimensions.
-->

# Examples {#nczarr_examples}

Here are a couple of examples using the _ncgen_ and _ncdump_ utilities.

1. Create an nczarr file using a local directory tree as storage.
    ```
    ncgen -4 -lb -o "file:///home/user/dataset.nzf#mode=nczarr,nzf" dataset.cdl
    ```
1. Display the content of an nczarr file using a local directory tree as storage.
    ```
    ncdump "file:///home/user/dataset.nzf#mode=nczarr,nzf"
    ```
1. Create an nczarr file using S3 as storage.
    ```
    ncgen -4 -lb -o "s3://datasetbucket" dataset.cdl
    ```
1. Create an nczarr file using S3 as storage and keeping to the pure
zarr format.
    ```
    ncgen -4 -lb -o "s3://datasetbucket#mode=zarr" dataset.cdl
    ```

# References {#nczarr_bib}

<a name="ref_aws">[1]</a> [Amazon Simple Storage Service Documentation](https://docs.aws.amazon.com/s3/index.html)<br>
<a name="ref_nczarr">[2]</a> [NetCDF ZARR Data Model Specification](https://www.unidata.ucar.edu/blogs/developer/en/entry/netcdf-zarr-data-model-specification)<br>
<a name="ref_python">[3]</a> [Python Documentation: 8.3. collections — High-performance container datatypes](https://docs.python.org/2/library/collections.html)<br>
<a name="ref_zarrv2">[4]</a> [Zarr Version 2 Specification](https://zarr.readthedocs.io/en/stable/spec/v2.html)<br>
<a name="ref_xarray">[5]</a> [XArray Zarr Encoding Specification](http://xarray.pydata.org/en/latest/internals.html#zarr-encoding-specification)<br>

# Appendix A. Building NCZarr Support {#nczarr_build}

Currently only the following build cases are supported.

Operating System | Build System | NCZarr       | S3 Support
-----------------------------------------------------------
Linux            | Automake     | yes          | yes
Linux            | CMake        | yes          | yes
Cygwin           | Automake     | yes          | no
OSX              | Automake     | unknown      | unknown
OSX              | CMake        | unknown      | unknown
Visual Studio    | CMake        | yes          | tests fail

Note: S3 support includes both compiling the S3 support code as well as running the S3 tests.

# Automake

There are several options relevant to NCZarr support and to Amazon S3 support.
These are as follows.

1. _--enable-nczarr_ -- enable the NCZarr support. If disabled, then all of the following options are disabled or irrelevant.
2. _--enable-s3-sdk_ -- enable the use of the aws s3 sdk.
2. _--enable-s3-tests_ -- the s3 tests are currently only usable by Unidata personnel, so they are disabled by default.
<!--
3. '--enable-xarray-dimension' -- this enables the xarray support described in the section on <a href="#nczarr_compatibility">compatibility</a>.
-->

A note about using S3 with Automake. Automake does not handle C++ libraries, so
if S3 support is desired, and using Automake, then LDFLAGS
must be properly set, namely to this.
````
LDFLAGS="$LDFLAGS -L/usr/local/lib -laws-cpp-sdk-s3"
````
The above assumes that these libraries were installed in
'/usr/local/lib', so the above requires modification if they
were installed elsewhere.

Note also that if S3 support is enabled, then you need to have a
C++ compiler installed because part of the S3 support code is
written in C++.

# CMake

The necessary CMake flags are as follows (with defaults)

1. -DENABLE_NCZARR=on -- equivalent to the Automake _--enable-nczarr_ option.
2. -DENABLE_S3_SDK=off -- quivalent to the Automake _--enable-s3-sdk_ option.
3. -DENABLE_S3_TESTS=off -- equivalent to the Automake _--enable-s3-tests_ option.

Note that unlike Automake, CMake can properly locate C++ libraries,
so it should not be necessary to specify _-laws-cpp-sdk-s3_ assuming
that the aws s3 libraries are installed in the default location.
For CMake with Visual Studio, the default location is here:
````
C:/Program Files (x86)/aws-cpp-sdk-all
````

## Testing S3 Support

The relevant tests for S3 support are
_nczarr_test/run_ut_mapapi.sh_ and _nczarr_test/run_it_test2.sh_.

Currently, by default, testing of S3 with NCzarr is supported only for
Unidata members of the NetCDF Development Group. This is because
it uses a specific bucket on a specific internal S3 appliance that
is inaccessible to the general user. This is controlled by
the _--enable_s3_tests_ option.

However, an untested mechanism exists by which others may be able
to run the tests. If someone else wants to attempt these tests, then
they need to define the environment variable name _NCS3PATH_.
The form of this variable is as follows:
````
NCS3PATH="https://<host>/<bucket>/<prefix>
````
This assumes a Path Style address (see above) where
* host -- the complete host part of the url
* bucket -- a bucket in which testing can occur without fear of
damaging anything.
* prefix - prefix of the key; the actual root, typically _test_,
is appended to this to get the root key used by the test.

Example:
````
s3.us-west.amazonaws.com/testingbucket/segment1/segment2
````
If anyone tries to use this mechanism, it would be appreciated
it any difficulties were reported to Unidata.

# Appendix B. Building aws-sdk-cpp {#nczarr_s3sdk}

In order to use the S3 storage driver, it is necessary to
install the Amazon [aws-sdk-cpp library](https://github.com/aws/aws-sdk-cpp.git).

As a starting point, here are the CMake options used by Unidata
to build that library. It assumes that it is being executed
in a build directory, `build` say, and that `build/../CMakeLists.txt exists`.
```
cmake -DBUILD_ONLY=s3
````

The expected set of installed libraries are as follows:
* aws-cpp-sdk-s3
* aws-cpp-sdk-core

# Appendix C. Amazon S3 Imposed Limits {#nczarr_s3limits}

The Amazon S3 cloud storage imposes some significant limits
that are inherited by NCZarr (and Zarr also, for that matter). 

Some of the relevant limits are as follows:
1. The maximum object size is 5 Gigabytes with a total
for all objects limited to 5 Terabytes.
2. S3 key names can be any UNICODE name with a maximum length of 1024
bytes. Note that the limit is defined in terms of bytes and not (Unicode) characters. This affects the depth to which groups can be nested because the key encodes the full path name of a group.

# Appendix D. Alternative Mechanisms for Accessing Remote Datasets 

The NetCDF-C library contains an alternate mechanism for accessing data
store in Amazon S3: The byte-range mechanism.
The idea is to treat the remote data as if it was a big file. This remote
"file" can be randomly accessed using the HTTP Byte-Range header.

In the Amazon S3 context, a copy of a dataset, a netcdf-3 or netdf-4 file,
is uploaded into a single object in some bucket. Then using the key to this object,
it is possible to tell the netcdf-c library to treat the object as a remote file
and to use the HTTP Byte-Range protocol to access the contents of the object.
The dataset object is referenced using a URL with the trailing fragment containing
the string ````#mode=bytes````.

An examination of the test program _nc_test/test_byterange.sh_ shows simple examples
using the _ncdump_ program. One such test is specified as follows:
````
https://noaa-goes16.s3.amazonaws.com/ABI-L1b-RadC/2017/059/03/OR_ABI-L1b-RadC-M3C13_G16_s20170590337505_e20170590340289_c20170590340316.nc#mode=bytes
````

This mechanism generalizes to work with most servers that support byte-range access.
Specifically, Thredds servers support such access using the HttpServer access method
as can be seen from this URL taken from the above test program.
````
https://thredds-test.unidata.ucar.edu/thredds/fileServer/irma/metar/files/METAR_20170910_0000.nc#bytes
````

# __Point of Contact__ {#nczarr_poc}

__Author__: Dennis Heimbigner<br>
__Email__: dmh at ucar dot edu<br>
__Initial Version__: 4/10/2020<br>
__Last Revised__: 6/8/2020
