#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <string.h>

#include "uthash.h"
#include "mys_alltoall.h"

#include <mys.h>

typedef struct localhost_t {
    char node_name[MPI_MAX_INFO_VAL];
    MPI_Comm intra_comm;
    MPI_Comm inter_comm;
    int intra_myrank;
    int intra_nranks;
    int *intra_ranks;
} localhost_t;

typedef struct nid_t {
    char name[MPI_MAX_INFO_VAL]; /* we'll use first field as the key */
    int id;
    UT_hash_handle hh; /* makes this structure hashable */
} nid_t;

nid_t *nid_insert_node(nid_t *head, const char *name, int id) {
    nid_t *s = (nid_t *)malloc(sizeof(nid_t));
    strncpy(s->name, name, MPI_MAX_INFO_VAL);
    s->id = id;
    HASH_ADD_STR(head, name, s);
    return s;
}

void nid_destroy_node(nid_t *head) {
    nid_t *s = NULL;
    nid_t *tmp = NULL;
    HASH_ITER(hh, head, s, tmp) {
        HASH_DEL(head, s);
        free(s);
    }
}

nid_t *nid_find_node(nid_t *head, const char *name) {
    nid_t *s = NULL;
    HASH_FIND_STR(head, name, s);
    return s;
}

localhost_t get_localhost(MPI_Comm comm)
{
    localhost_t result;
    int myrank, nranks;
    MPI_Comm_size(comm, &nranks);
    MPI_Comm_rank(comm, &myrank);

    int nkeys = 0;
    MPI_Info_get_nkeys(MPI_INFO_ENV, &nkeys);

    char host_value[MPI_MAX_INFO_VAL];
    for (int i = 0; i < nkeys; i++) {
        char key[MPI_MAX_INFO_KEY];
        MPI_Info_get_nthkey(MPI_INFO_ENV, i, key);
        if (strncmp(key, "host", MPI_MAX_INFO_KEY) != 0)
            continue;

        int flag;
        MPI_Info_get(MPI_INFO_ENV, key, sizeof(host_value), host_value, &flag);
    }
    strncpy(result.node_name, host_value, MPI_MAX_INFO_VAL);

    int *rank_to_nid = (int *)calloc(sizeof(int), nranks);


    if (myrank == 0) {
        int counter = 0;
        rank_to_nid[myrank] = counter++;

        nid_t *head = NULL;
        nid_insert_node(head, host_value, rank_to_nid[myrank]);

        for (int rank = 1; rank < nranks; rank++) {
            char recvbuf[MPI_MAX_INFO_VAL];
            memset(recvbuf, 0, sizeof(recvbuf));
            MPI_Recv(recvbuf, MPI_MAX_INFO_VAL, MPI_CHAR, rank, 107749, comm, MPI_STATUS_IGNORE);

            nid_t *s = nid_find_node(head, recvbuf);
            if (s == NULL) {
                rank_to_nid[rank] = counter++;
                nid_insert_node(head, recvbuf, rank_to_nid[rank]);
            } else {
                rank_to_nid[rank] = s->id;
            }
        }

        nid_destroy_node(head);
    } else {
        MPI_Send(host_value, MPI_MAX_INFO_VAL, MPI_CHAR, 0, 107749, comm);
    }

    MPI_Bcast(rank_to_nid, nranks, MPI_INT, 0, comm);
    result.intra_comm = MPI_COMM_NULL;
    MPI_Comm_split(comm, rank_to_nid[myrank], myrank, &result.intra_comm);

    MPI_Comm_size(result.intra_comm, &result.intra_nranks);
    MPI_Comm_rank(result.intra_comm, &result.intra_myrank);
    MPI_Comm_split(comm, result.intra_myrank, myrank, &result.inter_comm);

    result.intra_ranks = (int *)malloc(result.intra_nranks * sizeof(int));
    MPI_Allgather(&myrank, 1, MPI_INT, result.intra_ranks, 1, MPI_INT, result.intra_comm);

    return result;
}

static int printed = 0;

int topo_alltoall(
    const void *sendbuf,
    int sendcount,
    MPI_Datatype sendtype,
    void *recvbuf,
    int recvcount,
    MPI_Datatype recvtype,
    MPI_Comm comm
)
{
    localhost_t local = get_localhost(comm);
    int node_offset = -1;
    MPI_Scan(&local.intra_nranks, &node_offset, 1, MPI_INT, MPI_SUM, local.inter_comm);
    MPI_Bcast(&node_offset, 1, MPI_INT, 0, local.intra_comm);
    int my_new_rank = node_offset + local.intra_myrank;

    int myrank, nranks;
    MPI_Comm_size(comm, &nranks);
    MPI_Comm_rank(comm, &myrank);

    MPI_Comm ordered_comm = MPI_COMM_NULL;
    MPI_Comm_split(comm, 0, my_new_rank, &ordered_comm);

    int new_rank;
    MPI_Comm_rank(ordered_comm, &new_rank);

    if (printed == 0) {
        DLOG_ORDERED("My new rank is %d %d", my_new_rank, new_rank);
        printed = 1;
    }

    int ret = mys_alltoall(sendbuf, sendcount, sendtype, recvbuf, recvcount, recvtype, ordered_comm);


    MPI_Comm_free(&ordered_comm);
    MPI_Comm_free(&local.inter_comm);
    MPI_Comm_free(&local.intra_comm);
    free(local.intra_ranks);

    return ret;
}
