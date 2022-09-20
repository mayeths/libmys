#pragma once

#include <stdint.h>
#include "macro.h"

#if defined(ARCH_AARCH64)
#define HRNAME "High-resolution timer by AArch64 assembly"
#define HRTICK_T uint64_t
#define HRTICK(v) __asm__ __volatile__("mrs %0, CNTVCT_EL0" : "=r"(v));
#define HRFREQ(v) __asm__ __volatile__("mrs %0, CNTFRQ_EL0" : "=r"(v));

#else /* ~1us precision */
#include <time.h>
#define HRNAME "High-resolution timer by time.h clock_gettime"
#define HRTICK_T double
#define HRTICK(v) do { \
    struct timespec _t; \
    clock_gettime(CLOCK_REALTIME, &_t); \
    v = _t.tv_sec + _t.tv_nsec / 1000000000.; \
} while (0)
#define HRFREQ(v) do { v = 1; } while (0)
#endif

static inline const char *hrname() { return HRNAME; }
static inline double hrfreq() { HRTICK_T f; HRFREQ(f); return ((double)f); }
static inline double hrtime() { HRTICK_T t; HRTICK(t); return ((double)t) / hrfreq(); }
static inline double hrdiff(double t0) { return hrtime() - t0; }
