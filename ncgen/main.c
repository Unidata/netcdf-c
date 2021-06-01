/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
/* $Id: main.c,v 1.33 2010/05/26 21:43:36 dmh Exp $ */
/* $Header: /upc/share/CVS/netcdf-3/ncgen/main.c,v 1.33 2010/05/26 21:43:36 dmh Exp $ */

#include "includes.h"
#include "ncoffsets.h"
#include "ncpathmgr.h"
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

/* Default is netcdf-3 mode 1 */
#define DFALTCMODE 0

/*Mnemonic*/
#define DEPRECATED 1

/* For error messages */
char* progname; /* Global: not reclaimed */
char* cdlname; /* Global: not reclaimed */

/* option flags */
int nofill_flag;
char* mainname; /* name to use for main function; defaults to "main"*/
Language l_flag;
int syntax_only;
int header_only;

/* flags for tracking what output format to use */
int k_flag;    /* > 0  => -k was specified on command line*/
int format_attribute; /* 1=>format came from format attribute */
int enhanced_flag; /* 1 => netcdf-4 */
int cdf5_flag; /* 1 => cdf5 | maybe netcdf-4 */
int specials_flag; /* 1=> special attributes are present */
int usingclassic;
int cmode_modifier;
int diskless;
int ncloglevel;
int wholevarsize;

GlobalSpecialData globalspecials;

size_t zerosvector[NC_MAX_VAR_DIMS];
size_t onesvector[NC_MAX_VAR_DIMS];

char* binary_ext = ".nc";

struct Vlendata* vlendata;

char *netcdf_name = NULL; /* command line -o file name */
char *datasetname = NULL; /* name from the netcdf <name> {} || from -N */

extern FILE *ncgin;

/* Forward */
static char* ubasename(char*);
void usage( void );

int main( int argc, char** argv );

/* Define tables vs modes for legal -k values*/
struct Kvalues legalkinds[] = {
    /* NetCDF-3 classic format (32-bit offsets) */
    {"classic", NC_FORMAT_CLASSIC, !DEPRECATED}, /* canonical format name */
    {"nc3", NC_FORMAT_CLASSIC, !DEPRECATED},	    /* short format name */
    {"1", NC_FORMAT_CLASSIC, !DEPRECATED},	/* deprecated, use "-3" or "-k nc3" instead */

    /* NetCDF-3 64-bit offset format */
    {"64-bit offset", NC_FORMAT_64BIT_OFFSET, !DEPRECATED}, /* canonical format name */
    {"nc6", NC_FORMAT_64BIT_OFFSET, !DEPRECATED},		/* short format name */
    {"2", NC_FORMAT_64BIT_OFFSET, !DEPRECATED},     /* deprecated, use "-6" or "-k nc6" instead */
    {"64-bit-offset", NC_FORMAT_64BIT_OFFSET, !DEPRECATED}, /* aliases */

    /* NetCDF-4 HDF5-based format */
    {"netCDF-4", NC_FORMAT_NETCDF4, !DEPRECATED}, /* canonical format name */
    {"nc4", NC_FORMAT_NETCDF4, !DEPRECATED},	     /* short format name */
    {"3", NC_FORMAT_NETCDF4, !DEPRECATED},   /* deprecated, use "-4" or "-k nc4" instead */
    {"netCDF4", NC_FORMAT_NETCDF4, !DEPRECATED},  /* aliases */
    {"hdf5", NC_FORMAT_NETCDF4, !DEPRECATED},
    {"enhanced", NC_FORMAT_NETCDF4, !DEPRECATED},
    {"netcdf-4", NC_FORMAT_NETCDF4, !DEPRECATED},
    {"netcdf4", NC_FORMAT_NETCDF4, !DEPRECATED},

    /* NetCDF-4 HDF5-based format, restricted to classic data model */
    {"netCDF-4 classic model", NC_FORMAT_NETCDF4_CLASSIC, !DEPRECATED}, /* canonical format name */
    {"nc7", NC_FORMAT_NETCDF4_CLASSIC, !DEPRECATED}, /* short format name */
    {"4", NC_FORMAT_NETCDF4_CLASSIC, !DEPRECATED}, /* deprecated, use "-7" or -k nc7" instead */
    {"netCDF-4-classic", NC_FORMAT_NETCDF4_CLASSIC, !DEPRECATED}, /* aliases */
    {"netCDF-4_classic", NC_FORMAT_NETCDF4_CLASSIC, !DEPRECATED},
    {"netCDF4_classic", NC_FORMAT_NETCDF4_CLASSIC, !DEPRECATED},
    {"hdf5-nc3", NC_FORMAT_NETCDF4_CLASSIC, !DEPRECATED},
    {"enhanced-nc3", NC_FORMAT_NETCDF4_CLASSIC, !DEPRECATED},

    /* CDF-5 format */
    {"5", NC_FORMAT_64BIT_DATA, !DEPRECATED},
    {"64-bit-data", NC_FORMAT_64BIT_DATA, !DEPRECATED},
    {"64-bit data", NC_FORMAT_64BIT_DATA, !DEPRECATED},
    {"nc5", NC_FORMAT_64BIT_DATA, !DEPRECATED},
    {"cdf5", NC_FORMAT_64BIT_DATA, !DEPRECATED},
    {"cdf-5", NC_FORMAT_64BIT_DATA, !DEPRECATED},

    /* null terminate*/
    {NULL,0,0}
};

#ifndef _MSC_VER
struct Languages {
    char* name;
    Language flag;
} legallanguages[] = {
{"b", L_BINARY},
{"c", L_C},
{"C", L_C},
{"f77", L_F77},
{"fortran77", L_F77},
{"Fortran77", L_F77},
{"j", L_JAVA},
{"java", L_JAVA},
{NULL,L_UNDEFINED}
};
#else
typedef struct Languages {
		char* name;
		Language flag;
} Languages;

struct Languages legallanguages[] = {
{"b", L_BINARY},
{"c", L_C},
{"C", L_C},
{"f77", L_F77},
{"fortran77", L_F77},
{"Fortran77", L_F77},
{"j", L_JAVA},
{"java", L_JAVA},
{NULL,L_UNDEFINED}
};
#endif

/* strip off leading path */
/* result is malloc'd */

static char *
ubasename(char *logident)
{
    char* sep;

    sep = strrchr(logident,'/');
#ifdef MSDOS
    if(sep == NULL) sep = strrchr(logident,'\\');
#endif
    if(sep == NULL) return logident;
    sep++; /* skip past the separator */
    return sep;
}

void
usage(void)
{
    derror("Usage: %s"
" [-1]"
" [-3]"
" [-4]"
" [-5]"
" [-6]"
" [-7]"
" [-b]"
" [-d]"
" [-D debuglevel]"
" [-h]"
" [-k kind ]"
" [-l language=b|c|f77|java]"
" [-M <name>]"
" [-n]"
" [-o outfile]"
" [-P]"
" [-x]"
" [-N datasetname]"
" [-L loglevel]"
" [-H]"
" [-W generate whole var upload]"
" [file ... ]",
	   progname);
    derror("netcdf library version %s", nc_inq_libvers());
}

int
main(
	int argc,
	char *argv[])
{
    int code = 0;
    int c;
    FILE *fp;
	struct Languages* langs;

    init_netcdf();

    opterr = 1;			/* print error message if bad option */
    progname = nulldup(ubasename(argv[0]));
    cdlname = NULL;
    netcdf_name = NULL;
    datasetname = NULL;
    l_flag = 0;
    nofill_flag = 0;
    syntax_only = 0;
    header_only = 0;
    mainname = "main";

    k_flag = 0;
    format_attribute = 0;
    enhanced_flag = 0;
    cdf5_flag = 0;
    specials_flag = 0;
    diskless = 0;
    wholevarsize = 1024;
#ifdef LOGGING
    ncloglevel = NC_TURN_OFF_LOGGING;
#else
    ncloglevel = -1;
#endif
    memset(&globalspecials,0,sizeof(GlobalSpecialData));

    while ((c = getopt(argc, argv, "134567bcdD:fhHk:l:M:no:Pv:xL:N:B:W:")) != EOF)
      switch(c) {
	case 'b': /* for binary netcdf output, ".nc" extension */
	  if(l_flag != 0) {
	    fprintf(stderr,"Please specify only one language\n");
	    return 1;
	  }
	  l_flag = L_BINARY;
	  break;
	case 'B':
	  /* Ignore, but keep for back compatibility */
	  break;
	case 'c': /* for c output, old version of "-lc" */
	  if(l_flag != 0) {
	    fprintf(stderr,"Please specify only one language\n");
	    return 1;
	  }
	  l_flag = L_C;
	  fprintf(stderr,"-c is deprecated: please use -lc\n");
	  break;
	case 'd':
	  debug = 1;
	  break;
	case 'D':
	  debug = atoi(optarg);
	  break;
	case 'f': /* for f77 output, old version of "-lf" */
	  if(l_flag != 0) {
	    fprintf(stderr,"Please specify only one language\n");
	    return 1;
	  }
	  l_flag = L_F77;
	  fprintf(stderr,"-f is deprecated: please use -lf77\n");
	  break;
	case 'h':
	  usage();
	  goto done;
	case 'H':
	  header_only = 1;
	  break;
        case 'v': /* a deprecated alias for "kind" option */
	    /*FALLTHRU*/
	case 'k': { /* for specifying variant of netCDF format to be generated
		     Possible values are:
		     Format names:
		       "classic" or "nc3"
		       "64-bit offset" or "nc6"
		       "64-bit data" or "nc5" or "cdf-5"
		       "netCDF-4" or "nc4"
		       "netCDF-4 classic model" or "nc7"
		       "netCDF-5" or "nc5" or "cdf5"
		     Format version numbers (deprecated):
		       1 (=> classic)
		       2 (=> 64-bit offset)
		       3 (=> netCDF-4)
		       4 (=> netCDF-4 classic model)
                       5 (=> classic 64 bit data aka CDF-5)
		   */
	    struct Kvalues* kvalue;
            if(optarg == NULL) {
                derror("-k flag has no value");
		return 2;
            }
            for(kvalue=legalkinds;kvalue->name;kvalue++) {
                if(strcmp(optarg,kvalue->name) == 0) {
                  k_flag = kvalue->k_flag;
		  if(kvalue->deprecated)
		      fprintf(stderr,"-k%s is deprecated; use corresponding -k<name> or -3|-4|-5|-6|-7\n",optarg);
                  break;
                }
            }
            if(kvalue->name == NULL) {
                derror("Invalid format: %s",optarg);
                return 2;
            }
	} break;
        case 'l': /* specify language, instead of using -c or -f or -b */
	{
            char* lang_name = NULL;
	    if(l_flag != 0) {
              fprintf(stderr,"Please specify only one language\n");
              return 1;
	    }
            if(!optarg) {
              derror("%s: output language is null", progname);
              return(1);
            }
            lang_name = (char*) emalloc(strlen(optarg)+1);
            (void)strcpy(lang_name, optarg);

            for(langs=legallanguages;langs->name != NULL;langs++) {
              if(strcmp(lang_name,langs->name)==0) {
                l_flag = langs->flag;
                break;
              }
            }
	    if(langs->name == NULL) {
              derror("%s: output language %s not implemented",progname, lang_name);
              nullfree(lang_name);
              return(1);
	    }
            nullfree(lang_name);
	}; break;
	case 'L':
	    ncloglevel = atoi(optarg);
	    break;
	case 'M': /* Determine the name for the main function */
	    mainname = nulldup(optarg);
	    break;
	case 'n':		/* old version of -b, uses ".cdf" extension */
	  if(l_flag != 0) {
	    fprintf(stderr,"Please specify only one language\n");
	    return 1;
	  }
	  l_flag = L_BINARY;
          binary_ext = ".cdf";
	  break;
	case 'N':		/* to explicitly specify dataset name */
	  if(datasetname) efree(datasetname);
	  datasetname = nulldup(optarg);
	  break;
	case 'o':		/* to explicitly specify output name */
	  if(netcdf_name) efree(netcdf_name);
	  netcdf_name = NC_shellUnescape(optarg);
	  break;
	case 'P': /* diskless with persistence */
	  diskless = 1;
	case 'W':
	  wholevarsize = atoi(optarg);
	  break;
	case 'x': /* set nofill mode to speed up creation of large files */
	  nofill_flag = 1;
	  break;
	case '3':		/* output format is classic (netCDF-3) */
	    k_flag = NC_FORMAT_CLASSIC;
	    break;
	case '6':		/* output format is 64-bit-offset (netCDF-3 version 2) */
	    k_flag = NC_FORMAT_64BIT_OFFSET;
	    break;
	case '4':		/* output format is netCDF-4 (variant of HDF5) */
	    k_flag = NC_FORMAT_NETCDF4;
	    break;
	case '5':		/* output format is CDF5 */
	    k_flag = NC_FORMAT_CDF5;
	    break;
	case '7':		/* output format is netCDF-4 (restricted to classic model)*/
	    k_flag = NC_FORMAT_NETCDF4_CLASSIC;
	    break;
	case '?':
	  usage();
	  return(8);
      }

    if(l_flag == 0) {
	l_flag = L_BINARY; /* default */
	/* Treat -k or -o as an implicit -lb assuming no other -l flags */
        if(k_flag == 0 && netcdf_name == NULL)
	    syntax_only = 1;
    }

#ifndef ENABLE_C
    if(l_flag == L_C) {
	  fprintf(stderr,"C not currently supported\n");
	  code=1; goto done;
    }
#endif
#ifndef ENABLE_BINARY
    if(l_flag == L_BINARY) {
	  fprintf(stderr,"Binary netcdf not currently supported\n");
	  code=1; goto done;
    }
#endif
#ifndef ENABLE_JAVA
    if(l_flag == L_JAVA) {
	  fprintf(stderr,"Java not currently supported\n");
	  code=1; goto done;
    }
#else
    if(l_flag == L_JAVA && mainname != NULL && strcmp(mainname,"main")==0)
      mainname = "Main";
#endif
#ifndef ENABLE_F77
    if(l_flag == L_F77) {
	  fprintf(stderr,"F77 not currently supported\n");
	  code=1; goto done;
    }
#endif

    if(l_flag != L_BINARY)
	diskless = 0;

    argc -= optind;
    argv += optind;

    if (argc > 1) {
	derror ("%s: only one input file argument permitted",progname);
	return(6);
    }

    fp = stdin;
    if (argc > 0 && strcmp(argv[0], "-") != 0) {
	char bom[4];
	size_t count;
	if ((fp = NCfopen(argv[0], "r")) == NULL) {
	    derror ("can't open file %s for reading: ", argv[0]);
	    perror("");
	    return(7);
	}
   	/* Check the leading bytes for an occurrence of a BOM */
        /* re: http://www.unicode.org/faq/utf_bom.html#BOM */
	/* Attempt to read the first four bytes */
	memset(bom,0,sizeof(bom));
	count = fread(bom,1,2,fp);
	if(count == 2) {
	    switch (bom[0]) {
	    case '\x00':
	    case '\xFF':
	    case '\xFE':
	        /* Only UTF-* is allowed; complain and exit */
		fprintf(stderr,"Input file contains a BOM indicating a non-UTF8 encoding\n");
		return 1;
	    case '\xEF':
		/* skip the BOM */
	        (void)fread(bom,1,1,fp);
	        break;
	    default: /* legal printable char, presumably; rewind */
	        rewind(fp);
		break;
	    }
	}
    }

    cdlname = NC_shellUnescape(argv[0]);
    if(cdlname != NULL) {
	if(strlen(cdlname) > NC_MAX_NAME)
	  cdlname[NC_MAX_NAME] = '\0';
    }

    parse_init();
    ncgin = fp;
    if(debug >= 2) {ncgdebug=1;}
    if(ncgparse() != 0)
        return 1;

    /* Compute the k_flag (1st pass) using rules in the man page (ncgen.1).*/

#ifndef ENABLE_CDF5
    if(k_flag == NC_FORMAT_CDF5) {
      derror("Output format CDF5 requested, but netcdf was built without cdf5 support.");
      return 0;
    }
#endif

#ifndef USE_NETCDF4
    if(enhanced_flag) {
	derror("CDL input is enhanced mode, but --disable-netcdf4 was specified during build");
	return 0;
    }
#endif

    if(l_flag == L_JAVA || l_flag == L_F77) {
        k_flag = NC_FORMAT_CLASSIC;
	if(enhanced_flag) {
	    derror("Java or Fortran requires classic model CDL input");
	    return 0;
	}
    }

    if(k_flag == 0)
      k_flag = globalspecials._Format;

    if(cdf5_flag && !enhanced_flag && k_flag == 0)
      k_flag = NC_FORMAT_64BIT_DATA;
    if(enhanced_flag && k_flag == 0)
      k_flag = NC_FORMAT_NETCDF4;

    if(enhanced_flag && k_flag != NC_FORMAT_NETCDF4) {
      if(enhanced_flag && k_flag != NC_FORMAT_NETCDF4 && k_flag != NC_FORMAT_64BIT_DATA) {
        derror("-k or _Format conflicts with enhanced CDL input");
        return 0;
      }
    }

    if(specials_flag > 0 && k_flag == 0)
#ifdef USE_NETCDF4
	k_flag = NC_FORMAT_NETCDF4;
#else
	k_flag = NC_FORMAT_CLASSIC;
#endif

    if(k_flag == 0)
	k_flag = NC_FORMAT_CLASSIC;

    /* Figure out usingclassic */
    switch (k_flag) {
    case NC_FORMAT_64BIT_DATA:
    case NC_FORMAT_CLASSIC:
    case NC_FORMAT_64BIT_OFFSET:
    case NC_FORMAT_NETCDF4_CLASSIC:
	usingclassic = 1;
	break;
    case NC_FORMAT_NETCDF4:
    default:
	usingclassic = 0;
	break;
    }

    /* compute cmode_modifier */
    switch (k_flag) {
    case NC_FORMAT_CLASSIC:
	cmode_modifier = 0; break;
    case NC_FORMAT_64BIT_OFFSET:
	cmode_modifier = NC_64BIT_OFFSET; break;
    case NC_FORMAT_NETCDF4:
	cmode_modifier = NC_NETCDF4; break;
    case NC_FORMAT_NETCDF4_CLASSIC:
	cmode_modifier = NC_NETCDF4 | NC_CLASSIC_MODEL; break;
    case NC_FORMAT_64BIT_DATA:
	cmode_modifier = NC_CDF5; break;
    default: ASSERT(0); /* cannot happen */
    }

    if(diskless)
	cmode_modifier |= (NC_DISKLESS|NC_NOCLOBBER);

    processsemantics();
    if(!syntax_only && error_count == 0)
        define_netcdf();

done:
    nullfree(netcdf_name);
    nullfree(datasetname);
    finalize_netcdf(code);
    return code;
}

void
init_netcdf(void) /* initialize global counts, flags */
{
    int i;
    memset((void*)&nullconstant,0,sizeof(NCConstant));
    fillconstant = nullconstant;
    fillconstant.nctype = NC_FILLVALUE;

    filldatalist = builddatalist(1);
    dlappend(filldatalist,&fillconstant);
    filldatalist->readonly = 1;

    codebuffer = bbNew();
    stmt = bbNew();
    error_count = 0; /* Track # of errors */

    for(i=0;i<NC_MAX_VAR_DIMS;i++) onesvector[i] = 1;
    memset(zerosvector,0,sizeof(zerosvector));

}

void
finalize_netcdf(int retcode)
{
    nc_finalize();
    exit(retcode);
}
