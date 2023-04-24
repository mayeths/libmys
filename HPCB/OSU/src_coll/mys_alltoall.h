#pragma once

#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "mys_alltoallv.h"

int mys_alltoall(
    const void *sendbuf,
    int sendcount,
    MPI_Datatype sendtype,
    void *recvbuf,
    int recvcount,
    MPI_Datatype recvtype,
    MPI_Comm comm
)
{
    int myrank = -1;
    int nranks = -1;
    MPI_Comm_rank(comm, &myrank);
    MPI_Comm_size(comm, &nranks);
    int *sendcounts = (int *)calloc(sizeof(int), nranks);
    int *recvcounts = (int *)calloc(sizeof(int), nranks);
    for (int rank = 0; rank < nranks; rank++) {
        sendcounts[rank] = sendcount;
        recvcounts[rank] = recvcount;
    }
    int *sdispls = (int *)calloc(sizeof(int), nranks);
    int *rdispls = (int *)calloc(sizeof(int), nranks);
    for (int rank = 1; rank < nranks; rank++) {
        sdispls[rank] = sdispls[rank - 1] + sendcount;
        rdispls[rank] = rdispls[rank - 1] + recvcount;
    }

    int ret = mys_alltoallv(
        sendbuf, sendcounts, sdispls, sendtype,
        recvbuf, recvcounts, rdispls, recvtype,
        comm
    );

    free(sendcounts);
    free(recvcounts);
    free(sdispls);
    free(rdispls);
    return ret;
}

