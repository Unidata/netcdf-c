User-Defined Formats {#udf}
====================================

# Introduction {#udf_intro}

NetCDF-C supports user-defined formats (UDFs) that allow developers to extend the library to work with custom file formats. The library provides 10 UDF slots (UDF0 through UDF9) that can be registered either programmatically or via RC file configuration.

User-defined formats enable:
- Support for proprietary or specialized file formats
- Custom storage backends
- Format translation and adaptation layers
- Integration with domain-specific data formats

# Available UDF Slots {#udf_slots}

The netCDF-C library provides 10 user-defined format slots:

- **UDF0** and **UDF1**: Original slots, mode flags in lower 16 bits
- **UDF2** through **UDF9**: Extended slots, mode flags in upper 16 bits

Each slot can be independently configured with its own dispatch table, initialization function, and optional magic number for automatic format detection.

# Registering UDFs Programmatically {#udf_programmatic}

## Using nc_def_user_format()

The `nc_def_user_format()` function registers a user-defined format:

```c
int nc_def_user_format(int mode_flag, NC_Dispatch *dispatch_table, 
                       char *magic_number);
```

**Parameters:**
- `mode_flag`: One of NC_UDF0 through NC_UDF9, optionally combined with other mode flags (e.g., NC_NETCDF4)
- `dispatch_table`: Pointer to your dispatch table structure
- `magic_number`: Optional magic number string (max NC_MAX_MAGIC_NUMBER_LEN bytes) for automatic format detection, or NULL

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

For each UDF slot (0-9), configure these keys:

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
   - Init function is called
   - Init function must call `nc_def_user_format()` to register the dispatch table
3. Dispatch table ABI version is verified
4. Magic number (if provided) is optionally verified

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
  - NetCDF-3: "CDF\001" or "CDF\002"
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

**Cause:** Plugin initialization error

**Solution:** Check plugin logs; verify nc_def_user_format() is called correctly

# Complete Example {#udf_example}

See `examples/C/udf_example.c` for a complete working example of implementing a user-defined format.

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
- [UDF Plugin Development Guide](udf_plugin_development.md) - Creating plugins
- [RC File Reference](auth.md) - RC file format details
- API Reference: nc_def_user_format(), nc_inq_user_format()

# References {#udf_references}

- NetCDF-C Dispatch Layer: docs/dispatch.md
- Example Implementation: examples/C/udf_example.c
- Test Suite: nc_test4/tst_udf*.c
