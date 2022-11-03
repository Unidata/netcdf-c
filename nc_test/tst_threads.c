#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

#define DLL_NETCDF
#include "netcdf.h"
#include "netcdf_threadsafe.h"

#define MODE3 3
#define MODE4 4

#define DFALTCYCLES 1
#define DFALTTHREADS 0

typedef struct ThreadData {
    int id;
    const char* format;
    int mode;
    NC_barrier_t* barrier;
} ThreadData;

#ifdef _WIN32
#define RETURNTYPE DWORD
#else
#define RETURNTYPE void*
#define WINAPI
#endif

typedef struct Options {
    int mode;
    int ncflags;
    int nthreads;
    int ncycles;
    const char* format;
} Options;

static RETURNTYPE threadprog(void* data);

/* forward */

static NC_barrier_t* barrier = NULL;

static void
usage(void)
{
   fprintf(stderr,"usage: tst_threads 3|4 \"filenameformatstring\"");
   exit(1);
}

static Options options;

/* Ensure this stays around */
static ThreadData* data = NULL;

int
main(int argc, char **argv)
{
    int i, c, stat = NC_NOERR;
    NC_threadset_t* threads = NULL;
    
    /* Initialize options */
    memset(&options,0,sizeof(Options));
    options.ncycles = DFALTCYCLES;
    options.nthreads = DFALTTHREADS;

    while ((c = getopt(argc, argv, "h34C:T:F:")) != EOF) {
	switch(c) {
	case 'h': usage(); break;
	case '3': options.mode = MODE3; break;
	case '4': options.mode = MODE4; break;
	case 'C': sscanf(optarg,"%d",&options.ncycles); break;
	case 'T': sscanf(optarg,"%d",&options.nthreads); break;
	case 'F': options.format = optarg; break;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   goto done;
	}
    }
    if(options.format == NULL || strlen(options.format)==0) usage();
    if(options.mode == 0) usage();
    if(options.ncycles <= 0) options.ncycles = DFALTCYCLES;
    if(options.nthreads <= 0) options.ncycles = DFALTTHREADS;
    
    switch (options.mode) {
    case MODE3: options.ncflags = NC_CLOBBER; break;
    case MODE4: options.ncflags = NC_CLOBBER|NC_NETCDF4; break;
    default: usage(); break;
    }
    
    data = (ThreadData*)calloc(options.nthreads,sizeof(ThreadData));

    if((stat=NC_threadset_create(options.nthreads,&threads))) goto done;
    if((stat = NC_barrier_create(options.nthreads,&barrier))) goto done;

    for(i=0; i<options.nthreads; i++) {
	data[i].id = i;
	data[i].format = options.format;
	data[i].mode = options.mode;
	data[i].barrier = barrier;
	if((stat = NC_thread_create(threadprog,&data[i],threads,i))) goto done;
    }

    if((stat=NC_threadset_join(threads))) goto done;
    if((stat=NC_barrier_destroy(barrier))) goto done;

done:
    if(stat) {
        fprintf(stderr,"*** fail: %s\n",nc_strerror(stat));
        exit(1);
    } else
        exit(0);
}

static RETURNTYPE WINAPI
threadprog(void* arg)
{
    int i, stat = NC_NOERR;
    int ncid = 0;
    ThreadData* data = (ThreadData*)arg;
    char filename[1024];

fprintf(stderr,"<<< data[%d]=%p\n",data->id,arg); fflush(stderr);

    if((stat=NC_barrier_wait(data->barrier))) goto done;

    fprintf(stderr,"starting thread: %d\n",data->id); fflush(stderr);

    snprintf(filename,sizeof(filename),data->format,data->id);
    for(i=0;i<options.ncycles;i++) {
        if((stat = nc_create(filename,data->mode,&ncid))) goto done;
	if((stat = nc_close(ncid))) goto done;
    }
done:
    if(stat) {fprintf(stderr,"***Fail: thread=%d err=%d %s\n",data->id,stat,nc_strerror(stat)); fflush(stderr);}
    printf("stopping thread: %d\n",data->id); fflush(stdout);
    return (RETURNTYPE)0;
}
