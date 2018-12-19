/**
 * @file
 *
 * Infer as much as possible from the omode + path.
 * Possibly rewrite the path.
 *
 * Copyright 2018 University Corporation for Atmospheric
 * Research/Unidata. See COPYRIGHT file for more info.
*/

#include "config.h"
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "ncdispatch.h"
#include "ncwinpath.h"
#include "ncinfermodel.h"
#include "netcdf_mem.h"
#include "fbits.h"

/* Define a mask of all possible format flags */
#define ANYFORMAT (NC_64BIT_OFFSET|NC_64BIT_DATA|NC_CLASSIC_MODEL|NC_NETCDF4)

/** @internal Magic number for HDF5 files. To be consistent with
 * H5Fis_hdf5, use the complete HDF5 magic number */
static char HDF5_SIGNATURE[MAGIC_NUMBER_LEN] = "\211HDF\r\n\032\n";

/* User-defined formats. */
NC_Dispatch *UDF0_dispatch_table = NULL;
char UDF0_magic_number[NC_MAX_MAGIC_NUMBER_LEN + 1] = "";
NC_Dispatch *UDF1_dispatch_table = NULL;
char UDF1_magic_number[NC_MAX_MAGIC_NUMBER_LEN + 1] = "";

/*
Define a table of legal "mode=" string values.
Note that only cases that can currently
take URLs are included.
l*/
static struct LEGALMODES {
    const char* tag;
    const int format; /* NC_FORMAT_XXX value */
    const int impl; /* NC_FORMATX_XXX value */
    const int iosp; /* NC_IOSP_XXX value */
    const int version;
} legalmodes[] = {
{"netcdf-3",NC_FORMAT_CLASSIC,NC_FORMATX_NC3,0,1},
{"classic",NC_FORMAT_CLASSIC,NC_FORMATX_NC3,0,1},
{"netcdf-4",NC_FORMAT_NETCDF4,NC_FORMATX_NC4,0,5},
{"enhanced",NC_FORMAT_NETCDF4,NC_FORMATX_NC4,0,5},
{"dap2",NC_FORMAT_CLASSIC,NC_FORMATX_DAP2,NC_IOSP_DAP2,1},
{"dap4",NC_FORMAT_NETCDF4,NC_FORMATX_DAP4,NC_IOSP_DAP4,1},
/* IO Handler tags */
{"s3",0,0,NC_IOSP_S3,1},
{"zarr",0,0,NC_IOSP_ZARR,1},
/* Version tags */
{"64bitoffset",0,0,0,NC_64BIT_OFFSET},
{"64bitdata",0,0,0,NC_64BIT_DATA},
{"cdf5",0,0,0,NC_64BIT_DATA},
{"hdf4",0,0,0,NC_NETCDF4},
{NULL,0,0,0,0},
};

/* Define the known URL protocols and their interpretation */
static struct NCPROTOCOLLIST {
    const char* protocol;
    const char* substitute;
    const char* mode;
} ncprotolist[] = {
    {"http",NULL,NULL},
    {"https",NULL,NULL},
    {"file",NULL,NULL},
    {"dods","http","dap2"},
    {"dap4","http","dap4"},
    {NULL,NULL,NULL} /* Terminate search */
};

/* If Defined, then use only stdio for all magic number io;
   otherwise use stdio or mpio as required.
 */
#undef DEBUG

/* Forward */
static int check_file_type(const char *path, int flags, int use_parallel, void *parameters, NCmodel* model, NCURI* uri);

static int openmagic(struct MagicFile* file);
static int readmagic(struct MagicFile* file, long pos, char* magic);
static int closemagic(struct MagicFile* file);
static int NC_interpret_magic_number(char* magic, NCmodel* model);
#ifdef DEBUG
static void printmagic(const char* tag, char* magic,struct MagicFile*);
#endif

/* Parse a mode string at the commas and nul terminate each tag */
static int
parseurlmode(const char* modestr0, char** listp)
{
    int stat = NC_NOERR;
    char* modestr = NULL;
    char* p = NULL;
    char* endp = NULL;

    /* Make modifiable copy */
    if((modestr=strdup(modestr0)) == NULL)
	{stat=NC_ENOMEM; goto done;}

    /* Split modestr at the commas or EOL */
    p = modestr;
    for(;;) {
        endp = strchr(p,',');
        if(endp == NULL) break;
	/* Null terminate each comma-separated string */
	*endp++ = '\0';
	p = endp;
    } 
    if(listp) *listp = modestr;
    modestr = NULL;

done:
    if(stat) {nullfree(modestr);}
    return stat;
}

/* Given a mode= argument, fill in the matching part of the model */
static int
processmodearg(const char* arg, NCmodel* model)
{
    int stat = NC_NOERR;
    struct LEGALMODES* legal = legalmodes;
    for(;legal->tag;legal++) {
	if(strcmp(legal->tag,arg)==0) {
	    if(model->format != 0 && legal->format != 0)
		{stat = NC_EINVAL; goto done;}
	    if(model->impl != 0 && legal->impl != 0)
		{stat = NC_EINVAL; goto done;}
	    if(model->iosp != 0 && legal->iosp != 0)
		{stat = NC_EINVAL; goto done;}
	    if(model->version != 0 && legal->version != 0)
		{stat = NC_EINVAL; goto done;}
	    if(legal->format != 0) model->format = legal->format;
	    if(legal->impl != 0) model->impl = legal->impl;
	    if(legal->iosp != 0) model->iosp = legal->iosp;
	    if(legal->version != 0) model->version = legal->version;
	}
    }
done:
    return stat;
}

/* Parse url fragment mode tag for model info */
static int
url_getmode(const char* modestr, NCmodel* model)
{
    int stat = NC_NOERR;
    char* args = NULL;
    char* p = NULL;

    if((stat=parseurlmode(modestr,&args))) goto done;
    p = args;
    for(;*p;p += (strlen(p)+1)) {
	if((stat = processmodearg(p,model))) goto done;
    }
done:
    nullfree(args);
    return stat;    
}

/*
Fill in the model fields to degree possible
using path+cmode. May rewrite path.
*/
int
NC_pathinfer(const char* path, int cmode, NCmodel* model, char** newpathp, NCURI** urip)
{
    int stat = NC_NOERR;
    int found = 0;
    struct NCPROTOCOLLIST* protolist;
    const char** fragp = NULL;
    int isurl = 0;
    NCURI* uri = NULL;

    if(path == NULL) return 0;

    /* Defaults */
    if(newpathp) *newpathp = NULL;
    if(urip) *urip = NULL;

    /* Parse the url */
    if(ncuriparse(path,&uri) != NCU_OK) {
	/* Not parseable as url; assume file path; need to process cmode */
	goto docmode;
    }

    /* From here to label docmode, we assume we are dealing with a URL */
    isurl = 1;

    /* Look up the protocol */
    for(found=0,protolist=ncprotolist;protolist->protocol;protolist++) {
        if(strcmp(uri->protocol,protolist->protocol) == 0) {
	    found = 1;
	    break;
	}
    }
    if(!found)
	{stat = NC_EINVAL; goto done;} /* unrecognized URL form */

    /* process the corresponding mode arg */
    if((stat=processmodearg(protolist->mode,model))) goto done;
    /* Substitute the protocol in any case */
    if(protolist->substitute) ncurisetprotocol(uri,protolist->substitute);

    /* Iterate over the url fragment parameters */
    for(fragp=ncurifragmentparams(uri);fragp && *fragp;fragp+=2) {
	const char* name = fragp[0];
	const char* value = fragp[1];
	if(strcmp(name,"protocol")==0)
	    name = value;
	if(strcasecmp(name,"dap2") == 0) {
	    model->format = NC_FORMAT_NC3;	    
	    model->impl = NC_FORMATX_DAP2;	    
	    /* No need to set iosp field */
	} else if(strcasecmp(name,"dap4") == 0) {
	    model->format = NC_FORMAT_NETCDF4;
	    model->impl = NC_FORMATX_DAP4;
	    /* No need to set iosp field */
	} else if(strcmp(name,"mode")==0) {
	    if((stat = url_getmode(value,model))) goto done;
	}
    }

docmode:
    /* Now process the cmode, but do not override already chosen flags */
    {
	if(fIsSet(cmode,NC_64BIT_OFFSET)) {
	   if(model->format == 0)
		model->format = NC_FORMAT_NC3;
	   if(model->format == NC_FORMAT_NC3 && model->version == 0)
		model->version = 2;
	   else {stat = NC_EINVAL; goto done;}
	}
	if(fIsSet(cmode,NC_64BIT_DATA)) {
	   if(model->format == 0)
		model->format = NC_FORMAT_64BIT_DATA;
	   if(model->format == NC_FORMAT_NC3 && model->version == 0)
		model->version = 5;
	   else {stat = NC_EINVAL; goto done;}
	}
	if(fIsSet(cmode,NC_NETCDF4)) {
	   if(model->format == 0)
		model->format = NC_FORMAT_NETCDF4;
	   if(model->format == NC_FORMAT_NETCDF4 && model->version == 0)
		model->version = 5;
	   else {stat = NC_EINVAL; goto done;}
	}
	if(fIsSet(cmode,NC_CLASSIC_MODEL)) {
	   if(model->format == 0)
		model->format = NC_FORMAT_NETCDF4;
	   if(model->format == NC_FORMAT_NETCDF4 && model->version == 0)
		model->version = 2;
	   else {stat = NC_EINVAL; goto done;}
	}
    }
    /* Final case, if no format, then assume netcdf classic */
    if(model->format == 0)
	model->format = NC_FORMAT_NC3;
    
    /* Infer missing fields where possible */
    if(isurl) {
	if(model->format == NC_FORMAT_NETCDF4 && model->iosp == 0 && model->impl == 0) {
	    model->iosp = NC_IOSP_DAP4;
	    model->impl = NC_FORMATX_DAP4;
	} else if(model->format == NC_FORMAT_NC3 && model->iosp == 0 && model->impl == 0) {
	    model->iosp = NC_IOSP_DAP2;
	    model->impl = NC_FORMATX_DAP2;
	} else if(model->iosp == 0 && model->impl == 0) {
	    model->iosp = NC_IOSP_DAP2;
	    model->impl = NC_FORMATX_DAP2;
	}
    } else if(!isurl && model->format != 0) {
	switch (model->format) {
	case NC_FORMAT_CLASSIC:
	    model->impl = NC_FORMATX_NC3;
	    model->version = 1;
	    break;
	case NC_FORMAT_64BIT_OFFSET:
	    model->impl = NC_FORMATX_NC3;
	    model->version = 2;
	    break;
	case NC_FORMAT_64BIT_DATA:
	    model->impl = NC_FORMATX_NC3;
	    model->version = 5;
	    break;
	case NC_FORMAT_NETCDF4:
	case NC_FORMAT_NETCDF4_CLASSIC:
	    model->impl = NC_FORMATX_NC4;
	    model->version = 5;
	    break;
	default: break;
	}
    }

    if(model->impl == 0) {stat = NC_ENOTNC; goto done;} /* could not interpret */

    /* Try to infer format from implementation */
    if(model->format == 0) {
        switch (model->impl) {
        case NC_FORMATX_NC3:
            model->format = NC_FORMAT_NC3;
            break;  
        case NC_FORMATX_NC4:
            model->format = NC_FORMAT_NETCDF4;
            break;  
        case NC_FORMATX_DAP2:
            model->format = NC_FORMAT_NC3;
            break;  
        case NC_FORMATX_DAP4:
            model->format = NC_FORMAT_NETCDF4;
            break;  
        case NC_FORMATX_ZARR:
            model->format = NC_FORMAT_NETCDF4;
            break;  
        default:
	    break;
        }
    }
done:
    if(stat == NC_NOERR && isurl) {
        if(newpathp)
	    *newpathp = ncuribuild(uri,NULL,NULL,NCURIALL);
        if(urip) *urip = uri;
    }
    return stat;
}

/**************************************************/
/*
   Infer model for this dataset using some
   combination of cmode, path, and reading the dataset.

@param path
@param omode
@param iscreate
@param useparallel
@param params
@param model
@param newpathp

*/

int
NC_infermodel(const char* path, int omode, int iscreate, int useparallel, void* params, NCmodel* model, char** newpathp)
{
    int stat = NC_NOERR;
    char* newpath = NULL;
    NCURI* uri = NULL;

    /* First get whatever we can from path+cmode */
    stat = NC_pathinfer(path,omode,model,&newpath,&uri);
    if(stat && stat != NC_ENOTNC) goto done; /* true error */
    if(newpath) path = newpath;

    if(model->impl == 0) {
        if(iscreate)
	    {stat = NC_EINVAL; goto done;} /* cannot infer how to process dataset */
        /* At this point, we may have an impl but do not have a format,
           so we need to try to read the file */
        if(!iscreate && model->impl == 0) {
	    /* Ok, we need to try to read the file */
	    if((stat = check_file_type(path, omode, useparallel, params, model, uri))) goto done;
	}
    }
	
done:
    if(uri) ncurifree(uri);
    if(stat == NC_NOERR && newpathp) {*newpathp = newpath; newpath = NULL;}
    nullfree(newpath);
    return stat;
}

/**************************************************/
/* return 1 if path looks like a url; 0 otherwise */
int
NC_testurl(const char* path)
{
    int isurl = 0;
    NCURI* tmpurl = NULL;

    if(path == NULL) return 0;

    /* Ok, try to parse as a url */
    if(ncuriparse(path,&tmpurl)==NCU_OK) {
	/* Do some extra testing to make sure this really is a url */
        /* Look for a known/accepted protocol */
        struct NCPROTOCOLLIST* protolist;
        for(protolist=ncprotolist;protolist->protocol;protolist++) {
	    if(strcmp(tmpurl->protocol,protolist->protocol) == 0) {
	        isurl=1;
		break;
	    }
	}
	ncurifree(tmpurl);
	return isurl;
    }
    return 0;
}

/**************************************************/
/**
 * Provide a hidden interface to allow utilities
 * to check if a given path name is really a url.
 * If not, put null in basenamep, else put basename of the url
 * minus any extension into basenamep; caller frees.
 * Return 1 if it looks like a url, 0 otherwise.
 */

int
nc__testurl(const char* path, char** basenamep)
{
    NCURI* uri;
    int ok = 0;
    if(ncuriparse(path,&uri) == NCU_OK) {
	char* slash = (uri->path == NULL ? NULL : strrchr(uri->path, '/'));
	char* dot;
	if(slash == NULL) slash = (char*)path; else slash++;
        slash = nulldup(slash);
        if(slash == NULL)
            dot = NULL;
        else
            dot = strrchr(slash, '.');
        if(dot != NULL &&  dot != slash) *dot = '\0';
	if(basenamep)
            *basenamep=slash;
        else if(slash)
            free(slash);
        ncurifree(uri);
	ok = 1;
    }
    return ok;
}

/**************************************************/
/**
 * @internal Given an existing file, figure out its format and return
 * that format value (NC_FORMATX_XXX) in model arg. Assume any path
 * conversion was already performed at a higher level.
 *
 * @param path File name.
 * @param flags
 * @param use_parallel
 * @param parameters
 * @param model Pointer that gets the model to use for the dispatch table.
 * @param version Pointer that gets version of the file.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner
*/
static int
check_file_type(const char *path, int flags, int use_parallel,
		   void *parameters, NCmodel* model, NCURI* uri)
{
    char magic[NC_MAX_MAGIC_NUMBER_LEN];
    int status = NC_NOERR;
    int diskless = ((flags & NC_DISKLESS) == NC_DISKLESS);
    int inmemory = ((flags & NC_INMEMORY) == NC_INMEMORY);
    struct MagicFile magicinfo;

    memset((void*)&magicinfo,0,sizeof(magicinfo));
    magicinfo.path = path; /* do not free */
    magicinfo.uri = uri; /* do not free */
    magicinfo.parameters = parameters; /* do not free */
    magicinfo.inmemory = inmemory;
    magicinfo.diskless = diskless;
    magicinfo.use_parallel = use_parallel;

    if((status = openmagic(&magicinfo))) goto done;

    /* Verify we have a large enough file */
    if(magicinfo.filelen < MAGIC_NUMBER_LEN)
	{status = NC_ENOTNC; goto done;}
    if((status = readmagic(&magicinfo,0L,magic)) != NC_NOERR) {
	status = NC_ENOTNC;
	goto done;
    }

    /* Look at the magic number */
    if(NC_interpret_magic_number(magic,model) == NC_NOERR
	&& model->format != 0) {
        if (model->format == NC_FORMAT_NC3 && use_parallel)
            /* this is called from nc_open_par() and file is classic */
            model->impl = NC_FORMATX_PNETCDF;
        goto done; /* found something */
    }

    /* Remaining case when implementation is an HDF5 file;
       search forward at starting at 512
       and doubling to see if we have HDF5 magic number */
    {
	long pos = 512L;
        for(;;) {
	    if((pos+MAGIC_NUMBER_LEN) > magicinfo.filelen)
		{status = NC_ENOTNC; goto done;}
            if((status = readmagic(&magicinfo,pos,magic)) != NC_NOERR)
	        {status = NC_ENOTNC; goto done; }
            NC_interpret_magic_number(magic,model);
            if(model->impl == NC_FORMATX_NC4) break;
	    /* double and try again */
	    pos = 2*pos;
        }
    }
done:
    closemagic(&magicinfo);
    return status;
}

/**
\internal
\ingroup datasets
Provide open, read and close for use when searching for magic numbers
*/
static int
openmagic(struct MagicFile* file)
{
    int status = NC_NOERR;
    assert((file->inmemory) ? file->parameters != NULL : 1);
    if(file->inmemory) {
	/* Get its length */
	NC_memio* meminfo = (NC_memio*)file->parameters;
	file->filelen = (long long)meminfo->size;
	goto done;
    }
#ifdef USE_PARALLEL
    if (file->use_parallel) {
	int retval;
	MPI_Offset size;
        assert(file->parameters);
	if((retval = MPI_File_open(((NC_MPI_INFO*)file->parameters)->comm,
                                   (char*)file->path,MPI_MODE_RDONLY,
                                   ((NC_MPI_INFO*)file->parameters)->info,
                                   &file->fh)) != MPI_SUCCESS) {
#ifdef MPI_ERR_NO_SUCH_FILE
            int errorclass;
            MPI_Error_class(retval, &errorclass);
            if (errorclass == MPI_ERR_NO_SUCH_FILE)
#ifdef NC_ENOENT
                status = NC_ENOENT;
#else
                status = errno;
#endif
            else
#endif
            status = NC_EPARINIT;
            goto done;
        }
	/* Get its length */
	if((retval=MPI_File_get_size(file->fh, &size)) != MPI_SUCCESS)
	    {status = NC_EPARINIT; goto done;}
	file->filelen = (long long)size;
	goto done;
    }
#endif /* USE_PARALLEL */
    {
        if(file->path == NULL || strlen(file->path)==0)
	    {status = NC_EINVAL; goto done;}
#ifdef _MSC_VER
        file->fp = fopen(file->path, "rb");
#else
        file->fp = fopen(file->path, "r");
#endif
	if(file->fp == NULL)
	    {status = errno; goto done;}
	/* Get its length */
	{
	int fd = fileno(file->fp);
#ifdef _MSC_VER
	__int64 len64 = _filelengthi64(fd);
	if(len64 < 0)
            {status = errno; goto done;}
	file->filelen = (long long)len64;
#else
	off_t size;
	size = lseek(fd, 0, SEEK_END);
	if(size == -1)
	    {status = errno; goto done;}
	file->filelen = (long long)size;
#endif
	rewind(file->fp);
	}
	goto done;
    }

done:
    return status;
}

static int
readmagic(struct MagicFile* file, long pos, char* magic)
{
    int status = NC_NOERR;
    memset(magic,0,MAGIC_NUMBER_LEN);
    if(file->inmemory) {
	char* mempos;
	NC_memio* meminfo = (NC_memio*)file->parameters;
	if((pos + MAGIC_NUMBER_LEN) > meminfo->size)
	    {status = NC_EINMEMORY; goto done;}
	mempos = ((char*)meminfo->memory) + pos;
	memcpy((void*)magic,mempos,MAGIC_NUMBER_LEN);
#ifdef DEBUG
	printmagic("XXX: readmagic",magic,file);
#endif
	goto done;
    }
#ifdef ENABLE_S3
    if(file->protocol == NCUPROTO_S3) {
	NCbytes* buf = ncbytesnew();
	size_t start = (size_t)pos;
	size_t count = MAGIC_NUMBER_LEN;
	status = nc_s3_read(file->curl,start,count,buf);
	if(ncbytesength(buf) != count)
	    status = NC_EINVAL;
	else
	    memcpy(magic,ncbytescontents(buf),count);
	ncbytesfree(buf);
	goto done;
    }
#endif
#ifdef USE_PARALLEL
    if (file->use_parallel) {
	MPI_Status mstatus;
	int retval;
	if((retval = MPI_File_read_at_all(file->fh, pos, magic,
                     MAGIC_NUMBER_LEN, MPI_CHAR, &mstatus)) != MPI_SUCCESS)
	    {status = NC_EPARINIT; goto done;}
	goto done;
    }
#endif /* USE_PARALLEL */
    {
	int count;
	int i = fseek(file->fp,pos,SEEK_SET);
	if(i < 0)
	    {status = errno; goto done;}
	for(i=0;i<MAGIC_NUMBER_LEN;) {/* make sure to read proper # of bytes */
	    count=fread(&magic[i],1,(size_t)(MAGIC_NUMBER_LEN-i),file->fp);
	    if(count == 0 || ferror(file->fp))
		{status = errno; goto done;}
	    i += count;
	}
	goto done;
    }
done:
    if(file && file->fp) clearerr(file->fp);
    return status;
}

/**
 * Close the file opened to check for magic number.
 *
 * @param file pointer to the MagicFile struct for this open file.
 * @returns NC_NOERR for success
 * @returns NC_EPARINIT if there was a problem closing file with MPI
 * (parallel builds only).
 * @author Dennis Heimbigner
 */
static int
closemagic(struct MagicFile* file)
{
    int status = NC_NOERR;
    if(file->inmemory) goto done; /* noop*/
#ifdef ENABLE_S3
    if(file->protocol == NCUPROTO_S3) {
	status = nc_s3_close(file->curl);
	goto done;
    }
#endif

#ifdef USE_PARALLEL
    if (file->use_parallel) {
	int retval;
	if((retval = MPI_File_close(&file->fh)) != MPI_SUCCESS)
		{status = NC_EPARINIT; goto done;}
	goto done;
    }
#endif
    {
	if(file->fp) fclose(file->fp);
	goto done;
    }
done:
    return status;
}

/*!
  Interpret the magic number found in the header of a netCDF file.
  This function interprets the magic number/string contained in the header of a netCDF file and sets the appropriate NC_FORMATX flags.

  @param[in] magic Pointer to a character array with the magic number block.
  @param[out] model Pointer to an integer to hold the corresponding netCDF type.
  @param[out] version Pointer to an integer to hold the corresponding netCDF version.
  @returns NC_NOERR if a legitimate file type found
  @returns NC_ENOTNC otherwise

\internal
\ingroup datasets

*/
static int
NC_interpret_magic_number(char* magic, NCmodel* model)
{
    int status = NC_NOERR;
    /* Look at the magic number */
#ifdef USE_NETCDF4
    if (strlen(UDF0_magic_number) && !strncmp(UDF0_magic_number, magic,
                                              strlen(UDF0_magic_number)))
    {
	model->impl = NC_FORMATX_UDF0;
	model->version = 6; /* redundant */
	goto done;
    }
    if (strlen(UDF1_magic_number) && !strncmp(UDF1_magic_number, magic,
                                              strlen(UDF1_magic_number)))
    {
	model->impl = NC_FORMATX_UDF1;
	model->version = 7; /* redundant */
	goto done;
    }
#endif /* USE_NETCDF4 */

    /* Use the complete magic number string for HDF5 */
    if(memcmp(magic,HDF5_SIGNATURE,sizeof(HDF5_SIGNATURE))==0) {
	model->impl = NC_FORMATX_NC4;
	model->version = 5; /* redundant */
	goto done;
    }
    if(magic[0] == '\016' && magic[1] == '\003'
              && magic[2] == '\023' && magic[3] == '\001') {
	model->impl = NC_FORMATX_NC_HDF4;
	model->version = 4; /* redundant */
	goto done;
    }
    if(magic[0] == 'C' && magic[1] == 'D' && magic[2] == 'F') {
        if(magic[3] == '\001') {
	    model->impl = NC_FORMATX_NC3;
            model->version = 1; /* netcdf classic version 1 */
	    goto done;
	}
        if(magic[3] == '\002') {
	    model->impl = NC_FORMATX_NC3;
            model->version = 2; /* netcdf classic version 2 */
	    goto done;
        }
        if(magic[3] == '\005') {
	  model->impl = NC_FORMATX_NC3;
          model->version = 5; /* cdf5 file */
	  goto done;
	}
     }
     /* No match  */
     status = NC_ENOTNC;
     goto done;

done:
     return status;
}

#ifdef DEBUG
static void
printmagic(const char* tag, char* magic, struct MagicFile* f)
{
    int i;
    fprintf(stderr,"%s: inmem=%d ispar=%d magic=",tag,f->inmemory,f->use_parallel);
    for(i=0;i<MAGIC_NUMBER_LEN;i++) {
        unsigned int c = (unsigned int)magic[i];
	c = c & 0x000000FF;
	if(c == '\n')
	    fprintf(stderr," 0x%0x/'\\n'",c);
	else if(c == '\r')
	    fprintf(stderr," 0x%0x/'\\r'",c);
	else if(c < ' ')
	    fprintf(stderr," 0x%0x/'?'",c);
	else
	    fprintf(stderr," 0x%0x/'%c'",c,c);
    }
    fprintf(stderr,"\n");
    fflush(stderr);
}
#endif

