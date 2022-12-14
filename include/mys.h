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

#include "./mys3/cJSON/cJSON.h"
#include "./mys3/matrixmarket/mmio.h"
#include "./mys3/stb/stb_image.h"

/* Implementation */

#if defined(MYS_IMPL) && !defined(__MYS_IMPL_ONCE__)
#define __MYS_IMPL_ONCE__
#include "./mys/impl.h"
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

#ifdef __cplusplus
}
#endif
