/* 
 * Copyright (c) 2024 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
// See hypre/utilities/mpistubs.h & hypre/utilities/mpistubs.c
#pragma once

#include "_config.h"

//-------------------- MPI types and constants --------------------//
#ifdef MYS_NO_MPI
// MPI stubs to generate serial codes without mpi
/////// MPI_Comm
typedef int mys_MPI_Comm;
#define mys_MPI_COMM_SELF   1
#define mys_MPI_COMM_WORLD  0
#define mys_MPI_COMM_NULL  -1
#define mys_MPI_COMM_TYPE_SHARED 0
/////// MPI_Info
typedef int mys_MPI_Info;
#define mys_MPI_INFO_NULL   0
#define mys_MPI_INFO_ENV    1
/////// MPI_Datatype
typedef int mys_MPI_Datatype;
#define mys_MPI_FLOAT         0
#define mys_MPI_DOUBLE        1
#define mys_MPI_LONG_DOUBLE   2
#define mys_MPI_INT           3
#define mys_MPI_CHAR          4
#define mys_MPI_LONG          5
#define mys_MPI_BYTE          6
#define mys_MPI_LONG_LONG_INT 9
#define mys_MPI_DOUBLE_INT    10
#define mys_MPI_INT32_T       11
#define mys_MPI_INT64_T       12
#define mys_MPI_UINT32_T      13
#define mys_MPI_UINT64_T      14
/////// MPI_Request
typedef struct mys_MPI_Request_s mys_MPI_Request_s;
typedef mys_MPI_Request_s *mys_MPI_Request;
#define mys_MPI_REQUEST_NULL  ((mys_MPI_Request)0)
/////// MPI_Op
typedef int mys_MPI_Op;
#define mys_MPI_SUM           0
#define mys_MPI_MIN           1
#define mys_MPI_MAX           2
#define mys_MPI_MAXLOC        5
#define mys_MPI_MINLOC        6
/////// MPI_Status
typedef int mys_MPI_Status;
#define mys_MPI_STATUS_IGNORE   ((mys_MPI_Status *) 0) // Follow <mpi.h>
#define mys_MPI_STATUSES_IGNORE ((mys_MPI_Status *) 0)
/////// Misc Constants
#define mys_MPI_SUCCESS           0
#define mys_MPI_THREAD_SINGLE     1
#define mys_MPI_THREAD_FUNNELED   2
#define mys_MPI_THREAD_SERIALIZED 3
#define mys_MPI_THREAD_MULTIPLE   4
#define mys_MPI_IN_PLACE          ((void *)1)
#else
// MPI stubs to generate parallel codes with mpi
#include <mpi.h>
/////// MPI_Comm
typedef MPI_Comm mys_MPI_Comm;
#define mys_MPI_COMM_WORLD        MPI_COMM_WORLD
#define mys_MPI_COMM_SELF         MPI_COMM_SELF
#define mys_MPI_COMM_NULL         MPI_COMM_NULL
#define mys_MPI_COMM_TYPE_SHARED  MPI_COMM_TYPE_SHARED
/////// MPI_Info
typedef MPI_Info mys_MPI_Info;
#define mys_MPI_INFO_NULL         MPI_INFO_NULL
#define mys_MPI_INFO_ENV          MPI_INFO_ENV
/////// MPI_Datatype
typedef MPI_Datatype mys_MPI_Datatype;
#define mys_MPI_INT               MPI_INT
#define mys_MPI_LONG_LONG_INT     MPI_LONG_LONG_INT
#define mys_MPI_FLOAT             MPI_FLOAT
#define mys_MPI_DOUBLE            MPI_DOUBLE
#define mys_MPI_LONG_DOUBLE       MPI_LONG_DOUBLE
#define mys_MPI_CHAR              MPI_CHAR
#define mys_MPI_LONG              MPI_LONG
#define mys_MPI_BYTE              MPI_BYTE
#define mys_MPI_REAL              MPI_REAL
#define mys_MPI_COMPLEX           MPI_COMPLEX
#define mys_MPI_DOUBLE_INT        MPI_DOUBLE_INT
#define mys_MPI_INT32_T           MPI_INT32_T
#define mys_MPI_INT64_T           MPI_INT64_T
#define mys_MPI_UINT32_T          MPI_UINT32_T
#define mys_MPI_UINT64_T          MPI_UINT64_T
/////// MPI_Request
typedef MPI_Request mys_MPI_Request;
#define mys_MPI_REQUEST_NULL      MPI_REQUEST_NULL
/////// MPI_Op
typedef MPI_Op mys_MPI_Op;
#define mys_MPI_MAX               MPI_MAX
#define mys_MPI_MIN               MPI_MIN
#define mys_MPI_SUM               MPI_SUM
#define mys_MPI_MAXLOC            MPI_MAXLOC
#define mys_MPI_MINLOC            MPI_MINLOC
/////// MPI_Status
typedef MPI_Status mys_MPI_Status;
#define mys_MPI_STATUS_IGNORE     MPI_STATUS_IGNORE
#define mys_MPI_STATUSES_IGNORE   MPI_STATUSES_IGNORE
/////// Misc Constants
#define mys_MPI_SUCCESS           MPI_SUCCESS
#define mys_MPI_THREAD_SINGLE     MPI_THREAD_SINGLE
#define mys_MPI_THREAD_FUNNELED   MPI_THREAD_FUNNELED
#define mys_MPI_THREAD_SERIALIZED MPI_THREAD_SERIALIZED
#define mys_MPI_THREAD_MULTIPLE   MPI_THREAD_MULTIPLE
#define mys_MPI_IN_PLACE          MPI_IN_PLACE
#endif

//-------------------- MPI prototypes --------------------//
MYS_PUBLIC int mys_MPI_Init(int *argc, char ***argv);
MYS_PUBLIC int mys_MPI_Init_thread(int *argc, char ***argv, int required, int *provided);
MYS_PUBLIC int mys_MPI_Initialized(int *flag);
MYS_PUBLIC int mys_MPI_Finalize();
MYS_PUBLIC int mys_MPI_Abort(mys_MPI_Comm comm, int errorcode);
MYS_PUBLIC int mys_MPI_Comm_rank(mys_MPI_Comm comm, int *rank);
MYS_PUBLIC int mys_MPI_Comm_size(mys_MPI_Comm comm, int *size);
MYS_PUBLIC int mys_MPI_Comm_split(mys_MPI_Comm comm, int n, int m, mys_MPI_Comm *comms);
MYS_PUBLIC int mys_MPI_Comm_split_type(mys_MPI_Comm comm, int split_type, int key, mys_MPI_Info info, mys_MPI_Comm *newcomm);
MYS_PUBLIC int mys_MPI_Comm_dup(mys_MPI_Comm comm, mys_MPI_Comm *newcomm);
MYS_PUBLIC int mys_MPI_Comm_free(mys_MPI_Comm *comm);
MYS_PUBLIC int mys_MPI_Recv(void *buf, int count, mys_MPI_Datatype datatype, int source, int tag, mys_MPI_Comm comm, mys_MPI_Status *status);
MYS_PUBLIC int mys_MPI_Send(const void *buf, int count, mys_MPI_Datatype datatype, int dest, int tag, mys_MPI_Comm comm);
MYS_PUBLIC int mys_MPI_Irecv(void *buf, int count, mys_MPI_Datatype datatype, int source, int tag, mys_MPI_Comm comm, mys_MPI_Request *request);
MYS_PUBLIC int mys_MPI_Isend(const void *buf, int count, mys_MPI_Datatype datatype, int dest, int tag, mys_MPI_Comm comm, mys_MPI_Request *request);
MYS_PUBLIC int mys_MPI_Waitall(int count, mys_MPI_Request *array_of_requests, mys_MPI_Status *array_of_statuses);
MYS_PUBLIC int mys_MPI_Barrier(mys_MPI_Comm comm);
MYS_PUBLIC int mys_MPI_Bcast(void *buffer, int count, mys_MPI_Datatype datatype, int root, mys_MPI_Comm comm);
MYS_PUBLIC int mys_MPI_Gather(void *sendbuf, int sendcount, mys_MPI_Datatype sendtype, void *recvbuf, int recvcount, mys_MPI_Datatype recvtype, int root, mys_MPI_Comm comm);
MYS_PUBLIC int mys_MPI_Allreduce(void *sendbuf, void *recvbuf, int count, mys_MPI_Datatype datatype, mys_MPI_Op op, mys_MPI_Comm comm);
MYS_PUBLIC int mys_MPI_Allgather(void *sendbuf, int sendcount, mys_MPI_Datatype sendtype, void *recvbuf, int recvcount, mys_MPI_Datatype recvtype, mys_MPI_Comm comm);
MYS_PUBLIC int mys_MPI_Probe(int source, int tag, mys_MPI_Comm comm, mys_MPI_Status *status);
MYS_PUBLIC int mys_MPI_Get_count(mys_MPI_Status *status, mys_MPI_Datatype datatype, int *count);
MYS_PUBLIC double mys_MPI_Wtime();
