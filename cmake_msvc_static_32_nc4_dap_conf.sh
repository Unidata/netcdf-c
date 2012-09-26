#!/bin/bash

HDF5_LIB="/c/HDF5-Static/1.8.9/lib/hdf5.lib"
HDF5HL_LIB="/c/HDF5-Static/1.8.9/lib/hdf5_hl.lib"
HDF5_INC="/c/HDF5-Static/1.8.9/include"

ZLIB_LIB="/c/GnuWin32/lib/zlib.lib"

CURL_LIB="/c/Users/wfisher/Desktop/libcurl32-msvc/libcurl.lib"
CURL_INC="/c/Users/wfisher/Desktop/libcurl32-msvc/include"

SZIP_LIB="/c/HDF5-Static/1.8.9/lib/libszip.lib"

cmake .. -D"HDF5_LIBRARIES=$HDF5_LIB:$HDF5HL_LIB" -D"HDF5_INCLUDE_DIRS=$HDF5_INC" -D"ZLIB_LIBRARY=$ZLIB_LIB" -D"CURL_LIBRARY=$CURL_LIB" -D"CURL_INCLUDE_DIR=$CURL_INC" -D"USE_SZIP=ON" -D"SZIP_LIBRARY=$SZIP_LIB"

