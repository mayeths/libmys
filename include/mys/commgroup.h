/* 
 * Copyright (c) 2025 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#pragma once

#include "_config.h"
#include "mpistubs.h"
#include "require.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>

typedef struct mys_commgroup_t
{
    int group_id; // nonnegative counting sequence (0,1,2,3,...,group_num-1)
    int group_num;
    mys_MPI_Comm global_comm;
    int global_myrank;
    int global_nranks;
    mys_MPI_Comm local_comm;
    int local_myrank;
    int local_nranks;
    mys_MPI_Comm inter_comm; // communicator for the same local_myrank among groups (ranks are ordered by group_id)
    int *_group_ids; // size=global_nranks. _group_ids[global_rank] is the group to which the rank belongs
    int *_local_ranks; // size=global_nranks. _local_ranks[global_rank] is the local_rank in group
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
MYS_PUBLIC mys_commgroup_t *mys_commgroup_create(mys_MPI_Comm global_comm, int group_color, int group_key);
/**
 * @brief Create communication group information based on node
 * @return The group handle
 */
MYS_PUBLIC mys_commgroup_t *mys_commgroup_create_node(mys_MPI_Comm global_comm);
/**
 * @brief Create communication group information based on node
 * @return The group handle
 * 
 * @note This routine require MYS_ENABLE_NUMA.
 */
#ifdef MYS_ENABLE_NUMA
MYS_PUBLIC mys_commgroup_t *mys_commgroup_create_numa(mys_MPI_Comm global_comm);
#else
#define mys_commgroup_create_numa(global_comm) mys_numa_capability_is_not_available("mys_commgroup_create_numa", __FILE__, __LINE__)
#endif
/**
 * @brief Duplicate a commgroup
 * @return The group handle
 */
MYS_PUBLIC mys_commgroup_t *mys_commgroup_dup(mys_commgroup_t *group);
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

/*
build: mpicc -I${MYS_DIR}/include a.c -lnuma -lrt
run: salloc -w kp101 --exclusive mpirun -n 9 -rf ./a.rf ./a.out
a.rf:
rank 0=kp101 slot=0
rank 1=kp101 slot=1
rank 2=kp101 slot=2
rank 3=kp101 slot=35
rank 4=kp101 slot=100
rank 5=kp101 slot=96
rank 6=kp101 slot=97
rank 7=kp101 slot=98
rank 8=kp101 slot=99
----------------- code
#define MYS_IMPL
#define MYS_ENABLE_NUMA
#include <mys.h>
#include <mpi.h>

#include <stdlib.h>
#include <stdio.h>

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
    commgroup = mys_commgroup_create_numa(MPI_COMM_WORLD);
    // commgroup = mys_commgroup_create(MPI_COMM_WORLD, myrank, myrank);
    // if (myrank == 0) {
    //     commgroup = mys_commgroup_create(MPI_COMM_WORLD, 0, myrank);
    // } else {
    //     commgroup = mys_commgroup_create(MPI_COMM_WORLD, 1, myrank);
    // }
    // commgroup = mys_commgroup_create(MPI_COMM_WORLD, myrank < nranks / 2, myrank);

    {
        mys_string_t *local_rank_str = mys_string_create();
        mys_string_fmt(local_rank_str, "query all ranks' local rank:");
        for (int global_rank = 0; global_rank < commgroup->global_nranks; global_rank++) {
            int local_rank = mys_query_local_rank(commgroup, global_rank);
            mys_string_fmt(local_rank_str, " %d", local_rank);
        }
        ILOG_ORDERED("%s", local_rank_str->text);
        mys_string_destroy(&local_rank_str);
    }

    {
        mys_string_t *group_id_str = mys_string_create();
        mys_string_fmt(group_id_str, "query all ranks' group id:");
        for (int global_rank = 0; global_rank < commgroup->global_nranks; global_rank++) {
            int group_id = mys_query_group_id(commgroup, global_rank);
            mys_string_fmt(group_id_str, " %d", group_id);
        }
        ILOG_ORDERED("%s", group_id_str->text);
        mys_string_destroy(&group_id_str);
    }

    {
        mys_string_t *neighbor_str = mys_string_create();
        mys_string_fmt(neighbor_str, "query neighbors' global rank:");
        for (int group_id = 0; group_id < commgroup->group_num; group_id++) {
            int neighbor = mys_query_neighbor(commgroup, group_id);
            mys_string_fmt(neighbor_str, " %d", neighbor);
        }
        ILOG_ORDERED("%s", neighbor_str->text);
        mys_string_destroy(&neighbor_str);
    }

    {
        mys_string_t *brother_str = mys_string_create();
        mys_string_fmt(brother_str, "mys query brothers' global rank:");
        for (int local_rank = 0; local_rank < commgroup->local_nranks; local_rank++) {
            int global_rank = mys_query_brother(commgroup, local_rank);
            mys_string_fmt(brother_str, " %d", global_rank);
        }
        ILOG_ORDERED("%s", brother_str->text);
        mys_string_destroy(&brother_str);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    mys_commgroup_release(commgroup);
    MPI_Finalize();
}
----------------- example output
[I::000 a.c:039] query all ranks' local rank: 0 1 2 0 4 0 1 2 3
[I::001 a.c:039] query all ranks' local rank: 0 1 2 0 4 0 1 2 3
[I::002 a.c:039] query all ranks' local rank: 0 1 2 0 4 0 1 2 3
[I::003 a.c:039] query all ranks' local rank: 0 1 2 0 4 0 1 2 3
[I::004 a.c:039] query all ranks' local rank: 0 1 2 0 4 0 1 2 3
[I::005 a.c:039] query all ranks' local rank: 0 1 2 0 4 0 1 2 3
[I::006 a.c:039] query all ranks' local rank: 0 1 2 0 4 0 1 2 3
[I::007 a.c:039] query all ranks' local rank: 0 1 2 0 4 0 1 2 3
[I::008 a.c:039] query all ranks' local rank: 0 1 2 0 4 0 1 2 3
[I::000 a.c:050] query all ranks' group id: 0 0 0 1 2 2 2 2 2
[I::001 a.c:050] query all ranks' group id: 0 0 0 1 2 2 2 2 2
[I::002 a.c:050] query all ranks' group id: 0 0 0 1 2 2 2 2 2
[I::003 a.c:050] query all ranks' group id: 0 0 0 1 2 2 2 2 2
[I::004 a.c:050] query all ranks' group id: 0 0 0 1 2 2 2 2 2
[I::005 a.c:050] query all ranks' group id: 0 0 0 1 2 2 2 2 2
[I::006 a.c:050] query all ranks' group id: 0 0 0 1 2 2 2 2 2
[I::007 a.c:050] query all ranks' group id: 0 0 0 1 2 2 2 2 2
[I::008 a.c:050] query all ranks' group id: 0 0 0 1 2 2 2 2 2
[I::000 a.c:061] query neighbors' global rank: 0 3 5
[I::001 a.c:061] query neighbors' global rank: 1 -1 6
[I::002 a.c:061] query neighbors' global rank: 2 -1 7
[I::003 a.c:061] query neighbors' global rank: 0 3 5
[I::004 a.c:061] query neighbors' global rank: -1 -1 4
[I::005 a.c:061] query neighbors' global rank: 0 3 5
[I::006 a.c:061] query neighbors' global rank: 1 -1 6
[I::007 a.c:061] query neighbors' global rank: 2 -1 7
[I::008 a.c:061] query neighbors' global rank: -1 -1 8
[I::000 a.c:072] mys query brothers' global rank: 0 1 2
[I::001 a.c:072] mys query brothers' global rank: 0 1 2
[I::002 a.c:072] mys query brothers' global rank: 0 1 2
[I::003 a.c:072] mys query brothers' global rank: 3
[I::004 a.c:072] mys query brothers' global rank: 5 6 7 8 4
[I::005 a.c:072] mys query brothers' global rank: 5 6 7 8 4
[I::006 a.c:072] mys query brothers' global rank: 5 6 7 8 4
[I::007 a.c:072] mys query brothers' global rank: 5 6 7 8 4
[I::008 a.c:072] mys query brothers' global rank: 5 6 7 8 4
*/
