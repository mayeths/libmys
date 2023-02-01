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
#ifdef MYS_NEED_WTIME_START_TICK
double mys_wtime_start = (double)-1;
#endif

MYS_API void mys_ensure_myspi_init()
{
#if defined(MYS_NO_MPI)
    return;
#else
    int inited;
    MPI_Initialized(&inited);
    if (inited) return;
    MPI_Init_thread(NULL, NULL, MPI_THREAD_SINGLE, &inited);
    fprintf(stdout, ">>>>> ===================================== <<<<<\n");
    fprintf(stdout, ">>>>> Nevel let libmys init MPI you dumbass <<<<<\n");
    fprintf(stdout, ">>>>> ===================================== <<<<<\n");
    fflush(stdout);
#endif
}

MYS_API int mys_myrank()
{
#if defined(MYS_NO_MPI)
    return 0;
#else
    mys_ensure_myspi_init();
    int myrank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    return myrank;
#endif
}

MYS_API int mys_nranks()
{
#if defined(MYS_NO_MPI)
    return 1;
#else
    mys_ensure_myspi_init();
    int nranks = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &nranks);
    return nranks;
#endif
}

MYS_API void mys_barrier()
{
#if defined(MYS_NO_MPI)
    return;
#else
    mys_ensure_myspi_init();
    MPI_Barrier(MPI_COMM_WORLD);
#endif
}

#endif /*__MYS_IMPL_H__*/
