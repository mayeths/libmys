/**
 * @file mys.h
 * @author mayeths (wow@mayeths.com)
 * @brief Include all libmys C headers into one header (Require C99)
 * 
 */

#if __STDC_VERSION__ < 199901L && __cplusplus < 201103L
#error Require at least c99 to parse *.h in libmys
#endif

/* Indicate we have include libmys */
#ifndef MYS_VERSION
#define MYS_VERSION 230201L
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "./mys/config.h"
#include "./mys/macro.h"

#if defined(OS_LINUX) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

#if !defined(MYS_NO_MPI)
#include <mpi.h>
#endif


/* Primary Library (mys) */
#ifdef __cplusplus
extern "C" {
#endif
#include "./mys/assert.h"
#include "./mys/base64.h"
#include "./mys/env.h"
#include "./mys/hash.h"
#include "./mys/hrtime.h"
#include "./mys/log.h"
#include "./mys/myspi.h"
#include "./mys/os.h"
#include "./mys/partition.h"
#include "./mys/rand.h"
#include "./mys/statistic.h"
#include "./mys/thread.h"
#ifdef CUDA_ARCH
#include "./mys/cuda.cuh"
#endif
#ifdef __cplusplus
}
#endif


/* Third-Party Library (mys3) */

#include "./mys3/cJSON/cJSON.h"
#include "./mys3/matrixmarket/mmio.h"
#include "./mys3/stb/stb_image.h"


/* Implementation */

#if defined(MYS_IMPL) && !defined(__MYS_IMPL_ONCE__)
#define __MYS_IMPL_ONCE__
#include "./mys/mys.c"
#if defined(MYS_ENABLE_CJSON)
#include "./mys3/cJSON/cJSON.impl.h"
#endif
#if defined(MYS_ENABLE_MATRIXMARKET)
#include "./mys3/matrixmarket/mmio.impl.h"
#endif
#if defined(MYS_ENABLE_STB)
#include "./mys3/stb/stb_image.impl.h"
#endif
#endif
