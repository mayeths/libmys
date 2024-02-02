#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <limits.h>

#define MYS_MODIFIED_ON "2024.02.02"

// Report a warning if time granularity >= "TIMER_THRESHOLD"
#ifndef TIMER_THRESHOLD
#define TIMER_THRESHOLD (50 * 1e-6)
#endif

// Report a warning if rank is too slow than the fast one
#ifndef IMBALANCE_THRESHOLD
#define IMBALANCE_THRESHOLD 0.10
#endif

// Report a warning if any mintime < "MINTIME_THRESHOLD"
#ifndef MINTIME_THRESHOLD
#define MINTIME_THRESHOLD (10 * 1e-3)
#endif

// The minimum size of local array in eacy rank
// Choose by empirical
#ifndef MINSIZE_THRESHOLD
#define MINSIZE_THRESHOLD "32MB"
#endif

// Run each kernel "NTIMES" times and reports the best result for any
// iteration after the firsttherefore the minimum value for NTIMES is 2.
#ifdef NTIMES
#if NTIMES <= 1
#define NTIMES 10
#endif
#endif
#ifndef NTIMES
#define NTIMES 10
#endif

// Use "STREAM_TYPE" as the element type
#ifndef STREAM_TYPE
#define STREAM_TYPE double
#endif

// The "SCALAR" 0.42 allows over 2000 iterations for 32-bit IEEE arithmetic
// and over 18000 iterations for 64-bit IEEE arithmetic.
#ifndef SCALAR
#define SCALAR 0.42
#endif

#define AILGN_4(n) ((n) & (~3))
#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif


#endif /*_COMMON_H_*/