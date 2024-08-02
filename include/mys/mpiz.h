#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <mpi.h>
#include <unistd.h>
#include <sched.h>

#include "_config.h"

typedef struct mys_commgroup_t
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
} mys_commgroup_t;

/**
 * @brief Create communication group information based on group color assignment
 * 
 * @param comm The parent communicator of the group
 * @param group_color Control of group assignment (nonnegative integer, can be discontinuous. `group_id` will follow the order of color).
 * @param group_key Control of rank in group assignment (integer, can be discontinuous).
 * @return The group handle
 * 
 * @note Use mys_commgroup_create_node(comm) to construct node based group.
 */
MYS_PUBLIC mys_commgroup_t *mys_commgroup_create(MPI_Comm global_comm, int group_color, int group_key);
/**
 * @brief Create communication group information based on node
 * @return The group handle
 */
MYS_PUBLIC mys_commgroup_t *mys_commgroup_create_node(MPI_Comm global_comm);
/**
 * @brief Create communication group information based on node
 * @return The group handle
 * 
 * @note This routine require MYS_ENABLE_NUMA.
 */
MYS_PUBLIC mys_commgroup_t *mys_commgroup_create_numa(MPI_Comm global_comm);
/**
 * @brief Release communication group information
 * @param group The group handle
 */
MYS_PUBLIC void mys_commgroup_release(mys_commgroup_t *group);
/**
 * @brief Query the group to which the rank belongs (Support querying rank in other group)
 * @param global_rank The global rank to be query
 * @return The group id. -1 if provided invalid global_rank
 */
MYS_PUBLIC int mys_query_group_id(mys_commgroup_t *group, int global_rank);
/**
 * @brief Query the local rank of the global rank (Support querying rank in other group)
 * @param global_rank The global rank to be query
 * @return The local rank. -1 if provided invalid global_rank
 */
MYS_PUBLIC int mys_query_local_rank(mys_commgroup_t *group, int global_rank);
/**
 * @brief Query the global rank of the local rank (ONLY support querying rank in the same group)
 * @param local_rank The local rank to be query
 * @return The global rank. -1 if provided invalid local_rank
 */
MYS_PUBLIC int mys_query_brother(mys_commgroup_t *group, int local_rank);
/**
 * @brief Query the global rank of each group member that has the same `local_myrank` (Support querying rank in other group)
 * @param group_id The groud id to be query
 * @return The global rank of group member. -1 if provided invalid local_rank,
 *      or that group doesn't has corresponding neighbor that has the same `local_myrank`
 */
MYS_PUBLIC int mys_query_neighbor(mys_commgroup_t *group, int group_id);

/* mpicc -I${MYS_DIR}/include a.c && mpirun -n 5 ./a.out 3
#include <stdlib.h>
#include <stdio.h>

#define MYS_IMPL
#include <mys.h>
#include <mpi.h>

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

    mys_commgroup_t *commgroup = NULL;
    // commgroup = mys_commgroup_create_node(MPI_COMM_WORLD);
    // commgroup = mys_commgroup_create(MPI_COMM_WORLD, myrank, myrank);
    // if (myrank == 0) {
    //     commgroup = mys_commgroup_create(MPI_COMM_WORLD, 0, myrank);
    // } else {
    //     commgroup = mys_commgroup_create(MPI_COMM_WORLD, 1, myrank);
    // }
    commgroup = mys_commgroup_create(MPI_COMM_WORLD, myrank < nranks / 2, myrank);

    if (commgroup->global_myrank == who) {
        printf("Rank %d has %d friends\n", commgroup->global_myrank, commgroup->local_nranks);
        printf("Node info:");
        for (int global_rank = 0; global_rank < commgroup->global_nranks; global_rank++) {
            int group_id = mys_query_group_id(commgroup, global_rank);
            printf(" %d", group_id);
        }
        printf("\n");
        for (int group_id = 0; group_id < commgroup->group_num; group_id++) {
            int neighbor = mys_query_neighbor(commgroup, group_id);
            printf("Group %d neighbor is %d\n", group_id, neighbor);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    mys_commgroup_release(commgroup);
    MPI_Finalize();
}
*/
