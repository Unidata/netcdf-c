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

#include "netcdf.h"
#include "netcdf_threadsafe.h"

#define MODE3 (NC_CLOBBER)
#define MODE4 (NC_CLOBBER|NC_NETCDF4)

#define DFALTCYCLES 1
#define DFALTTHREADS 0

typedef struct NC_ThreadData {
    int id;
    int mode;
    const char* format;
    pthread_barrier_t* barrier;
} NC_ThreadData;

typedef struct NC_Threadset {
    unsigned nthreads;
    pthread_t* threadset;
} NC_Threadset;

typedef struct Options {
    int mode;
    int nthreads;
    int ncycles;
    const char* format;
} Options;

static pthread_barrier_t barrier;

static Options options;

/* Ensure this stays around */
static NC_ThreadData* data = NULL;

/* forward */
static void* threadprog(void* data);
static void NC_threadset_join(NC_Threadset* threadset);
static void usage(const char*);

int
main(int argc, char **argv)
{
    int i, c, stat = NC_NOERR;
    NC_Threadset* threadset = NULL;
    
    /* Initialize options */
    memset(&options,0,sizeof(Options));
    options.ncycles = DFALTCYCLES;
    options.nthreads = DFALTTHREADS;
    options.mode = -1; /* to detect if not set */

    while ((c = getopt(argc, argv, "h34C:T:F:")) != EOF) {
	switch(c) {
	case 'h': usage(NULL); break;
	case '3': options.mode = MODE3; break;
	case '4': options.mode = MODE4; break;
	case 'C': sscanf(optarg,"%d",&options.ncycles); break;
	case 'T': sscanf(optarg,"%d",&options.nthreads); break;
	case 'F': options.format = strdup(optarg); break;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   goto done;
	}
    }
    if(options.format == NULL || strlen(options.format)==0) usage("no format");
    if(options.mode < 0) usage("no mode");
    if(options.ncycles <= 0) options.ncycles = DFALTCYCLES;
    if(options.nthreads <= 0) options.ncycles = DFALTTHREADS;

    data = (NC_ThreadData*)calloc(options.nthreads,sizeof(NC_ThreadData));
    if(data == NULL) {
        fprintf(stderr,"out of memory\n");
	exit(1);
    }

    threadset = (NC_Threadset*)calloc(1,sizeof(NC_Threadset));
    if(threadset == NULL) {
        fprintf(stderr,"out of memory\n");
	exit(1);
    }

    threadset->nthreads = options.nthreads;
    threadset->threadset = (pthread_t*)calloc(threadset->nthreads,sizeof(pthread_t));

    pthread_barrier_init(&barrier, NULL, options.nthreads);

    for(i=0; i<options.nthreads; i++) {
	data[i].id = i;
	data[i].format = options.format;
	data[i].mode = options.mode;
	data[i].barrier = &barrier;
        pthread_create(&threadset->threadset[i], NULL, threadprog, (void*)&data[i]);
    }

    NC_threadset_join(threadset);
    pthread_barrier_destroy(&barrier);

    if(data) free(data);
    if(threadset) {free(threadset->threadset); free(threadset);}

done:
    nc_finalize();
    if(stat) {
        fprintf(stderr,"*** fail: %s\n",nc_strerror(stat));
        exit(1);
    } else
        exit(0);
}

static void*
threadprog(void* arg)
{
    int i, stat = NC_NOERR;
    int ncid = 0;
    NC_ThreadData* data = (NC_ThreadData*)arg;
    char filename[1024];

    fprintf(stderr,"starting thread: %d\n",data->id); fflush(stderr);

    pthread_barrier_wait(data->barrier);

    snprintf(filename,sizeof(filename),data->format,data->id);
    for(i=0;i<options.ncycles;i++) {
        stat = nc_create(filename,data->mode,&ncid);
        fprintf(stderr, ">>> (1) filename=%s data->mode=%d stat=%d\n", filename, data->mode, stat);
        (void)nc_close(ncid);
        if (stat) goto done;
    }
done:
    if(stat) {fprintf(stderr,"***Fail: thread=%d err=%d %s\n",data->id,stat,nc_strerror(stat)); fflush(stderr);}
    printf("stopping thread: %d\n",data->id); fflush(stdout);
    pthread_exit(NULL);
    return NULL;
}

static void
NC_threadset_join(NC_Threadset* threadset)
{
    unsigned i;
    for(i=0; i<threadset->nthreads; i++) {
        pthread_join(threadset->threadset[i], NULL);
    }
}

static void
usage(const char* msg)
{
   fprintf(stderr,"usage: tst_threads [-h][-3|-4][-F <filenameformatstring>][-C <ncycles>][-T <nthreads>]\n");
   if(msg != NULL)
       fprintf(stderr,"error: %s\n",msg);
   exit(1);
}
