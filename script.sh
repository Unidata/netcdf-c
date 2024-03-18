#!/bin/bash

# NETCDF_FIND_SHARED_LIBS
# NC_USE_STATIC_CRT
# NETCDF_PACKAGE

# check NETCDF_ENABLE_NETCDF_4
# check NETCDF_ENABLE_TESTS

while IFS= read -r -u 3 var
do
    prefixed_var="NETCDF_$var"
    echo "Replacing $var with $prefixed_var..."
    grep --exclude=variables.txt -rl "$var" . | xargs sed -i '' "s/$var/$prefixed_var/g"
    echo "Press enter to continue to the next variable..."
    read
    git add .
    git commit -m "Replace $var with $prefixed_var"
done 3< variables.txt
