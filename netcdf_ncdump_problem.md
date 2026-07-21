# ncdump Failure with User-Defined Format Files

## Summary

`ncdump -hs` can successfully read metadata from a NetCDF-C User-Defined Format
(UDF) file, but it fails at the end of the header dump with an error such as:

```text
ncdump: unrecognized file format: 9
```

The number (`9` in this example) is the NetCDF-C extended format code for the
UDF slot used by the file. The failure occurs when `ncdump` tries to print the
virtual `_Format` global attribute, not while reading the file itself.

## Observed Symptom

Opening a NEP GeoTIFF file with `ncdump -hs` produces the full dimension,
variable, and attribute output, then aborts after the `// global attributes:`
line:

```text
netcdf ABBA_2022_C61_HNL {
dimensions:
        x = 55877 ;
        y = 41013 ;
        bnds = 2 ;
variables:
        ubyte data(y, x) ;
                data:_Storage = "contiguous" ;
        ...
// global attributes:
ncdump: unrecognized file format: 9
```

The file is a standard little-endian TIFF recognized by NEP's GeoTIFF UDF
handler. NEP maps GeoTIFF standard TIFF to UDF slot 1:

```c
/* include/geotiffdispatch.h */
#define NC_FORMATX_NC_GEOTIFF NC_FORMATX_UDF1
```

NetCDF-C defines `NC_FORMATX_UDF1` as 9:

```c
/* netcdf.h */
#define NC_FORMATX_UDF0      (8)
#define NC_FORMATX_UDF1      (9)
```

NEP's `NC_GEOTIFF_inq_format()` returns that code for `nc_inq_format()`:

```c
/* src/geotifffile.c */
int
NC_GEOTIFF_inq_format(int ncid, int *formatp)
{
    (void)ncid;
    if (formatp)
        *formatp = NC_FORMATX_NC_GEOTIFF;
    return NC_NOERR;
}
```

## Root Cause

`ncdump` prints the synthetic `_Format` attribute by calling `nc_inq_format()`
and then passing the returned code to `kind_string()`:

```c
/* ncdump/ncdump.c */
static void
pr_att_global_format(int ncid, int kind)
{
    pr_att_name(ncid, "", NC_ATT_FORMAT);
    printf(" = ");
    printf("\"%s\"", kind_string(kind));
    printf(" ;\n");
}
```

`kind_string()` only recognizes the legacy `NC_FORMAT_*` values (1–4):

```c
/* ncdump/ncdump.c */
static const char *
kind_string(int kind)
{
    switch (kind) {
    case NC_FORMAT_CLASSIC:
        return "classic";
    case NC_FORMAT_64BIT_OFFSET:
        return "64-bit offset";
    case NC_FORMAT_CDF5:
        return "cdf5";
    case NC_FORMAT_NETCDF4:
        return "netCDF-4";
    case NC_FORMAT_NETCDF4_CLASSIC:
        return "netCDF-4 classic model";
    default:
       error("unrecognized file format: %d", kind);
        return "unrecognized";
    }
}
```

For a UDF file, `nc_inq_format()` returns the extended format code (`9` for
UDF1, `8` for UDF0, etc.), but `kind_string()` has no case for those values.
It falls through to the `default` branch and emits the error.

`kind_string_extended()` has the same limitation: it handles
`NC_FORMATX_NC3`, `NC_FORMATX_NC_HDF5`, `NC_FORMATX_NC_HDF4`,
`NC_FORMATX_PNETCDF`, `NC_FORMATX_DAP2`, `NC_FORMATX_DAP4`, and
`NC_FORMATX_UNDEFINED`, but has no cases for `NC_FORMATX_UDF0` through
`NC_FORMATX_UDF9`.

## Difference Between `kind_string()` and `kind_string_extended()`

- **`kind_string(int kind)`** is used for the legacy `_Format` virtual global
  attribute. It expects the value returned by `nc_inq_format()`, which for
  built-in formats is one of the `NC_FORMAT_*` constants (`NC_FORMAT_CLASSIC`,
  `NC_FORMAT_NETCDF4`, etc.). It returns short names such as `"classic"` or
  `"netCDF-4"`.

- **`kind_string_extended(int kind, int mode)`** is used when the user
  requests extended format information (for example with `ncdump -k`). It
  expects the value returned by `nc_inq_format_extended()` and prints the
  format family plus the open mode in hexadecimal (for example,
  `"HDF5 mode=00001000"`). It works with the `NC_FORMATX_*` extended format
  codes rather than the legacy `NC_FORMAT_*` codes.

The UDF bug is triggered through `kind_string()` because `pr_att_global_format()`
calls `nc_inq_format()` and passes the result to `kind_string()`. For a UDF
file, `nc_inq_format()` returns the extended UDF code, which `kind_string()`
does not recognize.

## Why Metadata Prints Before the Error

`ncdump` reads dimensions, variables, attributes, and storage properties
through the UDF dispatch table before it attempts to print the `_Format`
virtual attribute. That is why the rest of the CDL output is correct even
though the dump aborts at the end.

## Workaround

Use `ncdump -h` without the `-s` flag. The `-s` option enables special virtual
attributes, including `_Format`. Without `-s`, `ncdump` does not call
`pr_att_global_format()` and therefore does not trigger the `kind_string()`
failure.

## Proposed Fix

Change `kind_string()` in `ncdump/ncdump.c` so that UDF format codes fall back
to a generic label instead of calling `error()`. The simplest fix is to add
a range check in the `default` case:

```c
default:
    if (kind >= NC_FORMATX_UDF0 && kind <= NC_FORMATX_UDF9)
        return "user-defined format";
    error("unrecognized file format: %d", kind);
    return "unrecognized";
```

This prevents `ncdump` from aborting while printing `_Format` for any UDF file.
Format-specific names (for example, mapping UDF1 to `"GeoTIFF"`) can be added
later, but the immediate behavior should be to print `"user-defined format"`
for every UDF slot.

`kind_string_extended()` should be updated the same way: either add cases for
`NC_FORMATX_UDF0` through `NC_FORMATX_UDF9`, or add a range check in the
default branch so that extended format output also prints
`"user-defined format"` instead of `"unrecognized"`.

## Impact

Until `ncdump` is fixed, any NEP UDF file opened with `ncdump -hs` will fail
at the end of the header dump. This affects:

- GeoTIFF / BigTIFF files (`NC_FORMATX_UDF0` / `NC_FORMATX_UDF1`)
- GRIB2 files (`NC_FORMATX_UDF2`)
- FITS files (`NC_FORMATX_UDF3`)
- NASA CDF files (`NC_FORMATX_UDF4`), once CDF auto-detection is working
- PDS4 files (`NC_FORMATX_UDF5`)

The underlying file reading works correctly; only the final format-name
annotation is broken.

## Related Issues

- `docs/netcdf_magic_problem.md` — discusses why CDF magic numbers cannot
  currently be placed in `.ncrc`, which is a separate but related UDF
  auto-detection problem.
- `include/geotiffdispatch.h` — defines `NC_FORMATX_NC_GEOTIFF` as
  `NC_FORMATX_UDF1`.
- `src/geotifffile.c` — `NC_GEOTIFF_inq_format()` returns the UDF1 code.
- `ncdump/ncdump.c` in the NetCDF-C source tree — `kind_string()`,
  `kind_string_extended()`, and `pr_att_global_format()`.
