/*********************************************************************
  *   Copyright 1993, UCAR/Unidata
  *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
  *********************************************************************/

#ifndef NCDAP_H
#define NCDAP_H 1

#ifndef nullfree
#define nullfree(m) {if((m)!=NULL) {free(m);} else {}}
#endif

/**************************************************/
/*
Collect single bit flags that
affect the operation of the system.
*/

typedef unsigned int NCFLAGS;
#  define SETFLAG(controls,flag) (((controls).flags) |= (flag))
#  define CLRFLAG(controls,flag) (((controls).flags) &= ~(flag))
#  define FLAGSET(controls,flag) (((controls.flags) & (flag)) != 0)

/* Defined flags */
#define NCF_NC3             (0x0001) /* DAP->netcdf-3 */
#define NCF_NC4             (0x0002) /* DAP->netcdf-4 */
#define NCF_NCDAP           (0x0004) /* Do libnc-dap mimic */
#define NCF_CACHE           (0x0008) /* Cache enabled/disabled */
#define NCF_UPGRADE         (0x0010) /* Do proper type upgrades */
#define NCF_UNCONSTRAINABLE (0x0020) /* Not a constrainable URL */
#define NCF_SHOWFETCH       (0x0040) /* show fetch calls */
#define NCF_ONDISK          (0x0080) /* cause oc to store data on disk */
#define NCF_WHOLEVAR        (0x0100) /* retrieve only whole variables (as opposed to partial variable) into cache */
#define NCF_PREFETCH        (0x0200) /* Cache prefetch enabled/disabled */
#define NCF_PREFETCH_EAGER  (0x0400) /* Do eager prefetch; 0=>lazy */
#define NCF_PREFETCH_ALL    (0x0800) /* Prefetch all variables */
/* Allow _FillValue/Variable type mismatch */
#define NCF_FILLMISMATCH    (0x1000)
/*COLUMBIA_HACK*/
#define NCF_COLUMBIA        (0x80000000) /* Hack for columbia server */

/* Define all the default on flags */
#define DFALT_ON_FLAGS (NCF_CACHE|NCF_PREFETCH)

/* Define a structure to hold all legal .daprc fields */
/* For char*, NULL means it was not defined, for int and long,
   -1 means it was not defined. KeepAlive.on == -1 indicates not defined.
*/
typedef struct NCRCFIELDS {
    int   HTTP_VERBOSE;
    int   HTTP_DEFLATE;
    char* HTTP_COOKIEJAR;
    char* HTTP_CREDENTIALS_USER;
    char* HTTP_CREDENTIALS_PASSWORD;
    char* HTTP_SSL_CERTIFICATE;
    char* HTTP_SSL_KEY;
    char* HTTP_SSL_KEYPASSWORD;
    char* HTTP_SSL_CAPATH;
    int   HTTP_SSL_VALIDATE;
    long  HTTP_TIMEOUT;
    char* HTTP_PROXY_SERVER;
    long  HTTP_READ_BUFFERSIZE;
    struct KeepAlive {int active; int wait; int repeat;}
           HTTP_KEEPALIVE;
} NCRCFIELDS;

typedef struct NCCONTROLS {
    NCFLAGS  flags;
    NCRCFIELDS rcfields;
} NCCONTROLS;

/* Misc. Constants */

#ifndef DFALTPACKETSIZE
#define DFALTPACKETSIZE 0x20000 /*approximately 100k bytes*/
#endif

#ifndef DFALTUSERAGENT
#define DFALTUSERAGENT "netCDF"
#endif

#endif /*NCDAP_H*/

