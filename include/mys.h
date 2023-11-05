/**
 * @file mys.h
 * @author mayeths (wow@mayeths.com)
 * @brief Include all libmys C headers into one header (Require GNU99)
 * 
 */
#if __STDC_VERSION__ < 199901L && __cplusplus < 201103L
#error Require at least c99 to parse *.h in libmys
#endif


#ifndef MYS_VERSION
#define MYS_VERSION 202311L
#endif


#include "./mys/_config.h"
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


#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////
////// C Declaration
////////////////////////////////////////
// Primary Library (mys)
#include "./mys/assert.h"
#include "./mys/atomic.h"
#include "./mys/base64.h"
#include "./mys/checkpoint.h"
#include "./mys/debug.h"
#include "./mys/env.h"
#include "./mys/guard.h"
#include "./mys/hash.h"
#include "./mys/hrtime.h"
#include "./mys/log.h"
#include "./mys/macro.h"
#include "./mys/memory.h"
#ifndef MYS_NO_MPI
#include "mys/mpiz.h"
#endif
#include "./mys/numa.h"
#include "./mys/os.h"
#include "./mys/partition.h"
#include "./mys/rand.h"
#include "./mys/statistic.h"
#include "./mys/string.h"
#include "./mys/thread.h"
// Third-Party Library (mys3)
#include "./mys3/cJSON/cJSON.h"
#include "./mys3/matrixmarket/mmio.h"
#include "./mys3/stb/stb_image.h"

////////////////////////////////////////
////// C Definition
////////////////////////////////////////
// Primary Library (mys)
#if (defined(MYS_IMPL) || defined(MYS_IMPL_LOCAL)) && !defined(__MYS_IMPL_ONCE__)
#define __MYS_IMPL_ONCE__
#include "./mys/impl/rand.c"
#include "./mys/impl/log.c"
#include "./mys/impl/hrtime.c"
#include "./mys/impl/os.c"
#include "./mys/impl/partition.c"
#include "./mys/impl/statistic.c"
#include "./mys/impl/thread.c"
#include "./mys/impl/base64.c"
#include "./mys/impl/hash.c"
#include "./mys/impl/checkpoint.c"
#include "./mys/impl/memory.c"
#include "./mys/impl/string.c"
#include "./mys/impl/guard.c"
#ifndef MYS_NO_MPI
#include "./mys/impl/mpiz.c"
#endif
#include "./mys/impl/debug.c"
#include "./mys/impl/math.c"
#include "./mys/impl/mpi.c"
#define _UTHASH_UNDEF_HASH
#define _UTHASH_UNDEF_LIST
#include "./mys/impl/uthash_hash.h"
#include "./mys/impl/uthash_list.h"
// Third-Party Library (mys3)
#ifdef MYS_ENABLE_CJSON
#include "./mys3/cJSON/cJSON.impl.h"
#endif
#ifdef MYS_ENABLE_MATRIXMARKET
#include "./mys3/matrixmarket/mmio.impl.h"
#endif
#ifdef MYS_ENABLE_STB
#include "./mys3/stb/stb_image.impl.h"
#endif
#endif

#ifdef __cplusplus
} // extern "C"
#endif


////////////////////////////////////////
////// C++
////////////////////////////////////////
#ifdef __cplusplus
#include "mys/linalg.hpp"
#include "mys/memory.hpp"
#ifndef MYS_NO_MPI
#include "mys/mpi.hpp"
#endif
#include "mys/raii.hpp"
#include "mys/string.hpp"
#include "mys/type.hpp"
#endif

////////////////////////////////////////
////// CUDA
////////////////////////////////////////
#ifdef CUDA_ARCH
#include "./mys/cuda.cuh"
#endif
