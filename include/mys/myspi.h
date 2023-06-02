#pragma once

#include <stdbool.h>
#ifndef MYS_NO_MPI
#include <mpi.h>
#endif

#include "_config.h"
#include "macro.h"
#include "thread.h"

typedef struct _mys_myspi_G_t {
    bool inited;
    mys_mutex_t lock;
    int myrank;
    int nranks;
#ifndef MYS_NO_MPI
    MPI_Comm comm;
#endif
} _mys_myspi_G_t;

extern _mys_myspi_G_t _mys_myspi_G;

MYS_API int mys_myrank();
MYS_API int mys_nranks();
#ifndef MYS_NO_MPI
MYS_API MPI_Comm mys_comm();
#endif
MYS_API void mys_barrier();
MYS_API void mys_sync();

#define MYRANK() mys_myrank()
#define NRANKS() mys_nranks()
#define BARRIER() mys_barrier()
