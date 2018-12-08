NetCDF ZARR Data Model Specification
====================================

# Abstract

*This document defines an abstract description of the netcdf Zarr
data model. Note that this document both supplements the existing
Zarr specification [] and also documents differences between the
netCDF Zarr model and the one described in the other specification.*

*Distribution of this document is currently restricted to Unidata.

# Copyright
_Copyright 2018, UCAR/Unidata<br>
See netcdf/COPYRIGHT file for copying and redistribution conditions._

# Point of Contact

__Author__: Dennis Heimbigner<br>
__Email__: dmh at ucar dot edu<br>
__Initial Version__: 11/28/2018<br>
__Last Revised__: 11/28/2018

[TOC]

# Introduction {#nczarr_intro}

The Unidata NetCDF group is proposing to provide access to cloud
storage (e.g. Amazon S3) by providing a mapping from a subset of
the full netCDF Enhanced (aka netCDF-4) data model to one or
more existing data models that already have mappings to
key-value pair cloud storage systems.

The initial target is to map that subset of netCDF-4 to the Zarr
[] data model. As part of that effort, we intend to produce a
set of related documents that provide a semi-formal definition
of the following.

1. An abstract representation of the NCZarr data model.
2. A description of the subset of the netCDF API that conforms
   to the NCZARR data model. This interface will be the basis
   for programatically reading and writing cloud data via the
   netcdf-c library.
3. A mapping of the NCZarr data model to some variant of the
   Zarr storage representation, This representation is a
   combination of a mapping to Json plus a mapping to an
   abstract key-value pair interface.

The term "semi-formal" is used because rather than provide
complete mathematical or operational semantics, prose text will
be used to describe the context-sensitive features of the model.
Complete formalization to produce an operationally defined
specification is a possible future activity.

This document describes the first item: the abstract
representation of the NCZarr data model.  It also documents any
differences between the NCZarr model and the existing Zarr data
model.

The approach to all of this is based on using modern compiler
technology. Compilers are centered around the notion of an
"abstract syntax tree" (aka AST).  The term "abstract
representation" will also be used as a synonym for AST.

The tree is constructed in a compiler by using some form of
parser to parse programs in the language into a tree containing
the essential elements of the program.  The nodes of the tree are
typed using some typing model such object-oriented classes.

In a compiler, and given the AST, the tree is processed
repeatedly to perform constext sensitive checking and to
annotate it with semantic information (such as linking type
references and type declarations).

Lastly, the tree is used to generate the final product.
Typically for a compiler, this is the code to execute.

This specification differs from compilers in a number of ways.

1. The AST is defined semi-formally using some typed language.
2. Instead of using a parser to construct the tree,
   an API is provided that effectively defines how to construct
   a legal instance of the tree. That is the purpose of document
   number 2 in the previous list.
3. The final step, code generation, is replaced by a mapping
   where each node in the abstract tree is mapped to a combination of
   key-value pairs and Json representations of the content of the
   node. It is this choice of mapping that ultimately connects
   the netcdf-c API to the Zarr formatted cloud storage.

# Formalism for Specifying the Data Model {#nczarr_formalism}

In order to represent the abstract structure of the NCZarr data
model, we must choose some suitable notation.  This notation
must meet the requirement that it is typed, meaning that the
nodes of the tree have a type and the structure of the node must
conform to that type.

Ideally, we would use Json as our notation since that is the
target representation used by the Zarr specification.
Unfortunately, Json is effectively typeless so we do not consider it
powerful enough to properly represent the data model. If some
way exists to do this, then this may be viable.

Some alternative possibilities are: parser generators
(e.g. Antlr, Bison), any strongly typed programming language
(e.g. Java, C++, Python, Haskell), and XML.  None of these is
ideal because they may be unfamiliar to scientific programmers

We choose Antlr4 [] as our formalism because it is designed for
such uses as this one. Additionally, it is concise and
provides a number of useful visualization and processing tools.

# Abstract Representation {#nczarr_abstract_intro}

The abstract representation is defined by using Antlr4
grammar rules. In a later section, the primitive, lexical
elements will also be specified using Antlr4 notation.
A complete Antlr4 grammar is defined in Appendix ?.

## Dataset {#nczarr_dataset}
````Antlr
dataset : NAME ZARRVERSION (dimension | variable | attribute | group)*
````
We term the root of the abstract representation as a _Dataset_.
A Dataset is also a Group, so it can contain variables, attributes, and (sub-)groups.

## Group {#nczarr_group}
````Antlr
group: NAME (dimension | variable | attribute | group)*
````
A Group contains a collection of dimension declarations, variable
declarations, attributes, and Groups.

## Attribute {#nczarr_attribute}
````Antlr
attribute : NAME value_type (CONSTANT)+
````
An Attribute contains a (ordered) set of values, where the values
are constants consistent with the specified type of the attribute.
An attribute must have at least one value.

## Dimension {#nczarr_dimension}
````Antlr
dimension: NAME SIZE
````
A Dimension declaration defines a named dimension where the
dimension has a specific specified size.

## Variable {#nczarr_variable}
````Antlr
variable: NAME type (dimref)* (attribute)*
````
A Variable declaration defines a named variable of a specified
type.  It also can reference a set of dimensions defining the
rank and size of the variable. If no dimensions are referenced,
then the variable is a scalar.

Additionally, any number of attributes can be associated with the variable
to define properties about the variable.

## Dimension Reference {#nczarr_dimref}
````Antlr
dimref: SIZE | FQN
````
A Dimension reference specifies one the dimensions of a variable
by either defining an anonymous dimension where the size is specified
directly, or by providing the fully qualified name refering to some
dimension defined in some Group via a ````<Dimension>```` declaration.

## Types {#nczarr_types}
````Antlr
type: value_type | compound ;
value_type: atomic_type | opaque;

atomic_type:   fixed_atomic_type
             | STRING_T // A sequence of utf8 characters with no embedded nul characters.
	     ;

fixed_atomic_type:
      BYTE_T   // A signed 8 bit integer
    | UBYTE_T  // An unsigned 8 bit integer
    | SHORT_T  // A signed 16 bit integer
    | USHORT_T // An unsigned 16 bit integer
    | INT_T    // A signed 32 bit integer
    | UINT_T   // An unsigned 32 bit integer
    | INT64_T  // A signed 64 bit integer
    | UINT64_T // An unsigned 64 bit integer
    ;

fixed_size_type: fixed_atomic_type | compound; // Fixed size objects

compound: NAME field+ ;
field: NAME fixed_size_type (SIZE)*;

opaque: NAME SIZE ;

````
Types are either atomic or are a user-defined compound type.
The atomic types are those can be used when specifying the type
of a variable or an attribute, the names are taken from the
corresponding netCDF-4 specification.  A variable or a compound
field can be typed using the a compound type.

As of version 2, Zarr defines the equivalent of netcdf-4 compound types.
A compound type is intended to be similar to that of a C language struct.
So it consists of an ordered sequence of one or more named, typed, dimensioned fields.

## Lexical Tokens {#nczarr_lexemes}
````Antlr

NAME: IDCHAR+
FQN: ([/])|([/](IDCHAR)+)+

SIZE: DIGITS // Non-negative integer
ZARRVERSION: DIGITS '.' DIGITS '.' DIGITS

// Type Lexemes
BYTE_T: 'byte'
UBYTE_T: 'ubyte'
SHORT_T: 'short'
USHORT_T: 'ushort'
INT_T: 'int'
UINT_T: 'uint'
INT64_T: 'int64'
UINT64_T: 'uint64'
STRING_T: 'string'

// Exact form is as usual, but will leave out for now
CONSTANT: INTEGER | UNSIGNED | STRING | FLOAT ;

fragment DIGITS: ['0'-'9']+
fragment UTF8: // Assume base character set is UTF8
fragment ASCII: [0-9a-zA-Z !#$%()*+:;<=>?@\[\]\\^_`|{}~] // Printable ASCII
fragment IDCHAR: (IDASCII|UTF8)
fragment IDASCII: [0-9a-zA-Z!#$%()*+:;<=>?@\[\]^_`|{}~] | '\\\\' | '\\/' | '\\ '

````
A NAME consists of a sequence of any legal non-control UTF-8 characters. A control character isc any UTF-8 character in the inclusive range 0x00 &mdash; 0x1F.

## Fully Qualified Names {#nczarr_fqn}

Every dimension and variable in a NCZarr Dataset has a Fully Qualified Name
(FQN), which provides a way to unambiguously reference it
in a dataset. Currently, the only case where this
is used is for referencing named dimensions from within
variable declarations.

These FQNs follow the common conventions of names for lexically
scoped identifiers.  In NCZarr scoping is provided by Groups
(and the group subtype *dataset*).
Just as with hierarchical file
systems or variables in many programming languages, a simple
grammar formally defines how the names are built using the names
of the FQN's components (see lexical grammar above).

The FQN for a "top-level" variable or dimension is defined purely by
the sequence of enclosing groups plus the variable's simple
name.

Notes:

1. Every dataset has a single outermost *dataset* node.
which semantically, acts like the root group.
Whatever name that dataset has is ignored for the purposes of forming the FQN and instead is treated as if it has the empty name ("").
2. There is no limit to the nesting of groups.

The character "/" has special meaning in the context of a fully qualified name. This means that if a name is added to the FQN and that name contains this character, then that characters must be specially escaped so that they will not be misinterpreted. The escape character itself must also be escaped, as must a blank.

The defined escapes are as follows.

<table border=1 width="25%">
<tr><th>Character<th>Escaped Form
<tr><th>/<th>\/
<tr><th>\<th>\\
<tr><th>blank <th>\blank
</table>

# Appendix ?. Supplementary Material {#nczarr_supplement}

## Specifying Context-Sensitive Elements {#nczarr_csensitive_spec}

The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY" and "OPTIONAL" in this document are to be interpreted as described in <nowiki>RFC 2119</nowiki>. <sup><nowiki>[</nowiki>[[#Ref-7|7]]<nowiki>]</nowiki></sup>

# Appendix ?. Complete Version of the Abstract Representation Specification

This is the complete Antlr specification in a form that can
be processed by Antlr.
````Antlr
grammar z ;
dataset : NAME ZARRVERSION (dimension | variable | attribute | group)* ;
group: NAME (dimension | variable | attribute | group)* ;
attribute : NAME value_type (CONSTANT)+ ;
dimension: NAME SIZE ;
variable: NAME type (dimref)* (attribute)* ;
dimref: SIZE | FQN ;
type: value_type | compound ;
value_type: atomic_type | opaque;
atomic_type:   fixed_atomic_type
             | STRING_T // A sequence of utf8 characters with no embedded nul characters.
	     ;
fixed_atomic_type:
      BYTE_T   // A signed 8 bit integer
    | UBYTE_T  // An unsigned 8 bit integer
    | SHORT_T  // A signed 16 bit integer
    | USHORT_T // An unsigned 16 bit integer
    | INT_T    // A signed 32 bit integer
    | UINT_T   // An unsigned 32 bit integer
    | INT64_T  // A signed 64 bit integer
    | UINT64_T // An unsigned 64 bit integer
    ;

fixed_size_type: fixed_atomic_type | compound | opaque; // Fixed size objects

atomic_type: 
      BYTE_T   // A signed 8 bit integer
    | UBYTE_T  // An unsigned 8 bit integer
    | SHORT_T  // A signed 16 bit integer
    | USHORT_T // An unsigned 16 bit integer
    | INT_T    // A signed 32 bit integer
    | UINT_T   // An unsigned 32 bit integer
    | INT64_T  // A signed 64 bit integer
    | UINT64_T // An unsigned 64 bit integer
    | STRING_T // A sequence of utf8 characters with no embedded nul characters.
    ;
compound: NAME field+ ;
field: NAME fixed_size_type (SIZE)*;

opaque: NAME SIZE ;

// Lexemes
NAME: IDCHAR+ ;
FQN: ([/])|([/](IDCHAR)+)+ ;

SIZE: DIGITS ; // Non-negative integer ;
ZARRVERSION: DIGITS '.' DIGITS '.' DIGITS ;

// Type Lexemes
BYTE_T: 'byte' ;
UBYTE_T: 'ubyte' ;
SHORT_T: 'short' ;
USHORT_T: 'ushort' ;
INT_T: 'int' ;
UINT_T: 'uint' ;
INT64_T: 'int64' ;
UINT64_T: 'uint64' ;
STRING_T: 'string' ;

// Exact form is as usual, but will leave out for now
CONSTANT: INTEGER | UNSIGNED | FLOAT | STRING ;

fragment INTEGER: [+-]?DIGITS ;
fragment UNSIGNED: DIGITS ;
fragment FLOAT: [+-]?DIGITS '.' DIGITS ;
fragment STRING: '"' ~["] '"' ;

fragment DIGITS: [0-9]+ ;
fragment UTF8: ASCII ; // Assume base character set is UTF8 ;
fragment IDCHAR: (IDASCII|UTF8) ;
fragment IDASCII: [0-9a-zA-Z]|[!#$%()*+:;<=>?@]|'['|']'|'\\'|[^_`|{}~]
                  |'\\\\'|'\\/'|'\\ ' ;
fragment ASCII: [0-9a-zA-Z]|[ !#$%()*+:;<=>?@]|'['|']'|'\\'|[^_`|{}~] ; // Printable ASCII
````

# Appendix ?. Differences With Respect to the Official Zarr Specification {#nczarr_diffs}

1. This document only describes the abstract model, it says nothing
about the underlying storage model.
2. In the Zarr specification, additional dictionary keys are disallowed,
here they are ignored.
3. FQN reference to dimensions? How is dimension disambiguated

# Issues
- forward references
- FQN
- single character constants
- structural typeing for structs rather then named types.
