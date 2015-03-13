Appendix A: Attribute Conventions {#attribute_conventions}
=====================

Attribute conventions are assumed by some netCDF generic applications, e.g., ‘units’ as the name for a string attribute that gives the units for a netCDF variable.

It is strongly recommended that applicable conventions be followed unless there are good reasons for not doing so. Below we list the names and meanings of recommended standard attributes that have proven useful. Note that some of these (e.g. units, valid_range, scale_factor) assume numeric data and should not be used with character data.

    Attribute names commencing with underscore ('_') are reserved for use by the netCDF library.

# Conventions


`units`

> A character string that specifies the units used for the variable's data. Unidata has developed a freely-available library of routines to convert between character string and binary forms of unit specifications and to perform various useful operations on the binary forms. This library is used in some netCDF applications. Using the recommended units syntax permits data represented in conformable units to be automatically converted to common units for arithmetic operations. For more information see Units.


`long_name`

> A long descriptive name. This could be used for labeling plots, for example. If a variable has no long_name attribute assigned, the variable name should be used as a default.

`_FillValue`

> The _FillValue attribute specifies the fill value used to pre-fill disk space allocated to the variable. Such pre-fill occurs unless nofill mode is set using nc_set_fill(). The fill value is returned when reading values that were never written. If _FillValue is defined then it should be scalar and of the same type as the variable. If the variable is packed using scale_factor and add_offset attributes (see below), the _FillValue attribute should have the data type of the packed data.

<p>

> It is not necessary to define your own _FillValue attribute for a variable if the default fill value for the type of the variable is adequate. However, use of the default fill value for data type byte is not recommended. Note that if you change the value of this attribute, the changed value applies only to subsequent writes; previously written data are not changed.

<p>

> Generic applications often need to write a value to represent undefined or missing values. The fill value provides an appropriate value for this purpose because it is normally outside the valid range and therefore treated as missing when read by generic applications. It is legal (but not recommended) for the fill value to be within the valid range.

`missing_value`

> This attribute is not treated in any special way by the library or conforming generic applications, but is often useful documentation and may be used by specific applications. The missing_value attribute can be a scalar or vector containing values indicating missing data. These values should all be outside the valid range so that generic applications will treat them as missing.

<p>

> When scale_factor and add_offset are used for packing, the value(s) of the missing_value attribute should be specified in the domain of the data in the file (the packed data), so that missing values can be detected before the scale_factor and add_offset are applied.


`valid_min`

> A scalar specifying the minimum valid value for this variable.

`valid_max`

> A scalar specifying the maximum valid value for this variable.

`valid_range`


> A vector of two numbers specifying the minimum and maximum valid values for this variable, equivalent to specifying values for both valid_min and valid_max attributes. Any of these attributes define the valid range. The attribute valid_range must not be defined if either valid_min or valid_max is defined.

<p>

> Generic applications should treat values outside the valid range as missing. The type of each valid_range, valid_min and valid_max attribute should match the type of its variable (except that for byte data, these can be of a signed integral type to specify the intended range).

<p>

> If neither valid_min, valid_max nor valid_range is defined then generic applications should define a valid range as follows. If the data type is byte and _FillValue is not explicitly defined, then the valid range should include all possible values. Otherwise, the valid range should exclude the _FillValue (whether defined explicitly or by default) as follows. If the _FillValue is positive then it defines a valid maximum, otherwise it defines a valid minimum. For integer types, there should be a difference of 1 between the _FillValue and this valid minimum or maximum. For floating point types, the difference should be twice the minimum possible (1 in the least significant bit) to allow for rounding error.

<p>

> If the variable is packed using scale_factor and add_offset attributes (see below), the _FillValue, missing_value, valid_range, valid_min, or valid_max attributes should have the data type of the packed data.

`scale_factor`

> If present for a variable, the data are to be multiplied by this factor after the data are read by the application that accesses the data.

> If valid values are specified using the valid_min, valid_max, valid_range, or _FillValue attributes, those values should be specified in the domain of the data in the file (the packed data), so that they can be interpreted before the scale_factor and add_offset are applied.

`add_offset`

> If present for a variable, this number is to be added to the data after it is read by the application that accesses the data. If both scale_factor and add_offset attributes are present, the data are first scaled before the offset is added. The attributes scale_factor and add_offset can be used together to provide simple data compression to store low-resolution floating-point data as small integers in a netCDF dataset. When scaled data are written, the application should first subtract the offset and then divide by the scale factor, rounding the result to the nearest integer to avoid a bias caused by truncation towards zero.

<p>

> When scale_factor and add_offset are used for packing, the associated variable (containing the packed data) is typically of type byte or short, whereas the unpacked values are intended to be of type float or double. The attributes scale_factor and add_offset should both be of the type intended for the unpacked data, e.g. float or double.

`Coordinates`

> Following the CF (Climate and Forecast) conventions for netCDF metadata, we define an auxiliary coordinate variable as any netCDF variable that contains coordinate data, but is not a coordinate variable (See Coordinate Variables). Unlike coordinate variables, there is no relationship between the name of an auxiliary coordinate variable and the name(s) of its dimension(s).

<p>

> The value of the coordinates attribute is a blank separated list of names of auxiliary coordinate variables and (optionally) coordinate variables. There is no restriction on the order in which the variable names appear in the coordinates attribute string.

`signedness`

> Deprecated attribute, originally designed to indicate whether byte values should be treated as signed or unsigned. The attributes valid_min and valid_max may be used for this purpose. For example, if you intend that a byte variable store only non-negative values, you can use valid_min = 0 and valid_max = 255. This attribute is ignored by the netCDF library.

`C_format`

> A character array providing the format that should be used by C applications to print values for this variable. For example, if you know a variable is only accurate to three significant digits, it would be appropriate to define the C_format attribute as "%.3g". The ncdump utility program uses this attribute for variables for which it is defined. The format applies to the scaled (internal) type and value, regardless of the presence of the scaling attributes scale_factor and add_offset.

`FORTRAN_format`

> A character array providing the format that should be used by FORTRAN applications to print values for this variable. For example, if you know a variable is only accurate to three significant digits, it would be appropriate to define the FORTRAN_format attribute as "(G10.3)".

`title`

> A global attribute that is a character array providing a succinct description of what is in the dataset.

`history`

> A global attribute for an audit trail. This is a character array with a line for each invocation of a program that has modified the dataset. Well-behaved generic netCDF applications should append a line containing: date, time of day, user name, program name and command arguments.

`Conventions`

> If present, 'Conventions' is a global attribute that is a character array for the name of the conventions followed by the dataset. Originally, these conventions were named by a string that was interpreted as a directory name relative to the directory /pub/netcdf/Conventions/ on the host ftp.unidata.ucar.edu. The web page http://www.unidata.ucar.edu/netcdf/conventions.html is now the preferred and authoritative location for registering a URI reference to a set of conventions maintained elsewhere. The FTP site will be preserved for compatibility with existing references, but authors of new conventions should submit a request to support-netcdf@unidata.ucar.edu for listing on the Unidata conventions web page.

<p>

> It may be convenient for defining institutions and groups to use a hierarchical structure for general conventions and more specialized conventions. For example, if a group named NUWG agrees upon a set of conventions for dimension names, variable names, required attributes, and netCDF representations for certain discipline-specific data structures, they may store a document describing the agreed-upon conventions in a dataset in the NUWG/ subdirectory of the Conventions directory. Datasets that followed these conventions would contain a global Conventions attribute with value "NUWG".

<p>

> Later, if the group agrees upon some additional conventions for a specific subset of NUWG data, for example time series data, the description of the additional conventions might be stored in the NUWG/Time_series/ subdirectory, and datasets that adhered to these additional conventions would use the global Conventions attribute with value "NUWG/Time_series", implying that this dataset adheres to the NUWG conventions and also to the additional NUWG time-series conventions.

<p>

> It is possible for a netCDF file to adhere to more than one set of conventions, even when there is no inheritance relationship among the conventions. In this case, the value of the 'Conventions' attribute may be a single text string containing a list of the convention names separated by blank space (recommended) or commas (if a convention name contains blanks).

<p>

> Typical conventions web sites will include references to documents in some form agreed upon by the community that supports the conventions and examples of netCDF file structures that follow the conventions.
