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
#include "_private.h"
#include "../mpistubs.h"
#include "../mpi.h"
#include <stdio.h>

typedef struct _mys_mpi_G_t {
    bool inited;
    mys_mutex_t lock;
    int myrank;
    int nranks;
    mys_MPI_Comm comm;
} _mys_mpi_G_t;

MYS_STATIC _mys_mpi_G_t _mys_mpi_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .myrank = -1,
    .nranks = -1,
    .comm = mys_MPI_COMM_WORLD,
};

MYS_PUBLIC void mys_mpi_init()
{
    if (_mys_mpi_G.inited == true)
        return;
    mys_mutex_lock(&_mys_mpi_G.lock);
    int inited;
    mys_MPI_Initialized(&inited);
    if (!inited) {
        mys_MPI_Init_thread(NULL, NULL, mys_MPI_THREAD_SINGLE, &inited);
        fprintf(stdout, ">>>>> ===================================== <<<<<\n");
        fprintf(stdout, ">>>>> Nevel let libmys init MPI you dumbass <<<<<\n");
        fprintf(stdout, ">>>>> ===================================== <<<<<\n");
        fflush(stdout);
    }
    mys_MPI_Comm_rank(_mys_mpi_G.comm, &_mys_mpi_G.myrank);
    mys_MPI_Comm_size(_mys_mpi_G.comm, &_mys_mpi_G.nranks);
    _mys_mpi_G.inited = true;
    mys_mutex_unlock(&_mys_mpi_G.lock);
}

MYS_PUBLIC void mys_mpi_finalize()
{
    mys_mutex_lock(&_mys_mpi_G.lock);
    mys_MPI_Finalize();
    mys_mutex_unlock(&_mys_mpi_G.lock);
}

MYS_PUBLIC int mys_mpi_myrank()
{
    mys_mpi_init();
    return _mys_mpi_G.myrank;
}

MYS_PUBLIC int mys_mpi_nranks()
{
    mys_mpi_init();
    return _mys_mpi_G.nranks;
}

MYS_PUBLIC int mys_mpi_barrier()
{
    mys_mpi_init();
    return mys_MPI_Barrier(_mys_mpi_G.comm);
}

MYS_PUBLIC int mys_mpi_sync()
{
    // At this point we use simple barrier for sync
    return mys_mpi_barrier();
}

MYS_PUBLIC mys_MPI_Comm mys_mpi_comm()
{
    return _mys_mpi_G.comm;
}

#ifndef MYS_NO_MPI
MYS_PUBLIC void mys_mpi_set_comm(mys_MPI_Comm comm)
{
    mys_mpi_init();
    mys_mutex_lock(&_mys_mpi_G.lock);
   _mys_mpi_G.comm = comm;
   mys_MPI_Comm_rank(_mys_mpi_G.comm, &_mys_mpi_G.myrank);
   mys_MPI_Comm_size(_mys_mpi_G.comm, &_mys_mpi_G.nranks);
    mys_mutex_unlock(&_mys_mpi_G.lock);
}
#endif
