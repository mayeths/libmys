/**
 * @file mys.h
 * @author mayeths (wow@mayeths.com)
 * @version 1.0
 * @brief Include all libmys C headers into one header (Require C99)
 * 
 */
// TODO: FIXME: Remove __MYS_H__ and __MYS_HPP__ due to introduce MYS_IMPL
#ifndef __MYS_H__
#define __MYS_H__

#if __STDC_VERSION__ < 199901L && __cplusplus < 201103L
#error Require at least c99 to parse *.h in libmys
#endif


#ifdef __cplusplus
extern "C" {
#endif

/* Primary library */

#include "./mys/config.h"
#include "./mys/debug.h"
#include "./mys/env.h"
#include "./mys/headers.h"
#include "./mys/hrtime.h"
#include "./mys/os.h"
#include "./mys/partition.h"
#include "./mys/random.h"
#include "./mys/statistic.h"
#include "./mys/thread.h"

/* Secondary library */

#ifdef MYS_IMPL_ALL
#define MYS_IMPL_CJSON
#define MYS_IMPL_MATRIXMARKET
#define MYS_IMPL_STB
#endif

#include "./cJSON/cJSON.h"
#ifdef MYS_IMPL_CJSON
#include "./cJSON/cJSON.impl.h"
#endif

#include "./matrixmarket/mmio.h"
#ifdef MYS_IMPL_MATRIXMARKET
#include "./matrixmarket/mmio.impl.h"
#endif

#include "./stb/stb_image.h"
#ifdef MYS_IMPL_STB
#include "./stb/stb_image.impl.h"
#endif

#ifdef __cplusplus
}
#endif


#endif /*__MYS_H__*/