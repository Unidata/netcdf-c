#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "netcdf.h"
#include "netcdf_threadsafe.h"

#define NTHREADS 1
#define NCYCLES 1

#define MODE3 (NC_CLOBBER)
#define MODE4 (NC_CLOBBER|NC_NETCDF4)

typedef struct ThreadData {
    int id;
    const char* format;
    int mode;
} ThreadData;

#ifdef _WIN32
#define RETURNTYPE DWORD
#else
#define RETURNTYPE void*
#define WINAPI
#endif

static RETURNTYPE threadprog(void* data);

/* forward */

static NC_barrier_t* barrier = NULL;

static void
usage(void)
{
   fprintf(stderr,"usage: tst_threads 3|4 \"filenameformatstring\"");
   exit(1);
}

/* Ensure this stays around */
static ThreadData data[NTHREADS];

int
main(int argc, char **argv)
{
    int i, stat = NC_NOERR;
    int mode = 0;
    const char* format = NULL;
    NC_threadset_t* threads = NULL;
    
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

    memset(data,0,sizeof(ThreadData));

    if((stat=NC_threadset_create(NTHREADS,&threads))) goto done;
    if((stat = NC_barrier_create(NTHREADS,&barrier))) goto done;

    for(i=0; i<NTHREADS; i++) {
	data[i].id = i;
	data[i].format = format;
	data[i].mode = mode;
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

    if((stat=NC_barrier_wait(&barrier))) goto done;

    fprintf(stderr,"starting thread: %d\n",data->id); fflush(stderr);

    snprintf(filename,sizeof(filename),data->format,data->id);
    for(i=0;i<NCYCLES;i++) {
        if((stat = nc_create(filename,data->mode,&ncid))) goto done;
	if((stat = nc_close(ncid))) goto done;
    }

done:
    if(stat) {fprintf(stderr,"***Fail: thread=%d err=%d %s\n",data->id,stat,nc_strerror(stat)); fflush(stderr);}
    printf("stopping thread: %d\n",data->id); fflush(stdout);
    return NULL;
}
