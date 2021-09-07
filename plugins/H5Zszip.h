#ifndef H5ZSZIP_H
#define H5ZSZIP_H

/* Macros for the szip filter */
#define H5Z_SZIP_USER_NPARMS    2       /* Number of parameters that users can set */
#define H5Z_SZIP_TOTAL_NPARMS   4       /* Total number of parameters for filter */
#define H5Z_SZIP_PARM_MASK      0       /* "User" parameter for option mask */
#define H5Z_SZIP_PARM_PPB       1       /* "User" parameter for pixels-per-block */
#define H5Z_SZIP_PARM_BPP       2       /* "Local" parameter for bits-per-pixel */
#define H5Z_SZIP_PARM_PPS       3       /* "Local" parameter for pixels-per-scanline */

#define MIN(x,y) ((x) < (y) ? (x) : (y))

#endif /*H5ZSZIP_H*/
