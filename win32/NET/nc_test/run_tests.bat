echo on
cd ..\%1
rem We need a non-netcdf file called tests.h to pass a test.
rem Here we create one with directory contents, then delete it.
dir > nc_test.o
nc_test
erase nc_test.o