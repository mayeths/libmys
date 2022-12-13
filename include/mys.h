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
#include "./mys/os.h"
#include "./mys/partition.h"
#include "./mys/random.h"
#include "./mys/statistic.h"
#include "./mys/thread.h"

/* Third-Party Library (mys3) */

#ifdef MYS_IMPL_ALL
#define MYS_IMPL_CJSON
#define MYS_IMPL_MATRIXMARKET
#define MYS_IMPL_STB
#endif

#include "./mys3/cJSON/cJSON.h"
#if defined(MYS_IMPL_CJSON) && !defined(MYS_IMPL_CJSON_YES)
#define MYS_IMPL_CJSON_YES
#include "./mys3/cJSON/cJSON.impl.h"
#endif

#include "./mys3/matrixmarket/mmio.h"
#if defined(MYS_IMPL_MATRIXMARKET) && !defined(MYS_IMPL_MATRIXMARKET_YES)
#define MYS_IMPL_MATRIXMARKET_YES
#include "./mys3/matrixmarket/mmio.impl.h"
#endif

#include "./mys3/stb/stb_image.h"
#if defined(MYS_IMPL_STB) && !defined(MYS_IMPL_STB_YES)
#define MYS_IMPL_STB_YES
#include "./mys3/stb/stb_image.impl.h"
#endif

#ifdef __cplusplus
}
#endif
