/*
https://www.cygwin.com/cygwin-ug-net/using-specialnames.html
https://docs.microsoft.com/en-us/archive/msdn-magazine/2016/september/c-unicode-encoding-conversions-with-stl-strings-and-win32-apis
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "netcdf.h"
#include "ncpathmgr.h"

static const unsigned char name1[] = {
'x','u','t','f','8','_',
'\xe6', '\xb5', '\xb7',
'\0'
};

static unsigned char name2[] = {
'x','u','t','f','8','_',
	   0xCE, 0x9A,	  /* GREEK CAPITAL LETTER KAPPA  : 2-bytes utf8 */
	   0xCE, 0xB1,	  /* GREEK SMALL LETTER LAMBDA   : 2-bytes utf8 */
	   0xCE, 0xBB,	  /* GREEK SMALL LETTER ALPHA    : 2-bytes utf8 */
	   0xCE, 0xB7,	  /* GREEK SMALL LETTER ETA      : 2-bytes utf8 */
	   0xCE, 0xBC,	  /* GREEK SMALL LETTER MU       : 2-bytes utf8 */
	   0xCE, 0xAD,    /* GREEK SMALL LETTER EPSILON WITH TONOS
			                                 : 2-bytes utf8 */
	   0xCF, 0x81,	  /* GREEK SMALL LETTER RHO      : 2-bytes utf8 */
	   0xCE, 0xB1,	  /* GREEK SMALL LETTER ALPHA    : 2-bytes utf8 */
	   0x00
       };

static char* name3 = "xutf8_사람/접는사람";

/* This is CP_1252 */ 
//static char* name4 = "xutf8_Å";
static char name4[8] = {'x','u','t','f','8','_',0XC5,0x00} ;

int
main()
{
    FILE* f;
    f = NCfopen((char*)name1,"w");
    if (f) fclose(f);
    f = NCfopen((char*)name2,"w");
    if (f) fclose(f);
    f = NCfopen((char*)name3,"w");
    if (f) fclose(f);
    printf("|name4|=%u\n",(unsigned)strlen(name4));
    f = NCfopen((char*)name4,"w");
    if (f) fclose(f);
    return 0;
}
