#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "netcdf.h"

#define NTHREADS 100
#define NCYCLES 10

#define MODE3 (NC_CLOBBER)
#define MODE4 (NC_CLOBBER|NC_NETCDF4)

typedef struct ThreadData {
    int id;
    const char* format;
    int mode;
} ThreadData;

/* forward */
static void* threadprog(void* data);

static pthread_barrier_t barrier;

static void
usage(void)
{
   fprintf(stderr,"usage: tst_threads 3|4 \"filenameformatstring\"");
   exit(1);
}

int
main(int argc, char **argv)
{
    int i;
    int mode = 0;
    const char* format = NULL;
    
    switch (argc) {
    case 0: 
    case 1:
	usage();
	break;
    default: case 3:
	format = argv[2];
	if(format == NULL || strlen(format)==0) usage();
	/*fall thru*/
    case 2:
        if(strcmp(argv[1],"3")==0) mode = MODE3;
        else if(strcmp(argv[1],"4")==0) mode = MODE4;
	else usage();
    }

    pthread_t threads[NTHREADS];
    ThreadData data[NTHREADS];
    
    memset(threads,0,sizeof(threads));
    memset(data,0,sizeof(ThreadData));

    pthread_barrier_init(&barrier, NULL, NTHREADS);

    for(i=0; i<NTHREADS; i++) {
	data[i].id = i;
	data[i].format = format;
	data[i].mode = mode;
        pthread_create(&threads[i], NULL, threadprog, &data[i]);
    }

    for(i=0; i<NTHREADS; i++) {
        pthread_join(threads[i], NULL);
    }

#if 0
    // wait for threads to terminate.
    for (int i=0; i<NTHREADS; i++) {
        pthread_join(threads[i], NULL);
        delete threads[i];
    }
#endif
}

static void*
threadprog(void* arg)
{
    int i, stat = NC_NOERR;
    int ncid = 0;
    ThreadData* data = (ThreadData*)arg;
    char filename[1024];

    pthread_barrier_wait(&barrier);

    fprintf(stderr,"starting thread: %d\n",data->id); fflush(stderr);

    snprintf(filename,sizeof(filename),data->format,data->id);
    for(i=0;i<NCYCLES;i++) {
        if((stat = nc_create(filename,data->mode,&ncid))) goto done;
	if((stat = nc_close(ncid))) goto done;
    }

done:
    if(stat) {fprintf(stderr,"***Fail: thread=%d err=%d %s\n",data->id,stat,nc_strerror(stat)); fflush(stderr);}
    printf("stopping thread: %d\n",data->id); fflush(stdout);
    pthread_exit(NULL);
    return NULL;
}
