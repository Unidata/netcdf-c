The NetCDF Zarr Extensions to Standard Zarr
====================================

# Abstract

*This document defines the proposed extensions
of Netcdf Zarr versus the standard Zarr
as defined in the Zarr specificiation [].*

[TOC]

# Introduction {#nczext_intro}

The Unidata goal is to maximize the degree to which our Zarr
implementation -- here called NCZarr -- can support the netCDF-4
(aka *Enhanced*) model used by netCDF.  We believe this
requires our NCZarr implementation to extend in some ways
the (Standard) Zarr as defined in the Zarr specification [].

However, we recognize that interoperability is also important so
we plan to support the following notions of interoperability
between the netcdf-c library and other packages that expect
standard Zarr.

1. Existing Zarr reading software we be able to effectively read
   NCZarr datasets.
2. The netcdf-c library can read a large subset of standard Zarr
   datasets. The cost will be that the resulting netcdf-4
   wrapper will use only a very limited subset of the whole
   netcdf-4 data model.(e.g. no named dimensions).

The remainder of this document assumes knowledge of both the Zarr
and netcdf-4 data models.

# Provenance {#nczext_provenance}

NetCDF-4 has a specific top-level attribute called
*_NCProperties* that is used to store provenance information
about how a dataset was constructed.

We propose to extend this provenance information to record the
cloud format used to store the data.

Specifically, the provenance information will be used to control our
interpretation of a dataset. We assume *_NCProperties*
is defined if and only if the dataset was created through the netcdf API
and hence is some version of the NCZarr dataset format.
If *_NCProperties* attribute is not present, then it is assumed that
the dataset must be treated as if it was consistent withstandard Zarr.

In addition, if the dataset is determined to be standard Zarr,
it will be treated as read-only. No attempt will be made to
support modification of such a dataset.

# Representing NetCDF-4 in NCZarr {#nczext_nczarrrep}

This section identifies the extensions and describes how the
netcdf-4 concepts are represented. As part of this, it
describes how the NCZarr can be usably read by a
standard Zarr software package.

In order for a standard Zarr software package to read NCZarr
(interoperability goal number 1), an important
change is required in the Zarr specification, where is says
that unrecognized object keys or unrecognized Json keys
are illegal. This needs to be changed so that such unrecognized
keys are instead ignored. This allows NCzarr to annotate the
dataset with extra, netcdf-4 specific information that will
support the netcdf-4 data model more completely.

## Shared Dimensions {#nczext_shareddims}

The notion of shared (i.e. named) dimensions is critical to netcdf because
it supports semantic equality. That is, if two variables reference
the same dimension name, then we are guaranteed that the two sizes
are intended to be the same. In standard Zarr the fact that two
variables happen to have dimension of size, say, 20, is no guarantee
that they represent the same dimension, only that by coincidence they
have the same size.

To support shared dimensions, we propose to associate an extra object
named *.zdims* with each group. The value of that key will be a Json
dictionary where the each key is a dimension name and its value
is the dimension size.

For each variable, then, there will be two associated keys: *shape* giving the
dimension sizes for backward compatibility, and a new *dimensions* key 
giving a list of dimension names that correspond to the values in the *shape* list.
The dimension names will be in the form of fully qualified names (e.g. */g1/g2/dimname*)
to unambiguously specify the dimension.

## Unlimited Dimensions {#nczext_unlimited}
T.B.D

## Named Type Definitions {#nczext_typedefs}

There is a fundamental disconnect between the netCDF-4 handling of user-defined types
and the Zarr handling of them, and especially compound (aka structure) types.
In netCDF-4, defining a compound type gives that type a unique name
that can be used anywhere a type needs to be specified: i.e. for a variable
or attribute. This, like named dimensions, ensures semantic equality
across all uses of that type.

Zarr, on the other hand, treats each use of a compound type as a
unique, anonymous declaration. Two variables may be defined as
having a compound type that to the eye look identical, but there
is no way to *guarantee* that they are semantically identical.

The guaranteed semantic equality  used in netcdf-4 is probably as important
as the semantic equality of shared dimensions. Hence, we intend to keep ita
in NCZarr.

To this end, we propose the following solution:
1. Add an additional, group level, object named *.ztypedefs* that will store
   all the netcdf-4 user type definitions: compound, opaque, and (eventually) enumeration.
2. When a variable is typed by a user-type, the existing Zarr model of storing the
   type declaration at each point of use will be maintained, but an additional
   Json key will be added named something like *typedef* to point to the FQN of the actual type.
3. This rule will apply for structure types (aka compound types), opaque types,
   and (eventually) enumerations.

As an aside, this same problem was encountered in the original
mapping of netcdf-4 to HDF5. Eventually HDF5 moved to support a
named type model.

## Atomic type mappings {#nczext_atomictypes}

Fortunately, there is good agreement about most of the atomic types
between netcdf-4 and standard Zarr. This table defines the mapping
(ignoring endian representation)
<table>
<tr><th>NetCDF-4 Type<th>Zarr Type<th>Description
<tr><td>byte<td>|i8<td>Signed 8 bit integer
<tr><td>ubyte<td>|u8<td>Unsigned 8 bit integer
<tr><td>short<td>|i16<td>Signed 16 bit integer
<tr><td>ushort<td>|u16<td>Unsigned 16 bit integer
<tr><td>int<td>|i32<td>Signed 32 bit integer
<tr><td>uint<td>|u32<td>Unsigned 32 bit integer
<tr><td>int64<td>|i64<td>Signed 64 bit integer
<tr><td>uint64<td>|u64<td>Unsigned 64 bit integer
</table>

## Char mapping {#nczext_char}

The Netcdf-4 *char* type is treated as
equivalent to an eight-bit, unsigned integer.
An extra tag is added to indicate that it is really
a netcdf-4 *char* type.

#### Opaque 
The netcdf *opaque* type is essentially equivalent to
a fixed size vector of 8-bit bytes. So we translate ````opaque(10)````
to ````|V10````. Since this is a user-defined type, it is stored in
*.ztypedefs* and any variable referencing this type will be tagged with both
````|V10```` and with the opaque type FQN.

# Variable Length Strings {#nczext_varstring}

Zarr does not directly support variable length strings but does
support fixed length strings. So supporting the netcdf-4 (variable length) String
type in NCZarr is a challenge. Basically the choices are to move the variable
string content out of line (i.e. somewhere else than in the chunk data)
or to find some way to specify the maximum string length whenever a
(variable length) string typed variable, attribute, or field is defined.
The former is undesirable because the strings then become unreadable
as standard Zarr.

The latter case -- annotating the dataset with maximum string length
information -- has two flaws.

1. It enforces a maximum length on strings, so they are not truly variable length
2. It is difficult to speciify because netcdf-4 only supports attributes
    for variables and not for fields of compound types and not for attributes
    themselves.

Tentatively, we propose the following solution:

1. Technically and given the way that Zarr stores attributes,
there is no need to do anything special for string-typed attributes.
This is because their value is a vector of Json strings, which are
already variable length.

2. For fields, we propose to define an attribute in the same group as the one in which
the containing compound type is defined. The name of this global attribute
is *cmpdname.fieldname.strlen* and its value is the maximum length of the
string-typed field.

3. For variables, an extra attribute is defined giving the maximum length.

So, fields and variables will appear to standard Zarr as properly typed
fixed length strings. But by interpreting the max size tags, netcdf-4
can also read them and pretend they are variable length. Not a good solution.
but tolerable.

Ideally, we think that Zarr should consider adding some kind of support
for truly variable length items: possibly by adding a *malloc()* style
heap mechanism.

#### VLEN {#nczext_vlen}

Currently, we have no plans to support the netcdf-4 VLEN type since it appears to
be a difficult undertaking in the context of the current Zarr specification.

# Mapping Standard Zarr to Netcdf-4 {#nczext_mapping}

Our other interoperabilty goal is to allow the netcdf-c library
to usably read existing a large subset of standard Zarr
datasets, albeit with significant representational loss.

To this end, we propose the following mapping of standard Zarr constructs
into a limited set of netcdf-4 constructs.

## Zarr Atomic Types

The following, non-integer, Zarr atomic types will be converted as follows:
<table>
<tr><th>Zarr Type<th>NetCDF-4 Type
<tr><td>b<td>ubyte
<tr><td>c<td>predefined Complex compound type
<tr><td>m<td>string
<tr><td>M<td>string
<tr><td>S<td>char array
<tr><td>U<td>string
<tr><td>V<td>Opaque
</table>

## Dimensions
Since all dimensions in standard Zarr are size only (anonymous),
each such dimension will be treated as a named dimension with a name
something like *AnonymousNN* where NN is the size.

## Structure Types
Each structure type will be declared as a separate netcdf-4 compound type with a name
constructed from the name of the associated variable and declared in the same
group as the variable.

# A Note on Naming {#nczext_naming}
 
The various proposed NCzarr names (e.g. *.zdims*) are provisional.
At some point, we will adopt naming conventions that are specific
to NCZarr. For example, we might actually name *dimensions*
to be *_ncz_dimensions*. The exact conventions are T.B.D.

# Copyright
_Copyright 2018, UCAR/Unidata<br>
See the netcdf-c
[COPYRIGHT file](https://github.com/Unidata/netcdf-c/blob/master/COPYRIGHT)
for copying and redistribution conditions._

# Point of Contact

__Author__: Dennis Heimbigner<br>
__Email__: dmh at ucar dot edu<br>
__Initial Version__: 12/11/2018<br>
__Last Revised__: 1/11/2019

# References {#nczext_references}
[] [Zarr Version 2 Specification](https://zarr.readthedocs.io/en/stable/spec/v2.html)<br>

