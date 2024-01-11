#include "../mpiz.h"

struct _commgroup_rc_t {
    int rank;
    int color;
};

static int _commgroup_sortfn(const void* a, const void* b)
{
    const struct _commgroup_rc_t *rc_a = (const struct _commgroup_rc_t *)a;
    const struct _commgroup_rc_t *rc_b = (const struct _commgroup_rc_t *)b;
    if (rc_a->color < rc_b->color) return -1;
    else if (rc_a->color > rc_b->color) return 1;
    else return 0;
}

MYS_API mys_commgroup_t mys_commgroup_create(MPI_Comm global_comm, int group_color, int group_key)
{
    struct _mys_commgroup_t *group = (struct _mys_commgroup_t *)malloc(
        sizeof(struct _mys_commgroup_t)
    );
    group->global_comm = global_comm;
    MPI_Comm_size(group->global_comm, &group->global_nranks);
    MPI_Comm_rank(group->global_comm, &group->global_myrank);

    MPI_Comm_split(group->global_comm, group_color, group_key, &group->local_comm);
    MPI_Comm_size(group->local_comm, &group->local_nranks);
    MPI_Comm_rank(group->local_comm, &group->local_myrank);

    group->group_num = 0;
    group->group_id = -1;
    int im_group_root = group->local_myrank == 0;
    MPI_Allreduce(&im_group_root, &group->group_num, 1, MPI_INT, MPI_SUM, global_comm);
    int nrequests = (group->global_myrank == 0) ? (group->group_num + 1) : (group->local_myrank == 0) ? 1 : 0;
    MPI_Request *requests = (MPI_Request *)malloc(sizeof(MPI_Request) * nrequests);
    struct _commgroup_rc_t *rank_colors = (struct _commgroup_rc_t *)malloc(
        sizeof(struct _commgroup_rc_t) * group->group_num
    );
    struct _commgroup_rc_t my_rank_color = {
        .rank = group->global_myrank,
        .color = group_color
    };

    if (group->global_myrank == 0) {
        for(int i = 0; i < group->group_num; i++)
            MPI_Irecv(&rank_colors[i], 1, MPI_2INT, MPI_ANY_SOURCE, 17749, global_comm, &requests[1 + i]);
    }
    if (group->local_myrank == 0) {
        MPI_Isend(&my_rank_color, 1, MPI_2INT, 0, 17749, global_comm, &requests[0]);
    }
    if (nrequests > 0) {
        MPI_Waitall(nrequests, requests, MPI_STATUSES_IGNORE);
    }

    if (group->global_myrank == 0) {
        // convert discrete color to continuous group id
        qsort(rank_colors, group->group_num, sizeof(struct _commgroup_rc_t), _commgroup_sortfn);
        for(int group_id = 0; group_id < group->group_num; group_id++) {
            int rank = rank_colors[group_id].rank;
            rank_colors[group_id].color = group_id;
            MPI_Isend(&rank_colors[group_id].color, 1, MPI_INT, rank, 18864, global_comm, &requests[1 + group_id]);
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
    free(rank_colors);
    return (mys_commgroup_t)group;
}

static int _mys_cg_sort_i4(const void* _a, const void* _b)
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

MYS_API mys_commgroup_t mys_commgroup_create2(MPI_Comm global_comm, int group_color, int group_key)
{
    mys_commgroup_t group = (mys_commgroup_t)malloc(sizeof(struct _mys_commgroup_t));
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
    qsort(temp, group->global_nranks, sizeof(int[4]), _mys_cg_sort_i4);
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

MYS_API void mys_commgroup_release(mys_commgroup_t group)
{
    assert(group != NULL);
    if (group == MYS_COMMGROUP_NULL) return;
    MPI_Comm_free(&group->local_comm);
    MPI_Comm_free(&group->inter_comm);
    free(group->_rows);
    free(group->_cols);
    free(group->_brothers);
    free(group->_neighbors);
}

MYS_API mys_commgroup_t mys_commgroup_create_node(MPI_Comm global_comm)
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

MYS_API int mys_query_group_id(mys_commgroup_t group, int global_rank)
{
    if (global_rank < 0 || global_rank >= group->global_nranks)
        return -1;
    else
        return group->_rows[global_rank];
}

MYS_API int mys_query_local_rank(mys_commgroup_t group, int global_rank)
{
    if (global_rank < 0 || global_rank >= group->global_nranks)
        return -1;
    else
        return group->_cols[global_rank];
}

MYS_API int mys_query_brother(mys_commgroup_t group, int local_rank)
{
    if (local_rank < 0 || local_rank >= group->local_nranks)
        return -1;
    else
        return group->_brothers[local_rank];
}

MYS_API int mys_query_neighbor(mys_commgroup_t group, int group_id)
{
    if (group_id < 0 || group_id >= group->group_num)
        return -1;
    else
        return group->_neighbors[group_id];
}
