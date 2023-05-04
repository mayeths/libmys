#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <string.h>

#include "uthash.h"

#include <mys.h>

typedef struct groupinfo_t {
    char node_name[MPI_MAX_INFO_VAL];
    int node_id;
    MPI_Comm intra_comm;
    MPI_Comm inter_comm;
    int intra_myrank;
    int inter_myrank;
    int intra_nranks;
    int inter_nranks;
    int *intra_ranks;
} groupinfo_t;

typedef struct nid_t {
    char name[MPI_MAX_INFO_VAL]; /* we'll use first field as the key */
    int id;
    UT_hash_handle hh; /* makes this structure hashable */
} nid_t;

nid_t *nid_insert_node(nid_t **head, const char *name, int id) {
    nid_t *s = (nid_t *)malloc(sizeof(nid_t));
    strncpy(s->name, name, MPI_MAX_INFO_VAL - 1);
    s->name[MPI_MAX_INFO_VAL - 1] = '\0';
    s->id = id;
    HASH_ADD_STR(*head, name, s);
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

typedef union policy_args_t {
    struct {} node_group;
    struct {
        int group_size;
    } fix_group;
    struct {} predef_group;
} policy_args_t;

enum { NODE_GROUP, FIX_GROUP, PREDEF_GROUP };

// groupinfo_t get_group(MPI_Comm comm, int group_policy, policy_args_t *policy_args)
groupinfo_t get_group(MPI_Comm comm)
{
    groupinfo_t result;
    int myrank, nranks;
    MPI_Comm_size(comm, &nranks);
    MPI_Comm_rank(comm, &myrank);

    int nkeys = 0;
    MPI_Info_get_nkeys(MPI_INFO_ENV, &nkeys);

    for (int i = 0; i < nkeys; i++) {
        char key[MPI_MAX_INFO_KEY];
        MPI_Info_get_nthkey(MPI_INFO_ENV, i, key);
        if (strncmp(key, "host", MPI_MAX_INFO_KEY) != 0)
            continue;

        int flag;
        MPI_Info_get(MPI_INFO_ENV, key, MPI_MAX_INFO_VAL, result.node_name, &flag);
    }
#ifdef FAKE_LOC
    if (myrank >= FAKE_LOC) {
        strncpy(result.node_name, "fake_node", MPI_MAX_INFO_VAL);
    }
#endif

    int *rank_to_nid = (int *)calloc(sizeof(int), nranks);


    if (myrank == 0) {
        int counter = 0;
        rank_to_nid[myrank] = counter++;

        nid_t *head = NULL;
        nid_insert_node(&head, result.node_name, rank_to_nid[myrank]);

        for (int rank = 1; rank < nranks; rank++) {
            char rank_node[MPI_MAX_INFO_VAL];
            memset(rank_node, 0, sizeof(rank_node));
            MPI_Recv(rank_node, MPI_MAX_INFO_VAL, MPI_CHAR, rank, 107749, comm, MPI_STATUS_IGNORE);

            nid_t *s = nid_find_node(head, rank_node);
            if (s == NULL) {
                rank_to_nid[rank] = counter++;
                nid_insert_node(&head, rank_node, rank_to_nid[rank]);
            } else {
                rank_to_nid[rank] = s->id;
            }
        }

        nid_destroy_node(head);
    } else {
        MPI_Send(result.node_name, MPI_MAX_INFO_VAL, MPI_CHAR, 0, 107749, comm);
    }

    MPI_Bcast(rank_to_nid, nranks, MPI_INT, 0, comm);
    result.intra_comm = MPI_COMM_NULL;
    MPI_Comm_split(comm, rank_to_nid[myrank], myrank, &result.intra_comm);
    result.node_id = rank_to_nid[myrank];

    MPI_Comm_size(result.intra_comm, &result.intra_nranks);
    MPI_Comm_rank(result.intra_comm, &result.intra_myrank);
    MPI_Comm_split(comm, result.intra_myrank, myrank, &result.inter_comm);

    result.intra_ranks = (int *)malloc(result.intra_nranks * sizeof(int));
    MPI_Allgather(&myrank, 1, MPI_INT, result.intra_ranks, 1, MPI_INT, result.intra_comm);

    MPI_Comm_size(result.inter_comm, &result.inter_nranks);
    MPI_Comm_rank(result.inter_comm, &result.inter_myrank);

    // DLOG_ORDERED("on node=%s (id=%d) intra_nranks=%d inter_nranks=%d", result.node_name, result.node_id, result.intra_nranks, result.inter_nranks);

    free(rank_to_nid);

    return result;
}
