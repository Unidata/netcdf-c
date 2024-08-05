Appendix D.3. Environment Variables and .RC files {#nc_env_quickstart}
==============================

The netCDF-c library provides several parameterization mechanisms to
control its behavior at run-time. The term _run-time_ means that the
library's behavior can be changed every time the library is initialized

The most commonly used parameterization mechanisms are:
1. Environment Variables -- accessed by the getenv() function.
2. .RC files -- accessed from the file system.

## Environment Variables {#nc_env_vars}

The following table describes (most of) the environment variables
used by the netcdf-c library. There are some not listed that are only
used for specialized debugging.

<table>
<tr><th>Name<th>Description
<tr><td>ALLUSERSPROFILE<td>This is more-or-less the Windows equivalent of "HOME"
<tr><td>AWS_ACCESS_KEY_ID<td>Used by the aws access libraries; overrides ${HOME}/.aws/config entries.
<tr><td>AWS_CONFIG_FILE<td>Used by the aws access libraries; specifies absolute path to the config file.
<tr><td>AWS_DEFAULT_REGION<td>Used by the aws access libraries; specifies default S3 region.
<tr><td>AWS_PROFILE<td>Used by the aws access libraries; specifies default AWS profile.
<tr><td>AWS_REGION<td>Used by the aws access libraries; specifies specific region to use.
<tr><td>AWS_SECRET_ACCESS_KEY<td>Used by the aws access libraries; overrides ${HOME}/.aws/config entries.
<tr><td>CURLOPT_VERBOSE<td>Causes libcurl to produce verbose output.
<tr><td>HOME<td>The user's home directory.
<tr><td>MSYS2_PREFIX<td>If platform is MSYS2, then specify the root prefix.
<tr><td>NC_DEFAULT_CREATE_PERMS<td>For NCZarr, specify the default creation permissions for a file.
<tr><td>NC_DEFAULT_DIR_PERMS<td>For NCZarr, specify the default creation permissions for a directory.
<tr><td>NCLOGGING<td>Specify the log level: one of "OFF","ERR","WARN","NOTE","DEBUG".
<tr><td>NCPATHDEBUG<td>Causes path manager to output debugging information.
<tr><td>NCRCENV_HOME<td>Overrides ${HOME} as the location of the .rc file.
<tr><td>NCRCENV_IGNORE<td>Causes any .rc files to be ignored.
<tr><td>NCRCENV_RC<td>The absolute path to use for the .rc file.
<tr><td>NCTRACING<td>Specify the level of tracing detail.
<tr><td>NCZARRFORMAT<td>Force use of a specific Zarr format version: 2 or 3.
<tr><td>NETCDF_LOG_LEVEL<td>Specify the log level for HDF5 logging (separate from e.g. NCLOGGING).
<tr><td>TEMP<td>For Windows platform, specifies the location of a directory to store temporary files.
<tr><td>USERPROFILE<td>For Windows platform, overrides ${HOME}.
</table>

## Resource Control (.rc) Files {#nc_env_rc}

In addition to using environment variables,
the netcdf-c library supports run-time configuration
of the library using the so-called ".rc" file mechanism.
This means that as part of its initialization, the netcdf-c
library will search for and process a set of files where
these files contain entries specifying (key,value) pairs.
These pairs are compiled into a single internal database
that can be queried by other parts of the netcdf-c library.

### Locating The _.rc_ Files

For historical reasons, multiple .rc files are allowed.

### Search Order

The netcdf-c library searches for, and loads from, the following files,
in this order:
1. $HOME/.ncrc
2. $HOME/.dodsrc
3. $CWD/.ncrc
4. $CWD/.dodsrc

*$HOME* is the user's home directory and *$CWD* is the current working directory.
Entries in later files override any of the earlier files

It is strongly suggested that you pick a uniform location and a uniform name
and use them always. Otherwise you may observe unexpected results
when the netcdf-c library loads an rc file you did not expect.

### RC File Format

The rc file format is a series of lines of the general form:
````
    [<URL>]<key>=<value>
````
where the bracket-enclosed URL is optional. Note that the brackets
are part of the line.

### URL Constrained RC File Entries

Each line of the rc file can begin with a URL enclosed in
square brackets. The parts of the URL that are used for choosing
an entry are the host, the port, and the URL path.
Note that the host+port is the only part used when searching for
libcurl related entries. This is because libcurl's authorization grain is not
any finer than host+port level.
The URL path may be used for non-curl related entries.
Also note that the protocol is ignored.

Here are some examples.
````
    [https://remotetest.unidata.ucar.edu/thredds]HTTP.VERBOSE=1
or
    [https://fake.ucar.edu:9090]HTTP.VERBOSE=0
````

For selection purposes, the host+port+path is used when the path argument
for _nc_open()_ or _nc_create()_ takes the form of a URL.
If the url request from, say, the _netcdf_open_ method
has a host,port, and path matching one of the prefixes in the rc file, then
the corresponding entry will be used, otherwise ignored.
This means that an entry with a matching host+port+path will take
precedence over an entry without a host+port+path.

For example, passing this URL to _nc_open_
````
    http://remotetest.unidata.ucar.edu/thredds/dodsC/testdata/testData.nc
````
will have HTTP.VERBOSE set to 1 because its host and path match the example above.

Similarly, using this path
````
    http://fake.ucar.edu:9090/dts/test.01
````
will have HTTP.VERBOSE set to 0 because its host+port matches the example above.

### Programmatic Access to .rc File

It is possible for client programs to query and modify the internal .rc database
through the following API.
* ````char* nc_rc_get(const char* key);````
    Get the value corresponding to key or return NULL if not defined. The caller must free the resulting value.
* ````int nc_rc_set(const char* key, const char* value);````
    Set/overwrite the value corresponding to the specified key.

Note that this API does not (currently) support URL prefixed keys, so the client will need to take this into account.

### Defined .rc file keys

There a a number of keys used by the netcdf-c library. Most of them
are authorization-related. The file "auth.md" describes these keys.

Other keys are as follows:
* libdap4/d4curlfunctions.c and oc2/ocinternal.c
    - HTTP.READ.BUFFERSIZE -- set the read buffer size for DAP2/4 connection
    - HTTP.KEEPALIVE -- turn on keep-alive for DAP2/4 connection
* libdispatch/ds3util.c
    - AWS.PROFILE -- alternate way to specify the default AWS profile
    - AWS.REGION --  alternate way to specify the default AWS region
* libnczarr/zinternal.c
    - ZARR.DIMENSION_SEPARATOR -- alternate way to specify the Zarr dimension separator character
* oc2/occurlfunctions.c
    - HTTP.NETRC -- alternate way to specify the path of the .netrc file

## Point of Contact {#nc_env_poc}

__Author__: Dennis Heimbigner<br>
__Email__: dmh at ucar dot edu<br>
__Initial Version__: 01/09/2023<br>
__Last Revised__: 07/30/2024
