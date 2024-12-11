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

#include <stdbool.h>
#include "_config.h"
#include "mpistubs.h"

//-------------------- Custom high-level MPI functions --------------------//
// For libmys external and internal use
MYS_PUBLIC void mys_mpi_init();
MYS_PUBLIC void mys_mpi_finalize();
MYS_PUBLIC int mys_mpi_myrank();
MYS_PUBLIC int mys_mpi_nranks();
MYS_PUBLIC int mys_mpi_barrier();
MYS_PUBLIC int mys_mpi_sync();
MYS_PUBLIC mys_MPI_Comm mys_mpi_comm();
MYS_PUBLIC void mys_mpi_set_comm(mys_MPI_Comm comm);
#define MYRANK() mys_mpi_myrank()
#define NRANKS() mys_mpi_nranks()
#define BARRIER() mys_mpi_barrier()
