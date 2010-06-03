Option Strict Off
Imports System.Text
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

'This is a super-primitive form to illustrate the use of the netcdf vb wrapper class.
'The wrapper class can be used in any VB project to provide netCDF calls.
'For this to work, you must first have the netcdf.dll in your path, or in the 
'directory that this program is running in.
'Ed Hartnett - 12/29/04

Public Class Form1
    Inherits System.Windows.Forms.Form

#Region " Windows Form Designer generated code "

    Public Sub New()
        MyBase.New()

        'This call is required by the Windows Form Designer.
        InitializeComponent()

        'Add any initialization after the InitializeComponent() call

    End Sub

    'Form overrides dispose to clean up the component list.
    Protected Overloads Overrides Sub Dispose(ByVal disposing As Boolean)
        If disposing Then
            If Not (components Is Nothing) Then
                components.Dispose()
            End If
        End If
        MyBase.Dispose(disposing)
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    Friend WithEvents Button1 As System.Windows.Forms.Button
    Friend WithEvents Button2 As System.Windows.Forms.Button
    Friend WithEvents out As System.Windows.Forms.TextBox
    <System.Diagnostics.DebuggerStepThrough()> Private Sub InitializeComponent()
        Me.Button1 = New System.Windows.Forms.Button
        Me.Button2 = New System.Windows.Forms.Button
        Me.out = New System.Windows.Forms.TextBox
        Me.SuspendLayout()
        '
        'Button1
        '
        Me.Button1.Location = New System.Drawing.Point(8, 24)
        Me.Button1.Name = "Button1"
        Me.Button1.Size = New System.Drawing.Size(75, 48)
        Me.Button1.TabIndex = 0
        Me.Button1.Text = "Test libvers and nc_str_error"
        '
        'Button2
        '
        Me.Button2.Location = New System.Drawing.Point(88, 32)
        Me.Button2.Name = "Button2"
        Me.Button2.TabIndex = 1
        Me.Button2.Text = "run test"
        '
        'out
        '
        Me.out.Location = New System.Drawing.Point(0, 96)
        Me.out.Multiline = True
        Me.out.Name = "out"
        Me.out.ReadOnly = True
        Me.out.ScrollBars = System.Windows.Forms.ScrollBars.Both
        Me.out.Size = New System.Drawing.Size(528, 432)
        Me.out.TabIndex = 2
        Me.out.Text = ""
        '
        'Form1
        '
        Me.AutoScaleBaseSize = New System.Drawing.Size(5, 13)
        Me.ClientSize = New System.Drawing.Size(536, 533)
        Me.Controls.Add(Me.out)
        Me.Controls.Add(Me.Button2)
        Me.Controls.Add(Me.Button1)
        Me.Name = "Form1"
        Me.Text = "Form1"
        Me.ResumeLayout(False)

    End Sub

#End Region

    Private Sub Button1_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button1.Click
        Dim s As String
        s = NetCDF.nc_inq_libvers()
        MsgBox("vers: " & s)
        s = NetCDF.nc_strerror(-33)
        MsgBox("err: " & s)

    End Sub

    Private Sub Button2_Click(ByVal sender As System.Object, ByVal e As System.EventArgs) Handles Button2.Click
        WriteTestFile("c:\test.nc")
        ReadTestFile("c:\test.nc")
    End Sub

    Private Sub WriteTestFile(ByVal name As String)
        Dim ncid As Int32
        Dim dimid As Int32
        Dim varid As Int32
        Dim dimids(0) As Integer
        Dim int_att(1) As Int32
        Dim atttext As New String("I am the very model of a modern Major General!")
        Dim varbyte() As Byte = {0, 1, 2}
        Dim attbyte() As Byte = {0, 255, 122}
        Dim varint() As Int32 = {-99, 99, 0}
        Dim two_d_int(,) As Int32 = {{1, 2}, {3, 4}}
        Dim flatten(8) As Int32
        Dim res As Int32, i As Integer, j As Integer

        'For i = 0 To 1
        '    For j = 0 To 1
        '        Log("data(" & Str(i) & ", " & Str(j) & ") = " & Str(two_d_int(i, j)))
        '    Next
        'Next
        'flatten = CType(two_d_int, Array)
        'For i = 0 To 3
        '    Log("data(" & Str(i) & ") = " & Str(flatten(i)))
        'Next

        'Create a netcdf file and write a bunch of stuff to it.
        res = NetCDF.nc_create(name, NetCDF.cmode.NC_CLOBBER, ncid)
        If (res <> 0) Then GoTo err
        Log("created file " & name)

        'Write some attributes.
        int_att(0) = 99
        int_att(1) = -99
        res = NetCDF.nc_put_att_int(ncid, NetCDF.NC_GLOBAL, "int_att", NetCDF.nc_type.NC_INT, 2, int_att)
        If (res <> 0) Then GoTo err

        res = NetCDF.nc_put_att_text(ncid, NetCDF.NC_GLOBAL, "for_employee_self_evaluations", _
            atttext.Length, atttext)
        If (res <> 0) Then GoTo err
        res = NetCDF.nc_put_att_uchar(ncid, NetCDF.NC_GLOBAL, "uchar_att", NetCDF.nc_type.NC_BYTE, _
            attbyte.Length, attbyte)
        If (res <> 0) Then GoTo err

        'Define dimensions
        res = NetCDF.nc_def_dim(ncid, "d1", 3, dimid)
        If (res <> 0) Then GoTo err
        res = NetCDF.nc_def_dim(ncid, "d2", 2, dimid)
        If (res <> 0) Then GoTo err
        res = NetCDF.nc_def_dim(ncid, "d3", 5, dimid)
        If (res <> 0) Then GoTo err
        res = NetCDF.nc_def_dim(ncid, "d4", 7, dimid)
        If (res <> 0) Then GoTo err

        'varids will start at 0
        dimids(0) = 0
        res = NetCDF.nc_def_var(ncid, "d1", NetCDF.nc_type.NC_INT, 1, dimids, varid)
        If (res <> 0) Then GoTo err
        'varid will be 1
        res = NetCDF.nc_def_var(ncid, "v1", NetCDF.nc_type.NC_INT, 1, dimids, varid)
        If (res <> 0) Then GoTo err
        'varid will be 2
        res = NetCDF.nc_def_var(ncid, "eternal_question", NetCDF.nc_type.NC_CHAR, 1, dimids, varid)
        If (res <> 0) Then GoTo err
        'varid will be 3
        res = NetCDF.nc_def_var(ncid, "v_byte", NetCDF.nc_type.NC_BYTE, 1, dimids, varid)
        If (res <> 0) Then GoTo err

        res = NetCDF.nc_enddef(ncid)
        If (res <> 0) Then GoTo err

        res = NetCDF.nc_put_var_int(ncid, 0, varint)
        If (res <> 0) Then GoTo err
        res = NetCDF.nc_put_var_int(ncid, 1, varint)
        If (res <> 0) Then GoTo err
        res = NetCDF.nc_put_var_text(ncid, 2, "why")
        If (res <> 0) Then GoTo err
        res = NetCDF.nc_put_var_uchar(ncid, 3, varbyte)
        If (res <> 0) Then GoTo err


        'Close the netCDF file.
        res = NetCDF.nc_close(ncid)
        If (res <> 0) Then GoTo err

        Exit Sub
err:
        MsgBox("error:" & NetCDF.nc_strerror(res))

    End Sub

    Private Sub ReadTestFile(ByVal name As String)
        Dim ncid As Int32
        Dim dimid As Int32
        Dim varid As Int32
        Dim natts As Int32, nvars As Int32, ndims As Int32, unlimdimid As Int32
        Dim dimlen As Int32, attlen As Int32
        Dim dimids(1) As Integer
        Dim xtype As NetCDF.nc_type
        Dim res As Int32
        Dim dn As String
        Dim i As Integer, j As Integer
        Dim dimname As New StringBuilder(NetCDF.netCDF_limits.NC_MAX_NAME)

        'Open the netcdf file and read it out.
        res = NetCDF.nc_open(name, NetCDF.cmode.NC_NOWRITE, ncid)
        If (res <> 0) Then GoTo err

        res = NetCDF.nc_inq(ncid, ndims, nvars, natts, unlimdimid)
        If (res <> 0) Then GoTo err

        Log("Read " & Str(ndims) & " dims, " & Str(nvars) & " vars, " & _
            Str(natts) & " global atts, and an unlimited dimension of " & Str(unlimdimid) & ".")

        'Read dimension metadata.
        For i = 0 To ndims - 1
            res = NetCDF.nc_inq_dim(ncid, i, dimname, dimlen)
            If (res <> 0) Then GoTo err
            Log("dimid: " & Str(i) & " name: " & dimname.ToString & " len: " & Str(dimlen))
        Next

        'Read global attributes.
        For i = 0 To natts - 1
            ReadAtt(ncid, NetCDF.NC_GLOBAL, i)
        Next

        'Read variables, including their attributes.
        For i = 0 To nvars - 1
            ReadVar(ncid, i)
        Next

        'Close the netCDF file.
        res = NetCDF.nc_close(ncid)
        If (res <> 0) Then GoTo err

        Exit Sub
err:
        MsgBox("error:" & NetCDF.nc_strerror(res))

    End Sub

    'Read an attribute, given an ncid for an open file, a varid (which can be 
    'NetCDF.NC_GLOBAL for file (i.e. global) attributes), and an attnum. This 
    'function will learn everything about the attribute, and log the results.
    Private Sub ReadAtt(ByVal ncid As Int32, ByVal varid As Int32, _
        ByVal attnum As Int32)

        Dim attname As New StringBuilder(NetCDF.netCDF_limits.NC_MAX_NAME)
        Dim atttext As StringBuilder, attbyte() As Byte, attint() As Int32
        Dim xtype As NetCDF.nc_type
        Dim attlen As Int32
        Dim res As Int32, i As Integer

        'First find the name, given the attnum.
        res = NetCDF.nc_inq_attname(ncid, varid, attnum, attname)
        If (res <> 0) Then GoTo err

        'Now find the type and length, using the name.
        res = NetCDF.nc_inq_att(ncid, varid, attname.ToString, xtype, attlen)
        If (res <> 0) Then GoTo err
        Log("varid: " & Str(varid) & " attnum: " & Str(attnum) & " name: " & _
            attname.ToString & " len: " & Str(attlen) & " type: " & Str(xtype))

        'Now read the value, depending on the type.
        Select Case xtype
            Case NetCDF.nc_type.NC_CHAR
                'There's possibly some size limits I should be checking for here?
                atttext = New StringBuilder(attlen)
                res = NetCDF.nc_get_att_text(ncid, varid, attname.ToString, atttext)
                If (res <> 0) Then GoTo err
                Log("text value: " & atttext.ToString)

            Case NetCDF.nc_type.NC_BYTE
                ReDim attbyte(attlen - 1)
                res = NetCDF.nc_get_att_uchar(ncid, varid, attname.ToString, attbyte)
                If (res <> 0) Then GoTo err
                For i = 0 To attlen - 1
                    Log("index: " & Str(i) & "value: " & Str(attbyte(i)))
                Next

            Case NetCDF.nc_type.NC_INT
                ReDim attint(attlen - 1)
                res = NetCDF.nc_get_att_int(ncid, varid, attname.ToString, attint)
                If (res <> 0) Then GoTo err
                For i = 0 To attlen - 1
                    Log("index: " & Str(i) & "value: " & Str(attint(i)))
                Next



        End Select

        Exit Sub
err:
        MsgBox("error:" & NetCDF.nc_strerror(res))

    End Sub

    'Read a var.
    Private Sub ReadVar(ByVal ncid As Int32, ByVal varid As Int32)

        Dim natts As Int32, nvars As Int32, ndims As Int32, unlimdimid As Int32
        Dim dimlen As Int32, attlen As Int32
        Dim dimids(1) As Integer
        Dim xtype As NetCDF.nc_type
        Dim varname As New StringBuilder(NetCDF.netCDF_limits.NC_MAX_NAME)
        Dim vartext As StringBuilder, varbyte() As Byte, varshort() As Short, varint() As Integer
        Dim varsingle() As Single, vardouble() As Double
        Dim attname As New StringBuilder(NetCDF.netCDF_limits.NC_MAX_NAME)
        Dim res As Int32, i As Integer, j As Integer

        'Learn about this var.
        res = NetCDF.nc_inq_var(ncid, varid, varname, xtype, ndims, dimids, natts)
        If (res <> 0) Then GoTo err
        Log("varid: " & Str(varid) & " name: " & varname.ToString & " ndims: " & Str(ndims) & _
            " type: " & Str(xtype))
        For i = 0 To ndims - 1
            Log("dim: " & Str(i) & " id: " & dimids(i))
        Next

        'Read attributes, if any.
        For i = 0 To natts - 1
            ReadAtt(ncid, varid, i)
        Next

        'Read data if it's easy.
        If (ndims = 1) Then
            'Find the length.
            res = NetCDF.nc_inq_dimlen(ncid, dimids(0), dimlen)
            If (res <> 0) Then GoTo err
            Select Case xtype
                Case NetCDF.nc_type.NC_BYTE
                    ReDim varbyte(dimlen - 1)
                    res = NetCDF.nc_get_var_uchar(ncid, varid, varbyte)
                    If (res <> 0) Then GoTo err
                    For i = 0 To dimlen - 1
                        Log("Ver byte value " & Str(i) & ": " & Str(varbyte(i)))
                    Next

                Case NetCDF.nc_type.NC_CHAR
                    vartext = New StringBuilder(dimlen - 1)
                    res = NetCDF.nc_get_var_text(ncid, varid, vartext)
                    If (res <> 0) Then GoTo err
                    Log("Var value: " & vartext.ToString)

                Case NetCDF.nc_type.NC_DOUBLE
                    ReDim varbyte(dimlen - 1)
                    res = NetCDF.nc_get_var_double(ncid, varid, vardouble)
                    If (res <> 0) Then GoTo err
                    For i = 0 To dimlen - 1
                        Log("Ver byte value " & Str(i) & ": " & Str(vardouble(i)))
                    Next

                Case NetCDF.nc_type.NC_INT
                Case NetCDF.nc_type.NC_FLOAT
                Case NetCDF.nc_type.NC_SHORT

            End Select
        End If

        Exit Sub
err:
        MsgBox("error:" & NetCDF.nc_strerror(res))

    End Sub

    'Append any string passed in to the textbox. Add a CR/LF.
    Private Sub Log(ByVal msg As String)
        out.Text = out.Text & msg & Chr(13) & Chr(10)
    End Sub


End Class
