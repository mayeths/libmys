/**
 * @file impl.h
 * @author mayeths (wow@mayeths.com)
 * @brief Implement non-static variables and fucntions
 * 
 * The biggest benifit of using standalone "impl.h"
 * instead of #ifdef MYS_IMPL in all other headers are,
 * we can use `#pragma once` in these other headers,
 * and the MYS_IMPL macro is still processed correctly
 */
#ifndef __MYS_IMPL_H__
#define __MYS_IMPL_H__

/*********************************************/
// C definition
/*********************************************/
#include "thread.h"
#include "errno.h"
#include "myspi.h"
#include "log.h"
#include "random.h"

mys_thread_local int mys_errno = 0;
mys_thread_local uint64_t __legacy_x = __UINT64_INVALID;
mys_thread_local uint64_t __splitmix64_x = __UINT64_INVALID;
mys_thread_local uint64_t __xoroshiro128_x[2] = {__UINT64_INVALID, __UINT64_INVALID};
mys_log_G_t mys_log_G = {
    .level = MYS_LOG_TRACE,
    .last_level = MYS_LOG_TRACE,
    .lock = MYS_MUTEX_INITIALIZER,
    .handlers = {
#ifndef MYS_LOG_DISABLE_STDOUT_HANDLER
        { .fn = mys_log_stdio_handler, .udata = NULL, .id = 10000 },
#endif
        { .fn = NULL, .udata = NULL, .id = 0 /* Uninitalized ID is 0 */ },
    },
};

mys_myspi_G_t mys_myspi_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .myrank = -1,
    .nranks = -1,
};

#ifdef MYS_NEED_WTIME_START_TICK
double mys_wtime_start = (double)-1;
#endif

MYS_API void mys_myspi_init()
{
    if (mys_myspi_G.inited == true)
        return;
    mys_mutex_lock(&mys_myspi_G.lock);
#if defined(MYS_NO_MPI)
    mys_myspi_G.myrank = 0;
    mys_myspi_G.nranks = 1;
#else
    int inited;
    MPI_Initialized(&inited);
    if (!inited) {
        MPI_Init_thread(NULL, NULL, MPI_THREAD_SINGLE, &inited);
        fprintf(stdout, ">>>>> ===================================== <<<<<\n");
        fprintf(stdout, ">>>>> Nevel let libmys init MPI you dumbass <<<<<\n");
        fprintf(stdout, ">>>>> ===================================== <<<<<\n");
        fflush(stdout);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &mys_myspi_G.nranks);
    MPI_Comm_rank(MPI_COMM_WORLD, &mys_myspi_G.myrank);
#endif
    mys_myspi_G.inited = true;
    mys_mutex_unlock(&mys_myspi_G.lock);
}

MYS_API int mys_myrank()
{
    mys_myspi_init();
    return mys_myspi_G.myrank;
}

MYS_API int mys_nranks()
{
    mys_myspi_init();
    return mys_myspi_G.nranks;
}

MYS_API void mys_barrier()
{
    mys_myspi_init();
#if defined(MYS_NO_MPI)
    return;
#else
    MPI_Barrier(MPI_COMM_WORLD);
#endif
}

#endif /*__MYS_IMPL_H__*/
