#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <mpi.h>

#include "_config.h"

typedef struct
{
    int group_id;
    int group_num;
    MPI_Comm global_comm;
    int global_myrank;
    int global_nranks;
    MPI_Comm local_comm;
    int local_myrank;
    int local_nranks;
    int *_rows; // size=global_nranks. _rows[global_rank] is the group to which the rank belongs
    int *_cols; // size=global_nranks. _cols[global_rank] is the local_rank in group
    int *_brothers; // size=local_nranks. _brothers[local_rank] is the global_rank on the same group
} _commgroup_struct_t;

typedef _commgroup_struct_t* commgroup_t; // handle
#define COMMGROUP_NULL ((commgroup_t) -1)

/**
 * @brief Create communication group information based on group id
 * 
 * @param comm The parent communicator of the group
 * @param group_id The group that this rank on
 * @return commgroup_t The group handle
 * 
 * @note Use MPIz_commgroup_create_node(comm) to construct group based on node
 */
MYS_STATIC commgroup_t MPIz_commgroup_create(MPI_Comm global_comm, int group_id)
{
    _commgroup_struct_t *group = (_commgroup_struct_t *)malloc(sizeof(_commgroup_struct_t));
    MPI_Comm_dup(global_comm, &group->global_comm);
    group->group_id = group_id;
    MPI_Comm_size(group->global_comm, &group->global_nranks);
    MPI_Comm_rank(group->global_comm, &group->global_myrank);

    MPI_Comm_split(group->global_comm, group->group_id, group->global_myrank, &group->local_comm);
    MPI_Comm_size(group->local_comm, &group->local_nranks);
    MPI_Comm_rank(group->local_comm, &group->local_myrank);

    group->_rows = (int *)malloc(sizeof(int) * group->global_nranks);
    group->_rows[group->global_myrank] = group->group_id;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, group->_rows, 1, MPI_INT, group->global_comm);

    group->_cols = (int *)malloc(sizeof(int) * group->global_nranks);
    group->_cols[group->global_myrank] = group->local_myrank;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, group->_cols, 1, MPI_INT, group->global_comm);

    group->_brothers = (int *)malloc(sizeof(int) * group->local_nranks);
    group->_brothers[group->local_myrank] = group->global_myrank;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, group->_brothers, 1, MPI_INT, group->local_comm);

    return (commgroup_t)group;
}

/**
 * @brief Release communication group information
 * @param group The group handle
 */
MYS_STATIC void MPIz_commgroup_release(commgroup_t group)
{
    assert(group != NULL);
    if (group == COMMGROUP_NULL) return;
    MPI_Comm_free(&group->global_comm);
    MPI_Comm_free(&group->local_comm);
    free(group->_rows);
    free(group->_cols);
    free(group->_brothers);
}

//////

static int _MPIz_split_group_compare_rank(const void* a, const void* b)
{
    const int *intA = (const int *)a;
    const int *intB = (const int *)b;
    if (*intA < *intB) return -1;
    else if (*intA > *intB) return 1;
    else return 0;
}

/**
 * @brief Create communication group information based on node
 * @return commgroup_t The group handle
 */
MYS_STATIC commgroup_t MPIz_commgroup_create_node(MPI_Comm global_comm)
{
    int global_nranks, global_myrank;
    MPI_Comm_size(global_comm, &global_nranks);
    MPI_Comm_rank(global_comm, &global_myrank);

    MPI_Comm local_comm = MPI_COMM_NULL;
    MPI_Comm_split_type(global_comm, MPI_COMM_TYPE_SHARED, global_myrank, MPI_INFO_NULL, &local_comm);

    int local_nranks, local_myrank;
    MPI_Comm_size(local_comm, &local_nranks);
    MPI_Comm_rank(local_comm, &local_myrank);

    int group_num = 0;
    int group_id = -1;
    int im_group_root = local_myrank == 0;
    MPI_Allreduce(&im_group_root, &group_num, 1, MPI_INT, MPI_SUM, global_comm);
    int *roots = (int *)malloc(sizeof(int) * group_num);
    int nrequests = (global_myrank == 0) ? (group_num + 1) : (local_myrank == 0) ? 1 : 0;
    MPI_Request *requests = (MPI_Request *)malloc(sizeof(MPI_Request) * nrequests);

    if (global_myrank == 0) {
        for(int i = 0; i < group_num; i++)
            MPI_Irecv(&roots[i], 1, MPI_INT, MPI_ANY_SOURCE, 17749, global_comm, &requests[1 + i]);
    }
    if (local_myrank == 0) {
        MPI_Isend(&global_myrank, 1, MPI_INT, 0, 17749, global_comm, &requests[0]);
    }
    if (nrequests > 0) {
        MPI_Waitall(nrequests, requests, MPI_STATUSES_IGNORE);
    }

    if (global_myrank == 0) {
        qsort(roots, group_num, sizeof(int), _MPIz_split_group_compare_rank);
        for(int i = 0; i < group_num; i++) {
            int rank = roots[i];
            roots[i] = i;
            MPI_Isend(&roots[i], 1, MPI_INT, rank, 18864, global_comm, &requests[1 + i]);
        }
    }
    if (local_myrank == 0) {
        MPI_Irecv(&group_id, 1, MPI_INT, 0, 18864, global_comm, &requests[0]);
    }
    if (nrequests > 0) {
        MPI_Waitall(nrequests, requests, MPI_STATUSES_IGNORE);
    }

    MPI_Bcast(&group_id, 1, MPI_INT, 0, local_comm);

    free(requests);
    free(roots);
    MPI_Comm_free(&local_comm);
    return MPIz_commgroup_create(global_comm, group_id);
}

/**
 * @brief Query the group to which the rank belongs (Support querying for rank on differnt group)
 * @return int The group id
 */
MYS_STATIC int MPIz_query_group_id(commgroup_t group, int global_rank)
{
    return group->_rows[global_rank];
}

/**
 * @brief Query the local rank of the global rank (Support querying for rank on differnt group)
 * @return int The local rank
 */
MYS_STATIC int MPIz_query_local_rank(commgroup_t group, int global_rank)
{
    return group->_cols[global_rank];
}

/**
 * @brief Query the global rank of the local rank (ONLY support querying for rank on the same group)
 * @return int The global rank
 */
MYS_STATIC int MPIz_query_global_rank(commgroup_t group, int local_rank)
{
    return group->_brothers[local_rank];
}

/* mpicc -I${MYS_DIR}/include a.c && mpirun -n 4 ./a.out 1
#include <stdlib.h>
#include <stdio.h>

#define MYS_IMPL
#include <mys.h>

int main(int argc, char **argv)
{
    int who = 0;
    if (argc > 1) {
        who = atoi(argv[1]);
    }
    MPI_Init(NULL, NULL);
    int myrank, nranks;
    MPI_Comm_size(MPI_COMM_WORLD, &nranks);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    commgroup_t commgroup = COMMGROUP_NULL;
    // commgroup = MPIz_commgroup_create_node(MPI_COMM_WORLD);
    // if (myrank == 0) {
    //     commgroup = MPIz_commgroup_create(MPI_COMM_WORLD, 0);
    // } else {
    //     commgroup = MPIz_commgroup_create(MPI_COMM_WORLD, 1);
    // }
    commgroup = MPIz_commgroup_create(MPI_COMM_WORLD, myrank);

    if (commgroup->global_myrank == who) {
        printf("Rank %d has %d friends\n", commgroup->global_myrank, commgroup->local_nranks);
        printf("Node info:");
        for (int global_rank = 0; global_rank < commgroup->global_nranks; global_rank++) {
            int group_id = MPIz_query_group_id(commgroup, global_rank);
            printf(" %d", group_id);
        }
        printf("\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPIz_commgroup_release(commgroup);
    MPI_Finalize();
}
*/
