/**
 * @file mys.h
 * @author mayeths (wow@mayeths.com)
 * @version 1.0
 * @brief Include all libmys C headers into one header (Require C99)
 * 
 */

#if __STDC_VERSION__ < 199901L && __cplusplus < 201103L
#error Require at least c99 to parse *.h in libmys
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

/* Primary Library (mys) */

#include "./mys/config.h"
#include "./mys/debug.h"
#include "./mys/env.h"
#include "./mys/hrtime.h"
#include "./mys/macro.h"
#include "./mys/myspi.h"
#include "./mys/os.h"
#include "./mys/partition.h"
#include "./mys/random.h"
#include "./mys/statistic.h"
#include "./mys/thread.h"

/* Third-Party Library (mys3) */

#ifdef MYS_IMPL
#if defined(MYS_ENABLE_CJSON)
#define MYS_IMPL_CJSON
#endif
#if defined(MYS_ENABLE_MATRIXMARKET)
#define MYS_IMPL_MATRIXMARKET
#endif
#if defined(MYS_ENABLE_STB)
#define MYS_IMPL_STB
#endif
#endif

#include "./mys3/cJSON/cJSON.h"
#if defined(MYS_IMPL_CJSON) && !defined(__MYS_IMPL_CJSON__)
#define __MYS3_IMPL_CJSON__
#include "./mys3/cJSON/cJSON.impl.h"
#endif

#include "./mys3/matrixmarket/mmio.h"
#if defined(MYS_IMPL_MATRIXMARKET) && !defined(__MYS_IMPL_MATRIXMARKET__)
#define __MYS_IMPL_MATRIXMARKET__
#include "./mys3/matrixmarket/mmio.impl.h"
#endif

#include "./mys3/stb/stb_image.h"
#if defined(MYS_IMPL_STB) && !defined(__MYS_IMPL_STB__)
#define __MYS_IMPL_STB__
#include "./mys3/stb/stb_image.impl.h"
#endif

#ifdef __cplusplus
}
#endif
