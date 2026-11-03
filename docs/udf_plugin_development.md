UDF Plugin Development Guide {#udf_plugin_development}
====================================

# Introduction {#udf_dev_intro}

This guide explains how to develop plugins for netCDF-C user-defined formats (UDFs). It covers dispatch table implementation, plugin structure, building, testing, and deployment.

# Plugin Architecture {#udf_dev_arch}

A UDF plugin consists of:
1. **Dispatch table**: Structure with function pointers implementing the netCDF API
2. **Initialization function**: Called during plugin loading to register the dispatch table
3. **Format-specific code**: Your implementation of file I/O and data operations

## Plugin Lifecycle

1. Library initialization (`nc_initialize()`)
2. RC file parsing (if configured)
3. Plugin library loading (`dlopen`/`LoadLibrary`)
4. Init function location (`dlsym`/`GetProcAddress`)
5. Init function execution; the init function returns a pointer to the plugin's dispatch table
6. The loader verifies the dispatch table ABI version and registers the table via `nc_def_user_format()`
7. Plugin remains loaded for process lifetime

# Dispatch Table Structure {#udf_dev_dispatch}

The `NC_Dispatch` structure is defined in `include/netcdf_dispatch.h`. It contains function pointers for all netCDF operations.

## Required Fields

```c
typedef struct NC_Dispatch {
    int model;              /* NC_FORMATX_UDF(n) for slot n (0-63) */
    int dispatch_version;   /* Must be NC_DISPATCH_VERSION */
    
    /* Function pointers for all netCDF operations */
    int (*create)(...);
    int (*open)(...);
    /* ... many more functions ... */
} NC_Dispatch;
```

## Minimal Dispatch Table Example

```c
#include "netcdf_dispatch.h"

static NC_Dispatch my_dispatcher = {
    NC_FORMATX_UDF0,        /* Use UDF slot 0 */
    NC_DISPATCH_VERSION,    /* Current ABI version */
    
    NC_RO_create,           /* Read-only: use predefined function */
    my_open,                /* Custom open function */
    NC_RO_redef,
    NC_RO__enddef,
    NC_RO_sync,
    my_abort,
    my_close,
    NC_RO_set_fill,
    my_inq_format,
    my_inq_format_extended,
    
    /* Inquiry functions - can use NC4_* defaults */
    NC4_inq,
    NC4_inq_type,
    /* ... continue for all required functions ... */
};
```

## Pre-defined Functions

Use these for operations your format doesn't support:

- **Read-only**: `NC_RO_*` - Returns NC_EPERM
- **Not NetCDF-4**: `NC_NOTNC4_*` - Returns NC_ENOTNC4
- **Not NetCDF-3**: `NC_NOTNC3_*` - Returns NC_ENOTNC3
- **No-op**: `NC_NOOP_*` - Returns NC_NOERR
- **Default implementations**: `NCDEFAULT_*` - Generic implementations
- **NetCDF-4 inquiry**: `NC4_*` - Use internal metadata model

# Initialization Function {#udf_dev_init}

The initialization function is called when your plugin is loaded.

## Function Signature

```c
NC_Dispatch* plugin_init(void);
```

The init function returns a pointer to the plugin's dispatch table. The plugin loader verifies the table's ABI version and registers it with `nc_def_user_format()`. The plugin does not register itself.

## Requirements

1. Must be exported (not static)
2. Must return a pointer to a fully populated NC_Dispatch structure, or NULL on failure
3. The dispatch table's `dispatch_version` field must be set to NC_DISPATCH_VERSION
4. Name must match RC file INIT key

## Example Implementation

```c
#include <netcdf.h>
#include "netcdf_dispatch.h"

/* Your dispatch table */
extern NC_Dispatch my_dispatcher;

/* Initialization function - must be exported */
NC_Dispatch* my_plugin_init(void)
{
    /* Additional initialization if needed */
    /* ... */

    /* Return the dispatch table; the loader registers it. */
    return &my_dispatcher;
}
```

When registering programmatically instead of through the RC-based loader, call `nc_def_user_format()` directly from your application code.

# Implementing Dispatch Functions {#udf_dev_functions}

## Open Function

```c
int my_open(const char *path, int mode, int basepe, size_t *chunksizehintp,
            void *parameters, const NC_Dispatch *dispatch, int ncid)
{
    /* Open your file format */
    /* Populate internal metadata structures */
    /* Return NC_NOERR on success */
}
```

## Close Function

```c
int my_close(int ncid, void *v)
{
    /* Clean up resources */
    /* Close file handles */
    return NC_NOERR;
}
```

## Format Inquiry Functions

```c
int my_inq_format(int ncid, int *formatp)
{
    if (formatp)
        *formatp = NC_FORMAT_NETCDF4;  /* Or appropriate format */
    return NC_NOERR;
}

int my_inq_format_extended(int ncid, int *formatp, int *modep)
{
    if (formatp)
        *formatp = NC_FORMAT_NETCDF4;
    if (modep)
        *modep = NC_NETCDF4;
    return NC_NOERR;
}
```

# Building Plugins {#udf_dev_build}

## Unix/Linux/macOS

```makefile
# Makefile for UDF plugin
CC = gcc
CFLAGS = -fPIC -I/usr/local/include
LDFLAGS = -shared -L/usr/local/lib -lnetcdf

myplugin.so: myplugin.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

install:
	cp myplugin.so /usr/local/lib/
```

## Windows

```batch
REM Build UDF plugin on Windows
cl /LD /I"C:\netcdf\include" myplugin.c /link /LIBPATH:"C:\netcdf\lib" netcdf.lib
```

## CMake Example

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyPlugin)

find_package(netCDF REQUIRED)

add_library(myplugin SHARED myplugin.c)
target_link_libraries(myplugin netCDF::netcdf)
target_include_directories(myplugin PRIVATE ${netCDF_INCLUDE_DIRS})

install(TARGETS myplugin LIBRARY DESTINATION lib)
```

# Testing Your Plugin {#udf_dev_test}

## Unit Testing

```c
/* test_plugin.c */
#include <netcdf.h>
#include <assert.h>

extern NC_Dispatch my_dispatcher;
extern NC_Dispatch* my_plugin_init(void);

int main() {
    int ret;
    NC_Dispatch *table;
    NC_Dispatch *disp;
    
    /* Test initialization */
    table = my_plugin_init();
    assert(table);
    assert(table->dispatch_version == NC_DISPATCH_VERSION);
    
    /* Register, as the plugin loader would */
    ret = nc_def_user_format(NC_UDF0, table, NULL);
    assert(ret == NC_NOERR);
    
    /* Verify registration */
    ret = nc_inq_user_format(NC_UDF0, &disp, NULL);
    assert(ret == NC_NOERR);
    assert(disp == &my_dispatcher);
    
    printf("Plugin tests passed\n");
    return 0;
}
```

## Integration Testing

```c
/* test_integration.c */
#include <netcdf.h>

int main() {
    int ncid, ret;
    
    /* Initialize and register plugin */
    my_plugin_init();
    
    /* Test file operations */
    ret = nc_open("testfile.dat", NC_UDF0, &ncid);
    if (ret != NC_NOERR) {
        fprintf(stderr, "Open failed: %s\n", nc_strerror(ret));
        return 1;
    }
    
    /* Test operations */
    int format;
    nc_inq_format(ncid, &format);
    
    nc_close(ncid);
    printf("Integration test passed\n");
    return 0;
}
```

## RC File Testing

Create `.ncrc`:
```ini
NETCDF.UDF0.LIBRARY=/path/to/myplugin.so
NETCDF.UDF0.INIT=my_plugin_init
NETCDF.UDF0.MAGIC=MYFMT
```

Test automatic loading:
```c
int main() {
    /* Plugin loads automatically during nc_initialize() */
    int ncid;
    nc_open("file_with_magic.dat", 0, &ncid);  /* Auto-detects format */
    nc_close(ncid);
    return 0;
}
```

# Debugging {#udf_dev_debug}

## Enable NetCDF Logging

```bash
export NC_LOG_LEVEL=3
./test_program
```

## Check Symbol Exports

```bash
# Unix
nm -D libmyplugin.so | grep init

# Windows
dumpbin /EXPORTS myplugin.dll
```

## GDB Debugging

```bash
gdb ./test_program
(gdb) break my_plugin_init
(gdb) run
(gdb) backtrace
```

## Common Issues

### Plugin not loaded
- Check RC file syntax
- Verify both LIBRARY and INIT are present
- Use absolute path for LIBRARY

### Init function not found
- Ensure function is not static
- Check function name matches INIT key
- Verify symbol is exported

### ABI version mismatch
- Recompile against current netCDF-C headers
- Check NC_DISPATCH_VERSION value

# Best Practices {#udf_dev_best}

1. **Error Handling**: Return appropriate NC_E* error codes
2. **Memory Management**: Clean up in close/abort functions
3. **Thread Safety**: Use thread-safe operations if needed
4. **Logging**: Use nclog functions for diagnostic output
5. **Documentation**: Document your format and API
6. **Testing**: Test all code paths thoroughly
7. **Versioning**: Version your plugin and document compatibility

# Example Plugin {#udf_dev_example}

See `nc_test4/tst_udf_self_load_plugin.c` for a complete working plugin that is built as a shared library and loaded through the RC-based loader by `nc_test4/tst_udf_self_load.c`.

# Reference {#udf_dev_ref}

- Dispatch table definition: `include/netcdf_dispatch.h`
- Pre-defined functions: `libdispatch/dreadonly.c`, `libdispatch/dnotnc*.c`
- Example implementations: `libhdf5/hdf5dispatch.c`, `libsrc/nc3dispatch.c`
- Plugin loader: `libdispatch/dudfplugins.c`
- Test plugins: `nc_test4/tst_udf.c`, `nc_test4/tst_udf_multi.c`, `nc_test4/tst_udf_self_load_plugin.c`
