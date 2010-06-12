$! --------------------------------------------------------------------------
$! For making FTEST.EXE on VMS
$! --------------------------------------------------------------------------
$!
$! $Id: makevms.com,v 1.3 1996/02/27 22:56:05 steve Exp $
$
$ ccc := cc /opt/nodebug/nolist/include=([-.src])
$
$ ccc JACKETS.C
$ fort FTEST.FOR
$
$ link/nodebug/notraceback/exec=FTEST.exe -
    ftest.obj, -
    jackets.obj, -
    [---.lib]netcdf.olb/lib, -
    sys$input/opt
	sys$library:vaxcrtl.exe/share
$
$ run ftest
$
