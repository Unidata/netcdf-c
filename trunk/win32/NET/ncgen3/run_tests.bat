:: Runs ncgen3 and ncdump tests.
echo on
..\%1\ncgen3 -b -o c0.nc ..\..\..\ncgen3\c0.cdl
..\%1\ncdump -n c1 c0.nc > c1.cdl
..\%1\ncgen3 -b c1.cdl
..\%1\ncdump c1.nc > c2.cdl
fc c1.cdl c2.cdl | find /i "FC: no differences encountered" > nul
if errorlevel==1 goto ERR_LABEL
..\%1\ncgen3 -b ..\..\..\ncdump\test0.cdl
..\%1\ncdump -n test1 test0.nc > test1.cdl
..\%1\ncgen3 -b test1.cdl
..\%1\ncdump test1.nc > test2.cdl
fc test1.cdl test2.cdl | find /i "FC: no differences encountered" > nul
if errorlevel==1 goto ERR_LABEL
exit \B 0

:ERR_LABEL
echo ************** ERROR - Comparison Not Correct! **********************
exit \B 1

