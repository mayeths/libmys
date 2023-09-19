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
    int group_id; // nonnegative counting sequence (0,1,2,3,...,group_num-1)
    int group_num;
    MPI_Comm global_comm;
    int global_myrank;
    int global_nranks;
    MPI_Comm local_comm;
    int local_myrank;
    int local_nranks;
    MPI_Comm inter_comm; // communicator for the same local_myrank among groups (ranks are ordered by group_id)
    int *_rows; // size=global_nranks. _rows[global_rank] is the group to which the rank belongs
    int *_cols; // size=global_nranks. _cols[global_rank] is the local_rank in group
    int *_brothers; // size=local_nranks. _brothers[local_rank] is the global_rank of each group_member in the same group
    int *_neighbors; // size=group_num. _neighbors[group_id] is the global_rank of each group member that has the same local_myrank to me
} _commgroup_struct_t;

typedef _commgroup_struct_t* commgroup_t; // handle
#define COMMGROUP_NULL ((commgroup_t) -1)

static int _MPIz_commgroup_rank_sortfn(const void* a, const void* b);

/**
 * @brief Create communication group information based on group color assignment
 * 
 * @param comm The parent communicator of the group
 * @param group_color Control of group assignment (nonnegative integer, can be discontinuous. `group_id` will follow the order of color).
 * @param group_key Control of rank in group assignment (integer, can be discontinuous).
 * @return The group handle
 * 
 * @note Use MPIz_commgroup_create_node(comm) to construct node based group.
 */
MYS_STATIC commgroup_t MPIz_commgroup_create(MPI_Comm global_comm, int group_color, int group_key)
{
    _commgroup_struct_t *group = (_commgroup_struct_t *)malloc(sizeof(_commgroup_struct_t));
    MPI_Comm_dup(global_comm, &group->global_comm);
    MPI_Comm_size(group->global_comm, &group->global_nranks);
    MPI_Comm_rank(group->global_comm, &group->global_myrank);

    MPI_Comm_split(group->global_comm, group_color, group_key, &group->local_comm);
    MPI_Comm_size(group->local_comm, &group->local_nranks);
    MPI_Comm_rank(group->local_comm, &group->local_myrank);

    group->group_num = 0;
    group->group_id = -1;
    int im_group_root = group->local_myrank == 0;
    MPI_Allreduce(&im_group_root, &group->group_num, 1, MPI_INT, MPI_SUM, global_comm);
    int *roots = (int *)malloc(sizeof(int) * group->group_num);
    int nrequests = (group->global_myrank == 0) ? (group->group_num + 1) : (group->local_myrank == 0) ? 1 : 0;
    MPI_Request *requests = (MPI_Request *)malloc(sizeof(MPI_Request) * nrequests);

    if (group->global_myrank == 0) {
        for(int i = 0; i < group->group_num; i++)
            MPI_Irecv(&roots[i], 1, MPI_INT, MPI_ANY_SOURCE, 17749, global_comm, &requests[1 + i]);
    }
    if (group->local_myrank == 0) {
        MPI_Isend(&group->global_myrank, 1, MPI_INT, 0, 17749, global_comm, &requests[0]);
    }
    if (nrequests > 0) {
        MPI_Waitall(nrequests, requests, MPI_STATUSES_IGNORE);
    }

    if (group->global_myrank == 0) {
        qsort(roots, group->group_num, sizeof(int), _MPIz_commgroup_rank_sortfn);
        for(int i = 0; i < group->group_num; i++) {
            int rank = roots[i];
            roots[i] = i;
            MPI_Isend(&roots[i], 1, MPI_INT, rank, 18864, global_comm, &requests[1 + i]);
        }
    }
    if (group->local_myrank == 0) {
        MPI_Irecv(&group->group_id, 1, MPI_INT, 0, 18864, global_comm, &requests[0]);
    }
    if (nrequests > 0) {
        MPI_Waitall(nrequests, requests, MPI_STATUSES_IGNORE);
    }

    MPI_Bcast(&group->group_id, 1, MPI_INT, 0, group->local_comm);

    group->_rows = (int *)malloc(sizeof(int) * group->global_nranks);
    group->_rows[group->global_myrank] = group->group_id;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, group->_rows, 1, MPI_INT, group->global_comm);

    group->_cols = (int *)malloc(sizeof(int) * group->global_nranks);
    group->_cols[group->global_myrank] = group->local_myrank;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, group->_cols, 1, MPI_INT, group->global_comm);

    group->_brothers = (int *)malloc(sizeof(int) * group->local_nranks);
    group->_brothers[group->local_myrank] = group->global_myrank;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, group->_brothers, 1, MPI_INT, group->local_comm);

    MPI_Comm_split(group->global_comm, group->local_myrank, group->group_id, &group->inter_comm);
    group->_neighbors = (int *)malloc(sizeof(int) * group->group_num);
    for (int i = 0; i < group->group_num; i++)
        group->_neighbors[i] = -1;
    group->_neighbors[group->group_id] = group->global_myrank;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, group->_neighbors, 1, MPI_INT, group->inter_comm);

    free(requests);
    free(roots);
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
    MPI_Comm_free(&group->inter_comm);
    free(group->_rows);
    free(group->_cols);
    free(group->_brothers);
    free(group->_neighbors);
}

/**
 * @brief Create communication group information based on node
 * @return The group handle
 */
MYS_STATIC commgroup_t MPIz_commgroup_create_node(MPI_Comm global_comm)
{
    int global_nranks, global_myrank;
    MPI_Comm_size(global_comm, &global_nranks);
    MPI_Comm_rank(global_comm, &global_myrank);

    MPI_Comm local_comm = MPI_COMM_NULL;
    int node_root = global_myrank;
    MPI_Comm_split_type(global_comm, MPI_COMM_TYPE_SHARED, global_myrank, MPI_INFO_NULL, &local_comm);
    MPI_Bcast(&node_root, 1, MPI_INT, 0, local_comm);
    MPI_Comm_free(&local_comm);

    return MPIz_commgroup_create(global_comm, node_root, global_myrank);
}

/**
 * @brief Query the group to which the rank belongs (Support querying rank in other group)
 * @param global_rank The global rank to be query
 * @return The group id. -1 if provided invalid global_rank
 */
MYS_STATIC int MPIz_query_group_id(commgroup_t group, int global_rank)
{
    if (global_rank < 0 || global_rank >= group->global_nranks)
        return -1;
    else
        return group->_rows[global_rank];
}

/**
 * @brief Query the local rank of the global rank (Support querying rank in other group)
 * @param global_rank The global rank to be query
 * @return The local rank. -1 if provided invalid global_rank
 */
MYS_STATIC int MPIz_query_local_rank(commgroup_t group, int global_rank)
{
    if (global_rank < 0 || global_rank >= group->global_nranks)
        return -1;
    else
        return group->_cols[global_rank];
}

/**
 * @brief Query the global rank of the local rank (ONLY support querying rank in the same group)
 * @param local_rank The local rank to be query
 * @return The global rank. -1 if provided invalid local_rank
 */
MYS_STATIC int MPIz_query_brother(commgroup_t group, int local_rank)
{
    if (local_rank < 0 || local_rank >= group->local_nranks)
        return -1;
    else
        return group->_brothers[local_rank];
}

/**
 * @brief Query the global rank of each group member that has the same `local_myrank` (Support querying rank in other group)
 * @param group_id The groud id to be query
 * @return The global rank of group member. -1 if provided invalid local_rank,
 *      or that group doesn't has corresponding neighbor that has the same `local_myrank`
 */
MYS_STATIC int MPIz_query_neighbor(commgroup_t group, int group_id)
{
    if (group_id < 0 || group_id >= group->group_num)
        return -1;
    else
        return group->_neighbors[group_id];
}


/////////////////////////////////////////////


static int _MPIz_commgroup_rank_sortfn(const void* a, const void* b)
{
    const int *intA = (const int *)a;
    const int *intB = (const int *)b;
    if (*intA < *intB) return -1;
    else if (*intA > *intB) return 1;
    else return 0;
}

/* mpicc -I${MYS_DIR}/include a.c && mpirun -n 5 ./a.out 3
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
    // commgroup = MPIz_commgroup_create(MPI_COMM_WORLD, myrank, myrank);
    // if (myrank == 0) {
    //     commgroup = MPIz_commgroup_create(MPI_COMM_WORLD, 0, myrank);
    // } else {
    //     commgroup = MPIz_commgroup_create(MPI_COMM_WORLD, 1, myrank);
    // }
    commgroup = MPIz_commgroup_create(MPI_COMM_WORLD, myrank < nranks / 2, myrank);

    if (commgroup->global_myrank == who) {
        printf("Rank %d has %d friends\n", commgroup->global_myrank, commgroup->local_nranks);
        printf("Node info:");
        for (int global_rank = 0; global_rank < commgroup->global_nranks; global_rank++) {
            int group_id = MPIz_query_group_id(commgroup, global_rank);
            printf(" %d", group_id);
        }
        printf("\n");
        for (int group_id = 0; group_id < commgroup->group_num; group_id++) {
            int neighbor = MPIz_query_neighbor(commgroup, group_id);
            printf("Group %d neighbor is %d\n", group_id, neighbor);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    MPIz_commgroup_release(commgroup);
    MPI_Finalize();
}
*/
