echo on
rem We need a non-netcdf file called driver.c to pass a test.
rem Here we create one with directory contents, then delete it.
dir > driver.o
..\%1\nctest
rem fc testfile.nc ..\..\..\nctest\testfile_nc.sav | find /i "FC: no differences encountered" > nul
rem if errorlevel==1 goto ERR_LABEL
erase driver.o
exit \B 0

:ERR_LABEL
echo ************** ERROR - Comparison Not Correct! **********************
exit \B 1
