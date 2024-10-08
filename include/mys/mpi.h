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
#pragma once

#include <stdbool.h>
#include "_config.h"
#include "thread.h"

//-------------------- MPI types and constants --------------------//
// Functions of libmys (except _mpi) should use these wrapper types
// and constants instead of calling MPI routines in libmys directly.
// The code below grabs the definitions from hypre/utilities/mpistubs.h
#ifdef MYS_NO_MPI
/////// MPI_Comm
typedef int _mys_MPI_Comm;
#define _mys_MPI_COMM_SELF   1
#define _mys_MPI_COMM_WORLD  0
#define _mys_MPI_COMM_NULL  -1
/////// MPI_Datatype
typedef int _mys_MPI_Datatype;
#define _mys_MPI_FLOAT         0
#define _mys_MPI_DOUBLE        1
#define _mys_MPI_LONG_DOUBLE   2
#define _mys_MPI_INT           3
#define _mys_MPI_CHAR          4
#define _mys_MPI_LONG          5
#define _mys_MPI_BYTE          6
#define _mys_MPI_LONG_LONG_INT 9
#define _mys_MPI_DOUBLE_INT    10
/////// MPI_Op
typedef int _mys_MPI_Op;
#define _mys_MPI_SUM           0
#define _mys_MPI_MIN           1
#define _mys_MPI_MAX           2
#define _mys_MPI_MAXLOC        5
#define _mys_MPI_MINLOC        6
/////// MPI_Status
typedef int _mys_MPI_Status;
#define _mys_MPI_STATUS_IGNORE   ((_mys_MPI_Status *) 0) // Follow <mpi.h>
#define _mys_MPI_STATUSES_IGNORE ((_mys_MPI_Status *) 0)
/////// Misc Constants
#define _mys_MPI_SUCCESS           0
#define _mys_MPI_THREAD_SINGLE     1
#define _mys_MPI_THREAD_FUNNELED   2
#define _mys_MPI_THREAD_SERIALIZED 3
#define _mys_MPI_THREAD_MULTIPLE   4
#define _mys_MPI_IN_PLACE          ((void *)0)
#else
#include <mpi.h>
/////// MPI_Comm
typedef MPI_Comm _mys_MPI_Comm;
#define _mys_MPI_COMM_WORLD        MPI_COMM_WORLD
#define _mys_MPI_COMM_SELF         MPI_COMM_SELF
/////// MPI_Datatype
typedef MPI_Datatype _mys_MPI_Datatype;
#define _mys_MPI_INT               MPI_INT
#define _mys_MPI_LONG_LONG_INT     MPI_LONG_LONG_INT
#define _mys_MPI_FLOAT             MPI_FLOAT
#define _mys_MPI_DOUBLE            MPI_DOUBLE
#define _mys_MPI_LONG_DOUBLE       MPI_LONG_DOUBLE
#define _mys_MPI_CHAR              MPI_CHAR
#define _mys_MPI_LONG              MPI_LONG
#define _mys_MPI_BYTE              MPI_BYTE
#define _mys_MPI_REAL              MPI_REAL
#define _mys_MPI_COMPLEX           MPI_COMPLEX
#define _mys_MPI_DOUBLE_INT        MPI_DOUBLE_INT
/////// MPI_Op
typedef MPI_Op _mys_MPI_Op;
#define _mys_MPI_MAX               MPI_MAX
#define _mys_MPI_MIN               MPI_MIN
#define _mys_MPI_SUM               MPI_SUM
#define _mys_MPI_MAXLOC            MPI_MAXLOC
#define _mys_MPI_MINLOC            MPI_MINLOC
/////// MPI_Status
typedef MPI_Status _mys_MPI_Status;
#define _mys_MPI_STATUS_IGNORE     MPI_STATUS_IGNORE
#define _mys_MPI_STATUSES_IGNORE   MPI_STATUSES_IGNORE
/////// Misc Constants
#define _mys_MPI_SUCCESS           MPI_SUCCESS
#define _mys_MPI_THREAD_SINGLE     MPI_THREAD_SINGLE
#define _mys_MPI_THREAD_FUNNELED   MPI_THREAD_FUNNELED
#define _mys_MPI_THREAD_SERIALIZED MPI_THREAD_SERIALIZED
#define _mys_MPI_THREAD_MULTIPLE   MPI_THREAD_MULTIPLE
#define _mys_MPI_IN_PLACE          MPI_IN_PLACE
#endif



//-------------------- Custom high-level MPI functions --------------------//
// For libmys external and internal use
MYS_PUBLIC void mys_mpi_init();
MYS_PUBLIC int mys_mpi_myrank();
MYS_PUBLIC int mys_mpi_nranks();
MYS_PUBLIC int mys_mpi_barrier();
MYS_PUBLIC int mys_mpi_sync();
MYS_PUBLIC _mys_MPI_Comm mys_mpi_comm();
MYS_PUBLIC void mys_mpi_set_comm(_mys_MPI_Comm comm);
#define MYRANK() mys_mpi_myrank()
#define NRANKS() mys_mpi_nranks()
#define BARRIER() mys_mpi_barrier()
