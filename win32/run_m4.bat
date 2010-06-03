:: Run m4 on netCDF source to generate some C files. For this to work
:: you need the m4 macro processor in your path. Get it from
:: www.cygwin.com.
echo on
\cygwin\bin\m4 -B10000 ..\..\..\libsrc\attr.m4 > ..\..\..\libsrc\attr.c
\cygwin\bin\m4 -B10000 ..\..\..\libsrc\ncx.m4 > ..\..\..\libsrc\ncx.c
\cygwin\bin\m4 -B10000 ..\..\..\libsrc\putget.m4 > ..\..\..\libsrc\putget.c
\cygwin\bin\m4 -B10000 ..\..\..\libsrc\t_ncxx.m4 > ..\..\..\libsrc\t_ncxx.c
\cygwin\bin\m4 -B10000 ..\..\..\nc_test\test_get.m4 > ..\..\..\nc_test\test_get.c
\cygwin\bin\m4 -B10000 ..\..\..\nc_test\test_put.m4 > ..\..\..\nc_test\test_put.c

