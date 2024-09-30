Appendix E. NetCDF-4 Plugin Path Support {#pluginpaths} 
==================================

[TOC]

## Plugin Path Overview {#pluginpaths_overview}

The processes by which plugins are installed into some directory
and the process by which plugins are located are unfortunately
complicated. This is in part due to the historical requirements
to support existing HDF5 and Zarr mechanisms.

This document describes the following major processes:
1. *Discovery* -- at run-time, any reference to a plugin must do a search
to locate a dynamic library that implements the plugin.
2. *Plugin Path Management* -- at run-time, the client program may wish to programmatically
set the sequence of directories to use in locating plugins.
3. *Installation* -- during the build of the netcdf-c library,
any compiled plugins may optionally be installed into some directory.

## Discovering a Specific Plugin at Run-Time

The netcdf-c library maintains an internal sequence of directory paths
-- collectively called the *plugin path* -- that controls the search
for plugin libraries. Basically, when looking for a specific plugin,
each directory in the plugin path is examined in order. For each such
directory, the files in that directory are checked to see if it
contains the specified plugin. The details of how a file is processed
is described in the document *filters.md*.

The netcdf-c search algorithm is closely tied to the HDF5
discovery process. The HDF5 process searches its own internal
plugin path (sequence of directories) in order to discover
a specific plugin library.

The addition of NCZarr support to the netcdf-c library
requires yet another plugin path (sequence of directories)
for its search process.

It is important to know that the plugin path is completely controlled
by a global plugin path. If it changes, then this global plugin path
is propagated to HDF5 and NCZarr to ensure that all such plugin paths
use the same sequence of directories for discovery.

## Programmatic Management of the Plugin Path

As of netcdf-c version 4.9.3, it is possible for a client program
to set the global plugin path to control plugin discovery.
Since the global path and the HDF5 and NCZarr paths are kept in
sync, this means that both HDF5 and NCZarr will look in the same
directories in order to locate specified plugins.
[Appendix E.1](#pluginpath_appendixe1) defines the current API for
managing the global plugin path.

Note that it is best practice for a client program to use the API
to set the plugin path before any calls to *nc_open* or *nc_create*.
Modifying the plugin paths later may fail because it cannot be guaranteed
that the underlying implementations (i.e. HDF5 or NCZarr) will
take notice of the change.

When the netcdf-c library initializes itself, it chooses an initial
global plugin path using the following rules, which are those used
by the HDF5 library:
1. If *HDF5\_PLUGIN\_PATH* environment variable is defined,
then its value is used as the initial plugin path.
2. If *HDF5\_PLUGIN\_PATH* is not defined, then the
initial plugin path is either:
    * /usr/local/hdf5/plugin -- for Linux/Unix/Cygwin,
    * %ALLUSERSPROFILE%/hdf5/lib/plugin -- for Windows/Mingw.

This initial global plugin path will be propagated to HDF5 and NCZarr.

## Installing Plugins at Build-Time

At build-time, the target location directory into which libraries implementing plugins are installed is specified using a special *./configure* option
````
--with-plugin-dir=<directorypath>
or
--with-plugin-dir
````
or its corresponding *cmake* option.
````
-DNETCDF_PLUGIN_INSTALL_DIR=<directorypath>
or
-DNETCDF_PLUGIN_INSTALL_DIR=YES
````
If this option is specified but with no specific directory,
then it defaults to one of three values:
1. If *HDF5\_PLUGIN\_PATH* defined, then use the last directory in that path,
2. else use `/usr/local/hdf5/lib/plugin` for linux/unix operating systems (including Cygwin) else use
3. `%ALLUSERSPROFILE%\\hdf5\\lib\\plugin` for Windows and MinGW.

If the option is specified with an absolute directory path, then all plugin libraries will be installed in that directory only.

If the option is not specified at all, or one of the following options is used,
then no attempt will be made to install plugins.
````
--without-plugin-dir
or (cmake)
-DNETCDF_PLUGIN_INSTALL_DIR=NO
````

## Multi-Threaded Access to the Plugin Path.
Specifically, note that modifying the plugin path must be done "atomically".
That is, in a multi-threaded environment, it is important that the sequence of actions involved in setting up the plugin path must be done by a single processor or in some other way as to guarantee that two or more processors are not simultaneously accessing the plugin path get/set operations.

As an example, assume there exists a mutex lock called PLUGINLOCK.
Then any processor accessing the plugin paths should operate as follows:
````
lock(PLUGINLOCK);
nc_plugin_path_get(...);
<rebuild plugin path>
nc_plugin_path_set(...);
unlock(PLUGINLOCK);
````

## Internal Architecture

It is assumed here that there only needs to be a single set of
plugin path directories that is shared by all filter code and is
independent of any file descriptor; it is global in other words.
This means, for example, that the path list for NCZarr and for
HDF5 will always be the same.

However, and internally, processing the set of plugin paths
depends on the particular NC_FORMATX value (NC_FORMATX_NC_HDF5
and NC_FORMATX_NCZARR, currently).  So the *nc_plugin_path_set*
function, will take the paths it is given and propagate them to
each of the NC_FORMATX dispatchers to store in a way that is
appropriate to the given dispatcher.

There is a complication with respect to the *nc_plugin_path_get* function.
It is possible for users to bypass the netcdf API and modify the HDF5 plugin paths directly. This can result in an inconsistent plugin path between the value
used by HDF5 and the global value used by netcdf-c. Since there is no obvious fix for this, we warn the user of this possibility and otherwise ignore it.

## Appendix E.1. Programmatic Plugin Path API{#pluginpath_appendixe1}

The API makes use of a counted vector of strings representing the sequence of directories in the path. The relevant type definition is as follows.
````
typedef struct NCPluginList {size_t ndirs; char** dirs;} NCPluginList;
````

The API proposed in this PR looks like this (from netcdf-c/include/netcdf_filter.h).

* ````int nc_plugin_path_ndirs(size_t* ndirsp);````

    This function returns the number of directories in the sequence if internal directories of the internal plugin path list.

    The argument is as follows:
    - *ndirsp* store the number of directories in this memory.

* ````int nc_plugin_path_get(NCPluginList* dirs);````

    This function returns the current sequence of directories from the internal plugin path list. Since this function does not modify the plugin path, it does not need to be locked; it is only when used to get the path to be modified that locking is required.

    The argument is as follows:
    - *dirs* counted vector for storing the sequence of directies in the internal path list.

    If the value of *dirs.dirs is NULL (the normal case), then memory is allocated to hold the vector of directories. Otherwise, use the memory of *dirs.dirs* to hold the vector of directories.

* ````int nc_plugin_path_set(const NCPluginList* dirs);````

    This function empties the current internal path sequence and replaces it with the sequence of directories argument. Using an *ndirs* argument of 0 will clear the set of plugin paths.

    The argument are as follows:
    - *dirs* counted vector for storing the sequence of directies in the internal path list.

*HDF5\_PLUGIN\_PATH* is a typical Windows or Unix style
path-list.  That is it is a sequence of absolute directory paths
separated by a specific separator character. For Windows, the
separator character is a semicolon (';') and for Unix, it is a
colon (':').

At the moment, NetCDF optionally (i.e. not overridden) uses the
existing HDF5 environment variable *HDF5\_PLUGIN\_PATH* to
locate the directories in which plugin libraries are located. It
also optionally uses the last directory in the path as the
installation directory. This is used both for the HDF5 filter
wrappers but also the NCZarr codec wrappers.

## History {#pluginpath_history}

*Author*: Dennis Heimbigner<br>
*Email*: dennis.heimbigner@gmail.com<br>
*Initial Version*: 9/28/2024<br>
*Last Revised*: 9/28/2024
