/*
 * This program tests some MPI functionality that is used in PIO. This
 * runs on three processors, and does the same MPI commands that are
 * done when async mode is used, with 1 IO task, and two computation
 * compoments, each of one task.
 *
 * Note that this test does not contain includes to pio headers, it is
 * pure MPI code.
 *
 * @author Ed Hartnett
 * @date 8/28/16
 */
#include <config.h>
#include <stdio.h>
#include <mpi.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 3

/* The name of this test. */
#define TEST_NAME "test_async_mpi"

/* Number of processors that will do IO. */
#define NUM_IO_PROCS 1

/* Number of computational components to create. */
#define COMPONENT_COUNT 2

#define ERR_AWFUL 1111
#define ERR_WRONG 1112

#define MSG_EXIT 42

/* Handle MPI errors. This should only be used with MPI library
 * function calls. */
#define MPIERR(e) do {                                                  \
        MPI_Error_string(e, err_buffer, &resultlen);                    \
        fprintf(stderr, "MPI error, line %d, file %s: %s\n", __LINE__, __FILE__, err_buffer); \
        MPI_Finalize();                                                 \
        return ERR_AWFUL;                                               \
    } while (0)

/* Handle non-MPI errors by finalizing the MPI library and exiting
 * with an exit code. */
#define ERR(e) do {                                                     \
        fprintf(stderr, "%d Error %d in %s, line %d\n", my_rank, e, __FILE__, __LINE__); \
        MPI_Finalize();                                                 \
        return e;                                                       \
    } while (0)

/* Global err buffer for MPI. When there is an MPI error, this buffer
 * is used to store the error message that is associated with the MPI
 * error. */
char err_buffer[MPI_MAX_ERROR_STRING];

/* This is the length of the most recent MPI error message, stored
 * int the global error string. */
int resultlen;

/* Put together a communicator with the correct number of tasks for
 * this test (3).
 */
int get_test_comm(int my_rank, int ntasks, int min_ntasks, int max_ntasks, MPI_Comm *comm)
{
    int ret;
    
    /* Check that a valid number of processors was specified. */
    if (ntasks < min_ntasks)
    {
        fprintf(stderr, "ERROR: Number of processors must be at least %d for this test!\n",
                min_ntasks);
        return ERR_AWFUL;
    }
    else if (ntasks > max_ntasks)
    {
        /* If more tasks are available than we need for this test,
         * create a communicator with exactly the number of tasks we
         * need. */
        int color, key;
        if (my_rank < max_ntasks)
        {
            color = 0;
            key = my_rank;
        }
        else
        {
            color = 1;
            key = my_rank - max_ntasks;
        }
        if ((ret = MPI_Comm_split(MPI_COMM_WORLD, color, key, comm)))
            MPIERR(ret);
    }
    else
    {
        if ((ret = MPI_Comm_dup(MPI_COMM_WORLD, comm)))
            MPIERR(ret);
    }
    return 0;
}

/*
 * This function is called by the IO task.  This function will not
 * return, unless there is an error.
 *
 * @param verbose non-zero to turn on printf statements. 
 * @param my_rank rank of this task.
 * @param io_rank rank of the IO processor in union_comm.
 * @param component_count number of computation components
 * @param union_comm array (length component_count) of union_comm
 * communicators.
 * @param comproot array (length component_count) of ints. The rank of
 * the computation leader in the union comm.
 * @param comp_comm array (length component_count) of computaion
 * component communicators.
 * @param io_comm MPI communicator for IO
 * @returns 0 for success, error code otherwise.
 * @author Ed Hartnett
 */
int msg_handler(int verbose, int my_rank, int io_rank, int component_count, MPI_Comm *union_comm,
                MPI_Comm *comp_comm, int *comproot, MPI_Comm io_comm)
{
    int msg = 0;
    MPI_Request req[component_count];
    MPI_Status status;
    int index;
    int open_components = component_count;
    int mpierr;

    /* Have IO comm rank 0 (the ioroot) register to receive
     * (non-blocking) for a message from each of the comproots. */
    if (!io_rank)
    {
        for (int cmp = 0; cmp < component_count; cmp++)
        {
            if (verbose)
                printf("my_rank %d cmp %d about to call MPI_Irecv comproot[cmp] %d union_comm[cmp] %d\n",
                       my_rank, cmp, comproot[cmp], union_comm[cmp]);
            if ((mpierr = MPI_Irecv(&msg, 1, MPI_INT, comproot[cmp], MPI_ANY_TAG,
                                    union_comm[cmp], &req[cmp])))
                MPIERR(mpierr);
            if (verbose)
                printf("my_rank %d MPI_Irecv req[%d] = %d\n", my_rank, cmp, req[cmp]);
        }
    }

    /* If the message is not -1, keep processing messages. */
    while (msg != -1)
    {
        if (verbose)
            printf("my_rank %d msg_handler at top of loop\n", my_rank);

        /* Wait until any one of the requests are complete. Once it
         * returns, the Waitany function automatically sets the
         * appropriate member of the req array to MPI_REQUEST_NULL. */
        if (!io_rank)
        {
            if (verbose)
            {
                printf("my_rank %d about to call MPI_Waitany req[0] = %d MPI_REQUEST_NULL = %d\n",
                       my_rank, req[0], MPI_REQUEST_NULL);
                for (int c = 0; c < component_count; c++)
                    printf("my_rank %d req[%d] = %d\n", my_rank, c, req[c]);
            }
            if ((mpierr = MPI_Waitany(component_count, req, &index, &status)))
                MPIERR(mpierr);
            if (verbose)
                printf("my_rank %d Waitany returned index = %d req[%d] = %d\n", my_rank, index, index, req[index]);
        }

        /* Broadcast the index and msg value to the rest of the IO tasks. */
        if (verbose)
            printf("my_rank %d about to MPI_Bcast io_comm %d index %d msg %d\n", my_rank, io_comm,
                   index, msg);
        if ((mpierr = MPI_Bcast(&index, 1, MPI_INT, 0, io_comm)))
            MPIERR(mpierr);
        if ((mpierr = MPI_Bcast(&msg, 1, MPI_INT, 0, io_comm)))
            MPIERR(mpierr);
        if (verbose)
            printf("my_rank %d MPI_Bcast io_comm %d index %d msg %d\n", my_rank, io_comm,
                   index, msg);

        /* Handle the message. This code is run on all IO tasks. */
        switch (msg)
        {
        case MSG_EXIT:
            if (verbose)
                printf("exit message received\n");
            msg = -1;
            break;
        default:
            return ERR_WRONG;
        }

        /* Listen for another msg from the component whose message we
         * just handled. */
        if (!io_rank && msg != -1)
        {
            if (verbose)
                printf("my_rank %d msg_handler about to Irecv index = %d comproot = %d union_comm = %d\n",
                       my_rank, index, comproot[index], union_comm[index]);
            if ((mpierr = MPI_Irecv(&msg, 1, MPI_INT, comproot[index], MPI_ANY_TAG, union_comm[index],
                                    &req[index])))
                MPIERR(mpierr);
            if (verbose)
                printf("my_rank %d msg_handler called MPI_Irecv req[%d] = %d\n", my_rank, index, req[index]);
        }

        if (verbose)
            printf("my_rank %d msg_handler done msg = %d open_components = %d\n", my_rank, msg, open_components);

        /* If there are no more open components, exit. */
        if (msg == -1)
        {
            --open_components;
            if (verbose)
                printf("open_components %d\n", open_components);
            if (open_components)
                msg = MSG_EXIT;
            else
                return 0;
        }
    }

    return 0;
}

/* Run simple async test. */
int main(int argc, char **argv)
{
    int my_rank = 0;    /* Zero-based rank of processor. */
    int ntasks;         /* Number of processors involved in current execution. */
    MPI_Comm test_comm; /* Communicator for tasks running tests. */
    int mpierr;         /* Return code from MPI functions. */
    int verbose = 0;    /* Non-zero to turn on printf statements. */
    int ret;            /* Return code from function calls. */

    /* Initialize MPI. */
    if ((ret = MPI_Init(&argc, &argv)))
        MPIERR(ret);

    /* Learn my rank and the total number of processors. */
    if ((ret = MPI_Comm_rank(MPI_COMM_WORLD, &my_rank)))
        MPIERR(ret);
    if ((ret = MPI_Comm_size(MPI_COMM_WORLD, &ntasks)))
        MPIERR(ret);

    /* Get test_comm. */
    if ((ret = get_test_comm(my_rank, ntasks, TARGET_NTASKS, TARGET_NTASKS, &test_comm)))
        ERR(ret);

    /* Ignore all but 3 tasks. */
    if (my_rank < TARGET_NTASKS)
    {
        MPI_Group world_group;
        MPI_Comm io_comm;
        MPI_Group io_group;
        int my_io_proc_list[1] = {0}; /* List of processors in IO component. */        
        int num_io_procs = 1;
        int num_procs_per_comp[COMPONENT_COUNT] = {1, 1};
        int in_io = my_rank ? 0 : 1; /* Non-zero if this task is in IO. */
        int io_rank = -1;            /* Rank of current process in IO comm. */
        int comp_rank = -1;
        int iomaster;  /* MPI_ROOT on master IO task, MPI_PROC_NULL otherwise. */
        MPI_Group group[COMPONENT_COUNT];       /* Group with comp tasks. */
        MPI_Group union_group[COMPONENT_COUNT]; /* Group with IO and comp tasks. */
        int my_proc_list[COMPONENT_COUNT][1] = {{1}, {2}};   /* Tasks for computation components. */
        MPI_Comm comp_comm[COMPONENT_COUNT];    
        MPI_Comm union_comm[COMPONENT_COUNT];
        MPI_Comm intercomm[COMPONENT_COUNT];
        int in_cmp[COMPONENT_COUNT] = {0, 0};    /* Is this process in this computation component? */
        
        /* Create group for world. */
        if ((ret = MPI_Comm_group(test_comm, &world_group)))
            MPIERR(ret);

        if (verbose)
            printf("MPI_GROUP_NULL %d MPI_COMM_NULL %d\n", MPI_GROUP_NULL, MPI_COMM_NULL);

        /* There is one shared IO comm. Create it. */
        if ((ret = MPI_Group_incl(world_group, num_io_procs, my_io_proc_list, &io_group)))
            MPIERR(ret);
        if ((ret = MPI_Comm_create(test_comm, io_group, &io_comm)))
            MPIERR(ret);
        MPI_Group_free(&io_group);
        if (verbose)
            printf("my_rank %d created io comm io_comm = %d\n", my_rank, io_comm);

        /* For processes in the IO component, get their rank within the IO
         * communicator. */
        if (in_io)
        {
            if ((ret = MPI_Comm_rank(io_comm, &io_rank)))
                MPIERR(ret);
            iomaster = !io_rank ? MPI_ROOT : MPI_PROC_NULL;
        }
        if (verbose)
            printf("my_rank %d in_io %d io_rank %d IO %s\n", my_rank, in_io,
                   io_rank, iomaster == MPI_ROOT ? "MASTER" : "SERVANT");
        
        /* For each computation component. */
        for (int cmp = 0; cmp < COMPONENT_COUNT; cmp++)
        {
            /* How many processors in the union comm? */
            int nprocs_union = num_io_procs + num_procs_per_comp[cmp];

            /* This will hold proc numbers from both computation and IO
             * components. */
            int proc_list_union[nprocs_union];

            /* All the processes in this component, and the IO component,
             * are part of the union_comm. */
            int union_rank = -1;
            int pidx;
            MPI_Comm io_comm2;
            
            /* Create a group for this component. */
            if ((ret = MPI_Group_incl(world_group, 1, my_proc_list[cmp], &group[cmp])))
                MPIERR(ret);
            if (verbose)
                printf("my_rank %d created component MPI group - group[%d] = %d\n", my_rank, cmp, group[cmp]);

            /* Add proc numbers from IO. */
            proc_list_union[0] = 0;

            /* Add proc numbers for this computation component. */
            for (int p = 0; p < num_procs_per_comp[cmp]; p++)
                proc_list_union[p + num_io_procs] = my_proc_list[cmp][p];

            /* Determine if current task is in this computation component. */
            for (pidx = 0; pidx < num_procs_per_comp[cmp]; pidx++)
                if (my_rank == my_proc_list[cmp][pidx])
                    break;
            in_cmp[cmp] = (pidx == num_procs_per_comp[cmp]) ? 0 : 1;
            if (verbose)
                printf("my_rank %d pidx = %d num_procs_per_comp[%d] = %d in_cmp[cmp] = %d\n",
                       my_rank, pidx, cmp, num_procs_per_comp[cmp], in_cmp[cmp]);

            /* Create an intracomm for this component. */
            if ((ret = MPI_Comm_create(test_comm, group[cmp], &comp_comm[cmp])))
                MPIERR(ret);
            MPI_Group_free(&group[cmp]);
            
            if (in_cmp[cmp])
            {
                /* Get the rank in this comp comm. */
                if ((ret = MPI_Comm_rank(comp_comm[cmp], &comp_rank)))
                    MPIERR(ret);
            }
            if (verbose)
                printf("my_rank %d intracomm created for cmp = %d comp_comm[cmp] = %d comp_rank = %d\n",
                       my_rank, cmp, comp_comm[cmp], comp_rank);
            
            /* If this is the IO component, make a copy of the IO comm for
             * each computational component. */
            if (in_io)
            {
                if ((ret = MPI_Comm_dup(io_comm, &io_comm2)))
                    MPIERR(ret);
                if (verbose)
                    printf("my_rank %d dup of io_comm = %d io_rank = %d\n", my_rank, io_comm, io_rank);
            }

            /* Create a group for the union of the IO component
             * and one of the computation components. */
            if ((ret = MPI_Group_incl(world_group, nprocs_union, proc_list_union, &union_group[cmp])))
                MPIERR(ret);
            if ((ret = MPI_Comm_create(test_comm, union_group[cmp], &union_comm[cmp])))
                MPIERR(ret);
            MPI_Group_free(&union_group[cmp]);
            if (verbose)
                printf("my_rank %d created union - union_group[%d] %d with %d procs union_comm[%d] %d\n",
                       my_rank, cmp, union_group[cmp], nprocs_union, cmp, union_comm[cmp]);


            if (in_io || in_cmp[cmp])
            {
                if ((ret = MPI_Comm_rank(union_comm[cmp], &union_rank)))
                    MPIERR(ret);
                if (verbose)
                    printf("my_rank %d union_rank %d\n", my_rank, union_rank);
                
                if (in_io)
                {
                    /* Create the intercomm from IO to computation component. */
                    if ((ret = MPI_Intercomm_create(io_comm, 0, union_comm[cmp],
                                                    1, cmp, &intercomm[cmp])))
                        MPIERR(ret);
                }
                else if (in_cmp[cmp])
                {
                    /* Create the intercomm from computation component to IO component. */
                    if ((ret = MPI_Intercomm_create(comp_comm[cmp], 0, union_comm[cmp],
                                                    0, cmp, &intercomm[cmp])))
                        MPIERR(ret);
                }
                if (verbose)
                    printf("my_rank %d intercomm created for cmp = %d\n", my_rank, cmp);
            } /* in_io || in_cmp */

            /* Free resources. */
            if (in_io)
                MPI_Comm_free(&io_comm2);
        } /* next computation component. */

        /* Now launch IO message processing on the IO task. */
        int comproot[COMPONENT_COUNT] = {1, 1};
        if (in_io)
            if ((ret = msg_handler(verbose, my_rank, 0, COMPONENT_COUNT, union_comm, comp_comm,
                                   comproot, io_comm)))
                ERR(ret);

        /* Send exit messages. */
        if (!in_io)
        {
            for (int cmp = 0; cmp < COMPONENT_COUNT; cmp++)
            {
            
                int msg = MSG_EXIT;
                int ioroot = 0;

                if (in_cmp[cmp])
                {
                    if (verbose)
                        printf("my_rank %d sending exit message on union_comm %d\n", my_rank, union_comm[cmp]);
                    if ((mpierr = MPI_Send(&msg, 1, MPI_INT, ioroot, 1, union_comm[cmp])))
                        MPIERR(mpierr);
                }
            }
        }

        /* Free MPI resources. */
        if (verbose)
            printf("my_rank %d freeing resources\n", my_rank);
        for (int cmp = 0; cmp < COMPONENT_COUNT; cmp++)
        {
            if (comp_comm[cmp] != MPI_COMM_NULL)            
                MPI_Comm_free(&comp_comm[cmp]);
            if (union_comm[cmp] != MPI_COMM_NULL)
                MPI_Comm_free(&union_comm[cmp]);
            if (in_io || in_cmp[cmp])
                MPI_Comm_free(&intercomm[cmp]);
        }
        MPI_Group_free(&world_group);
        if (io_comm != MPI_COMM_NULL)            
            MPI_Comm_free(&io_comm);            
    }

    /* Free the MPI communicator for this test. */
    MPI_Comm_free(&test_comm);    

    /* Finalize MPI. */
    MPI_Finalize();

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
