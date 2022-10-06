NetCDF-4 Filter QuickStart
==============================

\tableofcontents

<!--  double header is needed to workaround doxygen bug  -->

NetCDF-4 Filter QuickStart {#nc_filters_quickstart}
==============================

> For full **Filters** documentation, see \ref filters.


The `libnetcdf.so` library cannot talk to plugin libraries directly. Instead, it requires an "interface" library, which acts as a go-between.  The interface libraries are built by the netCDF libraries, when the underlying plugin libraries are detected during configure/build.

Building {#nc_filters_qs_building}
----------------------------------


Run-time {#nc_filters_qs_runtime}
---------------------------------



Example Workflow {#nc_filters_qs_workflow_example}
--------------------------------------------------

From scratch, the steps to get this to work are as follows, and assumes `libhdf5` was installed.

1. Install the filter library, and the associate development headers.
2. Configure netCDF with `--enable-plugins` and `--with-plugin-dir=$HOME/netcdf-plugins`
3. Ensure `blosc` is specified in the generated `libnetcdf.settings` file.  
4. Run make, make install.

Once built and installed, set the environmental variable `HDF5_PLUGIN_PATH=$HOME/netcdf-plugins`.  

The reason this works is because:

1. NetCDF builds the interface library.
2. `ncdump` knows where to find the interface library because `HDF5_PLUGIN_PATH` is set.  

