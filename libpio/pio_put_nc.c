/**
 * @file
 * PIO functions to write data.
 *
 * @author Ed Hartnett
 * @date  2016
 * @see http://code.google.com/p/parallelio/
 */

#include <config.h>
#include <pio.h>
#include <pio_internal.h>

/**
 * Get strided, muti-dimensional subset of a text variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars_text(int ncid, int varid, const PIO_Offset *start, const PIO_Offset *count,
                       const PIO_Offset *stride, const char *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, NC_CHAR, op);
}

/**
 * Get strided, muti-dimensional subset of an unsigned char variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars_uchar(int ncid, int varid, const PIO_Offset *start,
                        const PIO_Offset *count, const PIO_Offset *stride,
                        const unsigned char *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, NC_UBYTE, op);
}

/**
 * Get strided, muti-dimensional subset of a signed char variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars_schar(int ncid, int varid, const PIO_Offset *start, const PIO_Offset *count,
                        const PIO_Offset *stride, const signed char *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, NC_BYTE, op);
}

/**
 * Get strided, muti-dimensional subset of an unsigned 16-bit integer
 * variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars_ushort(int ncid, int varid, const PIO_Offset *start, const PIO_Offset *count,
                         const PIO_Offset *stride, const unsigned short *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, NC_USHORT, op);
}

/**
 * Get strided, muti-dimensional subset of a 16-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars_short(int ncid, int varid, const PIO_Offset *start,
                        const PIO_Offset *count, const PIO_Offset *stride, const short *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, NC_SHORT, op);
}

/**
 * Get strided, muti-dimensional subset of an unsigned integer
 * variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars_uint(int ncid, int varid, const PIO_Offset *start, const PIO_Offset *count,
                       const PIO_Offset *stride, const unsigned int *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, NC_UINT, op);
}

/**
 * Get strided, muti-dimensional subset of an integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars_int(int ncid, int varid, const PIO_Offset *start, const PIO_Offset *count,
                      const PIO_Offset *stride, const int *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, NC_INT, op);
}

/**
 * Get strided, muti-dimensional subset of a 64-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars_long(int ncid, int varid, const PIO_Offset *start, const PIO_Offset *count,
                       const PIO_Offset *stride, const long *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, PIO_LONG_INTERNAL, op);
}

/**
 * Get strided, muti-dimensional subset of a floating point variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars_float(int ncid, int varid, const PIO_Offset *start, const PIO_Offset *count,
                        const PIO_Offset *stride, const float *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, NC_FLOAT, op);
}

/**
 * Get strided, muti-dimensional subset of a 64-bit unsigned integer
 * variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars_longlong(int ncid, int varid, const PIO_Offset *start, const PIO_Offset *count,
                           const PIO_Offset *stride, const long long *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, NC_INT64, op);
}

/**
 * Get strided, muti-dimensional subset of a 64-bit floating point
 * variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars_double(int ncid, int varid, const PIO_Offset *start, const PIO_Offset *count,
                         const PIO_Offset *stride, const double *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, NC_DOUBLE, op);
}

/**
 * Get strided, muti-dimensional subset of an unsigned 64-bit integer
 * variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars_ulonglong(int ncid, int varid, const PIO_Offset *start, const PIO_Offset *count,
                            const PIO_Offset *stride, const unsigned long long *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, NC_UINT64, op);
}

/**
 * Get one value from an text variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1_text(int ncid, int varid, const PIO_Offset *index, const char *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, NC_CHAR, op);
}

/**
 * Get one value from an text variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1_uchar(int ncid, int varid, const PIO_Offset *index,
                        const unsigned char *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, NC_UBYTE, op);
}

/**
 * Get one value from an signed char variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1_schar(int ncid, int varid, const PIO_Offset *index,
                        const signed char *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, NC_BYTE, op);
}

/**
 * Get one value from an unsigned 16-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1_ushort(int ncid, int varid, const PIO_Offset *index,
                         const unsigned short *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, NC_USHORT, op);
}

/**
 * Get one value from a 16-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1_short(int ncid, int varid, const PIO_Offset *index,
                        const short *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, NC_SHORT, op);
}

/**
 * Get one value from an unsigned integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1_uint(int ncid, int varid, const PIO_Offset *index,
                       const unsigned int *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, NC_UINT, op);
}

/**
 * Get one value from an integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1_int(int ncid, int varid, const PIO_Offset *index, const int *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, NC_INT, op);
}

/**
 * Get one value from an floating point variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1_float(int ncid, int varid, const PIO_Offset *index, const float *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, NC_FLOAT, op);
}

/**
 * Get one value from an integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1_long(int ncid, int varid, const PIO_Offset *index, const long *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, PIO_LONG_INTERNAL, op);
}

/**
 * Get one value from an 64-bit floating point variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1_double(int ncid, int varid, const PIO_Offset *index,
                         const double *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, NC_DOUBLE, op);
}

/**
 * Get one value from an unsigned 64-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1_ulonglong(int ncid, int varid, const PIO_Offset *index,
                            const unsigned long long *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, NC_UINT64, op);
}

/**
 * Get one value from a 64-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1_longlong(int ncid, int varid, const PIO_Offset *index,
                           const long long *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, NC_INT64, op);
}

/**
 * Put muti-dimensional subset of a text variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara_text(int ncid, int varid, const PIO_Offset *start,
                       const PIO_Offset *count, const char *op)
{
    return PIOc_put_vars_text(ncid, varid, start, count, NULL, op);
}

/**
 * Put muti-dimensional subset of an unsigned char variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara_uchar(int ncid, int varid, const PIO_Offset *start,
                        const PIO_Offset *count, const unsigned char *op)
{
    return PIOc_put_vars_uchar(ncid, varid, start, count, NULL, op);
}

/**
 * Put muti-dimensional subset of a signed char variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara_schar(int ncid, int varid, const PIO_Offset *start,
                        const PIO_Offset *count, const signed char *op)
{
    return PIOc_put_vars_schar(ncid, varid, start, count, NULL, op);
}

/**
 * Put muti-dimensional subset of an unsigned 16-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara_ushort(int ncid, int varid, const PIO_Offset *start,
                         const PIO_Offset *count, const unsigned short *op)
{
    return PIOc_put_vars_ushort(ncid, varid, start, count, NULL, op);
}

/**
 * Put muti-dimensional subset of a 16-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara_short(int ncid, int varid, const PIO_Offset *start,
                        const PIO_Offset *count, const short *op)
{
    return PIOc_put_vars_short(ncid, varid, start, count, NULL, op);
}

/**
 * Put muti-dimensional subset of an unsigned integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara_uint(int ncid, int varid, const PIO_Offset *start,
                       const PIO_Offset *count, const unsigned int *op)
{
    return PIOc_put_vars_uint(ncid, varid, start, count, NULL, op);
}

/**
 * Put muti-dimensional subset of an integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara_int(int ncid, int varid, const PIO_Offset *start,
                      const PIO_Offset *count, const int *op)
{
    return PIOc_put_vars_int(ncid, varid, start, count, NULL, op);
}

/**
 * Put muti-dimensional subset of an integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara_long(int ncid, int varid, const PIO_Offset *start,
                       const PIO_Offset *count, const long *op)
{
    return PIOc_put_vars_long(ncid, varid, start, count, NULL, op);
}

/**
 * Put muti-dimensional subset of a floating point variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara_float(int ncid, int varid, const PIO_Offset *start,
                        const PIO_Offset *count, const float *op)
{
    return PIOc_put_vars_float(ncid, varid, start, count, NULL, op);
}

/**
 * Put muti-dimensional subset of an unsigned 64-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara_ulonglong(int ncid, int varid, const PIO_Offset *start,
                            const PIO_Offset *count, const unsigned long long *op)
{
    return PIOc_put_vars_ulonglong(ncid, varid, start, count, NULL, op);
}

/**
 * Put muti-dimensional subset of a 64-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara_longlong(int ncid, int varid, const PIO_Offset *start,
                           const PIO_Offset *count, const long long *op)
{
    return PIOc_put_vars_longlong(ncid, varid, start, count, NULL, op);
}

/**
 * Put muti-dimensional subset of a 64-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara_double(int ncid, int varid, const PIO_Offset *start,
                         const PIO_Offset *count, const double *op)
{
    return PIOc_put_vars_double(ncid, varid, start, count, NULL, op);
}

/**
 * Put all data to a text variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var_text(int ncid, int varid, const char *op)
{
    return PIOc_put_var_tc(ncid, varid, PIO_CHAR, op);
}

/**
 * Put all data to an unsigned char variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var_uchar(int ncid, int varid, const unsigned char *op)
{
    return PIOc_put_var_tc(ncid, varid, PIO_UBYTE, op);
}

/**
 * Put all data to a signed char variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var_schar(int ncid, int varid, const signed char *op)
{
    return PIOc_put_var_tc(ncid, varid, PIO_BYTE, op);
}

/**
 * Put all data to a 16-bit unsigned integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var_ushort(int ncid, int varid, const unsigned short *op)
{
    return PIOc_put_var_tc(ncid, varid, NC_USHORT, op);
}

/**
 * Put all data to a 16-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var_short(int ncid, int varid, const short *op)
{
    return PIOc_put_var_tc(ncid, varid, PIO_SHORT, op);
}

/**
 * Put all data to an unsigned integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var_uint(int ncid, int varid, const unsigned int *op)
{
    return PIOc_put_var_tc(ncid, varid, PIO_UINT, op);
}

/**
 * Put all data to an integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var_int(int ncid, int varid, const int *op)
{
    return PIOc_put_var_tc(ncid, varid, PIO_INT, op);
}

/**
 * Put all data to an integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var_long(int ncid, int varid, const long *op)
{
    return PIOc_put_var_tc(ncid, varid, PIO_LONG_INTERNAL, op);
}

/**
 * Put all data to a floating point variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var_float(int ncid, int varid, const float *op)
{
    return PIOc_put_var_tc(ncid, varid, PIO_FLOAT, op);
}

/**
 * Put all data to an unsigned 64-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var_ulonglong(int ncid, int varid, const unsigned long long *op)
{
    return PIOc_put_var_tc(ncid, varid, PIO_UINT64, op);
}

/**
 * Put all data to a 64-bit integer variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var_longlong(int ncid, int varid, const long long *op)
{
    return PIOc_put_var_tc(ncid, varid, PIO_INT64, op);
}

/**
 * Put all data to a 64-bit floating point variable.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param op pointer to the data to be written.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var_double(int ncid, int varid, const double *op)
{
    return PIOc_put_var_tc(ncid, varid, PIO_DOUBLE, op);
}

/**
 * Put all data to a variable of any type.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param buf pointer that will get the data.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var(int ncid, int varid, const void *op)
{
    return PIOc_put_var_tc(ncid, varid, NC_NAT, op);
}

/**
 * Get one value from a variable of any type.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param buf pointer that will get the data.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_var1(int ncid, int varid, const PIO_Offset *index, const void *op)
{
    return PIOc_put_var1_tc(ncid, varid, index, NC_NAT, op);
}

/**
 * Put muti-dimensional subset of a variable of any type.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param buf pointer that will get the data.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vara(int ncid, int varid, const PIO_Offset *start, const PIO_Offset *count,
                  const void *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, NULL, NC_NAT, op);
}

/**
 * Write strided, muti-dimensional subset of a variable of any type.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID number
 * @param start an array of start indicies (must have same number of
 * entries as variable has dimensions). If NULL, indices of 0 will be
 * used.
 * @param count an array of counts (must have same number of entries
 * as variable has dimensions). If NULL, counts matching the size of
 * the variable will be used.
 * @param stride an array of strides (must have same number of
 * entries as variable has dimensions). If NULL, strides of 1 will be
 * used.
 * @param buf pointer that will get the data.
 * @return PIO_NOERR on success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_put_vars(int ncid, int varid, const PIO_Offset *start, const PIO_Offset *count,
                  const PIO_Offset *stride, const void *op)
{
    return PIOc_put_vars_tc(ncid, varid, start, count, stride, NC_NAT, op);
}
