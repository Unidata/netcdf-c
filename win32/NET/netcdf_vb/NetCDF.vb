'* Copyright 2004 University Corporation for Atmospheric Research/Unidata
'* 
'* Portions of this software were developed by the Unidata Program at the 
'* University Corporation for Atmospheric Research.
'* 
'* Access and use of this software shall impose the following obligations
'* and understandings on the user. The user is granted the right, without
'* any fee or cost, to use, copy, modify, alter, enhance and distribute
'* this software, and any derivative works thereof, and its supporting
'* documentation for any purpose whatsoever, provided that this entire
'* notice appears in all copies of the software, derivative works and
'* supporting documentation.  Further, UCAR requests that the user credit
'* UCAR/Unidata in any publications that result from the use of this
'* software or in any product that includes this software. The names UCAR
'* and/or Unidata, however, may not be used in any advertising or publicity
'* to endorse or promote any products or commercial entity unless specific
'* written permission is obtained from UCAR/Unidata. The user also
'* understands that UCAR/Unidata is not obligated to provide the user with
'* any support, consulting, training or assistance of any kind with regard
'* to the use, operation and performance of this software nor to provide
'* the user with any updates, revisions, new versions or "bug fixes."
'* 
'* THIS SOFTWARE IS PROVIDED BY UCAR/UNIDATA "AS IS" AND ANY EXPRESS OR
'* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
'* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
'* DISCLAIMED. IN NO EVENT SHALL UCAR/UNIDATA BE LIABLE FOR ANY SPECIAL,
'* INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
'* FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
'* NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
'* WITH THE ACCESS, USE OR PERFORMANCE OF THIS SOFTWARE.
'
'This is a wrapper class for the netCDF dll.
'
'Get the netCDF dll from ftp://ftp.unidata.ucar.edu/pub/netcdf/contrib/win32
'Put it somewhere in your path, or else in the bin subdirectory of your
'VB project.
'
'Then include this class file in your project. Use the netcdf functions 
'like this:
'res = NetCDF.nc_create(name, NetCDF.cmode.NC_CLOBBER, ncid)
'If (res <> 0) Then GoTo err
'
'NetCDF was ported to dll by John Caron (as far as I know).
'This VB.NET wrapper created by Ed Hartnett, 3/10/4
'
'Some notes:
'   Although the dll can be tested (and has passed for release 
'3.5.0 and 3.5.1 at least), the VB wrapper class has not been
'extensively tested. Use at your own risk. Writing test code to
'test the netCDF interface is a non-trivial task, and one I haven't
'undertaken. The tests run verify common use of netCDF, for example
'creation of dims, vars, and atts of various types, and ensuring that
'they can be written and read back. But I don't check type conversion,
'or boundery conditions. These are all tested in the dll, but not the
'VB wrapper.
'
'This class consists mearly of some defined enums, consts and declares, 
'all inside a class called NetCDF.
'
'Passing strings: when passing in a string to a function, use a string,
'when passing in a pointer to a string so that the function can fill it 
'(for example when requesting an attribute's name), use a 
'System.Text.StringBuilder.
'
'Since VB doesn't have an unsigned byte, I've left those functions 
'out of the wrapper class. If you need to read unsigned bytes, read them as 
'shorts, and netcdf will automatically convert them for you.
'
'
Imports System.Runtime.InteropServices
Imports System.Text
Public Class NetCDF

    ' The netcdf external data types
    Enum nc_type
        NC_BYTE = 1 '/* signed 1 byte integer */
        NC_CHAR = 2 '/* ISO/ASCII character */
        NC_SHORT = 3 '/* signed 2 byte integer */
        NC_INT = 4 '/* signed 4 byte integer */
        NC_FLOAT = 5 '/* single precision floating point number */
        NC_DOUBLE = 6 '/* double precision floating point number */
    End Enum

    Enum cmode
        NC_NOWRITE = 0
        NC_WRITE = &H1      '/* read & write */
        NC_CLOBBER = 0
        NC_NOCLOBBER = &H4  '/* Don't destroy existing file on create */
        NC_FILL = 0         '/* argument to ncsetfill to clear NC_NOFILL */
        NC_NOFILL = &H100   '/* Don't fill data section an records */
        NC_LOCK = &H400     '/* Use locking if available */
        NC_SHARE = &H800   '/* Share updates, limit cacheing */
    End Enum

    '/*
    ' * 	Default fill values, used unless _FillValue attribute is set.
    ' * These values are stuffed into newly allocated space as appropriate.
    ' * The hope is that one might use these to notice that a particular datum
    ' * has not been set.
    ' */
    Public Const NC_FILL_BYTE As Byte = 255
    Public Const NC_FILL_CHAR As Byte = 0
    Public Const NC_FILL_SHORT As Int16 = -32767
    Public Const NC_FILL_INT As Int32 = -2147483647
    Public Const NC_FILL_FLOAT As Single = 9.96921E+36     '/* near 15 * 2^119 */
    Public Const NC_FILL_DOUBLE As Double = 9.969209968386869E+36

    '* 'size' argument to ncdimdef for an unlimited dimension
    Public Const NC_UNLIMITED As Int32 = 0

    '* attribute id to put/get a global attribute
    Public Const NC_GLOBAL As Int32 = -1

    '* These maximums are enforced by the interface, to facilitate writing
    '* applications and utilities.  However, nothing is statically allocated to
    '* these sizes internally.
    Enum netCDF_limits
        NC_MAX_DIMS = 10  '/* max dimensions per file */
        NC_MAX_ATTRS = 2000 '/* max global or per variable attributes */
        NC_MAX_VARS = 2000 '/* max variables per file */
        NC_MAX_NAME = 128 '/* max length of a name */
        NC_MAX_VAR_DIMS = 10 '/* max per variable dimensions */
    End Enum

    ' LIBRARY-WIDE AND FILE OPERATIONS
    '
    ' const char *nc_inq_libvers(void);
    Declare Ansi Function nc_inq_libvers Lib "netcdf.dll" () As String
    ' const char *nc_strerror(int ncerr1);
    Declare Ansi Function nc_strerror Lib "netcdf.dll" (ByVal ncerr1 As Int32) As String
    ' int nc_create(const char *path, int cmode, int *ncidp);
    Declare Ansi Function nc_create Lib "netcdf.dll" (ByVal path As String, ByVal cmode As Int32, _
        ByRef ncidp As Int32) As Int32
    ' int nc_open(const char *path, int mode, int *ncidp);
    Declare Ansi Function nc_open Lib "netcdf.dll" (ByVal path As String, ByVal cmode As Int32, _
        ByRef ncidp As Int32) As Int32
    ' int nc_set_fill(int ncid, int fillmode, int *old_modep);
    Declare Ansi Function nc_set_fill Lib "netcdf.dll" (ByVal ncid As Int32, ByVal fillmode As Int32, _
        ByRef old_modep As Int32) As Int32
    ' int nc_redef(int ncid);
    Declare Ansi Function nc_redef Lib "netcdf.dll" (ByVal ncid As Int32) As Int32
    ' int nc_enddef(int ncid);
    Declare Ansi Function nc_enddef Lib "netcdf.dll" (ByVal ncid As Int32) As Int32
    ' int nc_sync(int ncid);
    Declare Ansi Function nc_sync Lib "netcdf.dll" (ByVal ncid As Int32) As Int32
    ' int nc_abort(int ncid);
    Declare Ansi Function nc_abort Lib "netcdf.dll" (ByVal ncid As Int32) As Int32
    ' int nc_close(int ncid);
    Declare Ansi Function nc_close Lib "netcdf.dll" (ByVal ncid As Int32) As Int32

    ' INQ FUNCTIONS
    '
    ' int nc_inq(int ncid, int *ndimsp, int *nvarsp, int *nattsp, int *unlimdimidp);
    Declare Ansi Function nc_inq Lib "netcdf.dll" (ByVal ncid As Int32, ByRef ndimsp As Int32, _
        ByRef nvarsp As Int32, ByRef nattsp As Int32, ByRef unlimdimidp As Int32) As Int32
    ' int  nc_inq_ndims(int ncid, int *ndimsp);
    Declare Ansi Function nc_inq_ndims Lib "netcdf.dll" (ByVal ncid As Int32, ByRef ndimsp As Int32) As Int32
    ' int  nc_inq_nvars(int ncid, int *nvarsp);
    Declare Ansi Function nc_inq_nvars Lib "netcdf.dll" (ByVal ncid As Int32, ByRef nvarsp As Int32) As Int32
    ' int  nc_inq_natts(int ncid, int *nattsp);
    Declare Ansi Function nc_inq_natts Lib "netcdf.dll" (ByVal ncid As Int32, ByRef nattsp As Int32) As Int32
    ' int  nc_inq_unlimdim(int ncid, int *unlimdimidp);
    Declare Ansi Function nc_inq_unlimdim Lib "netcdf.dll" (ByVal ncid As Int32, ByRef unlimdimidp As Int32) As Int32
    ' int nc_def_dim(int ncid, const char *name, size_t len, int *idp);
    Declare Ansi Function nc_def_dim Lib "netcdf.dll" (ByVal ncid As Int32, ByVal name As String, _
        ByVal len As Int32, ByRef idp As Int32) As Int32
    ' int nc_inq_dimid(int ncid, const char *name, int *idp);
    Declare Ansi Function nc_inq_dimid Lib "netcdf.dll" (ByVal ncid As Int32, ByVal name As String, _
        ByRef idp As Int32) As Int32
    ' int nc_inq_dim(int ncid, int dimid, char *name, size_t *lenp);
    Declare Ansi Function nc_inq_dim Lib "netcdf.dll" (ByVal ncid As Int32, ByVal dimid As Int32, _
        ByVal name As StringBuilder, ByRef lenp As Int32) As Int32
    ' int  nc_inq_dimname(int ncid, int dimid, char *name);
    Declare Ansi Function nc_inq_dimname Lib "netcdf.dll" (ByVal ncid As Int32, ByVal dimid As Int32, _
        ByVal name As String) As Int32
    ' int  nc_inq_dimlen(int ncid, int dimid, size_t *lenp);
    Declare Ansi Function nc_inq_dimlen Lib "netcdf.dll" (ByVal ncid As Int32, ByVal dimid As Int32, _
        ByRef lenp As Int32) As Int32
    ' int nc_rename_dim(int ncid, int dimid, const char *name);
    Declare Ansi Function nc_rename_dim Lib "netcdf.dll" (ByVal ncid As Int32, ByVal dimid As Int32, _
        ByVal name As String) As Int32
    ' int nc_inq_att(int ncid, int varid, const char *name, nc_type *xtypep, size_t *lenp);
    Declare Ansi Function nc_inq_att Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
       ByVal name As String, ByRef xtypep As NetCDF.nc_type, ByRef lenp As Int32) As Int32
    ' int  nc_inq_attid(int ncid, int varid, const char *name, int *idp);
    Declare Ansi Function nc_inq_attid Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
       ByVal name As String, ByRef xtypep As NetCDF.nc_type, ByRef lenp As Int32) As Int32
    ' int  nc_inq_atttype(int ncid, int varid, const char *name, nc_type *xtypep);
    Declare Ansi Function nc_inq_atttype Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
       ByVal name As String, ByRef xtypep As NetCDF.nc_type, ByRef lenp As Int32) As Int32
    ' int  nc_inq_attlen(int ncid, int varid, const char *name, size_t *lenp);
    Declare Ansi Function nc_inq_attlen Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
       ByVal name As String, ByRef lenp As Int32) As Int32
    ' int nc_inq_attname(int ncid, int varid, int attnum, char *name);
    Declare Ansi Function nc_inq_attname Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
       ByVal attnum As Int32, ByVal name As StringBuilder) As Int32

    ' ATTRIBUTE READING AND WRITING

    ' int nc_copy_att(int ncid_in, int varid_in, const char *name, int ncid_out, int varid_out);
    Declare Ansi Function nc_copy_att Lib "netcdf.dll" (ByVal ncid_in As Int32, ByVal varid_in As Int32, _
       ByVal name As String, ByVal ncid_out As Int32, ByVal varid_out As Int32) As Int32
    ' int nc_rename_att(int ncid, int varid, const char *name, const char *newname);
    Declare Ansi Function nc_rename_att Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
       ByVal name As String, ByRef newname As String) As Int32
    ' int nc_del_att(int ncid, int varid, const char *name);
    Declare Ansi Function nc_del_att Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
       ByVal name As String) As Int32
    ' int nc_put_att_text(int ncid, int varid, const char *name,
    '	size_t len, const char *op);
    Declare Ansi Function nc_put_att_text Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, ByVal len As Int32, ByVal op As String) As Int32
    ' int nc_get_att_text(int ncid, int varid, const char *name, char *ip);
    Declare Ansi Function nc_get_att_text Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, ByVal op As StringBuilder) As Int32
    '
    ' int nc_put_att_uchar(int ncid, int varid, const char *name, nc_type xtype,
    '	size_t len, const unsigned char *op);
    Declare Ansi Function nc_put_att_uchar Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, ByVal xtype As NetCDF.nc_type, ByVal len As Int32, <[In]()> ByVal op() As Byte) As Int32
    ' int nc_get_att_uchar(int ncid, int varid, const char *name, unsigned char *ip);
    Declare Ansi Function nc_get_att_uchar Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, <[In](), Out()> ByVal ip() As Byte) As Int32
    '
    ' NOTE: There's no integral signed byte type in VB, so I won't implement these. If you need to 
    ' read an attribute of unsigned type, use the nc_get_att_int to read it directly into shorts, which
    ' will preserve the data. Ahhh... what the heck. Here they are anyway, for the signed-byte-writing freaks.
    ' int nc_put_att_schar(int ncid, int varid, const char *name, nc_type xtype,
    '	size_t len, const signed char *op);
    Declare Ansi Function nc_put_att_schar Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, ByVal xtype As NetCDF.nc_type, ByVal len As Int32, <[In]()> ByVal op() As Byte) As Int32
    ' int nc_get_att_schar(int ncid, int varid, const char *name, signed char *ip);
    Declare Ansi Function nc_get_att_schar Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, <[In](), Out()> ByVal ip() As Byte) As Int32
    ' int nc_put_att_short(int ncid, int varid, const char *name, nc_type xtype,
    '	size_t len, const short *op);
    Declare Ansi Function nc_put_att_short Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, ByVal xtype As NetCDF.nc_type, ByVal len As Int32, <[In]()> ByVal op() As Int16) As Int32
    ' int nc_get_att_short(int ncid, int varid, const char *name, short *ip);
    Declare Ansi Function nc_get_att_short Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, <[In](), Out()> ByVal ip() As Int16) As Int32
    ' int nc_put_att_int(int ncid, int varid, const char *name, nc_type xtype,
    '	size_t len, const int *op);
    Declare Ansi Function nc_put_att_int Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, ByVal xtype As NetCDF.nc_type, ByVal len As Int32, <[In]()> ByVal op() As Int32) As Int32
    ' int nc_get_att_int(int ncid, int varid, const char *name, int *ip);
    Declare Ansi Function nc_get_att_int Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, <[In](), Out()> ByVal ip() As Int32) As Int32
    ' int nc_put_att_long(int ncid, int varid, const char *name, nc_type xtype,
    '	size_t len, const long *op);
    Declare Ansi Function nc_put_att_long Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, ByVal xtype As NetCDF.nc_type, ByVal len As Int32, <[In]()> ByVal op() As Int32) As Int32
    ' int nc_get_att_long(int ncid, int varid, const char *name, long *ip);
    Declare Ansi Function nc_get_att_long Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, <[In](), Out()> ByVal ip() As Int32) As Int32
    ' int nc_put_att_float(int ncid, int varid, const char *name, nc_type xtype,
    '	size_t len, const float *op);
    Declare Ansi Function nc_put_att_float Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, ByVal xtype As NetCDF.nc_type, ByVal len As Int32, <[In]()> ByVal op() As Single) As Int32
    ' int nc_get_att_float(int ncid, int varid, const char *name, float *ip);
    Declare Ansi Function nc_get_att_float Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, <[In](), Out()> ByVal ip() As Single) As Int32
    ' int nc_put_att_double(int ncid, int varid, const char *name, nc_type xtype,
    '	size_t len, const double *op);
    Declare Ansi Function nc_put_att_double Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, ByVal xtype As NetCDF.nc_type, ByVal len As Int32, <[In]()> ByVal op() As double) As Int32
    ' int nc_get_att_double(int ncid, int varid, const char *name, double *ip);
    Declare Ansi Function nc_get_att_double Lib "netcdf.dll" (ByVal ncid As Int32, ByVal varid As Int32, _
      ByVal name As String, <[In](), Out()> ByVal ip() As Double) As Int32

    ' VARIABLE CREATION AND INQ

    ' int nc_def_var(int ncid, const char *name,
    '	 nc_type xtype, int ndims, const int *dimidsp, int *varidp);
    Declare Ansi Function nc_def_var Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal name As String, ByVal xtype As nc_type, ByVal ndims As Int32, _
        <[In]()> ByVal dimids() As Integer, ByRef varid As Int32) As Int32
    ' int nc_inq_var(int ncid, int varid, char *name, nc_type *xtypep, int *ndimsp, int *dimidsp, int *nattsp);
    Declare Ansi Function nc_inq_var Lib "netcdf.dll" (ByVal ncid As Int32, _
    ByVal varid As Int32, ByVal name As StringBuilder, ByRef xtypep As nc_type, ByRef ndimsp As Int32, _
    <Out()> ByVal dimidsp() As Integer, ByRef nattsp As Int32) As Int32

    ' int nc_inq_varid(int ncid, const char *name, int *varidp);
    Declare Ansi Function nc_inq_varid Lib "netcdf.dll" (ByVal ncid As Int32, _
      ByVal name As String, ByRef varid As Int32) As Int32
    ' int  nc_inq_varname(int ncid, int varid, char *name);
    Declare Ansi Function nc_inq_varname Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, ByVal name As StringBuilder) As Int32
    ' int  nc_inq_vartype(int ncid, int varid, nc_type *xtypep);
    Declare Ansi Function nc_inq_vartype Lib "netcdf.dll" (ByVal ncid As Int32, _
    ByVal varid As Int32, ByRef xtypep As nc_type) As Int32
    ' int  nc_inq_varndims(int ncid, int varid, int *ndimsp);
    Declare Ansi Function nc_inq_varndims Lib "netcdf.dll" (ByVal ncid As Int32, _
   ByVal varid As Int32, ByRef ndimsp As Int32) As Int32
    ' int  nc_inq_vardimid(int ncid, int varid, int *dimidsp);
    Declare Ansi Function nc_inq_vardimid Lib "netcdf.dll" (ByVal ncid As Int32, _
    ByVal varid As Int32, <Out()> ByVal dimidsp() As Integer, ByRef nattsp As Int32) As Int32
    ' int  nc_inq_varnatts(int ncid, int varid, int *nattsp);
    Declare Ansi Function nc_inq_varnatts Lib "netcdf.dll" (ByVal ncid As Int32, _
    ByVal varid As Int32, ByRef nattsp As Int32) As Int32
    ' int nc_rename_var(int ncid, int varid, const char *name);
    Declare Ansi Function nc_rename_var Lib "netcdf.dll" (ByVal ncid As Int32, _
      ByVal varid As Int32, ByVal name As String) As Int32
    '
    'READING AND WRITING ONE VALUE AT A TIME
    '
    ' int nc_put_var1_text(int ncid, int varid, const size_t *indexp, const char *op);
    Declare Ansi Function nc_put_var1_text Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
        ByVal op As String) As Int32
    ' int nc_get_var1_text(int ncid, int varid, const size_t *indexp, char *ip);
    Declare Ansi Function nc_get_var1_text Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
        ByVal ip As StringBuilder) As Int32
    ' int nc_put_var1_uchar(int ncid, int varid, const size_t *indexp,
    '	const unsigned char *op);
    Declare Ansi Function nc_put_var1_uchar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
        <[In](), Out()> ByVal op() As Byte) As Int32
    ' int nc_get_var1_uchar(int ncid, int varid, const size_t *indexp,
    '	unsigned char *ip);
    Declare Ansi Function nc_get_var1_uchar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
        <[In](), Out()> ByVal ip() As Byte) As Int32
    ' int nc_put_var1_schar(int ncid, int varid, const size_t *indexp,
    '	const signed char *op);
    Declare Ansi Function nc_put_var1_schar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
        <[In](), Out()> ByVal op() As Byte) As Int32
    ' int nc_get_var1_schar(int ncid, int varid, const size_t *indexp,
    '	signed char *ip);
    Declare Ansi Function nc_get_var1_schar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
        <[In](), Out()> ByVal ip() As Byte) As Int32
    ' int nc_put_var1_short(int ncid, int varid, const size_t *indexp,
    '	const short *op);
    Declare Ansi Function nc_put_var1_short Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
        <[In](), Out()> ByVal op() As Int16) As Int32
    ' int nc_get_var1_short(int ncid, int varid, const size_t *indexp,
    '	short *ip);
    Declare Ansi Function nc_get_var1_short Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
       <[In](), Out()> ByVal ip() As Int16) As Int32
    ' int nc_put_var1_int(int ncid, int varid, const size_t *indexp, const int *op);
    Declare Ansi Function nc_put_var1_int Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
        <[In](), Out()> ByVal op() As Int32) As Int32
    ' int nc_get_var1_int(int ncid, int varid, const size_t *indexp, int *ip);
    Declare Ansi Function nc_get_var1_int Lib "netcdf.dll" (ByVal ncid As Int32, _
      ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
      <[In](), Out()> ByVal ip() As Int32) As Int32
    ' int nc_put_var1_long(int ncid, int varid, const size_t *indexp, const long *op);
    Declare Ansi Function nc_put_var1_long Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
       <[In](), Out()> ByVal op() As Int32) As Int32
    ' int nc_get_var1_long(int ncid, int varid, const size_t *indexp, long *ip);
    Declare Ansi Function nc_get_var1_long Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
       <[In](), Out()> ByVal ip() As Int32) As Int32
    ' int nc_put_var1_float(int ncid, int varid, const size_t *indexp, const float *op);
    Declare Ansi Function nc_put_var1_float Lib "netcdf.dll" (ByVal ncid As Int32, _
      ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
      <[In](), Out()> ByVal op() As Single) As Int32
    ' int nc_get_var1_float(int ncid, int varid, const size_t *indexp, float *ip);
    Declare Ansi Function nc_get_var1_float Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
       <[In](), Out()> ByVal ip() As Single) As Int32
    ' int nc_put_var1_double(int ncid, int varid, const size_t *indexp, const double *op);
    Declare Ansi Function nc_put_var1_double Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
        <[In](), Out()> ByVal op() As Double) As Int32
    ' int nc_get_var1_double(int ncid, int varid, const size_t *indexp, double *ip);
    Declare Ansi Function nc_get_var1_double Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal indexp() As Int32, _
        <[In](), Out()> ByVal ip() As Double) As Int32

    'READING AND WRITING SUBSETS OF ARRAYS, WITH START AND COUNT ARRAYS

    ' int nc_put_vara_text(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const char *op);
    Declare Ansi Function nc_put_vara_text Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, ByVal op As String) As Int32
    ' int nc_get_vara_text(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, char *ip);
    Declare Ansi Function nc_get_vara_text Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, ByVal op As StringBuilder) As Int32
    ' int nc_put_vara_uchar(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const unsigned char *op);
    Declare Ansi Function nc_put_vara_uchar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal op() As Byte) As Int32
    ' int nc_get_vara_uchar(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, unsigned char *ip);
    Declare Ansi Function nc_get_vara_uchar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal ip() As Byte) As Int32
    ' int nc_put_vara_schar(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const signed char *op);
    Declare Ansi Function nc_put_vara_schar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal op() As Byte) As Int32
    ' int nc_get_vara_schar(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, signed char *ip);
    Declare Ansi Function nc_get_vara_schar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal ip() As Byte) As Int32
    ' int nc_put_vara_short(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const short *op);
    Declare Ansi Function nc_put_vara_short Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal op() As Short) As Int32
    ' int nc_get_vara_short(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, short *ip);
    Declare Ansi Function nc_get_vara_short Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal ip() As short) As Int32
    ' int nc_put_vara_int(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const int *op);
    Declare Ansi Function nc_put_vara_int Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal op() As Int32) As Int32
    ' int nc_get_vara_int(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, int *ip);
    Declare Ansi Function nc_get_vara_int Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal ip() As Int32) As Int32
    ' int nc_put_vara_long(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const long *op);
    Declare Ansi Function nc_put_vara_long Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal op() As Int32) As Int32
    ' int nc_get_vara_long(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, long *ip);
    Declare Ansi Function nc_get_vara_long Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal ip() As Int32) As Int32
    ' int nc_put_vara_float(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const float *op);
    Declare Ansi Function nc_put_vara_float Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal op() As Single) As Int32
    ' int nc_get_vara_float(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, float *ip);
    Declare Ansi Function nc_get_vara_float Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal ip() As Single) As Int32
    ' int nc_put_vara_double(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const double *op);
    Declare Ansi Function nc_put_vara_double Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal op() As Double) As Int32
    ' int nc_get_vara_double(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, double *ip);
    Declare Ansi Function nc_get_vara_double Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal ip() As Double) As Int32

    'READING AND WRITING SUBSETS OF ARRAYS WITH START, COUNT, and STRIDE ARRAYS

    ' int nc_put_vars_text(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const char *op);
    Declare Ansi Function nc_put_vars_text Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        ByVal op As String) As Int32
    ' int nc_get_vars_text(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	char *ip);
    Declare Ansi Function nc_get_vars_text Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
       <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
       ByVal op As StringBuilder) As Int32
    ' int nc_put_vars_uchar(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const unsigned char *op);
    Declare Ansi Function nc_put_vars_uchar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal op() As Byte) As Int32
    ' int nc_get_vars_uchar(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	unsigned char *ip);
    Declare Ansi Function nc_get_vars_uchar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal ip() As Byte) As Int32
    ' int nc_put_vars_schar(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const signed char *op);
    Declare Ansi Function nc_put_vars_schar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal op() As Byte) As Int32
    ' int nc_get_vars_schar(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	signed char *ip);
    Declare Ansi Function nc_get_vars_schar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal ip() As Byte) As Int32
    ' int nc_put_vars_short(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const short *op);
    Declare Ansi Function nc_put_vars_short Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal op() As Int16) As Int32
    ' int nc_get_vars_short(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	short *ip);
    Declare Ansi Function nc_get_vars_short Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal ip() As Int16) As Int32
    ' int nc_put_vars_int(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const int *op);
    Declare Ansi Function nc_put_vars_int Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal op() As Int32) As Int32
    ' int nc_get_vars_int(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	int *ip);
    Declare Ansi Function nc_get_vars_int Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal ip() As Int32) As Int32
    ' int nc_put_vars_long(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const long *op);
    Declare Ansi Function nc_put_vars_long Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal op() As Int32) As Int32
    ' int nc_get_vars_long(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	long *ip);
    Declare Ansi Function nc_get_vars_long Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
       <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
       <[In](), Out()> ByVal ip() As Int32) As Int32
    ' int nc_put_vars_float(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const float *op);
    Declare Ansi Function nc_put_vars_float Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal op() As Single) As Int32
    ' int nc_get_vars_float(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	float *ip);
    Declare Ansi Function nc_get_vars_float Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
       <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
       <[In](), Out()> ByVal ip() As Single) As Int32
    ' int nc_put_vars_double(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const double *op);
    Declare Ansi Function nc_put_vars_double Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal op() As Double) As Int32
    ' int nc_get_vars_double(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	double *ip);
    Declare Ansi Function nc_get_vars_double Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal ip() As Double) As Int32

    'READING AND WRITING MAPPED ARRAYS

    ' int nc_put_varm_text(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	const char *op);
    Declare Ansi Function nc_put_varm_text Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal imapp() As Int32, ByVal op As String) As Int32
    ' int nc_get_varm_text(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	char *ip);
    Declare Ansi Function nc_get_varm_text Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
       <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
       <[In](), Out()> ByVal imapp() As Int32, ByVal op As StringBuilder) As Int32
    ' int nc_put_varm_uchar(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	const unsigned char *op);
    Declare Ansi Function nc_put_varm_uchar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal op() As Byte) As Int32
    ' int nc_get_varm_uchar(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	unsigned char *ip);
    Declare Ansi Function nc_get_varm_uchar Lib "netcdf.dll" (ByVal ncid As Int32, _
         ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
         <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
         <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal ip() As Byte) As Int32
    ' int nc_put_varm_schar(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	const signed char *op);
    Declare Ansi Function nc_put_varm_schar Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
       <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
       <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal op() As Byte) As Int32
    ' int nc_get_varm_schar(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	signed char *ip);
    Declare Ansi Function nc_get_varm_schar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal ip() As Byte) As Int32
    ' int nc_put_varm_short(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	const short *op);
    Declare Ansi Function nc_put_varm_short Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
       <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
       <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal op() As Int16) As Int32
    ' int nc_get_varm_short(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	short *ip);
    Declare Ansi Function nc_get_varm_short Lib "netcdf.dll" (ByVal ncid As Int32, _
         ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
         <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
         <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal ip() As Int16) As Int32
    ' int nc_put_varm_int(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	const int *op);
    Declare Ansi Function nc_put_varm_int Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
       <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
       <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal op() As Int32) As Int32
    ' int nc_get_varm_int(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	int *ip);
    Declare Ansi Function nc_get_varm_int Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal ip() As Int32) As Int32
    ' int nc_put_varm_long(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	const long *op);
    Declare Ansi Function nc_put_varm_long Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
       <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
       <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal op() As Int32) As Int32
    ' int nc_get_varm_long(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	long *ip);
    Declare Ansi Function nc_get_varm_long Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal ip() As Int32) As Int32
    ' int nc_put_varm_float(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	const float *op);
    Declare Ansi Function nc_put_varm_float Lib "netcdf.dll" (ByVal ncid As Int32, _
      ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
      <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
      <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal op() As Single) As Int32
    ' int nc_get_varm_float(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	float *ip);
    Declare Ansi Function nc_get_varm_float Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
       <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
       <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal ip() As Single) As Int32
    ' int nc_put_varm_double(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t *imapp, 
    '	const double *op);
    Declare Ansi Function nc_put_varm_double Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
       <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
       <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal op() As Double) As Int32
    ' int nc_get_varm_double(int ncid, int varid,
    '	const size_t *startp, const size_t *countp, const ptrdiff_t *stridep,
    '	const ptrdiff_t * imap, 
    '	double *ip);
    Declare Ansi Function nc_get_varm_double Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal startp() As Int32, _
        <[In](), Out()> ByVal countp() As Int32, <[In](), Out()> ByVal stridep() As Int32, _
        <[In](), Out()> ByVal imapp() As Int32, <[In](), Out()> ByVal ip() As Double) As Int32
    '
    'READING AND WRITING VARS ALL AT ONCE
    '
    ' int nc_put_var_text(int ncid, int varid, const char *op);
    Declare Ansi Function nc_put_var_text Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, ByVal op As String) As Int32
    ' int nc_get_var_text(int ncid, int varid, char *ip);
    Declare Ansi Function nc_get_var_text Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, ByVal ip As StringBuilder) As Int32
    ' int nc_put_var_uchar(int ncid, int varid, const unsigned char *op);
    Declare Ansi Function nc_put_var_uchar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal op() As Byte) As Int32
    ' int nc_get_var_uchar(int ncid, int varid, unsigned char *ip);
    Declare Ansi Function nc_get_var_uchar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal ip() As Byte) As Int32
    ' int nc_put_var_schar(int ncid, int varid, const signed char *op);
    Declare Ansi Function nc_put_var_schar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal op() As Byte) As Int32
    ' int nc_get_var_schar(int ncid, int varid, signed char *ip);
    Declare Ansi Function nc_get_var_schar Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal ip() As Byte) As Int32
    ' int nc_put_var_short(int ncid, int varid, const short *op);
    Declare Ansi Function nc_put_var_short Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal op() As Int16) As Int32
    ' int nc_get_var_short(int ncid, int varid, short *ip);
    Declare Ansi Function nc_get_var_short Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal ip() As Int16) As Int32
    ' int nc_put_var_int(int ncid, int varid, const int *op);
    Declare Ansi Function nc_put_var_int Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal op() As Int32) As Int32
    ' int nc_get_var_int(int ncid, int varid, int *ip);
    Declare Ansi Function nc_get_var_int Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal ip() As Int32) As Int32
    ' int nc_put_var_long(int ncid, int varid, const long *op);
    Declare Ansi Function nc_put_var_long Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal op() As Int32) As Int32
    ' int nc_get_var_long(int ncid, int varid, long *ip);
    Declare Ansi Function nc_get_var_long Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal ip() As Int32) As Int32
    ' int nc_put_var_float(int ncid, int varid, const float *op);
    Declare Ansi Function nc_put_var_float Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal op() As Single) As Int32
    ' int nc_get_var_float(int ncid, int varid, float *ip);
    Declare Ansi Function nc_get_var_float Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal ip() As Single) As Int32
    ' int nc_put_var_double(int ncid, int varid, const double *op);
    Declare Ansi Function nc_put_var_double Lib "netcdf.dll" (ByVal ncid As Int32, _
       ByVal varid As Int32, <[In](), Out()> ByVal op() As Double) As Int32
    ' int nc_get_var_double(int ncid, int varid, double *ip);
    Declare Ansi Function nc_get_var_double Lib "netcdf.dll" (ByVal ncid As Int32, _
        ByVal varid As Int32, <[In](), Out()> ByVal ip() As Double) As Int32



End Class
