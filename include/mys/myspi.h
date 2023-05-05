#pragma once

#include <stdbool.h>

#include "config.h"
#include "macro.h"
#include "thread.h"

typedef struct _mys_myspi_G_t {
    bool inited;
    mys_mutex_t lock;
    int myrank;
    int nranks;
} _mys_myspi_G_t;

extern _mys_myspi_G_t _mys_myspi_G;

MYS_API int mys_myrank();
MYS_API int mys_nranks();
MYS_API void mys_barrier();
MYS_API void mys_sync();

#define MYRANK() mys_myrank()
#define NRANKS() mys_nranks()
#define BARRIER() mys_barrier()
