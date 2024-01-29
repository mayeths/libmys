/**
 * @file mys.h
 * @author mayeths (wow@mayeths.com)
 * @brief Include all libmys C headers into one header (Require GNU99)
 * 
 */
#if defined(__cplusplus) && __cplusplus < 201103L
#error Require C++11 or higher. Consider removing -std or use -std=c++11 in compiler command line arguments
#elif !defined(__cplusplus) && __STDC_VERSION__ < 199901L
#error Require GNU C99 or higher. Consider removing -std or use -std=gnu99 in compiler command line arguments
#elif !defined(__cplusplus) && defined(__STRICT_ANSI__)
#error Require GNU C instead of ANSI C or ISO C. Consider removing -ansi or change -std=c99 to -std=gnu99 in compiler command line arguments
#else


#ifndef MYS_VERSION
#define MYS_VERSION 202311L
#endif


#include "mys/_config.h"
#if !defined(OS_LINUX) && !defined(OS_MACOS)
#error Port me
#endif
#if defined(OS_LINUX) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#if defined(OS_MACOS) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
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
#include "mys/assert.h"
#include "mys/atomic.h"
#include "mys/base64.h"
#include "mys/checkpoint.h"
#include "mys/debug.h"
#include "mys/guard.h"
#include "mys/hash.h"
#include "mys/hrtime.h"
#include "mys/log.h"
#include "mys/macro.h"
#include "mys/memory.h"
#ifndef MYS_NO_MPI
#include "mys/mpiz.h"
#endif
#include "mys/os.h"
#include "mys/linalg.h"
#include "mys/rand.h"
#include "mys/statistic.h"
#include "mys/string.h"
#include "mys/thread.h"
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
#include "mys/impl/string.c"
#include "mys/impl/guard.c"
#ifndef MYS_NO_MPI
#include "mys/impl/mpiz.c"
#endif
#include "mys/impl/debug.c"
#include "mys/impl/math.c"
#include "mys/impl/mpi.c"
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
