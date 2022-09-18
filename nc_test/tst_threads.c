#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* Define a single check for PTHREADS vs WIN32 */
#ifdef _WIN32
/* Win32 syncronization has priority */
#undef USEPTHREADS
#else
#ifdef HAVE_PTHREADS
#define USEPTHREADS
#endif
#endif

#ifdef USEPTHREADS
#include <pthread.h>
#ifdef __APPLE__
#include "ncmutex.h"
#endif
#else
#include <windows.h>
#include <synchapi.h>
#endif

#include "netcdf.h"

#define NTHREADS 4
#define NCYCLES 4

#define MODE3 (NC_CLOBBER)
#define MODE4 (NC_CLOBBER|NC_NETCDF4)

typedef struct ThreadData {
    int id;
    const char* format;
    int mode;
} ThreadData;

#ifdef USEPTHREADS
#define RETURNTYPE void*
#define WINAPI
#else
#define RETURNTYPE DWORD
#endif

/* forward */
static RETURNTYPE threadprog(void* data);

#ifdef USEPTHREADS
static pthread_barrier_t barrier;
#else
static SYNCHRONIZATION_BARRIER barrier;
#endif

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
    int i;
    int mode = 0;
    const char* format = NULL;
    
#ifdef USEPTHREADS
    pthread_t threads[NTHREADS];
#else
    HANDLE threads[NTHREADS];
#endif
    
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

    memset(threads,0,sizeof(threads));
    memset(data,0,sizeof(ThreadData));

#ifdef USEPTHREADS
    pthread_barrier_init(&barrier, NULL, NTHREADS);
#else
    InitializeSynchronizationBarrier(&barrier,(LONG)NTHREADS,(LONG)-1);
#endif

    for(i=0; i<NTHREADS; i++) {
	data[i].id = i;
	data[i].format = format;
	data[i].mode = mode;
#ifdef USEPTHREADS
        pthread_create(&threads[i], NULL, threadprog, &data[i]);
#else
fprintf(stderr,">>> &data[%d]=%p\n",i,&data[i]); fflush(stderr);
        threads[i] = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            threadprog,             // thread function name
            &data[i],               // argument to thread function 
            0,                      // use default creation flags 
            NULL);         // returns the thread identifier 
#endif
    }

#ifdef USEPTHREADS
    for(i=0; i<NTHREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_barrier_destroy(&barrier);
#else
    WaitForMultipleObjects(NTHREADS, threads, TRUE, INFINITE);
#endif

}

static RETURNTYPE WINAPI
threadprog(void* arg)
{
    int i, stat = NC_NOERR;
    int ncid = 0;
    ThreadData* data = (ThreadData*)arg;
    char filename[1024];

fprintf(stderr,"<<< data[%d]=%p\n",data->id,arg); fflush(stderr);

#ifdef USEPTHREADS
    pthread_barrier_wait(&barrier);
#else
    EnterSynchronizationBarrier(&barrier,SYNCHRONIZATION_BARRIER_FLAGS_BLOCK_ONLY);
#endif

    fprintf(stderr,"starting thread: %d\n",data->id); fflush(stderr);

    snprintf(filename,sizeof(filename),data->format,data->id);
    for(i=0;i<NCYCLES;i++) {
        if((stat = nc_create(filename,data->mode,&ncid))) goto done;
	if((stat = nc_close(ncid))) goto done;
    }

done:
    if(stat) {fprintf(stderr,"***Fail: thread=%d err=%d %s\n",data->id,stat,nc_strerror(stat)); fflush(stderr);}
    printf("stopping thread: %d\n",data->id); fflush(stdout);
#ifdef USEPTHREADS
    pthread_exit(NULL);
    return NULL;
#else
    ExitThread(0);
    return 0;
#endif
}
