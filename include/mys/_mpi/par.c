// All functions here should use PMPI routines to provide functionality.
#pragma once

#include <stdio.h>
#include "../_config.h"
#include "id.h"
#include "mpi.h"


//-------------------- Elementary low-level MPI functions --------------------//

MYS_STATIC int _mys_MPI_Initialized(int *flag)
{
    return PMPI_Initialized(flag);
}

MYS_STATIC int _mys_MPI_Init_thread(int *argc, char ***argv, int required, int *provided)
{
    return PMPI_Init_thread(argc, argv, required, provided);
}

MYS_STATIC int _mys_MPI_Comm_rank(_mys_MPI_Comm comm, int *rank)
{
    return PMPI_Comm_rank(comm, rank);
}

MYS_STATIC int _mys_MPI_Comm_size(_mys_MPI_Comm comm, int *size)
{
    return PMPI_Comm_size(comm, size);
}

MYS_STATIC int _mys_MPI_Probe(int source, int tag, _mys_MPI_Comm comm, _mys_MPI_Status *status)
{
    return PMPI_Probe(source, tag, comm, status);
}

MYS_STATIC int _mys_MPI_Get_count(_mys_MPI_Status *status, _mys_MPI_Datatype datatype, int *count)
{
    return PMPI_Get_count(status, datatype, count);
}

MYS_STATIC int _mys_MPI_Recv(void *buf, int count, _mys_MPI_Datatype datatype, int source, int tag, _mys_MPI_Comm comm, _mys_MPI_Status *status)
{
    return PMPI_Recv(buf, count, datatype, source, tag, comm, status);
}

MYS_STATIC int _mys_MPI_Send(const void *buf, int count, _mys_MPI_Datatype datatype, int dest, int tag, _mys_MPI_Comm comm)
{
    return PMPI_Send(buf, count, datatype, dest, tag, comm);
}

MYS_STATIC int _mys_MPI_Barrier(_mys_MPI_Comm comm)
{
    return PMPI_Barrier(comm);
}

MYS_STATIC int _mys_MPI_Allreduce(void *sendbuf, void *recvbuf, int count, _mys_MPI_Datatype datatype, _mys_MPI_Op op, _mys_MPI_Comm comm)
{
    return PMPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
}

MYS_STATIC int _mys_MPI_Bcast(void *buffer, int count, _mys_MPI_Datatype datatype, int root, _mys_MPI_Comm comm)
{
   return PMPI_Bcast(buffer, count, datatype, root, comm);
}

MYS_STATIC double _mys_MPI_Wtime()
{
    return PMPI_Wtime();
}
