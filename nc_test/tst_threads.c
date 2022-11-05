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

#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include "netcdf.h"
#include "netcdf_threadsafe.h"

#define MODE3 3
#define MODE4 4

#define DFALTCYCLES 1
#define DFALTTHREADS 0

typedef struct NC_ThreadData {
    int id;
    const char* format;
    int mode;
    pthread_barrier_t* barrier;
} NC_ThreadData;

typedef struct NC_Threadset {
    unsigned nthreads;
    pthread_t* threadset;
} NC_Threadset;

typedef struct Options {
    int mode;
    int ncflags;
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
static void usage(void);

int
main(int argc, char **argv)
{
    int i, c, stat = NC_NOERR;
    NC_Threadset* threadset = NULL;
    
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

    data = (NC_ThreadData*)calloc(options.nthreads,sizeof(NC_ThreadData));

    threadset = (NC_Threadset*)calloc(1,sizeof(NC_Threadset));
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

fprintf(stderr,"<<< data[%d]=%p\n",data->id,arg); fflush(stderr);

    pthread_barrier_wait(data->barrier);

    fprintf(stderr,"starting thread: %d\n",data->id); fflush(stderr);

    snprintf(filename,sizeof(filename),data->format,data->id);
    for(i=0;i<options.ncycles;i++) {
        if((stat = nc_create(filename,data->mode,&ncid))) goto done;
	if((stat = nc_close(ncid))) goto done;
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
usage(void)
{
   fprintf(stderr,"usage: tst_threads 3|4 \"filenameformatstring\"");
   exit(1);
}
