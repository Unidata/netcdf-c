Introduction to FAN Language and Utilities
============================================


Introduction to FAN Language and Utilities (FAN Version 2.0) {#fan_utils}
===========================================

<center>

Harvey Davies
CSIRO Division of Atmospheric Research,
Private Bag No. 1, Mordialloc 3195, Australia
email: hld@dar.csiro.au

Scientific Visitor from January to August 1996 at
UCAR/Unidata Program Center,
P.O. Box 3000, Boulder, Colorado 80307-3000, USA
email: hld@ucar.edu

</center>

------------------------------------------------------------------------

Introduction
============

*FAN (File Array Notation)* is an array-oriented language for
identifying data items in files for the purpose of extraction or
modification. FAN specifications consist of

-   one or more filenames
-   one or more variable (array) names or ID numbers
-   attribute names or ID numbers (optional)
-   dimension names or ID numbers (optional)
-   subscripts in various possible forms (optional)

NetCDF is the only format currently supported. However FAN is intended
to be generic and it is hoped that there will eventually also be FAN
interfaces to various other formats.

This document describes the FAN language and four utilities based on
FAN. The use of these utilities can greatly decrease the need for
programming in Fortran or C. They can be called from the Unix command
line and shell scripts.

The first is `nc2text` which prints selected data from netCDF variables.
The standard utility `ncdump` can also print data from netCDF variables,
but only entire variables and only together with metadata in CDL form.

The second is `ncmeta` which prints selected metadata from one or more
netCDF files. This metadata can include rank, shape, file names,
variable names, dimension names and attribute names.

The third is `ncrob` which reads data from one or more netCDF variables,
performs some process on it and then either prints the result or writes
it to a netCDF array. The letters \``rob`' in \``ncrob`' stand for
*Reduce Or Broadcast*. *Reduce* means to produce an array (e.g. sum,
mean, maximum) with less dimensions than the original. *Broadcast* means
to copy one array to another, recycling values if necessary. An example
is copying the same vector to each row of a matrix. It is possible to
process large volumes of data (e.g. 100 MB) using `ncrob`.

The fourth is `text2nc` which can be used to read small volumes (say up
to a few thousand lines) of ASCII data and copy it into netCDF
variables. It is also possible to use `text2nc` to create, modify and
delete attributes.

This document does not cover other ways of using FAN. These include some
local (CSIRO DAR) utilities (e.g. contouring program `con_cif`), the
array-oriented languages IDL and J (for which there are FAN interfaces)
and direct use of the C API (application programmer interface).

Simple Examples
---------------

Let us start with a simple netCDF file `vec.nc` which is printed (in
CDL) as follows:

    $ ncdump vec.nc
    netcdf vec {
    dimensions:
            n = UNLIMITED ; // (5 currently)
    variables:
            float v(n) ;
    data:
     v = 10 , 20.3 , 30.2 , 40.9 , 50  ;
    }

Here \``$`' is the UNIX command-line prompt. The following uses
`nc2text` to print the whole array `v`:

    $ nc2text vec.nc v
    10 20.3 30.2 40.9 50

Individual elements can be selected using subscripts. For example:

    $ nc2text vec.nc 'v[0]'
    10
    $ nc2text vec.nc 'v[3]'
    40.9

Several can be selected using a subscript consisting of a list of
indices such as:

    $ nc2text vec.nc 'v[0 3 1 3]'
    10 40.9 20.3 40.9

We can write to a netCDF file using `text2nc`. The following changes the
third element from 30.2 to 30.7 and then prints `v`:

    $ echo 30.7 | text2nc vec.nc 'v[2]'
    $ nc2text vec.nc v
    10 20.3 30.7 40.9 50

Here `text2nc` reads ASCII text data from standard input, which in this
case is a pipe connected to the standard output of `echo`. Since the
dimension has `UNLIMITED` size, we can append values as follows:

    $ echo 60.5 70.2 | text2nc vec.nc 'v[5 6]'
    $ nc2text vec.nc v
    10 20.3 30.7 40.9 50 60.5 70.2

Next we use `ncrob` to calculate and print the arithmetic mean of `v`.

    $ ncrob -r am vec.nc v /
    40.3714

The option `-r am` specifies that an *arithmetic mean* is to be
calculated. The following example stores the mean in the same file,
naming the variable `v_mean`:

    $ ncrob -r am vec.nc v / v_mean
    $ nc2text vec.nc v_mean
    40.3714

The \``/`' separates the input from the output. If no output is
specified then results are printed. In fact `ncrob` can be used in place
of `nc2text` to print data from a netCDF file. E.g.

    $ ncrob vec.nc v /
    10 20.3 30.7 40.9 50 60.5 70.2
    $ ncrob vec.nc v_mean /
    40.3714

Finally we use `ncmeta` to print metadata. The shape is printed by:

    $ ncmeta v vec.nc
    5

and the following prints the variable name, dimension name and shape:

    $ ncmeta -w vds v vec.nc
    v n 5

What is New in Version 2?
-------------------------

The utility `ncmeta` is new.

There are significant enhancements to the utility `ncrob`. It can now
print results as well as write them to netCDF files. (This means that
`nc2text` is no longer really needed.) In version 1 the output FAN
specification could only be a single (final) argument. There can now be
zero (implying printed output) or more output arguments following a
\``/`' which separates input arguments from output arguments. (The old
convention is deprecated but still supported.) It is now possible to
create new variables without specifying the `-c` option or an output
filename. There is a facility for merging dimensions. There are several
new options related to printing and similar to those of `nc2text`. A
number of bugs in `ncrob` have been fixed, including one with a serious
effect on speed.

FAN Language
============

High-level Syntax
-----------------

A FAN specification can be either a single command-line argument or span
several arguments. Use of multiple arguments decreases the need for
quoting and allows use of UNIX *wildcarding* (a.k.a. *globbing*)
facilities. A FAN specification can have any of the following forms:

  **Syntax**        |    **Meaning**
  ---------------------|-----------------------------------------------------
  *fanio* `/` *fanio*   | netCDF input and netCDF output
  *fanio* `/`          | netCDF input and output to `stdout` (i.e. printed)
  *fanio*              | Either netCDF input or netCDF output (but not both)


where *fanio* is a FAN input/output specification, which has the form:
*pair* `;` *pair* `;` *pair* `;` ...
A semicolon (\``;`') has the same effect as commencing a new argument.
Any sequence of one or more whitespace characters (space, tab, newline)
is equivalent to a single space.

A *pair* can take any of the following forms:
*filename vas*
*vas filename*
*filename*
*vas*

A *filename* must contain at least one period (\``.`') to distinguish it
from a variable name. This will be the case if netCDF filenames have a
conventional suffix such as the recommended `.nc`. (In any case it is
always possible to prefix a redundant \``./`' directory as in
\``./unconventional`' or \``/usr/./IdidItMyWay`'!)

A *vas* is a *variable or attribute specification* which can have any of
the following forms:
*var*
*var*`[`*subscript*`,` *subscript*`,` *subscript*`,` ...`]`
*var*`[`*subscript*`,` *subscript*`,` *subscript*`,` ...`)`
*var*`(`*subscript*`,` *subscript*`,` *subscript*`,` ...`]`
*var*`(`*subscript*`,` *subscript*`,` *subscript*`,` ...`)`
*var*`:`*att*
`:`*att*

where *var* is a variable name or ID number and *att* is an attribute
name or ID number. It is usually more convenient to identify variables,
attributes and dimensions by name rather than ID number. The use of ID
numbers is discussed in Section [*Using ID Numbers*](#Using_ID_Numbers).
Attributes are discussed in Section [*Attributes*](#Attributes).

A pair without a *filename* or *vas* uses that of the previous pair. The
first pair has no effect by itself unless it contains both a *filename*
and a *vas*. Thus the following all access the same values:

    $ nc2text 'vec.nc v[0 4]'
    10 50
    $ nc2text 'v[0 4] vec.nc'
    10 50
    $ nc2text vec.nc 'v[0 4]'
    10 50
    $ nc2text 'v[0 4]' vec.nc
    10 50
    $ nc2text '  v [  0   4 ]      vec.nc  '
    10 50

The following are equivalent ways of concatenating variables `v` and
`v_mean`:

    $ nc2text 'vec.nc v' 'vec.nc v_mean'
    10 20.3 30.7 40.9 50 60.5 70.2
    40.3714
    $ nc2text 'vec.nc v' 'v_mean'
    10 20.3 30.7 40.9 50 60.5 70.2
    40.3714
    $ nc2text 'vec.nc v; v_mean'
    10 20.3 30.7 40.9 50 60.5 70.2
    40.3714
    $ nc2text vec.nc v v_mean
    10 20.3 30.7 40.9 50 60.5 70.2
    40.3714

Now let us copy file `vec.nc` to `vec_new.nc` and then demonstrate
concatenation of data from different files:

    $ cp vec.nc vec_new.nc
    $ nc2text v vec.nc vec_new.nc
    10 20.3 30.7 40.9 50 60.5 70.2
    10 20.3 30.7 40.9 50 60.5 70.2
    $ nc2text v vec*.nc
    10 20.3 30.7 40.9 50 60.5 70.2
    10 20.3 30.7 40.9 50 60.5 70.2

Note the use of UNIX *wildcarding* facilities in the latter example
using the *metacharacter* \``*`' in `vec*.nc` which matches both
`vec.nc` and `vec_new.nc`.

Subscripts
----------

As mentioned in Section [*High level Syntax*](#High_level_Syntax),
subscripts are enclosed by either \``[`' or \``(`' on the left and
either \``]`' or \``)`' on the right.

A left bracket \``[`' implies the C convention of starting subscripts at
0; while a left parenthesis \``(`' implies the Fortran convention of
starting at 1. This starting value of 0 or 1 is called the *index
origin*. A mnemonic to associate *left* with *index origin* is an
*x-axis with the origin on the left*.

The right hand delimiter controls the relative significance of multiple
dimensions. A \``]`' implies conventional *row-major* (or
*lexicographic*) order in which the rightmost subscript varies fastest;
while a \``)`' implies the Fortran convention of *column-major* order in
which the leftmost subscript varies fastest.

So far our examples have involved only a single dimension. Now consider
a netCDF file `mat.nc` containing a 2-dimensional array (i.e. a matrix).
We print it as follows:

    $ ncdump mat.nc
    netcdf mat {
    dimensions:
            row = 2 ;
            col = 3 ;
    variables:
            short M(row, col) ;
    data:
     M =
      11, 12, 13,
      21, 22, 23 ;
    }

The following are equivalent ways of printing the final element:

    $ nc2text 'mat.nc M[1,2]'
    23
    $ nc2text 'mat.nc M(2,3]'
    23
    $ nc2text 'mat.nc M(3,2)'
    23
    $ nc2text 'mat.nc M[2,1)'
    23

Subscript values can be less than the index origin and are then relative
to the end. So the final element could also be accessed by:

    $ nc2text 'mat.nc M[-1,-1]'
    23
    $ nc2text 'mat.nc M(0,0)'
    23

As we have seen before, a subscript can contain a list of indices. Thus
one could use any of the following to select all rows, but exclude the
middle column:

    $ nc2text mat.nc 'M[0 1,0 2]'
    11 13
    21 23
    $ nc2text mat.nc 'M(1 2,1 3]'
    11 13
    21 23
    $ nc2text mat.nc 'M(1 3,1 2)'
    11 13
    21 23

### Triplet Notation

A sequence of indices forming an *arithmetic progression* as in

    $ nc2text vec.nc 'v[0 2 4 6]'
    10 30.7 50 70.2

can be specified using a generalization of Fortran 90 *triplet
notation*, in this case:

    $ nc2text vec.nc 'v[0:6:2]'
    10 30.7 50 70.2

The triplet `0:6:2` means *0 to 6 in steps of 2*. A *triplet* can take
two forms:
*start*`:`*finish*`:`*stride*
*start*`:`*finish*
The second form implies a stride of 1. It is possible to omit *start*
and/or *finish*. Let  I  be the index-origin (0 or 1). If the stride is
positive then *start* defaults to  I  (i.e. first element) and *finish*
to  I-1  (i.e. final element). These are reversed for a negative stride;
*start* defaults to  I-1  and *finish* to  I. E.g.

    $ nc2text vec.nc v
    10 20.3 30.7 40.9 50 60.5 70.2
    $ nc2text vec.nc 'v[:6:2]'
    10 30.7 50 70.2
    $ nc2text vec.nc 'v[0::2]'
    10 30.7 50 70.2
    $ nc2text vec.nc 'v[::2]'
    10 30.7 50 70.2
    $ nc2text vec.nc 'v[0:2]'
    10 20.3 30.7
    $ nc2text vec.nc 'v[:2]'
    10 20.3 30.7
    $ nc2text vec.nc 'v[2:]'
    30.7 40.9 50 60.5 70.2
    $ nc2text vec.nc 'v[::-1]'
    70.2 60.5 50 40.9 30.7 20.3 10

Note how the latter example reverses the order. A triplet can
wrap-around the start or end. This is useful with cyclic dimensions such
as longitude. Wrap-around is shown by:

    $ nc2text vec.nc 'v[3:1]'
    40.9 50 60.5 70.2 10 20.3
    $ nc2text vec.nc 'v[1:3:-1]'
    20.3 10 70.2 60.5 50 40.9

But the following does not imply wrap-around:

    $ nc2text vec.nc 'v[0:-1:1]'
    10 20.3 30.7 40.9 50 60.5 70.2

since `-1` means *final* (i.e. same as `6`). Each subscript can contain
any number of triplets and individual values. The colon (`:`) operator
has higher precedence than concatenation. This is shown by the
following:

    $ nc2text vec.nc 'v[2 :4]'
    30.2 40.9 50

which is equivalent to:

    $ nc2text vec.nc 'v[2:4]'
    30.2 40.9 50

However parentheses can be used to override this precedence rule. E.g.

    $ nc2text vec.nc 'v[2 (:4)]'
    30.2 10 20.3 30.2 40.9 50

### Omitting Subscripts

An omitted subscript implies the whole dimension. Thus we can print the
first row of `mat` as follows:

    $ nc2text mat.nc 'M[0]'
    11 12 13

and exclude the middle column by:

    $ nc2text mat.nc 'M[,0 -1]'
    11 13
    21 23

### Dimension Names

Dimension names play an important role in FAN. Instead of:

    $ nc2text mat.nc 'M(2 1,1 3]'
    21 23
    11 13

one can use:

    $ nc2text mat.nc 'M(row=2 1,col=1 3]'
    21 23
    11 13

This is clearer for human readers. But specifying dimension names also
provides the important facility of transposing dimensions. For example
this allows `ncrob` to produce statistics (e.g. means) for rows as well
as the normal columns. To transpose the above matrix, one could specify:

    $ nc2text mat.nc 'M(col=1 3,row=2 1]'
    21 11
    23 13

since the order in which dimensions are specified controls their order
in the output. To transpose a whole matrix one need only specify the
dimension names as in the following:

    $ nc2text mat.nc 'M[col,row]'
    11 21
    12 22
    13 23

or using column-major order:

    $ nc2text mat.nc 'M(row,col)'
    11 21
    12 22
    13 23

In fact only one dimension name is needed, since any not mentioned are
appended in their input order. E.g.

    $ nc2text mat.nc 'M[col]'
    11 21
    12 22
    13 23

### Indirect Indexing

So far we have located elements using direct index values. FAN also
allows an indirect method using *coordinate variables* (i.e. variables
with the same names as dimensions). Consider the following geographic
netCDF file `geog.nc`:

    $ ncdump geog.nc
    netcdf geog {
    dimensions:
            lat = 3 ;
            lon = 4 ;
    variables:
            float lat(lat) ;
                    lat:units = "degrees_north" ;
            float lon(lon) ;
                    lon:units = "degrees_east" ;
            double tsur(lat, lon) ;
    data:
     lat = -45 , 0 , 45  ;
     lon = -180 , -90 , 0 , 90  ;
     tsur =
      11, 12, 13, 14,
      21, 22, 23, 24,
      31, 32, 33, 34 ;
    }

FAN provides several *indirect indexing operators*. Perhaps the most
useful of these is \``~`', which gives the index of the coordinate value
*closest to* its argument. Thus:

    $ nc2text geog.nc 'lat[~-40]'
    -45

prints the latitude closest to 40°S and

    $ nc2text geog.nc 'tsur[~-40,~10]'
    13

prints the element of `tsur` closest to the point 40°S, 10°E. Note that
FAN knows nothing about circular wrap-around and does not consider 360°
to be equal to 0°. The following shows how indirect indexing can be used
within triplets:

    $ nc2text geog.nc 'tsur[ lat = ~90:~-90:-2 , lon = ~10: ]'
    33 34
    13 14

This gives every second latitude from that closest the north pole to
that closest the south pole, and all longitudes from that closest to
10°E to the final one. The other indirect indexing operators are as
follows:


------------ | -----------------------------------------------------------------------------------------
`@ max <`    | index value corresponding to maximum coordinate value less than argument
`@ max <=`   | index value corresponding to maximum coordinate value less than or equal to argument
`@ min >`    | index value corresponding to minimum coordinate value greater than argument
`@ min >=`   | index value corresponding to minimum coordinate value greater than or equal to argument

Thus the following prints the minimum longitude greater than 10°E:

    $ nc2text geog.nc 'lon[@ min > 10]'
    90

and the following retrieves the rows from the *maximum latitude less
than or equal to 30°N* to the *closest latitude to 90°N*, and the
columns from the second (i.e 1 with respect to index origin of 0) to
*minimum longitude greater than 0*.

    $ nc2text geog.nc 'tsur[lat= @max<=30 : ~90, lon= 1 : @min > 0]'
    22 23 24
    32 33 34

### Offsets

It is possible to specify *offsets* using an expression of the form\
*index* `+` *offset*\
where *offset* is an integer constant (which can be negative). The
offset must be the right hand argument of \``+`'. Note that this \``+`'
operator has even higher precedence than \``:`'. Here are some examples
of the use of offsets:

    $ nc2text geog.nc 'lon[ ~-100 + -1 : ~-360 + 2 ]'
    -180 -90 0

prints the longitudes from that *one before the closest to 100°W* to
that *two beyond the closest to 360°W*. Note how the negative offset is
specified as \``+ -1`', which is *not* equivalent to \``-1`' as in:

    $ nc2text geog.nc 'lon[ ~-100-1 : ~-360 + 2 ]'
    -90 90 -180 -90 0

which is equivalent to both the following (Note the wrap-around.):

    $ nc2text geog.nc 'lon[ (~-100) (-1:~-360 + 2) ]'
    -90 90 -180 -90 0
    $ nc2text geog.nc 'lon[ 1 3:2 ]'
    -90 90 -180 -90 0

One use for offsets is to append along the `UNLIMITED` dimension without
needing to know its current size. The expression \``-1+1`' represents
the index value for appending immediately after the current final
record. Thus we could append a value to variable `v` in file
`vec_new.nc` (whose `UNLIMITED` dimension `n` has the current size 7)
by:

    $ echo 80 | text2nc 'vec_new.nc v[-1 + 1]'
    $ nc2text 'vec_new.nc v'
    10 20.3 30.7 40.9 50 60.5 70.2 80

Then we could append two more values by:

    $ echo 90 100.1 | text2nc 'vec_new.nc v[ -1 + 1 : -1 + 2 ]'
    $ nc2text 'vec_new.nc v'
    10 20.3 30.7 40.9 50 60.5 70.2 80 90 100.1

giving a new size of 10 for the `UNLIMITED` dimension.

### Coordinate Variable Unit Conversion

In file `geog.nc` the `units` attribute is `degrees_north` for `lat` and
`degrees_east` for `lon`. One may want to specify coordinate values in
some other units. The following shows how this can be done by appending
the unit (enclosed in braces i.e. \``{}`') to the value:

    $ nc2text geog.nc 'tsur[ lat=~0.8{radian}, lon = ~ -1.5 { radian } ]'
    32

giving the value at the point closest to latitude 0.8 radians north and
longitude 1.5 radians west. This unit conversion (like that during FAN
input and output) is done using the Unidata units library discussed in
Appendix C of [User Guide](#user_guide) \ref user_guide.

[](){#Attributes}

Attributes
----------

As noted in Section [*High level Syntax*](#High_level_Syntax) an
attribute *vas* can take two forms:\
*var*`:`*att*\
`:`*att*\
As in CDL, the latter denotes a *global attribute*. The following writes
the global attribute `title` and then reads and prints it:

    $ echo 'Sample geographic file' | text2nc -h 'geog.nc :title'
    $ nc2text 'geog.nc :title'
    Sample geographic file

(The `-h` flag means \`*Do not append a line to the global attribute*
`history`'.)

Attributes cannot have subscripts, so there is no way of accessing only
part of an attribute. Attributes are automatically created if they do
not exist and their type and size can be changed. The following gives
variable `lat` the new attribute `valid_range` (with type `float`) and
then prints it:

    $ echo -90 90 | text2nc -h -t float 'geog.nc lat:valid_range'
    $ nc2text 'geog.nc lat:valid_range'
    -90 90

The following gives variable `lat` another new attribute `foo` (by
copying variable `v` from file `vec.nc`), then modifies it, then deletes
it.

    $ nc2text 'vec.nc v[:4]' | text2nc -h -t double 'geog.nc lat:foo'
    $ nc2text 'geog.nc lat:foo'
    10 20.3 30.2 40.9 50
    $ echo 'Hello' | text2nc -h 'geog.nc lat:foo' # Modify attribute 'lat:foo'
    $ nc2text 'geog.nc lat:foo'
    Hello
    $ text2nc -h 'geog.nc lat:foo' < /dev/null    # Delete attribute 'lat:foo'

Note how one can delete attributes by changing their size to 0. The file
`/dev/null` is a standard UNIX pseudo-file that is empty for input.

[](){#Using_ID_Numbers}

Using ID Numbers for Variables, Dimensions and Attributes
---------------------------------------------------------

It is possible to use ID numbers in place of names for variables,
dimensions and attributes. However dimension ID numbers must be followed
by `=` so they can be distinguished from index values. ID numbers begin
at 0 regardless of the index origin. Negative values are relative to the
end, which is represented by `-1`.

There are some situations where ID numbers are more convenient than
names. For example, one might adopt the convention that coordinate
variables should be defined first, after which there should be only a
single other (main) variable in each file. A shell-script to process
such files can refer to the main variable as `-1`. The following shows
the use of such a variable ID number:

    $ nc2text geog.nc -1
    11 12 13 14
    21 22 23 24
    31 32 33 34

The following prints the first attribute of the second variable:

    $ nc2text geog.nc '1:0'
    degrees_east

The following Korn shell script `pratts` prints all the non-global
attributes in the files specified by its arguments.

    $ cat pratts
    #!/bin/ksh
    for FILE
    do
        integer VARID=0
        # Following true if variable VARID exists
        while VARNAME="$(ncmeta -s -w v $FILE $VARID)"; test -n "$VARNAME"
        do
            integer ATTID=0
            # Following true if attribute ATTID exists
            while ATTNAME="$(ncmeta -s -w a $FILE $VARID:$ATTID)"; test -n "$ATTNAME"
            do
            print -n "$FILE $VARNAME:$ATTNAME "
            nc2text $FILE "$VARNAME:$ATTNAME"
            (( ATTID += 1 ))
            done
            (( VARID += 1 ))
        done
    done

We can use `pratts` on file `geog.nc` as follows:

    $ pratts geog.nc
    geog.nc lat:units degrees_north
    geog.nc lat:valid_range -90 90
    geog.nc lon:units degrees_east

FAN Utilities
=============

Introduction to Utilities
-------------------------

This section provides a more detailed description of the four FAN
utilities, `nc2text`, `text2nc`, `ncmeta` and `ncrob`, commencing with
some features common to several utilities. The usage summaries in
Sections [*nc2text Usage*](#nc2text_Usage), [*text2nc
Usage*](#text2nc_Usage), [*ncmeta Usage*](#ncmeta_Usage) and [*ncrob
Usage*](#ncrob_Usage) can be printed by entering the command name
without any arguments.

All netCDF types (`char, byte, short, long, float` and `double`) can be
read and written. During input/output there is automatic conversion to
or from type `double`, which is used for internal storage and
processing.

### Options Common to several Utilities

The two flags `-h` and `-H` specify what is to be written to the global
attribute `history`. The `-h` flag means \`*Do not write any history*'.
The `-H` flag means \`*Exclude time-stamp and user-name* (`LOGNAME`)
*from history*'. This flag is useful in program testing, since it causes
the same values to be written to `history` each time, thus facilitating
comparison of actual output with that expected.

Section 4.5 of [NetCDF User's Guide](/software/netcdf/guide_toc.html)
explains the two aspects of error-handling: suppression of error
messages and fatality of errors. The default mode is *verbose* and
*fatal*. *Non-verbose (silent)* mode is set by flag `-s`. *Non-fatal
(persevere)* mode is set by flag `-p`.

The `-e` flag means \`*Write error messages to* `stdout` *not*
`stderr`'.

The option \``-t` *type*' sets the data-type for new variables or
attributes. Valid values are `char, byte, short, long, float` and
`double`. These can be abbreviated to their first letter.

The option \``-u` *unit*' sets the unit of measure for ASCII text data,
providing conversion to or from those defined by netCDF `units`
attributes.

[](){#Scaling}

### Scaling and Unit Conversion

All netCDF input and output values are transformed by a linear equation
defined by the attributes `add_offset`, `scale_factor` and `units`;
together with any unit defined by the `-u` option mentioned above. The
output `units` attribute is defined or modified in some situations such
as when it is undefined but the corresponding input attribute is
defined.

All unit conversion is done using the *Units Library* documented in
Appendix C of [NetCDF User's Guide](/software/netcdf/guide_toc.html).
The environment variable `UDUNITS_PATH` can be used to specify a
non-standard units file. (See `man` document `udunits(3)`.)

### Missing Values

Values read from a netCDF file are considered missing if outside the
*valid range* defined by the attribute `valid_range` or the attributes
`valid_min`, and `valid_max`. If these do not define either the minimum
or the maximum then an attempt is made to define it based on the
principle that the *missing value* must be outside the valid range. The
*missing value* is defined by the attribute `missing_value`, or if this
is undefined then the *fill value* (defined by attribute `_FillValue` if
defined, otherwise the default fill value for the data type).

### Environment Variables

The environment variable `UDUNITS_PATH` was mentioned in Section
[*Scaling*](#Scaling). The environment variable `COLUMNS` (default: 80)
defines the page width and is used to print data of type `character`.

nc2text
-------

This utility prints variable and attribute values from netCDF files.

[](){#nc2text_Usage}

### Usage

    Usage: nc2text [-eps] [-f %s] [-m %s] [-n %d] [-u %s] <FANI>
    <FANI> netCDF FAN specification for input
    -e            Write error messages to stdout not stderr
    -p            Persevere after errors
    -s            Silent mode: Suppress warning messages
    -f <string>:  Format for output (default: C_format attribute ("%G" if none))
    -m <string>:  Missing value for output (default: _ )
    -n <integer>: Number of fields per line of output (default: 10 if numeric)
                  (Environment variable COLUMNS defines default for characters)
    -u <string>:  Unit of measure for output (default: unit in file)

### Examples

The following prints the first three elements of variable `v` of file
`vec.nc`:

    $ nc2text 'vec.nc v[0 1 2]'
    10 20.3 30.2

The following uses `text2nc` to

-   set attribute `v:units` to \``degF`'
-   set attribute `v:valid_min` to -460°F (just below 0°K)
-   modify `v[2]` so it is less than this valid minimum i.e. missing.

<!-- -->

    $ echo degF | text2nc vec.nc 'v:units'
    $ echo -460 | text2nc -t float vec.nc 'v:valid_min'
    $ echo -999 | text2nc vec.nc 'v[2]'

Then we print four Celsius temperatures per line. The text \``MISSING`'
is printed for missing values. Normal values are printed using the C
format `%8.4f` (equivalent to the Fortran format `F8.4` i.e 4 decimal
places with a total field width of 8 characters).

    $ nc2text -f '%8.4f' -m ' MISSING' -n 4 -u degC vec.nc 'v[:4]'
    -12.2222  -6.5000  MISSING   4.9444
     10.0000

ncmeta
------

This utility prints metadata from netCDF files. This metadata can
include rank, shape, file names, variable names, dimension names and
attribute names.

[](){#ncmeta_Usage}

### Usage

    Usage: ncmeta [-eps] [-w <LETTERS>] <FANI>
    <FANI> netCDF FAN specification for input
    -e            Write error messages to stdout not stderr
    -p            Persevere after errors
    -s            Silent mode: Suppress warning messages
    -w <LETTERS>: What to print using following (default: s)
                     a: attribute names
                     d: dimension names
                     f: file names
                     r: rank (number of dimensions)
                     s: shape (dimension sizes)
                     v: variable names

    Example: ncmeta -w fvs abc.nc var1 var2

[](){#ncmeta_Examples}

### Examples

The following examples print the shape of the specified variables:

    $ ncmeta vec.nc v
    5
    $ ncmeta geog.nc tsur
    3 4

The following example prints the filename, variable name, rank,
dimensions and shape of the specified variables:

    $ ncmeta -w fvrds vec.nc v 'geog.nc tsur' lat lon
    vec.nc v 1 n 5
    geog.nc tsur 2 lat lon 3 4
    geog.nc lat 1 lat 3
    geog.nc lon 1 lon 4

The following example prints the variable name and attribute name of the
first (0) attribute of the first (0) variable:

    $ ncmeta -w va geog.nc '0:0'
    lat units

ncrob
-----

This utility reads data from one or more netCDF variables, performs some
process on it and then either prints the result or writes it to one or
more netCDF variables. The type of process is defined by option \``-r`
*string*', where *string* is one of the following:

  ------------- ------------------------------------------------
  `am`          arithmetic mean
  `broadcast`   cyclic copy
  `count`       number of non-missing values
  `fill`        fill with missing values
  `gm`          geometric mean
  `max`         maximum
  `min`         minimum
  `prod`        product
  `sd`          unadjusted standard deviation (divisor is  n )
  `sd1`         adjusted standard deviation (divisor is  n-1 )
  `sum`         sum
  `sum2`        sum of squares of values
  ------------- ------------------------------------------------

A `broadcast` copies successive elements from input to output. Whenever
the end of input is reached, reading begins again at the start of input.
The whole process continues until reaching the end of output.

A `fill` simply fills the output variable with missing values. There
must be input, although it is used only to define the shape of new
variables.

The other processes are all *reductions*, in the sense that they reduce
the *rank* (number of dimensions). The number of input elements ( I )
must be a multiple of both the number of output elements ( N ) and the
number of weights ( M ) (if any, as specified by option `-w`).

If the process is `count` and there are no weights then the result is
the number of non-missing values. If there are weights then the result
is the sum of the weights of the non-missing values.

Let vector  X~0~, X~1~, ...,  X~i~, ..., X ~I-1~  represent the selected
input data elements in the specified order. Similarly, let vector
 Y~0~, Y~1~, ...  Y~j~, ..., Y ~N-1~  represent the resultant output
data. Let  n = I ° N.

If the process is `sum` and there are no weights then

  Y~j~ =  **Sum**~i=0,n-1~ X ~Ni+j~  

If weights  W~0~, W~1~, ...,  W~k~, ..., W ~M-1~  are defined and
 m = I ° M  then

  Y~j~ =  **Sum**~i=0,n-1~ W ~**floor**((Ni+j)/m)~X ~Ni+j~  

where  **floor**( x )  represents the floor of  x  i.e. the greatest
integer  &lt;= x.

This is calculated using the following algorithm:

 n  `:=`  I°N \
 m  `:=`  I°M \
**for**  j  **from** 0 **to**  N-1 \
`   `  Y~j~  `:=` 0\
**for**  i  **from** 0 **to**  I-1 \
`   `  j  `:=`  i **mod** n \
`   `  k  `:=`  **floor**( i/m ) \
`   ` **if**  Y~j~ °=  `missing_value`\
`   ` `   ` **if** `valid_min`  &lt;= X~i~ &lt;=  `valid_max`\
`   ` `   ` `   `  Y~j~  `:=`  Y~j~ + W ~k~ X~i~ \
`   ` `   ` **else if** `suddenDeath`\
`   ` `   ` `   `  Y~j~  `:=` `missing_value`\

Note that this definition of  k  means that the first  m  elements have
the first weight  W~0~ , the next  m  have the second weight  W~1~ , and
so on.

As an example consider an input array which is a matrix  A  with  R 
rows and  C  columns. Thus  I=RC. If we want column sums then the output
vector would be of length  C  i.e.  N=C. Now  n= I ° N =  R. So the
unweighted sum is

  Y~j~ =  **Sum**~i=0,R-1~ X ~Ci+j~ = **Sum** ~i=0,R-1~ A~ij~  

and the weighted sum is

  Y~j~ =  **Sum**~i=0,R-1~ W ~**floor**((Ci+j)/C)~  X~Ci+j~ = **Sum**
~i=0,R-1~ W~i~  A~ij~  

If the process is `prod` and there are no weights then

  Y~j~ =  **Product**~i=0,n-1~ X ~Ni+j~  

If weights are defined then

  Y~j~ =  **Product**~i=0,n-1~ X ~Ni+j~ ^W\\ ~**floor**((Ni+j)/m)~^  

In general the shape (dimension vector) of the destination should match
the trailing dimensions of the source. Then the reduction process
operates over those leading dimensions absent from the destination.

Note that FAN allows you to transpose dimensions by specifying them in
an order different from that in the file. Thus the leading source
dimensions are those specified first. The order of the remaining
dimensions must match those of the destination.

The other reduction processes are treated similarly. However `min` and
`max` do not allow weights.

If the `-m` flag is specified then the result is missing if any of the
values it depends on is missing (*sudden death mode*). Otherwise missing
values are omitted (*filter mode*) i.e. essentially treated as having a
weight of 0.

The `-b` option sets the size of the input buffer. This can improve
efficiency when reading very large variables.

The `-c` option creates a new destination variable with the specified
rank (number of dimensions). If the variable already exists then this
option is ignored. If the destination file does not exist then it is
created. The variable is created with the same attributes as the (first
if several) source variable, and the specified number of its trailing
dimensions, together with any associated coordinate variables. However a
broadcast is slightly different in that a new leading dimension is
created from the leading source dimensions by taking the product of
their sizes (so the total number of elements is unchanged) and
concatenating their names. The data-type of the new variable is
specified using option `-t` and defaults to the type of the source
variable.

[](){#ncrob_Usage}

### Usage

    Usage: ncrob [options] <FANI> / <FANO>
    <FANI>: FAN specification for input
    <FANO>: FAN specification for output (default: stdout)
    -e            Write error messages to stdout not stderr
    -H            Exclude time-stamp & LOGNAME from history
    -h            Do not write history
    -m            If any value missing then result missing
    -p            Persevere after errors
    -s            Silent mode: Suppress warning messages
    -b <int>:     Max. buffer size (Kbytes) (default: 512)
    -c <int>:     Rank (decrement if < 0) of any Created variable including stdout
                  (default: input rank for broadcast, else -1)
    -f <string>:  Format for stdout (default: C_format attribute ("%G" if none))
    -M <string>:  Missing value for stdout (default: _ )
    -n <integer>: Number of fields per line for stdout (default: 10 if numeric)
                  (Environment variable COLUMNS defines default for characters)
    -r <string>:  Reduction type (am broadcast count fill gm max min prod
                   sd sd1 sum sum2) (default: broadcast)
    -t char|byte|short|long|float|double: new variable type (default: input type)
    -u <string>:  Unit of measure for stdout (default: unit in file)
    -w <reals>:   Weight vector(e.g. -w '3 1.5 .8')

If the \``/`' is omitted then the final argument is taken as `<FANO>`.
(This version 1 convention is deprecated.) If `<FANO>` does not specify
a filename or variable name then the first one in `<FANI>` is used.

### Examples

The following prints the variable `M` in file `mat.nc`:

    $ ncrob mat.nc M /
    11 12 13
    21 22 23

The following prints the column sums, row means and overall product:

    $ ncrob -r sum mat.nc M /       # sum of each column
    32 34 36
    $ ncrob -r am mat.nc 'M[col]' / # arithmetic mean of each row
    12 22
    $ ncrob -r prod -c 0 -f '%.0f' mat.nc M / # overall product
    18234216

The first two commands have no `-c` option, so the rank of the result is
one less than that of the input. The third specifies `-c 0`, so the
result has rank 0, i.e. is scalar. The following attempts to put this
same data into three new variables in the same file:

    $ ncrob -h -r sum mat.nc M / col_sum
    $ ncrob -h -r am mat.nc 'M[col]' / row_am
    $ ncrob -h -r prod -c 0 mat.nc M / prod
    $ ncdump mat.nc
    netcdf mat {
    dimensions:
            row = 2 ;
            col = 3 ;
    variables:
            short M(row, col) ;
            short col_sum(col) ;
            short row_am(row) ;
            short prod ;
    data:
     M =
      11, 12, 13,
      21, 22, 23 ;
     col_sum = 32, 34, 36 ;
     row_am = 12, 22 ;
     prod = _ ;
    }

Why is `prod` dumped as `_` (i.e. fill) rather than 18234216 as before?
The problem is that `ncrob` has created a new variable of the same type
as the source variable, which in this case is `short` and incapable of
storing such a large number. The solution is to specify the type using
the `-t` option. Let's also create a new file `prod.nc`. E.g.

    $ ncrob -h -r prod -c 0 -t long mat.nc M / prod.nc prod
    $ ncdump prod.nc
    netcdf prod {
    variables:
            long prod ;
    data:
     prod = 18234216 ;
    }

Next let's calculate a weighted mean of each column. Let's give the
first row twice the weight of the second:

    $ ncrob -r am -w '2 1' M mat.nc /
    14.3333 15.3333 16.3333

Thus the mean of the first column is (2 \* 11 + 1 \* 21)/3 = 14.3333.
Negative weights can be used to obtain differences. E.g.

    $ ncrob -r sum -w '1 -1' M mat.nc /            # row1 - row2
    -10 -10 -10
    $ ncrob -r sum -w '1 -1' 'M(col=3 1]' mat.nc / # col3 - col1

Finally we demonstrate broadcasting. Let's first copy the matrix `M` to
two variables in a new file called `new.nc`. One new variable has the
same name (`M`) and shape. The other is named `V` and is a vector with
the new dimension `row_col` formed from dimensions `row` and `col`.

    $ ncrob -h M mat.nc / new.nc
    $ ncrob -h -c 1 M mat.nc / new.nc V
    $ ncdump new.nc
    netcdf new {
    dimensions:
            row = 2 ;
            col = 3 ;
            row_col = 6 ;
    variables:
            short M(row, col) ;
            short V(row_col) ;
    data:
     M =
      11, 12, 13,
      21, 22, 23 ;
     V = 11, 12, 13, 21, 22, 23 ;
    }

Now let's broadcast the variable `col_sum` in file `mat.nc` to these
variables `M` and `V` in the file `new.nc`:

    $ ncrob -h mat.nc col_sum / new.nc M V
    $ ncdump new.nc
    netcdf new {
    dimensions:
            row = 2 ;
            col = 3 ;
            row_col = 6 ;
    variables:
            short M(row, col) ;
            short V(row_col) ;
    data:
     M =
      32, 34, 36,
      32, 34, 36 ;
     V = 32, 34, 36, 32, 34, 36 ;
    }

Four copies of the input were needed to fill the output.

text2nc
-------

This utility reads ASCII text data from standard input and writes it to
a netCDF variable or attribute. The netCDF file and variable must
already exist. However, as discussed in Section
[*Attributes*](#Attributes), `text2nc` can create attributes, delete
them, and modify their type, size and value. If end-of-input occurs
before end-of-output then the input values are recycled.

[](){#text2nc_Usage}

### Usage

    Usage: text2nc [-eHhps] [-m %f] [-u %s] <FANO>
    <FANO>: netCDF FAN specification for output
    -e           Write error messages to stdout not stderr
    -H           Exclude time-stamp & LOGNAME from history
    -h           Do not write history
    -p           Persevere after errors
    -s           Silent mode: Suppress warning messages
    -m <real>:   Missing value for input (default: 1.79769E+308)
    -t char|byte|short|long|float|double: data-type (for attributes only)
                 (default: double for numeric input data, else char)
    -u <string>: Unit of measure for input (default: unit in file)

[](){#text2nc_Examples}

### Examples

Let's start with the following file:

    $ ncdump vec.nc
    netcdf vec {
    dimensions:
            n = UNLIMITED ; // (5 currently)
    variables:
            float v(n) ;
    data:
     v = 10 , 20.3 , 30.2 , 40.9 , 50  ;
    }

Let's assume these data are Celsius temperatures, so we define a valid
minimum by:

    $ echo -273.2 | text2nc -h vec.nc 'v:valid_min'

Then we modify two existing values:

    $ echo 15 17 | text2nc -h -u degC vec.nc 'v[0 3]'
    $ ncdump vec.nc
    netcdf vec {
    dimensions:
            n = UNLIMITED ; // (5 currently)
    variables:
            float v(n) ;
                    v:valid_min = -273.2 ;
                    v:units = "degC" ;
    data:
     v = 15 , 20.3 , 30.2 , 17 , 50  ;
    }

Note that the `units` attribute was created because we specified
`-u degC`, but there was no existing `units` attribute. Now let's append
three values and print the resulting `v`:

    $ echo -999 32 1e9 | text2nc -h -u degF -m 1e9 vec.nc 'v[-1+1:-1+3]'
    $ nc2text -f '%0.1f' vec.nc v
    15.0 20.3 30.2 17.0 50.0 _ 0.0 _

The first value (`-999`) is treated as missing because (even after
conversion to Celsius) it is less than the valid mininum of `-273.2`.
The second value (32°F) is converted to 0°C. The third value (`1e9`) is
treated as missing because it matches the input missing value specified
by `-m 1e9`.

Finally, let's change every second value to 0°K:

    $ echo 0 | text2nc -h -u degK vec.nc 'v[1::2]'
    $ nc2text -f '%0.1f' vec.nc v
    15.0 -273.1 30.2 -273.1 50.0 -273.1 0.0 -273.1
