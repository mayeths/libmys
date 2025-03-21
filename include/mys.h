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
#ifndef MYS_VERSION
#define MYS_VERSION 202408L
#endif

#include "mys/_config.h"

#if defined(__cplusplus) && __cplusplus < 201103L
#error Require C++11 or higher. Consider removing -std or use -std=c++11 in compiler command line arguments
#elif !defined(__cplusplus) && __STDC_VERSION__ < 199901L
#error Require GNU C99 or higher. Consider removing -std or use -std=gnu99 in compiler command line arguments
// #elif !defined(__cplusplus) && defined(__STRICT_ANSI__)
// #error Require GNU C instead of ANSI C or ISO C. Consider removing -ansi or change -std=c99 to -std=gnu99 in compiler command line arguments
#elif !defined(OS_LINUX) && !defined(OS_MACOS) && !defined(OS_FREEBSD)
#error Port me
#else

#if defined(OS_LINUX) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE 1
#endif
#if defined(OS_MACOS) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#ifdef COMPILER_ICC
#warning Using intel compiler you may need to define MYS_DISABLE_DEBUG macro for now. Sorry for this inconvenient.
#endif
#ifdef OS_OS_FREEBSD
#define MYS_DISABLE_DEBUG
#endif

#ifndef MYS_NO_MPI
#include <mpi.h>  // intel mpiicpc throw error if mpi.h is include in extern "C"
#endif

////////////////////////////////////////
////// C Declaration
////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
// Primary Library (mys)
#include "mys/algorithm.h"
#include "mys/assert.h"
#include "mys/atomic.h"
#include "mys/base64.h"
#include "mys/checkpoint.h"
#ifndef MYS_DISABLE_DEBUG // if you use inte compiler like icc, then this may helps
#include "mys/debug.h"
#endif
#include "mys/errno.h"
#include "mys/format.h"
#include "mys/guard.h"
#include "mys/hash.h"
#include "mys/hrtime.h"
#include "mys/log.h"
#include "mys/macro.h"
#include "mys/memory.h"
#include "mys/mpistubs.h"
#include "mys/misc.h"
#include "mys/commgroup.h"
#include "mys/net.h"
#include "mys/os.h"
#include "mys/linalg.h"
#include "mys/pool.h"
#include "mys/pmparser.h"
#include "mys/rand.h"
#include "mys/require.h"
#include "mys/statistic.h"
#include "mys/string.h"
#include "mys/thread.h"
#include "mys/trace.h"
#include "mys/table.h"
// Third-Party Library (mys3)
#include "mys3/cJSON/cJSON.h"
#include "mys3/matrixmarket/mmio.h"
#include "mys3/stb/stb_image.h"
#ifdef __cplusplus
} // extern "C"
#endif


////////////////////////////////////////
////// C Definition
////////////////////////////////////////
// Primary Library (mys)
#if (defined(MYS_IMPL) || defined(MYS_IMPL_LOCAL)) && !defined(__MYS_IMPL_ONCE__)
#define __MYS_IMPL_ONCE__
#include "mys/impl/uthash_hash.h"
#include "mys/impl/uthash_list.h"
#include "mys/impl/algorithm.c"
#include "mys/impl/errno.c"
#include "mys/impl/format.c"
#include "mys/impl/rand.c"
#include "mys/impl/log.c"
#include "mys/impl/hrtime.c"
#include "mys/impl/os.c"
#include "mys/impl/linalg.c"
#include "mys/impl/statistic.c"
#include "mys/impl/thread.c"
#include "mys/impl/base64.c"
#include "mys/impl/hash.c"
#include "mys/impl/checkpoint.c"
#include "mys/impl/memory.c"
#include "mys/impl/pool.c"
#include "mys/impl/pmparser.c"
#include "mys/impl/string.c"
#include "mys/impl/guard.c"
#include "mys/impl/trace.c"
#include "mys/impl/mpistubs.c"
#include "mys/impl/misc.c"
#include "mys/impl/require.c"
#include "mys/impl/commgroup.c"
#include "mys/impl/table.c"
#ifndef MYS_DISABLE_DEBUG
#include "mys/impl/debug.c"
#endif
#include "mys/impl/math.c"
#include "mys/impl/net.c"
// Third-Party Library (mys3)
#ifdef MYS_ENABLE_CJSON
#include "mys3/cJSON/cJSON.c"
#endif
#ifdef MYS_ENABLE_MATRIXMARKET
#include "mys3/matrixmarket/mmio.c"
#endif
#ifdef MYS_ENABLE_STB
#include "mys3/stb/stb_image.c"
#endif
#endif
#define _UTHASH_UNDEF_HASH
#define _UTHASH_UNDEF_LIST
#include "mys/impl/uthash_hash.h"
#include "mys/impl/uthash_list.h"


////////////////////////////////////////
////// C++
////////////////////////////////////////
#ifdef __cplusplus
#include "mys/linalg.hpp"
#endif

////////////////////////////////////////
////// CUDA
////////////////////////////////////////
#ifdef CUDA_ARCH
#include "mys/cuda.cuh"
#endif

#include "mys/complex.h"

#endif // __STDC_VERSION__ < 199901L && __cplusplus < 201103L
