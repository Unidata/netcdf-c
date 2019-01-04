NetCDF ZARR Storage Representation
====================================

# Abstract

*This document defines a mapping of the netcdf Zarr
data model to a combination of Json and an abstract key-value
pair model. This document corresponds most closely to the
existing Zarr specification [].*

*Note that this document both supplements the existing
Zarr specification [] and also documents differences between the
netCDF Zarr model and the one described in the other specification.
Additionally, some text is taken from the Zarr specification*

*Distribution of this document is currently restricted to Unidata.*

[TOC]

# Introduction {#nczstore_intro}

This document is a companion document to the 
*NetCDF ZARR Data Model Specification*[].
That document provides a semi-formal and abstract representation of
the NCZarr data model independent of any implementation.

This document describes how that abstract representation is mapped
to a form similar to that defined in the existing Zarr specification [].
That mapping consists of a description of how the abstract representation
is defined in terms of a combination of Json and an abstract key-value pair model.
The goal is to be as consistent with Zarr as possible but to define and discuss
any differences deemed necessary to support netCDF.

# Interoperability Goals {#nczstore_interoperability}

Given the NCZarr supporting netcdf-c library and given, for example,
a Python Zarr reader that knows nothing of NCZarr, we propose the
following interoperability goals for reading various datasets.

1. netcdf-c library reading a Zarr formated dataset --
allow the dataset to be read and to make its data accessible
through the netcdf-c API.
2. Python Zarr Reader reading an NCZarr formated dataset --
allow the dataset to be read and to make its data accessible
through the netcdf-c API.

The first case requires the netcdf-c library to (1) recognize
that the dataset is not NCZArr and (2) provide a usable
netcdf-4 data model representation of the Zarr dataset.
Part 1 is doable if we assume that the dataset contains the
*_NCProperties* provenance attribute if and only if the file
is NCZarr. Part 2 will be defined as asides in the rest of
this document.

For the second case (reading NCZarr as if it were Zarr)
requires that (1) the Zarr reader ignores information it does
not recognize (e.g. the NCZarr specific attributes)
and (2) there is a subset of the NCZarr format in the file that
is legal Zarr.

Note that currently, the Zarr specification disallows ignoring
unrecognized keys. Ideally, the specification should be changed
to support this capability.

# Abstract Key-Value Pair Model {#nczstore_keyvalue_model}

The abstract key-value pair model used by the Zarr
implementation is based on the Python MutableMapping
interface. We adopt a subset of that interface for NCZarr. This
interface is subject to modification as experience is gained, and the
actual names may be changed.

<table>
<tr><th>Method<th>Informal Description<th>Notes
<tr><td>newmap<td>Create an instance of a map.
<tr><td>freemap<td>Reclaim a map instance; will not reclaim keys and values.
<tr><td>contains<td>Map contains specified key.
<tr><td>ith<td>Get ith pair from the map.
<tr><td>len<td>Get number of pairs in the map.
<tr><td>keys<td>Get a list of the keys in the map.
<tr><td>get<td>Get item corresponding to a specified key.
<tr><td>set<td>Insert the specified item; replaces any
                       existing pair with matching key.
<tr><td>remove<td>Delete the item specified by the key; return the
                       corresponding value.
<tr><td>clear<td>Remove all items in the map.
</table>

Notes:

1. Rather than use an iterator approach, it is assumed that one can
   ask for the i'th pair in the map. This is apparently the way an older
   version of Mapping worked. This has consequences
   if the map is being simultaneously modified.
2. In line with the Zarr specification, the key is assumed to be US-ASCII string
   (note that it is not UTF8), and the value is a sequence of 8-bit bytes.

# Basic Cloud Storage Model {#nczstore_cloud_storage}

Since the goal is to store data in "the cloud", it is appropriate
to discuss the typical cloud storage model. There appears
to be convergence on the Amazon S3 model by all the major players,
we will briefly outline that storage model; this is a paraphrase
of some official Amazon documentation [].

The S3 model consists of *buckets* containing zero or more *objects*.

## S3 Objects
An *object* is a collection of *meta-data* plus some opaque *data*.
The meta-data is a collection of key-value pairs, some of which are
automatically provided by S3, and the rest are provided by the user.

An object a key (name) that is unique within a bucket.
UTF-8 is the defined character set for object names, but
S3 strongly recommends and defined a more limited character set.

The object name-space is technically flat, but a simulated tree
structure can be imposed on objects by defining a *path*
structure on the object name: "/x/y/object", for example.

## S3 Object Meta-Data
The object meta-data is a set of key-value pairs. The meta-data is divided
into system metdata and user-defined metadata.

A key is technically UTF-8, but depending on the access method, it must be limited
to lower-case US-ASCII.

Additionally, there are size limits on meta-data size.

Notes:

1. The concept and use of regions is orthogonal to the bucket+object model,
   so it is not discussed here.
2. Zarr apparently does not use the S3 versioning mechanism, so it is not
   discussed here. It is, however, a topic that should be explored.

# Mapping {#nczstore_mapping}

The mapping of each element in the abstract representation
is described in the following subsections.

## Dataset {#nczstore_dataset}
The dataset is the root group and so
contains what a group would contain.

In addition, the dataset object contains a key named *.zdataset*
with the following Json dictionary entries.
<table>
<tr><th>Key<th>Description
<tr><td>nczarr_version<td>Version of the nczarr format used throughout
the dataset.
</table>

## Group {#nczstore_group}
The groups are organized as an an object that
contains the key *.zgroup*, which in turn has the following
Json dictionary entries.
<table>
<tr><th>Key<th>Description
<tr><td>zarr_format<td>Version of the storage specification.
<tr><td>.zattrs<td>Group level attributes
</table>
Note that we assume that the *zarr_format* key is only used in the root group and all
other occurrences of this key in subgroups are ignored or assumed to be the same.

## Variable {#nczstore_variable}

The primary element of Zarr is the variable, aka *array*.

Each Zarr array is represented by an object (in the S3 sense).
Associated with the variable is a meta-data key called *.zarray*.
The value of this key is a string serialized Json dictionary with these keys.
<table>
<tr><th>Key<th>Description
<tr><td>zarr_format<td>Version of the version of the storage specification for this variable.
<tr><td>shape<td>A list of integers defining the length of each dimension of the array.
<tr><td>chunks<td>A list of integers defining the chunk sizes.
<tr><td>dtype<td>The data type for the array (including the endianness).
<tr><td>compressor<td>The primary compression codec filter id plus parameters.
<tr><td>fill_value<td>The default value to use for uninitialized portions of the array.
<tr><td>order<td>The layout of bytes in the chunk: either row-major or column-major.
<tr><td>filters<td>Additional filters specified by id.
</table>

## Attribute {#nczstore_attribute}

Attributes associated with variables are stored in a variable object or a group object
under the key *.zattrs*. If this key is missing, then the set of attributes
is treated as the empty set.

Since netcdf also allows group level attributes, the *.zattr* key can occur
inside a group object.

An single attribute is stored as a key-value pair inside the .zattr dictionary.
This causes a significant problem for netcdf, because it's attributes are typed
and technically, Zarr attributes are not. So, we propose to add a second key entry
called *.zattr_type* to parallel *.zattr*. This has the same keys as *.zattr*, but
the value is the type of the attribute.

## Dimension {#nczstore_dimension}

There is a proposal outstanding for adding named dimensions.
However, until ratified, we propose to define named dimensions
in a group under the object key named *.zdims*. Inside that
there will be a Json dictionary where the each key is a dimension name
and its value is the dimension size.

Constructing the fully qualified name (FQN) for a dimension will consist of suffixing the group 
FQN with '/' and then the dimension name taken from *.zdims*

It will not be possible to use the NCZarr API to define an anonymous
dimension. But if the netcdf library encounters a variable with an non-named dimension
specifying only the size, it will create a pseudo-named dimension called
*_anonymousNN*, where NN is the size of the dimension.

### Unlimited Dimensions

Representing Unlimited dimensions is T.B.D.

## Dimension Reference {#nczstore_dimref}

Currently, the dimension sizes associated with a variable
are integer constants (i.e. anonymous dimensions). In order
to support the netcdf dimension concept, we need to augment
the Zarr representation with a list of dimension FQNs.
Ideally, this would replace the existing shape specification
of the variable, but for backwards compatibility, it will
be necessary to add a new variable key in parallel with the *shape*
key. This key will be called *dimensions* and will consist of
a list of FQNs.

## Types {#nczstore_types}
The Zarr type system is mostly compatible with the netcdf-4 type system
with the following considerations.

### Encoding of Corresponding Types
The following table shows the mapping from netcdf-4 types
to corresponding Zarr types. Note that the assumption is
the default encoding is little endian.
<table>
<tr><th>NetCDF-4 Type<th>Zarr Type<th>Description
<tr><td>byte<td><i8<td>Signed 8 bit integer
<tr><td>ubyte<td><u8<td>Unsigned 8 bit integer
<tr><td>short<td><i16<td>Signed 16 bit integer
<tr><td>ushort<td><u16<td>Unsigned 16 bit integer
<tr><td>int<td><i32<td>Signed 32 bit integer
<tr><td>uint<td><u32<td>Unsigned 32 bit integer
<tr><td>int64<td><i64<td>Signed 64 bit integer
<tr><td>uint64<td><u64<td>Unsigned 64 bit integer
</table>

### Encoding of Missing netcdf-4 Types
Zarr has some types that are not directly supported by netcdf-4. The types
and their handling is defined here

#### Boolean
Defining variables or attributes using the boolean type is not supported.
If encountered in the meta-data, it is treated as a 1-byte unsigned integer.

#### Time-Related Types
Defining variables or attributes using the Datetime or Timedelta types is not supported.
If encountered, it is treated as a string with
an associated built-in attribute giving the units.

#### The Complex Type
If this type is encountered on reading a non-NCZarr Zarr file, then
a compound type named "_Complex" will be added to the list of
types defined in the root group. The associated type will then be
treated as equivalent to thie compound type.

#### The V Type
The definition of, say, ````|V29````, appears to be the equivalent vector
of 29 8-bit unsigned bytes. It will be always treated as an *opaque*
type (see below).

### Encoding of missing Zarr Types
This is the inverse case. Some netcdf-4 types are
not directly available in Zarr and must be encoded using
(or abusing) the available Zarr types.

#### Char
Netcdf-4 has a *char* type. We treat this type as
equivalent to an eight-bit, unsigned integer.

#### Opaque 
The netcdf *opaque* type is essentially equivalent of 
an fixed size vector of 8-bit bytes. So we translate ````opaque(10)````
to ````|V10````. 

#### Variable Length Strings
This is discussed in detail in section \ref nczstore_varstring.

#### Enumerations
Currently, Zarr apparently has no support for enumeration types.
We propose to add it at some point. For know, we note the following
elements of an enumeration type.

1. It has a base type that is one of the integer types.
2. It has a set of legal values (the enumeration contants)
   taken from a subset of the possible values of the base type.
3. Each element of the set of legal values has an associated name
   unique within the enumeration type.

#### VLEN
Currently, we have no plans to support VLEN since it appears to
be a difficult undertaking in the context of the current Zarr specification.

# Type Definitions {#nczstore_typedefs}

There is a fundamental disconnect between the netCDF-4 handling of user-defined types
and the Zarr handling of them (i.e. specifically compound types).
In netCDF-4, defining a compound type gives that type a unique name
that can be used anywhere a type needs to be specified: i.e. for a variable
or attribute. This, like named dimensions, ensures semantic compatibility
across all uses of that type.

Zarr, on the other hand, treats each use of a compound type as a unique,
anonymous declaration. Two variables may be defined as having a compound
type that to the eye look identical, but there is no way to "say" that they
are identical. It is interesting to note that Zarr appears to be inconsistent 
about this because it does have a complex type that is essentially a named,
two field compound type. So, every where compound is used, one can be assured
it is the same type and not just one that happens to look structurally alike.

The guaranteed semantic equality  used in netcdf-4 is probably as important
as the semantic equality of shared dimensions. Hence, we intend to stick with it
in NCZarr.

To this end, we propose the following solution:
1. Add an additional, group level, key named *.ztypedefs* that will store
   all the netcdf-4 user type definitions: compound, opaque, and (eventually) enumeration.
2. When a variable is typed by a user-type, the existing Zarr model of storing the
   type declaration at each point of use will be maintained, but an additional
   Json key will be added named *typedef* to point to the FQN of the actual type.

Note: This same problem was encountered in mapping netcdf-4 to HDF5, but eventually
HDF5 moved to a named type model.

# Variable Length Strings {#nczstore_varstring}

Zarr does not directly support variable length strings but does
support fixed length strings. So supporting variable length strings
in NCZarr is a challenge. Basically the choices are to move the variable
string content out of line (i.e. somewhere else than in the chunk data)
or to find some way to specify the maximum string length whenever a
(variable length) string typed variable, attribute, or field is defined.
The former is undesirable because the strings then become unreadable
by, say, a Python Zarr reader which does not know about any NCZarr convention.

The latter case -- annotating the dataset with maximum string length
information -- is difficult because netcdf-4 only supports attributes
for variables and not for fields of compound types and not for attributes
themselves.

Technically and given the way that Zarr stores attributes,
there is no need to do anything special for string-typed attributes.
This is because their value(s) is a list of Json strings, which are
already variable length.

For fields, we define an attribute in the same group as the one in which
the containing compound type is defined. The name of this global attribute
is *cmpdname.fieldname* and its value is the maximum length of the
string-typed field.


# Data Representation {#nczstore_data}

NetCDF-4 places few constraints on how data is represented. It
assumes that logically, it has access to a multi-dimensional
array of values and that it can read/write subsets of that data
in certain specific ways as specified by the
````nc_get/put_vars()```` functions. That is, it can select a
hyperslab of the data and retrieve it and optionally using striding.

The basic Zarr storage layout works well for this and is pretty similar
to the existing way that netcdf-4 accesses HDF5 data.
Specifically, it is assumed that the whole of the data is divided
into chunks that are stored independently. In this case, it means
that each chunk is an object whose data is the chunk data.

A request for a hyperslab must figure out the set of chunks that
together cover the hyperslab and then must figure out an
efficient read/write strategy.

It appears that the Zarr specification does not say how the data for
a structure typed variable is stored. Nor is the idea of aligning of
structure fields (as in netCDF-4/HDF5) defined.
For the latter, we assume tight packing. For the former, we rely in the
fact that all of the fields are recursively fixed size, that we can
layout instance of the data as a single block of bytes.
Note that this is because in our abstract specification, we explicitly
disallowed string-type fields.

# Provenance Information {#nczstore_provenance}

Recently, netcdf-4 has added a specific top-level attribute
called *_NCProperties* that is used to store information about
how a dataset was constructed.  Most notably, it stores the
netcdf-c library version, and the HDF5 library version.

We propose to extend the provenance information to record the cloud format
used to store the data.

We propose to use the provenance information to control our
interpretation of a dataset. Specifically, we assume *_NCProperties*
is defined if and only if the dataset was created through the netcdf API.
It *_NCProperties* is not present, then it is assumed that the dataset
must be treated as if it was consistent with the existing Zarr specification [].

This means two things:
1. The dataset is considered read-only
2. We must define how to map standard Zarr meta-data into usuable netcdf-4 meta data.

We propose that item 2 is defined as follows.
1. All dimensions in standard Zarr are size only (anonymous). Named dimensions
   of the form AnonymousNN will be created for each such dimension.
2. Each compound type will be declared as a separate netcdf-4 type.
3. When a fixed length string is encountered?

# Comments {#nczstore_comments}

1. Zarr conflates the data with some kinds of meta-data (e.g. endianness).
   Note that since this is specifiable in the .zarray meta-data, it would
   seem redundant to include it with the data. Is this a filters issue?
2. Zarr has way more flexibility than is probably desirable. It is possible
   to have each variable in a dataset use a different storage format
   (even if only one is currently defined). This presents the same
   problem as custom filters: if the library accessing the dataset
   does not support all formats, then only part of the data may be readable.
   It would be better to know this up-front when initially opening the
   dataset. 
3. The use of Json to store meta-data may eventually be problem
   for datasets with very large amounts of meta-data. We have encountered
   this for netcdf-4 datasets and fixing it is not trivial unless taken
   into account from the beginning.

# Differences

1. The Zarr specification says 
````Other keys MUST NOT be present within the metadata object````.
NCZarr takes the "read-broadly, write narrowly" approach so it
allows extra keys, but ignores them if it does not understand
their semantics.

# Copyright
_Copyright 2018, UCAR/Unidata<br>
See netcdf/COPYRIGHT file for copying and redistribution conditions._

# Point of Contact

__Author__: Dennis Heimbigner<br>
__Email__: dmh at ucar dot edu<br>
__Initial Version__: 12/6/2018<br>
__Last Revised__: 12/8/2018

# References
[] https://zarr.readthedocs.io/en/stable/spec/v2.html<br>
[] https://docs.python.org/2/library/collections.html<br>
[] https://docs.aws.amazon.com/AmazonS3/latest/dev/Introduction.html#CoreConcepts<br>


