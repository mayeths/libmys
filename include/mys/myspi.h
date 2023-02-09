#pragma once

#include <stdbool.h>

#include "config.h"
#include "macro.h"
#include "thread.h"

typedef struct mys_myspi_G_t {
    bool inited;
    mys_mutex_t lock;
    int myrank;
    int nranks;
} mys_myspi_G_t;

extern mys_myspi_G_t mys_myspi_G;

MYS_API int mys_myrank();
MYS_API int mys_nranks();
MYS_API void mys_barrier();

#define MYRANK() mys_myrank()
#define NRANKS() mys_nranks()
#define BARRIER() mys_barrier()
