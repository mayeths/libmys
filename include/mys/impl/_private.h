#pragma once

// This file is used to include declare of private functions that internally used by libmys.
// We don't expect users to use them.

#include "../_config.h"
#include "../mpi.h"

// mpi.c

MYS_STATIC int _mys_MPI_Initialized(int *flag);
MYS_STATIC int _mys_MPI_Init_thread(int *argc, char ***argv, int required, int *provided);
MYS_STATIC int _mys_MPI_Comm_rank(_mys_MPI_Comm comm, int *rank);
MYS_STATIC int _mys_MPI_Comm_size(_mys_MPI_Comm comm, int *size);
MYS_STATIC int _mys_MPI_Recv(void *buf, int count, _mys_MPI_Datatype datatype, int source, int tag, _mys_MPI_Comm comm, _mys_MPI_Status *status);
MYS_STATIC int _mys_MPI_Send(const void *buf, int count, _mys_MPI_Datatype datatype, int dest, int tag, _mys_MPI_Comm comm);
MYS_STATIC int _mys_MPI_Barrier(_mys_MPI_Comm comm);
MYS_STATIC int _mys_MPI_Allreduce(void *sendbuf, void *recvbuf, int count, _mys_MPI_Datatype datatype, _mys_MPI_Op op, _mys_MPI_Comm comm);
MYS_STATIC int _mys_MPI_Probe(int source, int tag, _mys_MPI_Comm comm, _mys_MPI_Status *status);
MYS_STATIC int _mys_MPI_Get_count(_mys_MPI_Status *status, _mys_MPI_Datatype datatype, int *count);
MYS_STATIC double _mys_MPI_Wtime();
