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
#include "../_config.h"
#include "../errno.h"
#include "../mpistubs.h"
#include "../commgroup.h"
#include "../memory.h"

#ifdef MYS_ENABLE_NUMA
#include <numa.h>
#endif

static int _mys_commgroup_sort_str(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

#define _MAX_HOSTNAME_LEN 256
static void _mys_commgroup_get_rank2node(mys_MPI_Comm comm, int *rank2node)
{
    int myrank, nranks;
    char hostname[_MAX_HOSTNAME_LEN];

    mys_MPI_Comm_rank(comm, &myrank);
    mys_MPI_Comm_size(comm, &nranks);

    gethostname(hostname, _MAX_HOSTNAME_LEN);

    char *all_hostnames = (char *)mys_calloc2(MYS_ARENA_COMMGROUP, _MAX_HOSTNAME_LEN, nranks);
    mys_MPI_Allgather(hostname, _MAX_HOSTNAME_LEN, mys_MPI_CHAR, all_hostnames, _MAX_HOSTNAME_LEN, mys_MPI_CHAR, comm);

    char **unique_names = (char **)mys_calloc2(MYS_ARENA_COMMGROUP, sizeof(char *), nranks);
    int unique_count = 0;

    for (int i = 0; i < nranks; ++i) {
        char *current_hostname = all_hostnames + i * _MAX_HOSTNAME_LEN;
        int found = 0;
        for (int j = 0; j < unique_count; ++j) {
            if (strcmp(unique_names[j], current_hostname) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            unique_names[unique_count] = (char *)mys_calloc2(MYS_ARENA_COMMGROUP, _MAX_HOSTNAME_LEN, 1);
            strcpy(unique_names[unique_count], current_hostname);
            unique_count++;
        }
    }

    qsort(unique_names, unique_count, sizeof(char *), _mys_commgroup_sort_str);

    int *name2node = (int *)mys_calloc2(MYS_ARENA_COMMGROUP, sizeof(int), nranks);
    for (int i = 0; i < unique_count; ++i) {
        name2node[i] = i;
    }

    int mynode = -1;
    for (int j = 0; j < unique_count; ++j) {
        if (strcmp(unique_names[j], hostname) == 0) {
            mynode = name2node[j];
            break;
        }
    }
    mys_MPI_Allgather(&mynode, 1, mys_MPI_INT, rank2node, 1, mys_MPI_INT, comm);

    for (int i = 0; i < unique_count; ++i) {
        mys_free2(MYS_ARENA_COMMGROUP, unique_names[i], _MAX_HOSTNAME_LEN);
    }
    mys_free2(MYS_ARENA_COMMGROUP, unique_names, sizeof(char *) * nranks);
    mys_free2(MYS_ARENA_COMMGROUP, all_hostnames, _MAX_HOSTNAME_LEN * nranks);
    mys_free2(MYS_ARENA_COMMGROUP, name2node, sizeof(int) * nranks);
}
#undef _MAX_HOSTNAME_LEN

static int _mys_commgroup_sort_i4(const void* _a, const void* _b)
{
    const int *a = (const int *)_a;
    const int *b = (const int *)_b;
    if (a[0] < b[0]) return -1;
    else if (a[0] > b[0]) return 1;
    else if (a[1] < b[1]) return -1;
    else if (a[1] > b[1]) return 1;
    else if (a[2] < b[2]) return -1;
    else if (a[2] > b[2]) return 1;
    else if (a[3] < b[3]) return -1;
    else if (a[3] > b[3]) return 1;
    else return 0;
}

MYS_PUBLIC mys_commgroup_t *mys_commgroup_create(mys_MPI_Comm global_comm, int group_color, int group_key)
{
    AS_GE_INT(group_color, 0);
    mys_commgroup_t *group = (mys_commgroup_t *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(mys_commgroup_t));
    group->global_comm = global_comm;
    mys_MPI_Comm_size(group->global_comm, &group->global_nranks);
    mys_MPI_Comm_rank(group->global_comm, &group->global_myrank);
    mys_MPI_Comm_split(group->global_comm, group_color, group_key, &group->local_comm);
    mys_MPI_Comm_size(group->local_comm, &group->local_nranks);
    mys_MPI_Comm_rank(group->local_comm, &group->local_myrank);
    mys_MPI_Comm_split(group->global_comm, group->local_myrank, group_color, &group->inter_comm); // overhead for so many comm?
    int iam_group_root = group->local_myrank == 0;
    mys_MPI_Allreduce(&iam_group_root, &group->group_num, 1, mys_MPI_INT, mys_MPI_SUM, global_comm);
    // mys_MPI_Comm_split(group->global_comm, iam_group_root ? 0 : 1, group_color, &group->inter_comm);
    // if (!iam_group_root) mys_MPI_Comm_free(&group->inter_comm);

    int *temp = (int *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(int) * 4 * group->global_nranks);
    temp[group->global_myrank * 4 + 0] = group_color;
    temp[group->global_myrank * 4 + 1] = group->local_myrank;
    temp[group->global_myrank * 4 + 2] = group->global_myrank;
    temp[group->global_myrank * 4 + 3] = 0; // dummy
    mys_MPI_Allgather(mys_MPI_IN_PLACE, 4, mys_MPI_INT, temp, 4, mys_MPI_INT, global_comm);
    qsort(temp, group->global_nranks, sizeof(int[4]), _mys_commgroup_sort_i4);
    group->_neighbors = (int *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(int) * group->group_num);
    group->_group_sizes = (int *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(int) * group->group_num);
    group->_group_ids = (int *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(int) * group->global_nranks);
    group->_local_ranks = (int *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(int) * group->global_nranks);
    group->_brothers = (int *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(int) * group->local_nranks);
    for (int i = 0; i < group->group_num; i++) group->_neighbors[i] = -1;
    for (int i = 0; i < group->global_nranks; i++) group->_group_ids[i] = -1;
    for (int i = 0; i < group->global_nranks; i++) group->_local_ranks[i] = -1;
    for (int i = 0; i < group->local_nranks; i++) group->_brothers[i] = -1;
    for (int i = 0; i < group->group_num; i++) group->_group_sizes[i] = 0;

    int group_index = -1;
    int last_color = -1;
    for(int i = 0; i < group->global_nranks; i++) {
        int o_group_color = temp[i * 4 + 0];
        int o_local_rank  = temp[i * 4 + 1];
        int o_global_rank = temp[i * 4 + 2];
        if (o_group_color != last_color) {
            group_index += 1;
            last_color = o_group_color;
        }
        if (o_global_rank == group->global_myrank) {
            group->group_id = group_index;
        }
        if (o_local_rank == group->local_myrank) {
            group->_neighbors[group_index] = o_global_rank;
        }
        group->_group_ids[o_global_rank] = group_index;
        group->_local_ranks[o_global_rank] = o_local_rank;
        group->_group_sizes[group_index] += 1;
        if (o_group_color == group_color) {
            group->_brothers[o_local_rank] = o_global_rank;
        }
    }
    mys_free2(MYS_ARENA_COMMGROUP, temp, sizeof(int) * 4 * group->global_nranks);

    return group;
}

MYS_PUBLIC void mys_commgroup_release(mys_commgroup_t *group)
{
    assert(group != NULL);
    if (group == NULL) return;
    mys_MPI_Comm_free(&group->local_comm);
    mys_MPI_Comm_free(&group->inter_comm);
    mys_free2(MYS_ARENA_COMMGROUP, group->_neighbors, sizeof(int) * group->group_num);
    mys_free2(MYS_ARENA_COMMGROUP, group->_group_sizes, sizeof(int) * group->group_num);
    mys_free2(MYS_ARENA_COMMGROUP, group->_group_ids, sizeof(int) * group->global_nranks);
    mys_free2(MYS_ARENA_COMMGROUP, group->_local_ranks, sizeof(int) * group->global_nranks);
    mys_free2(MYS_ARENA_COMMGROUP, group->_brothers, sizeof(int) * group->local_nranks);
    mys_free2(MYS_ARENA_COMMGROUP, group, sizeof(mys_commgroup_t));
}

MYS_PUBLIC mys_commgroup_t *mys_commgroup_create_node(mys_MPI_Comm global_comm)
{
    int global_nranks, global_myrank;
    mys_MPI_Comm_size(global_comm, &global_nranks);
    mys_MPI_Comm_rank(global_comm, &global_myrank);
#if 1
    int *rank2node = (int *)mys_calloc2(MYS_ARENA_COMMGROUP, sizeof(int), global_nranks);
    _mys_commgroup_get_rank2node(global_comm, rank2node);
    mys_commgroup_t *node_group = mys_commgroup_create(global_comm, rank2node[global_myrank], global_myrank);
    mys_free2(MYS_ARENA_COMMGROUP, rank2node, sizeof(int) * global_nranks);
    return node_group;
#else
    mys_MPI_Comm local_comm = mys_MPI_COMM_NULL;
    int node_root = global_myrank;
    mys_MPI_Comm_split_type(global_comm, mys_MPI_COMM_TYPE_SHARED, global_myrank, mys_MPI_INFO_NULL, &local_comm);
    mys_MPI_Bcast(&node_root, 1, mys_MPI_INT, 0, local_comm);
    mys_MPI_Comm_free(&local_comm);

    return mys_commgroup_create(global_comm, node_root, global_myrank);
#endif
}

#ifdef MYS_ENABLE_NUMA
MYS_PUBLIC mys_commgroup_t *mys_commgroup_create_numa(mys_MPI_Comm global_comm)
{
    int global_nranks, global_myrank;
    mys_MPI_Comm_size(global_comm, &global_nranks);
    mys_MPI_Comm_rank(global_comm, &global_myrank);

    int cpu = sched_getcpu();
    int numa = numa_node_of_cpu(cpu);
    int numa_num = numa_num_configured_nodes();
    if (cpu == -1 || numa == -1 || numa_num == -1)
        return NULL;

    mys_MPI_Comm local_comm = mys_MPI_COMM_NULL;
    mys_MPI_Comm_split_type(global_comm, mys_MPI_COMM_TYPE_SHARED, global_myrank, mys_MPI_INFO_NULL, &local_comm);
    int node_id = global_myrank;
    mys_MPI_Bcast(&node_id, 1, mys_MPI_INT, 0, local_comm);
    mys_MPI_Comm_free(&local_comm);

    int group_color = node_id * numa_num + numa;
    int group_key = cpu;
    return mys_commgroup_create(global_comm, group_color, group_key);
}
#endif

MYS_PUBLIC mys_commgroup_t *mys_commgroup_dup(mys_commgroup_t *g)
{
    mys_commgroup_t *group = (mys_commgroup_t *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(mys_commgroup_t));
    group->group_id = g->group_id;
    group->group_num = g->group_num;
    group->global_comm = g->global_comm;
    group->global_myrank = g->global_myrank;
    group->global_nranks = g->global_nranks;
    mys_MPI_Comm_dup(g->local_comm, &group->local_comm);
    group->local_myrank = g->local_myrank;
    group->local_nranks = g->local_nranks;
    mys_MPI_Comm_dup(g->inter_comm, &group->inter_comm);
    group->_group_ids = (int *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(int) * group->global_nranks);
    group->_local_ranks = (int *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(int) * group->global_nranks);
    group->_brothers = (int *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(int) * group->local_nranks);
    group->_neighbors = (int *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(int) * group->group_num);
    group->_group_sizes = (int *)mys_malloc2(MYS_ARENA_COMMGROUP, sizeof(int) * group->group_num);
    for (int i = 0; i < group->global_nranks; i++) group->_group_ids[i] = g->_group_ids[i];
    for (int i = 0; i < group->global_nranks; i++) group->_local_ranks[i] = g->_local_ranks[i];
    for (int i = 0; i < group->local_nranks; i++) group->_brothers[i] = g->_brothers[i];
    for (int i = 0; i < group->group_num; i++) group->_neighbors[i] = g->_neighbors[i];
    for (int i = 0; i < group->group_num; i++) group->_group_sizes[i] = g->_group_sizes[i];

    return group;
}


MYS_PUBLIC int mys_query_group_id(mys_commgroup_t *group, int global_rank)
{
    if (global_rank < 0 || global_rank >= group->global_nranks)
        return -1;
    else
        return group->_group_ids[global_rank];
}

MYS_PUBLIC int mys_query_local_rank(mys_commgroup_t *group, int global_rank)
{
    if (global_rank < 0 || global_rank >= group->global_nranks)
        return -1;
    else
        return group->_local_ranks[global_rank];
}

MYS_PUBLIC int mys_query_brother(mys_commgroup_t *group, int local_rank)
{
    if (local_rank < 0 || local_rank >= group->local_nranks)
        return -1;
    else
        return group->_brothers[local_rank];
}

MYS_PUBLIC int mys_query_neighbor(mys_commgroup_t *group, int group_id)
{
    if (group_id < 0 || group_id >= group->group_num)
        return -1;
    else
        return group->_neighbors[group_id];
}

MYS_PUBLIC int mys_query_group_size(mys_commgroup_t *group, int group_id)
{
    if (group_id < 0 || group_id >= group->group_num)
        return -1;
    else
        return group->_group_sizes[group_id];
}
