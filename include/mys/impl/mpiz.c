#include "_private.h"
#include "../mpiz.h"

#ifdef MYS_ENABLE_NUMA
#include <numa.h>
#endif

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

MYS_API mys_commgroup_t *mys_commgroup_create(MPI_Comm global_comm, int group_color, int group_key)
{
    mys_commgroup_t *group = (mys_commgroup_t *)malloc(sizeof(mys_commgroup_t));
    group->global_comm = global_comm;
    MPI_Comm_size(group->global_comm, &group->global_nranks);
    MPI_Comm_rank(group->global_comm, &group->global_myrank);
    MPI_Comm_split(group->global_comm, group_color, group_key, &group->local_comm);
    MPI_Comm_size(group->local_comm, &group->local_nranks);
    MPI_Comm_rank(group->local_comm, &group->local_myrank);
    MPI_Comm_split(group->global_comm, group->local_myrank, group_color, &group->inter_comm); // overhead for so many comm?
    int iam_group_root = group->local_myrank == 0;
    MPI_Allreduce(&iam_group_root, &group->group_num, 1, MPI_INT, MPI_SUM, global_comm);
    // MPI_Comm_split(group->global_comm, iam_group_root ? 0 : 1, group_color, &group->inter_comm);
    // if (!iam_group_root) MPI_Comm_free(&group->inter_comm);

    int *temp = (int *)malloc(sizeof(int) * 4 * group->global_nranks);
    temp[group->global_myrank * 4 + 0] = group_color;
    temp[group->global_myrank * 4 + 1] = group->local_myrank;
    temp[group->global_myrank * 4 + 2] = group->global_myrank;
    temp[group->global_myrank * 4 + 3] = 0; // dummy
    MPI_Allgather(MPI_IN_PLACE, 4, MPI_INT, temp, 4, MPI_INT, global_comm);
    qsort(temp, group->global_nranks, sizeof(int[4]), _mys_commgroup_sort_i4);
    group->_neighbors = (int *)malloc(sizeof(int) * group->group_num);
    group->_rows = (int *)malloc(sizeof(int) * group->global_nranks);
    group->_cols = (int *)malloc(sizeof(int) * group->global_nranks);
    group->_brothers = (int *)malloc(sizeof(int) * group->local_nranks);
    int group_index = -1;
    int last_color = -1;
    int neighbor_cnt = 0;
    int brother_cnt = 0;
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
            group->_neighbors[neighbor_cnt++] = o_global_rank;
        }
        group->_rows[o_global_rank] = group_index;
        group->_cols[o_global_rank] = o_local_rank;
        if (o_group_color == group_color) {
            group->_brothers[brother_cnt++] = o_global_rank;
        }
    }
    free(temp);

    return group;
}

MYS_API void mys_commgroup_release(mys_commgroup_t *group)
{
    assert(group != NULL);
    if (group == NULL) return;
    MPI_Comm_free(&group->local_comm);
    MPI_Comm_free(&group->inter_comm);
    free(group->_rows);
    free(group->_cols);
    free(group->_brothers);
    free(group->_neighbors);
}

MYS_API mys_commgroup_t *mys_commgroup_create_node(MPI_Comm global_comm)
{
    int global_nranks, global_myrank;
    MPI_Comm_size(global_comm, &global_nranks);
    MPI_Comm_rank(global_comm, &global_myrank);

    MPI_Comm local_comm = MPI_COMM_NULL;
    int node_root = global_myrank;
    MPI_Comm_split_type(global_comm, MPI_COMM_TYPE_SHARED, global_myrank, MPI_INFO_NULL, &local_comm);
    MPI_Bcast(&node_root, 1, MPI_INT, 0, local_comm);
    MPI_Comm_free(&local_comm);

    return mys_commgroup_create(global_comm, node_root, global_myrank);
}

#ifdef MYS_ENABLE_NUMA
MYS_API mys_commgroup_t *mys_commgroup_create_numa(MPI_Comm global_comm)
{
    int global_nranks, global_myrank;
    MPI_Comm_size(global_comm, &global_nranks);
    MPI_Comm_rank(global_comm, &global_myrank);

    int cpu = sched_getcpu();
    int numa = numa_node_of_cpu(cpu);
    int numa_num = numa_num_configured_nodes();
    if (cpu == -1 || numa == -1 || numa_num == -1)
        return NULL;

    MPI_Comm local_comm = MPI_COMM_NULL;
    MPI_Comm_split_type(global_comm, MPI_COMM_TYPE_SHARED, global_myrank, MPI_INFO_NULL, &local_comm);
    int node_id = global_myrank;
    MPI_Bcast(&node_id, 1, MPI_INT, 0, local_comm);
    MPI_Comm_free(&local_comm);

    int group_color = node_id * numa_num + numa;
    int group_key = cpu;
    return mys_commgroup_create(global_comm, group_color, group_key);
}
#endif

MYS_API int mys_query_group_id(mys_commgroup_t *group, int global_rank)
{
    if (global_rank < 0 || global_rank >= group->global_nranks)
        return -1;
    else
        return group->_rows[global_rank];
}

MYS_API int mys_query_local_rank(mys_commgroup_t *group, int global_rank)
{
    if (global_rank < 0 || global_rank >= group->global_nranks)
        return -1;
    else
        return group->_cols[global_rank];
}

MYS_API int mys_query_brother(mys_commgroup_t *group, int local_rank)
{
    if (local_rank < 0 || local_rank >= group->local_nranks)
        return -1;
    else
        return group->_brothers[local_rank];
}

MYS_API int mys_query_neighbor(mys_commgroup_t *group, int group_id)
{
    if (group_id < 0 || group_id >= group->group_num)
        return -1;
    else
        return group->_neighbors[group_id];
}
