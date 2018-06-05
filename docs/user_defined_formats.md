User-Defined Formats for NetCDF {#user_defined_formats}
===============================

[TOC]

User-Defined Formats {#udf_user_defined_formats}
=====================================

## Introduction {#udf_Introduction}

User-defined formats allow users to write their own adaptors for the
netCDF C library, so that it can read and (optionally) write a
proprietary format through the netCDF API.

User-defined format code is packaged into a separate library, the
user-defined format dispatch library. This library, when linked with
the netCDF library, will allow user programs to read their proprietary
format through the netCDF API. The proprietary format is treated as if
it were one of the netCDF C library native binary formats.

Coding the user-defined format dispatch library requires knowledge of
the netCDF library internals. User-defined format dispatch libraries
must be written in C.

## Using User-Defined Formats from C Programms {#udf_With_C}

A user-defined format can be added dynamically in the case of C programs.

```
      /* Add our test user defined format. */
      if (nc_def_user_format(NC_UDF0, &tst_dispatcher, NULL)) ERR;
```

The file can now be opened by netCDF:

```
      if (nc_open(FILE_NAME, NC_UDF0, &ncid)) ERR;
```

## Building NetCDF C Library with a User-Defined Format Library {#udf_Build_NetCDF_With_UDF}

Once a user-defined format library is created, it may built into a
netCDF install. This allows the netCDF Fortran APIs, and the netCDF
utilities (ncdump, ncgen, nccopy) to natively use the user-defined
format.

## Creating a User-Defined Format {#udf_Create_UDF}

Creators of user-defined format libraries will have to become familar
with the internals of the netCDF-4 code.

## Examples {#udf_Examples}

The most simple-case example of a user-defined format is provided in
test nc_test4/tst_udf.c.

A slightly more complex example, including the required
autoconf/automake files to build a user-defined format library, can be
found at the [sample user-defined format
library](https://github.com/NOAA-GSD/sample-netcdf-dispatch). In this
example, the HDF4 SD reader is re-implemented as an external
user-defined format. (This is unnecessary if you jusr want to read
HDF4 SD files, since the netCDF C library already includes an HDF4 SD
file reader. This user-defined format library uses the same code. It
is repackaged as a user-defined library to provide a working sample.)

