#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <mpi.h>

#include "_config.h"

// static void mys_init_mpiz(MPI_Comm comm);
// static void mys_fini_mpiz(MPI_Comm comm);
// static int mys_get_node_id(int global_rank);
// static int mys_get_local_rank(int global_rank);
// static int mys_get_rank(int local_id);
// static MPI_Comm mys_get_intra_node_comm();

typedef struct commloc_t {
    MPI_Comm global_comm;
    MPI_Comm local_comm;
    int node_id;
    int node_num;
    int local_myrank;
    int local_nranks;
    int global_myrank;
    int global_nranks;
    int *rows;
    int *cols;
    int *brothers;
} commloc_t;

typedef struct predef_locality_t {
    int node_id;
} predef_locality_t;

static int _split_node_compare_rank(const void* a, const void* b) {
    const int *intA = (const int *)a;
    const int *intB = (const int *)b;
    if (*intA < *intB) return -1;
    else if (*intA > *intB) return 1;
    else return 0;
}

/**
 * @brief Create locality information
 * 
 * @param comm 
 * @param predef 
 * @return commloc_t
 * 
 * @note MPIz_commloc_create(comm, NULL) for real topo
 */
MYS_STATIC commloc_t MPIz_commloc_create(MPI_Comm comm, predef_locality_t *predef)
{
    commloc_t loc;
    loc.global_comm = comm;
    MPI_Comm_size(loc.global_comm, &loc.global_nranks);
    MPI_Comm_rank(loc.global_comm, &loc.global_myrank);

    if (predef == NULL) {
        MPI_Comm_split_type(loc.global_comm, MPI_COMM_TYPE_SHARED, loc.global_myrank, MPI_INFO_NULL, &loc.local_comm);
    } else {
        MPI_Comm_split(loc.global_comm, predef->node_id, loc.global_myrank, &loc.local_comm);
    }
    MPI_Comm_size(loc.local_comm, &loc.local_nranks);
    MPI_Comm_rank(loc.local_comm, &loc.local_myrank);

    int is_node_root = loc.local_myrank == 0;
    MPI_Allreduce(&is_node_root, &loc.node_num, 1, MPI_INT, MPI_SUM, loc.global_comm);
    int *roots = (int *)malloc(sizeof(int) * loc.node_num);
    int nrequests = (loc.global_myrank == 0) ? (loc.node_num + 1) : (loc.local_myrank == 0) ? 1 : 0;
    MPI_Request *requests = (MPI_Request *)malloc(sizeof(MPI_Request) * nrequests);

    if (loc.global_myrank == 0) {
        for(int i = 0; i < loc.node_num; i++)
            MPI_Irecv(&roots[i], 1, MPI_INT, MPI_ANY_SOURCE, 1007749, loc.global_comm, &requests[1 + i]);
    }
    if (loc.local_myrank == 0) {
        MPI_Isend(&loc.global_myrank, 1, MPI_INT, 0, 1007749, loc.global_comm, &requests[0]);
    }
    if (nrequests > 0) {
        MPI_Waitall(nrequests, requests, MPI_STATUSES_IGNORE);
    }

    if (loc.global_myrank == 0) {
        qsort(roots, loc.node_num, sizeof(int), _split_node_compare_rank);
        for(int i = 0; i < loc.node_num; i++) {
            int rank = roots[i];
            roots[i] = i;
            MPI_Isend(&roots[i], 1, MPI_INT, rank, 2008864, loc.global_comm, &requests[1 + i]);
        }
    }
    if (loc.local_myrank == 0) {
        MPI_Irecv(&loc.node_id, 1, MPI_INT, 0, 2008864, loc.global_comm, &requests[0]);
    }
    if (nrequests > 0) {
        MPI_Waitall(nrequests, requests, MPI_STATUSES_IGNORE);
    }

    MPI_Bcast(&loc.node_id, 1, MPI_INT, 0, loc.local_comm);

    loc.rows = (int *)malloc(sizeof(int) * loc.global_nranks);
    loc.rows[loc.global_myrank] = loc.node_id;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, loc.rows, 1, MPI_INT, loc.global_comm);

    loc.cols = (int *)malloc(sizeof(int) * loc.global_nranks);
    loc.cols[loc.global_myrank] = loc.local_myrank;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, loc.cols, 1, MPI_INT, loc.global_comm);

    loc.brothers = (int *)malloc(sizeof(int) * loc.local_nranks);
    loc.brothers[loc.local_myrank] = loc.global_myrank;
    MPI_Allgather(MPI_IN_PLACE, 1, MPI_INT, loc.brothers, 1, MPI_INT, loc.local_comm);

    free(requests);
    free(roots);

    return loc;
}

MYS_STATIC void MPIz_commloc_release(commloc_t *loc)
{
    assert(loc != NULL);
    MPI_Comm_free(&loc->local_comm);
    free(loc->rows);
    free(loc->cols);
    free(loc->brothers);
}

// int main(int argc, char **argv)
// {
//     int who = 0;
//     if (argc > 1) {
//         who = atoi(argv[1]);
//     }
//     MPI_Init(NULL, NULL);
//     commloc_t commloc = MPIz_commloc_create(MPI_COMM_WORLD);
//     if (commloc.global_myrank == who) {
//         printf("[%d has %d friends]", commloc.global_myrank, commloc.local_nranks);
//         for (int i = 0; i < commloc.global_nranks; i++) {
//             printf(" %d", commloc.rows[i]);
//         }
//         printf("\n");
//     }
//     MPI_Barrier(MPI_COMM_WORLD);
//     MPIz_commloc_release(&commloc);
//     MPI_Finalize();
// }
