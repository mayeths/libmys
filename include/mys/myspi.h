#pragma once

#include "config.h"
#include "macro.h"

MYS_API void __mys_ensure_myspi_init();
MYS_API int __mys_myrank();
MYS_API int __mys_nranks();
MYS_API void __mys_barrier();

#define MYRANK() __mys_myrank()
#define NRANKS() __mys_nranks()
#define BARRIER() __mys_barrier()
