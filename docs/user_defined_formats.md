User-Defined Formats {#user_defined_formats}
====================================

# Introduction {#udf_intro}

NetCDF-C supports user-defined formats (UDFs) that allow developers to extend the library to work with custom file formats. The library provides 64 UDF slots (0 through 63) that can be registered either programmatically or via RC file configuration.

User-defined formats enable:
- Support for proprietary or specialized file formats
- Custom storage backends
- Format translation and adaptation layers
- Integration with domain-specific data formats

The [NetCDF Expansion Pack (NEP)](https://github.com/Intelligent-Data-Design-Inc/NEP) is a working example of this feature. It uses UDF plugins to read GRIB2, FITS, GeoTIFF, PDS4, CDF, DICOM, and PDB files through the standard netCDF API. Consult its source for complete, production UDF dispatch tables and plugin initialization code.

# Available UDF Slots {#udf_slots}

There are NC_MAX_UDF_FORMATS (64) slots, numbered 0 through 63. A UDF mode flag is built with the NC_UDF(n) macro: NC_UDF_FLAG (bit 6, 0x0040) marks the mode as user-defined, and the slot number is stored in a 6-bit field at bits 19-24 (NC_UDF_NUM_SHIFT/NC_UDF_NUM_MASK). The convenience macros NC_UDF0 through NC_UDF9 are equivalent to NC_UDF(0) through NC_UDF(9); NC_UDF0 keeps its historic value of 0x0040.

Each slot can be independently configured with its own dispatch table, initialization function, and optional magic number for automatic format detection. Because the slot number is a field rather than individual bits, only one UDF slot can be selected per mode; do not combine two NC_UDF(n) values with bitwise OR, as that produces the flag for a different slot.

# Registering UDFs Programmatically {#udf_programmatic}

## Using nc_def_user_format()

The `nc_def_user_format()` function registers a user-defined format:

```c
int nc_def_user_format(int mode_flag, NC_Dispatch *dispatch_table, 
                       char *magic_number);
```

**Parameters:**
- `mode_flag`: NC_UDF(n) for slot n (0-63), optionally combined with other mode flags (e.g., NC_NETCDF4)
- `dispatch_table`: Pointer to your dispatch table structure
- `magic_number`: Optional magic number string (max NC_MAX_MAGIC_NUMBER_LEN bytes) for automatic format detection, or NULL

A magic number cannot be registered with a netCDF-3 mode. Combining a magic number with NC_64BIT_OFFSET, NC_64BIT_DATA, or NC_CLASSIC_MODEL (without NC_NETCDF4) returns NC_EINVAL.

**Example:**

```c
#include <netcdf.h>

/* Your dispatch table */
extern NC_Dispatch my_format_dispatcher;

int main() {
    /* Register UDF in slot 0 with magic number */
    if (nc_def_user_format(NC_UDF0 | NC_NETCDF4, &my_format_dispatcher, 
                           "MYFORMAT")) {
        fprintf(stderr, "Failed to register UDF\n");
        return 1;
    }
    
    /* Now files with "MYFORMAT" magic number will use your dispatcher */
    int ncid;
    nc_open("myfile.dat", 0, &ncid);
    /* ... */
    nc_close(ncid);
    
    return 0;
}
```

## Querying Registered UDFs

Use `nc_inq_user_format()` to query registered formats:

```c
int nc_inq_user_format(int mode_flag, NC_Dispatch **dispatch_table, 
                       char *magic_number);
```

# Configuring UDFs via RC Files {#udf_rc}

UDFs can be automatically loaded from RC file configuration, eliminating the need to modify application code.

## RC File Location

RC files are searched in this order:
1. `$HOME/.ncrc`
2. `$HOME/.daprc`
3. `$HOME/.dodsrc`
4. `$CWD/.ncrc`
5. `$CWD/.daprc`
6. `$CWD/.dodsrc`

Later files override earlier ones. Use `NCRCENV_HOME` to override the home directory.

## RC File Format for UDFs

For each UDF slot (0-63), configure these keys:

```
NETCDF.UDF<N>.LIBRARY=<full-path-to-library>
NETCDF.UDF<N>.INIT=<initialization-function-name>
NETCDF.UDF<N>.MAGIC=<optional-magic-number>
```

**Example .ncrc file:**

```ini
# Load custom HDF format in UDF0
NETCDF.UDF0.LIBRARY=/usr/local/lib/libmyformat.so
NETCDF.UDF0.INIT=myformat_init
NETCDF.UDF0.MAGIC=MYFORMAT

# Load scientific data format in UDF3
NETCDF.UDF3.LIBRARY=/opt/scidata/lib/libscidata.so
NETCDF.UDF3.INIT=scidata_initialize
NETCDF.UDF3.MAGIC=SCIDATA

# Load analysis format in UDF7 (no magic number)
NETCDF.UDF7.LIBRARY=/home/user/analysis/libanalysis.so
NETCDF.UDF7.INIT=analysis_init
```

## RC Configuration Requirements

- **LIBRARY**: Must be a full absolute path to the shared library (.so on Unix, .dll on Windows)
- **INIT**: Name of the initialization function in the library
- **MAGIC**: Optional magic number for automatic format detection
- Both LIBRARY and INIT must be present; partial configuration is ignored with a warning

## Plugin Loading Process

Plugins are loaded during library initialization (`nc_initialize()`):

1. RC files are parsed
2. For each configured UDF slot:
   - Library is loaded using dlopen (Unix) or LoadLibrary (Windows)
   - Init function is located using dlsym or GetProcAddress
   - Init function is called; it returns a pointer to the plugin's dispatch table
   - The loader registers the returned dispatch table by calling `nc_def_user_format()`
3. Dispatch table ABI version is verified against NC_DISPATCH_VERSION
4. Magic number (if provided) is registered for automatic format detection

A plugin that fails to load produces a log warning but does not prevent library initialization. The other slots load normally.

**Note:** Library handles are intentionally not closed; they remain loaded for the lifetime of the process.

# Magic Numbers and Format Detection {#udf_magic}

Magic numbers enable automatic format detection when opening files.

## How Magic Numbers Work

When `nc_open()` is called without a specific format flag:
1. The file's first bytes are read
2. They are compared against all registered magic numbers
3. If a match is found, the corresponding UDF dispatcher is used

## Magic Number Best Practices

- Use unique, distinctive strings (4-8 bytes recommended)
- Place at the beginning of your file format
- Avoid conflicts with existing formats:
  - NetCDF-3: "CDF\001", "CDF\002", or "CDF\005"
  - HDF5: "\211HDF\r\n\032\n"
  - NetCDF-4: Same as HDF5
- Maximum length: NC_MAX_MAGIC_NUMBER_LEN bytes

## Example with Magic Number

```c
/* File format with magic number */
FILE *fp = fopen("mydata.dat", "wb");
fwrite("MYDATA", 1, 6, fp);  /* Magic number */
/* ... write your data ... */
fclose(fp);

/* Register UDF with magic number */
nc_def_user_format(NC_UDF0 | NC_NETCDF4, &my_dispatcher, "MYDATA");

/* Open automatically detects format */
int ncid;
nc_open("mydata.dat", 0, &ncid);  /* No mode flag needed! */
```

# Opening Files with an Explicit UDF Mode Flag {#udf_open_mode}

In addition to automatic detection by magic number, a file can be opened
explicitly with a UDF mode flag. When `nc_open()` or `nc_create()` is called
with a `NC_UDF(n)` mode flag, the corresponding registered UDF
dispatch table is used regardless of the file's actual contents. This makes it
possible to read or write any other file through
a custom dispatcher.

The UDF mode flag can be combined with behavioral flags such as `NC_WRITE` or
`NC_DISKLESS`, but it cannot be combined with other format-selection flags such
as `NC_NETCDF4` or `NC_64BIT_OFFSET`. The selected UDF
slot must already have been registered, either programmatically with
`nc_def_user_format()` or from an RC file; otherwise the open call will fail.

## Example

```c
#include <netcdf.h>

extern NC_Dispatch my_dispatcher;

int main() {
    int ncid;

    /* Register the UDF without a magic number. */
    if (nc_def_user_format(NC_UDF0, &my_dispatcher, NULL)) {
        fprintf(stderr, "Failed to register UDF\n");
        return 1;
    }

    /* Open an existing file and force it to use the UDF0 dispatcher. */
    if (nc_open("existing_file.nc", NC_UDF0, &ncid)) {
        fprintf(stderr, "Failed to open file with UDF0\n");
        return 1;
    }

    /* ... */

    nc_close(ncid);
    return 0;
}
```

The test `nc_test4/tst_udf_open_mode.c` demonstrates this behavior by creating a
valid NetCDF-4/HDF5 file and then opening it with `NC_UDF0` to verify that the
registered UDF dispatch table is used instead of the HDF5 dispatcher.

# Platform Considerations {#udf_platforms}

## Unix/Linux/macOS

- Shared libraries: `.so` extension
- Dynamic loading: `dlopen()` and `dlsym()`
- Library paths: Use absolute paths or ensure libraries are in `LD_LIBRARY_PATH`

## Windows

- Shared libraries: `.dll` extension
- Dynamic loading: `LoadLibrary()` and `GetProcAddress()`
- Library paths: Use absolute paths or ensure DLLs are in system PATH

## Building Plugins

Your plugin must be compiled as a shared library:

**Unix:**
```bash
gcc -shared -fPIC -o libmyplugin.so myplugin.c -lnetcdf
```

**Windows:**
```bash
cl /LD myplugin.c netcdf.lib
```

# Security Considerations {#udf_security}

- **Full paths required**: RC files must specify absolute library paths to prevent path injection attacks
- **Code execution**: Plugins execute arbitrary code in your process; only load trusted libraries
- **Validation**: The library verifies dispatch table ABI version but cannot validate plugin behavior
- **Permissions**: Ensure plugin libraries have appropriate file permissions

# Error Handling {#udf_errors}

Common errors and solutions:

## NC_EINVAL: Invalid dispatch table version

**Cause:** Plugin was compiled against a different version of netCDF-C

**Solution:** Recompile plugin against current netCDF-C version

## Plugin not loaded (no error)

**Cause:** Partial RC configuration (LIBRARY without INIT, or vice versa)

**Solution:** Check that both LIBRARY and INIT keys are present in RC file

## Library not found

**Cause:** Incorrect path in NETCDF.UDF*.LIBRARY

**Solution:** Use absolute path; verify file exists and has correct permissions

## Init function not found

**Cause:** Function name mismatch or missing export

**Solution:** Verify function name matches INIT key; ensure function is exported (not static)

## Init function fails

**Cause:** Plugin initialization error; init function returned NULL or a dispatch table with the wrong ABI version

**Solution:** Check plugin logs; verify the init function returns a valid NC_Dispatch pointer with dispatch_version set to NC_DISPATCH_VERSION

# Complete Example {#udf_example}

The test suite contains complete working examples. See `nc_test4/tst_udf.c` for programmatic registration, `nc_test4/tst_udf_multi.c` for use of all 10 slots at once, and `nc_test4/tst_udf_self_load_plugin.c` for a plugin loaded from an RC file.

# Troubleshooting {#udf_troubleshooting}

## Enable Logging

Set the `NC_LOG_LEVEL` environment variable to see plugin loading messages:

```bash
export NC_LOG_LEVEL=3
./myprogram
```

## Verify RC File is Read

Create a test RC file and check if it's being parsed:

```bash
echo "NETCDF.UDF0.LIBRARY=/tmp/test.so" > ~/.ncrc
echo "NETCDF.UDF0.INIT=test_init" >> ~/.ncrc
# Run your program and check for warnings about missing library
```

## Check Plugin Exports

Verify your init function is exported:

**Unix:**
```bash
nm -D libmyplugin.so | grep init
```

**Windows:**
```bash
dumpbin /EXPORTS myplugin.dll
```

## Test Plugin Loading

Use a minimal test program:

```c
#include <netcdf.h>
#include <stdio.h>

int main() {
    nc_initialize();  /* Triggers plugin loading */
    printf("Initialization complete\n");
    return 0;
}
```

# See Also {#udf_seealso}

- [Dispatch Table Architecture](dispatch.md) - Internal architecture
- [UDF Plugin Development Guide](#udf_plugin_development) - Creating plugins
- [RC File Reference](#auth_support) - RC file format details
- [NetCDF Expansion Pack (NEP)](https://github.com/Intelligent-Data-Design-Inc/NEP) - Production UDF plugins for GRIB2, FITS, GeoTIFF, PDS4, CDF, DICOM, and PDB
- API Reference: nc_def_user_format(), nc_inq_user_format()

# References {#udf_references}

- NetCDF-C Dispatch Layer: docs/dispatch.md
- Plugin Loader Implementation: libdispatch/dudfplugins.c
- Test Suite: nc_test4/tst_udf*.c
