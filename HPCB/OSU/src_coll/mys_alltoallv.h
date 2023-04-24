#pragma once

#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "mys_alltoallw.h"

int mys_alltoallv(
    const void *sendbuf,
    const int sendcounts[],
    const int sdispls[],
    MPI_Datatype sendtype,
    void *recvbuf,
    const int recvcounts[],
    const int rdispls[],
    MPI_Datatype recvtype,
    MPI_Comm comm
)
{

    int myrank = -1;
    int nranks = -1;
    MPI_Comm_rank(comm, &myrank);
    MPI_Comm_size(comm, &nranks);
    MPI_Datatype *sendtypes = (MPI_Datatype *)calloc(sizeof(MPI_Datatype), nranks);
    MPI_Datatype *recvtypes = (MPI_Datatype *)calloc(sizeof(MPI_Datatype), nranks);
    for (int rank = 0; rank < nranks; rank++) {
        sendtypes[rank] = sendtype;
        recvtypes[rank] = recvtype;
    }
    int *sdispls_ = (int *)calloc(sizeof(int), nranks);
    int *rdispls_ = (int *)calloc(sizeof(int), nranks);
    int stypesize = 0;
    int rtypesize = 0;
    MPI_Type_size(sendtype, &stypesize);
    MPI_Type_size(recvtype, &rtypesize);
    for (int rank = 1; rank < nranks; rank++) {
        sdispls_[rank] = sdispls[rank] * stypesize;
        rdispls_[rank] = rdispls[rank] * rtypesize;
    }

    int ret = mys_alltoallw(
        sendbuf, sendcounts, sdispls_, sendtypes,
        recvbuf, recvcounts, rdispls_, recvtypes,
        comm
    );

    free(sendtypes);
    free(recvtypes);
    free(sdispls_);
    free(rdispls_);

    return ret;
}
