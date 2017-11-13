#if defined(BGQDONT)

#include <pio.h>
#include <pio_internal.h>

#include <math.h>

#include <kernel/process.h>
#include <kernel/location.h>
#include <firmware/include/personality.h>
#include <mpix.h>

#define   Personality                    Personality_t

int rank;
int np;
int my_name_len;
char my_name[255];

void identity(const MPI_Comm comm, int *iotask)
{
    MPIX_Hardware_t hw;
    char message[100];
    /* Number of MPI tasks per Pset */
    int coreId;
    int *TasksPerPset;
    int *tmp;
    int i,ierr;
    Personality_t pers;

    MPI_Comm_rank(comm,&rank);
    MPI_Comm_size(comm,&np);
    MPI_Get_processor_name(my_name, &my_name_len);


    MPIX_Hardware(&hw);

    Kernel_GetPersonality(&pers, sizeof(pers));

    int numIONodes,numPsets,numNodesInPset,rankInPset;
    int numpsets, psetID, psetsize, psetrank;

    bgq_pset_info (comm, &numpsets, &psetID, &psetsize, &psetrank);

    numIONodes = numpsets;
    numNodesInPset = psetsize;
    rankInPset = rank;
    numPsets = numpsets;

    if(rank == 0) { printf("number of IO nodes in block: %i \n",numIONodes);}
    if(rank == 0) { printf("number of Psets in block : %i \n",numPsets);}
    if(rank == 0) { printf("number of compute nodes in Pset: %i \n",numNodesInPset);}

    int psetNum;
    psetNum = psetID;

#ifdef DEBUG
    if((*iotask)>0) {
        printf( "%04i (%-50s %s) %i yes\n", rank, my_name, message, psetNum );
    } else {
        printf( "%04i (%-50s %s) %i --\n", rank, my_name, message, psetNum);
    }
    printf("MPI task %6i is rank %3i in Pset: %3i \n",rank, rankInPset,psetNum);
#endif
    /* Determine which core on node....  I don't want to put more than one io-task per node */
    coreId = Kernel_PhysicalProcessorID ();

    TasksPerPset = malloc(numPsets*sizeof(int));
    tmp = malloc(numPsets*sizeof(int));
    for(i=0;i<numPsets;i++) tmp[i]=0;
    if(coreId == 0) {tmp[psetNum]=1;}
    ierr = MPI_Allreduce(tmp,TasksPerPset,numPsets,MPI_INT,MPI_SUM,comm);
    if(rank == 0) {
        for(i=0;i<numPsets;i++) {printf("Pset: %3i has %3i nodes \n",i,TasksPerPset[i]);}
    }
    free(tmp);
    free(TasksPerPset);
}

void determineiotasks(const MPI_Comm comm, int *numiotasks,int *base, int *stride, int *rearr,
                      bool *iamIOtask)
{

/*

  Returns the correct numiotasks and the flag iamIOtask

  Some concepts:

  processor set:     A group of processors on the Blue Gene system which have
  one or more IO processor (Pset)

  IO-node:           A special Blue Gene node dedicated to performing IO.  There
  are one or more per processor set

  IO-client:         This is software concept.  This refers to the MPI task
  which performs IO for the PIO library
*/
    int psetNum;
    int coreId;
    int iam;
    int task_count;

    MPIX_Hardware_t hw;
    /*  Get the personality  */
    Personality_t pers;

    MPIX_Hardware(&hw);

    MPI_Comm_rank(comm, &rank);

    MPI_Comm_size(comm, &np);

    MPI_Get_processor_name(my_name, &my_name_len);



    /* printf("Determine io tasks: proc %i: tasks= %i numiotasks=%i stride= %i \n", rank, np, (*numiotasks), (*stride)); */

    if((*rearr) > 0) {


        Kernel_GetPersonality(&pers, sizeof(pers));

        int numIONodes,numPsets,numNodesInPset,rankInPset;
        int numiotasks_per_node,remainder,numIONodes_per_pset;
        int lstride;

        /* Number of computational nodes in processor set */

        int numpsets, psetID, psetsize, psetrank;
        bgq_pset_info (comm,&numpsets, &psetID, &psetsize, &psetrank);
        numIONodes = numpsets;
        numNodesInPset = psetsize;

        /*     printf("Determine io tasks: me %i : nodes in pset= %i ionodes = %i\n", rank, numNodesInPset, numIONodes); */

        if((*numiotasks) < 0 ) {
            /* negative numiotasks value indicates that this is the number per IO-node */
            (*numiotasks) = - (*numiotasks);
            if((*numiotasks) > numNodesInPset) {
                numiotasks_per_node = numNodesInPset;
            } else  {
                numiotasks_per_node = (*numiotasks);
            }
            remainder = 0;
        } else if ((*numiotasks) > 0 ) {
            /* balance the number of iotasks to number of IO nodes */
            numiotasks_per_node = floor((float)(*numiotasks)/ (float) numIONodes);
            /* put a minumum here so that we have a chance  - though this may be too low */
            if (numiotasks_per_node < 1) {
                numiotasks_per_node = 1;
                *numiotasks = numIONodes;
            }
            remainder = (*numiotasks) - numiotasks_per_node * numIONodes;
        } else if ((*numiotasks) == 0 ) {
            if((*stride) > 0) {
                numiotasks_per_node = numNodesInPset/(*stride);
                if (numiotasks_per_node < 1) {
                    numiotasks_per_node = 1;
                    *numiotasks = numIONodes;
                }
            } else {
                numiotasks_per_node = 8;  /* default number of IO-client per IO-node is not otherwise specificied */
            }
            remainder = 0;
        }

        /* number of IO nodes with a larger number of io-client per io-node */
        if(remainder > 0) {
            if(rank ==0) {printf("Unbalanced IO-configuration: %i IO-nodes have %i IO-clients : %i IO-nodes have %i IO-clients \n",
                                 remainder, numiotasks_per_node+1, numIONodes-remainder,numiotasks_per_node);}
            lstride = min(np,floor((float)numNodesInPset/(float)(numiotasks_per_node+1)));
        } else {
            if(rank == 0) {
                printf("Balanced IO-configuration: %i IO-nodes have %i IO-clients\n",numIONodes-remainder, numiotasks_per_node);
            }
            lstride = min(np,floor((float)numNodesInPset/(float)numiotasks_per_node));
        }

        /* Number of processor sets */
        numPsets = numpsets;

        /* number of IO nodes in processor set (I need to add
           code to deal with the case where numIONodes_per_pset != 1 works
           correctly) */
        numIONodes_per_pset = numIONodes/numPsets;

        /* Determine which core on node....  I don't want to put more than one io-task per node */
        coreId = Kernel_PhysicalProcessorID ();

        /* What is the rank of this node in the processor set */

        psetNum = psetID;
        rankInPset = psetrank;

        /* printf("Pset #: %i has %i nodes in Pset; base = %i\n",psetNum,numNodesInPset, *base); */

        (*iamIOtask) = false;   /* initialize to zero */

        if (numiotasks_per_node == numNodesInPset)(*base) = 0;  /* Reset the base to 0 if we are using all tasks */


        /* start stridding MPI tasks from base task */
        iam = max(0,rankInPset-(*base));
        if (iam >= 0)  {
            /* mark tasks that will be IO-tasks  or IO-clients */
            /*    printf("iam = %d lstride = %d coreID = %d\n",iam,lstride,coreId); */
            if((iam % lstride == 0) && (coreId == 0) ) {  /* only io tasks indicated by stride and coreId = 0 */
                if((iam/lstride) < numiotasks_per_node) {
                    /* only set the first (numiotasks_per_node - 1) tasks */
                    (*iamIOtask) = true;
                } else if ((iam/lstride) == numiotasks_per_node) {
                    /*  If there is an uneven number of io-clients to io-nodes
                        allocate the first remainder - 1 processor sets to
                        have a total of numiotasks_per_node */
                    if(psetNum < remainder) {(*iamIOtask) = true;
                    };
                }
            }
            /*       printf("comm = %d iam = %d lstride = %d coreID = %d iamIOtask = %i \n",comm, iam,lstride,coreId,(*iamIOtask)); */
        }
    }else{
        /* We are not doing rearrangement.... so all tasks are io-tasks */
        (*iamIOtask) = true;
    }

    /*printf("comm = %d myrank = %i iotask = %i \n", comm, rank, (*iamIOtask));*/

    /* now we need to correctly determine the numiotasks */
    MPI_Allreduce(iamIOtask, &task_count, 1, MPI_INT, MPI_SUM, comm);

    (*numiotasks) = task_count;


}

int bgq_ion_id (void)
{
    int iA,  iB,  iC,  iD,  iE;                /* The local node's coordinates  */
    int nA,  nB,  nC,  nD,  nE;                /* Size of each torus dimension  */
    int brA, brB, brC, brD, brE;               /* The bridge node's coordinates */
    int io_node_route_id;

    Personality_t personality;

    Kernel_GetPersonality(&personality, sizeof(personality));

    iA  = personality.Network_Config.Acoord;
    iB  = personality.Network_Config.Bcoord;
    iC  = personality.Network_Config.Ccoord;
    iD  = personality.Network_Config.Dcoord;
    iE  = personality.Network_Config.Ecoord;

    nA  = personality.Network_Config.Anodes;
    nB  = personality.Network_Config.Bnodes;
    nC  = personality.Network_Config.Cnodes;
    nD  = personality.Network_Config.Dnodes;
    nE  = personality.Network_Config.Enodes;

    brA = personality.Network_Config.cnBridge_A;
    brB = personality.Network_Config.cnBridge_B;
    brC = personality.Network_Config.cnBridge_C;
    brD = personality.Network_Config.cnBridge_D;
    brE = personality.Network_Config.cnBridge_E;

/*
 * This is the bridge node, numbered in ABCDE order, E increments first.
 * It is considered the unique "io node route identifer" because each
 * bridge node only has one torus link to one io node.
 */

    io_node_route_id = brE + brD*nE + brC*nD*nE + brB*nC*nD*nE + brA*nB*nC*nD*nE;

    return io_node_route_id;

}



int bgq_pset_info (MPI_Comm comm, int* tot_pset, int* psetID, int* pset_size, int* rank_in_pset)
{
    MPI_Comm comp_comm, pset_comm, bridge_comm;
    int comp_rank, status, key, bridge_root, tot_bridges, cur_pset, itr, t_buf;
    int temp_id, rem_psets;
    MPI_Status mpi_status;

    MPI_Comm_rank (comm, &comp_rank);

    status = MPI_Comm_dup ( comm, &comp_comm);
    if ( MPI_SUCCESS != status)
    {
        printf(" Error duplicating communicator \n");
        MPI_Abort(comm, status);
    }

    // Compute the ION BridgeNode ID
    key = bgq_ion_id ();

    // Create the pset_comm per bridge node
    status = MPI_Comm_split ( comp_comm, key, comp_rank, &pset_comm);
    if ( MPI_SUCCESS != status)
    {
        printf(" Error splitting communicator \n");
        MPI_Abort(comm, status);
    }

    // Calculate the rank in pset and pset size
    MPI_Comm_rank (pset_comm, rank_in_pset);
    MPI_Comm_size (pset_comm, pset_size);

    // Create the Bridge root nodes communicator
    bridge_root = 0;
    if (0 == *rank_in_pset)
        bridge_root = 1;

    // Calculate the total number of bridge nodes / psets
    tot_bridges = 0;
    MPI_Allreduce (&bridge_root, &tot_bridges, 1, MPI_INT, MPI_SUM, comm);

    *tot_pset = tot_bridges;

    // Calculate the Pset ID
    cur_pset = 0;
    rem_psets = tot_bridges;
    if ((0 == comp_rank) && (bridge_root ==1))
    {
        *psetID = 0;
        rem_psets = tot_bridges-1;
        cur_pset++;
    }

    t_buf = 0; // Dummy value
    if (0 == comp_rank)
    {
        for (itr = 0; itr < rem_psets; itr++)
        {
            MPI_Recv (&t_buf,1, MPI_INT,  MPI_ANY_SOURCE, MPI_ANY_TAG,comm, &mpi_status);
            MPI_Send (&cur_pset, 1, MPI_INT, mpi_status.MPI_SOURCE, 0, comm);
            cur_pset++;
        }
    }

    if ((1 == bridge_root) && ( 0 != comp_rank))
    {
        MPI_Send (&t_buf, 1, MPI_INT, 0, 0, comm);
        MPI_Recv (&temp_id,1, MPI_INT, 0, 0, comm, &mpi_status);

        *psetID = temp_id;
        /*printf (" Pset ID is %d \n", *psetID);*/
    }
    // Broadcast the PSET ID to all ranks in the psetcomm
    MPI_Bcast ( psetID, 1, MPI_INT, 0, pset_comm);

    // Free the split comm
    MPI_Comm_free (&pset_comm);

    MPI_Barrier (comm);

    return 0;
}

#endif
