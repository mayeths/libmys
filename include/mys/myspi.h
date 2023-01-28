#pragma once

#include "config.h"
#include "macro.h"

MYS_API void mys_ensure_myspi_init();
MYS_API int mys_myrank();
MYS_API int mys_nranks();
MYS_API void mys_barrier();

#define MYRANK() mys_myrank()
#define NRANKS() mys_nranks()
#define BARRIER() mys_barrier()
